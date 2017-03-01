#include "MLTCore.h"
#include "tools/collision.h"
#include "tools/dotreflectionAGH.h"
#include "tools/brdfreflection.h"
#include "tools/dotdistribution.h"
#include <iostream>
#include <algorithm>

MLTCore::MLTCore(t_Mesh& _sceneMesh, t_TetraMesh& _sceneTetraMesh, CONF_CALCULATION_AGH& _confEnv, Core_ConfigurationAGH& _configurationTool, ReportManagerAGH* _reportTool)
	: NextEventEstimationCore(_sceneMesh, _sceneTetraMesh, _confEnv, _configurationTool, _reportTool)
{
	doDirectSoundCalculation = true;

	pLarge = *configurationTool->FastGetConfigValue(Core_ConfigurationAGH::MLT_LARGE_STEP_PROB);
	pSpecular = *configurationTool->FastGetConfigValue(Core_ConfigurationAGH::MLT_SPECULAR_REFL_PROB);
	mutation_number = *configurationTool->FastGetConfigValue(Core_ConfigurationAGH::MLT_MUTATION_NUMBER);
};

bool MLTCore::Run(CONF_PARTICULE_AGH configurationP)

//TODO: Mutacja kierunku w którym generowany jest promieñ!

//TODO: Przenieœæ initial seed przed mutacje, zapisaæ b, zastanowiæ siê nad stanem pocz¹tkowym! - DONE
//TODO: mno¿enie razy prawdopodobieñstwo SR w danym kierunku - powinno znacznie pomóc - DONE
{
	if (!configurationP.isShadowRay)
	{

		CONF_PARTICULE_MLT inputParticle(configurationP);
		inputParticle.SetInitialState(inputParticle);
		inputParticle.weight = 1/mutation_number;

		int totalParticleNumber = (*configurationTool->FastGetConfigValue(Core_ConfigurationAGH::IPROP_QUANT_PARTICLE_CALCULATION) * configurationTool->srcList.size());

		RunInitialSeed(inputParticle);
		Seeds.push_back(inputParticle);
		b += inputParticle.totalProbability / totalParticleNumber;
	}
	else
	{
		NextEventEstimationCore::Run(configurationP);
	}

	return true;
}

bool MLTCore::SeedIsEmpty()
{
	return Seeds.empty();
}

void MLTCore::RunMutation()
{
	CONF_PARTICULE_MLT inputParticle = Seeds.front();
	Seeds.pop_front();
	DoMutations(inputParticle, (*configurationTool->FastGetConfigValue(Core_ConfigurationAGH::IPROP_QUANT_PARTICLE_CALCULATION) * configurationTool->srcList.size()));
}

void MLTCore::CastShadowRays(CONF_PARTICULE_MLT& input_particle)
{
	CONF_PARTICULE_AGH shadowRay;

	while (!input_particle.shadowRays.empty())
	{
		shadowRay = input_particle.shadowRays.front();
		input_particle.shadowRays.pop_front();
		//applicationTools.outputTool->NewParticule(confPart);
		shadowRay.energie *= input_particle.weight;
		Run(shadowRay);
		//applicationTools.outputTool->SaveParticule();
	}
}

void MLTCore::DoMutations(CONF_PARTICULE_MLT& inputParticle, int totalParticleNumber)
{
	CONF_PARTICULE_MLT mutatedParticle;
	double ratio;
	int mutation_counter = 0;

	while (mutation_counter < mutation_number)
	{
		Mutate(inputParticle, mutatedParticle);
		Follow(mutatedParticle);

		ratio = std::min(mutatedParticle.totalProbability / inputParticle.totalProbability, 1.);

		inputParticle.weight += (1 - ratio) / ((inputParticle.totalProbability / b + pLarge)*(mutation_number));
		mutatedParticle.weight += (ratio + mutatedParticle.largeStep) / ((mutatedParticle.totalProbability / b + pLarge)*(mutation_number));

		if(mutatedParticle.largeStep==1)
		{
			CastShadowRays(inputParticle);
			inputParticle = mutatedParticle;
			inputParticle.largeStep = 0;
		}
		else if(ratio > GetRandValue())
		{
			CastShadowRays(inputParticle);
			inputParticle = mutatedParticle;
		}else
		{
			CastShadowRays(mutatedParticle);			
		}
		mutation_counter++;
	}

	CastShadowRays(inputParticle);
}

bool MLTCore::RunInitialSeed(CONF_PARTICULE_MLT& inputParticle)
{
	decimal densite_proba_absorption_atmospherique = configurationTool->freqList[inputParticle.frequenceIndex]->densite_proba_absorption_atmospherique;
	decimal absorption_atmospheric = configurationTool->freqList[inputParticle.frequenceIndex]->absorption_atmospherique;

	SetNextParticleCollision(inputParticle); //1er test de collision
	SetNextParticleCollisionWithObstructionElement(inputParticle); // Test de collision avec objet virtuel encombrant
	//Au premier pas de temps il faut enregistrer l'energie de la particule dans la maille courante
	//configurationP.stateParticule=PARTICULE_STATE_ALIVE;
	reportTool->ParticuleGoToNextTetrahedra(inputParticle, inputParticle.currentTetra);

	double distPerTimeStep = inputParticle.direction.length();

	while (inputParticle.stateParticule == PARTICULE_STATE_ALIVE && inputParticle.totalDistance / distPerTimeStep < confEnv.nbPasTemps)
	{
		if (inputParticle.stateParticule == PARTICULE_STATE_ALIVE)
			Movement(inputParticle); //Effectue un mouvement sur la distance restante
	}

	switch (inputParticle.stateParticule)
	{
	case PARTICULE_STATE_ALIVE:
		reportTool->statReport.partAlive++;
		break;
	case PARTICULE_STATE_ABS_SURF:
		reportTool->statReport.partAbsSurf++;
		break;
	case PARTICULE_STATE_ABS_ATMO:
		reportTool->statReport.partAbsAtmo++;
		break;
	case PARTICULE_STATE_ABS_ENCOMBREMENT:
		reportTool->statReport.partAbsEncombrement++;
		break;
	case PARTICULE_STATE_LOOP:
		reportTool->statReport.partLoop++;
		break;
	case PARTICULE_STATE_LOST:
		reportTool->statReport.partLost++;
		break;
	case PARTICULE_STATE_SHADOW_RAY_REACHED_DST:
		reportTool->statReport.partShadowRay++;
		break;
	}
	reportTool->statReport.partTotal++;
	return true;
}

void MLTCore::Mutate(const CONF_PARTICULE_MLT& input_particle, CONF_PARTICULE_MLT& mutetedParticle) const
{
	mutetedParticle = input_particle;
	mutetedParticle.shadowRays.clear();

	//bool large_step = (GetRandValue() < pLarge) ? 1 : 0;
	if (GetRandValue() < pLarge)
	{
		mutetedParticle.reflectionsNum = 0;
		mutetedParticle.refDir1.clear();
		mutetedParticle.refDir2.clear();
		mutetedParticle.travelProbability.clear();
		mutetedParticle.matAbsorbtions.clear();
		mutetedParticle.largeStep = 1;
		mutetedParticle.weight = 0;

	}else{
		for (int i = 0; i < input_particle.reflectionsNum; i++)
		{
			MutationGenerator(mutetedParticle.refDir1[i]);
			MutationGenerator(mutetedParticle.refDir2[i]);
			MutationGenerator(mutetedParticle.travelProbability[i]);
			MutationGenerator(mutetedParticle.matAbsorbtions[i]);
		}
	}
}

bool MLTCore::MoveToNextReflection(CONF_PARTICULE_MLT& configurationP, double rndDir1, double rndDir2, double rndTravelProbability, double rndMatAbsorbtion, float deltaT, float distanceToTravel)
{
	double distance;

	//******************************* FIND NEXT COLISION ***************************************//

	SetNextParticleCollision(configurationP);
	if (!GetNextCollision(configurationP, distance))
		return true;

	//******************************* PROPAGATION ***************************************//
	decimal absorption_atmospheric = configurationTool->freqList[configurationP.frequenceIndex]->absorption_atmospherique;
	decimal travelProbability = expf(-absorption_atmospheric * distance);
	
	if (rndTravelProbability >= travelProbability)
	{
		configurationP.energie = 0; // La particule est détruite
		configurationP.stateParticule = PARTICULE_STATE_ABS_ATMO;
		configurationP.totalProbability *= (1 - travelProbability);
		return true;
	}
	configurationP.totalProbability *= travelProbability;

	configurationP.totalDistance += distance;

	//******************************* ABSORBTION ***************************************//

	// Récuperation de l'information de la face
	t_cFace* faceInfo;
	faceInfo = configurationP.currentTetra->faces[configurationP.nextModelIntersection.idface].face_scene;

	//On stocke le materiau dans la variable materialInfo
	t_Material_BFreq* materialInfo = &(*faceInfo).faceMaterial->matSpectrumProperty[configurationP.frequenceIndex];

	//Test d'absorption en aléatoire
	if (rndMatAbsorbtion <= materialInfo->absorption)
	{
		//Particule absorbée
		if (configurationP.stateParticule == PARTICULE_STATE_ALIVE)
			configurationP.stateParticule = PARTICULE_STATE_ABS_SURF;
		configurationP.energie = 0.;
		configurationP.totalProbability *= materialInfo->absorption;
		return true;
	}

	configurationP.totalProbability *= (1 - materialInfo->absorption);

	//******************************* UPDATE TIME STEP NUMBER ***********************************//
	double timestepNum = configurationP.totalDistance / configurationP.direction.length();
	configurationP.pasCourant = configurationP.creationTime + int(timestepNum);

	if (*configurationTool->FastGetConfigValue(Core_ConfigurationAGH::IPROP_QUANT_TIMESTEP) < configurationP.pasCourant)
		return true;


	configurationP.elapsedTime = (timestepNum + configurationP.creationTime - configurationP.pasCourant) * deltaT;

	//******************************* SHADOW RAY GENERATION ***************************************//

	GenerateShadowRays(configurationP, materialInfo, faceInfo, deltaT, distanceToTravel, configurationP.shadowRays, &configurationP.totalProbability);

	//******************************* REFLECTION ***************************************//

	// Choix de la méthode de reflexion en fonction de la valeur de diffusion

	vec3 nouvDirection;
	vec3 faceNormal;
	double dirProbability;

	faceNormal = -faceInfo->normal;

	//Get direction for diffuse or specular part based on material info
	if (materialInfo->diffusion == 1 || pSpecular*((1 - rndMatAbsorbtion) / materialInfo->absorption) < materialInfo->diffusion)
	{
		nouvDirection = ReflectionLawsMLT::SolveDiffusePart(configurationP.direction, *materialInfo, faceNormal, configurationP, rndDir1, rndDir2, dirProbability);
		dirProbability *= (1 - pSpecular);
	}
	else
	{
		nouvDirection = ReflectionLawsMLT::SolveSpecularPart(configurationP.direction, *materialInfo, faceNormal, configurationP, rndDir1, rndDir2, dirProbability);
		dirProbability *= pSpecular;
	}

	//Calcul de la nouvelle direction de réflexion (en reprenant la célérité de propagation du son)
	configurationP.direction = nouvDirection * distanceToTravel;
	configurationP.totalProbability *= dirProbability;

	return false;
}

void MLTCore::Movement(CONF_PARTICULE_MLT& configurationP)
{
	double rndDir1 = GetRandValue();
	double rndDir2 = GetRandValue();
	double rndTravelProbability = GetRandValue();
	double rndMatAbsorbtion = GetRandValue();

	decimal deltaT = *configurationTool->FastGetConfigValue(Core_ConfigurationAGH::FPROP_TIME_STEP);
	decimal distanceToTravel = configurationP.direction.length();

	if (MoveToNextReflection(configurationP, rndDir1, rndDir2, rndTravelProbability, rndMatAbsorbtion, deltaT, distanceToTravel)) return;

	configurationP.reflectionsNum++;
	configurationP.travelProbability.push_back(rndTravelProbability);
	configurationP.matAbsorbtions.push_back(rndMatAbsorbtion);
	configurationP.refDir1.push_back(rndDir1);
	configurationP.refDir2.push_back(rndDir2);
}

void MLTCore::Follow(CONF_PARTICULE_MLT& propagationParticle)
{
	decimal deltaT = *configurationTool->FastGetConfigValue(Core_ConfigurationAGH::FPROP_TIME_STEP);
	decimal distanceToTravel = propagationParticle.direction.length();

	propagationParticle.ResetToInitialState();
	propagationParticle.totalProbability = 1;
	propagationParticle.totalDistance = 0;

	for (int i = 0; i < propagationParticle.reflectionsNum; i++)
	{
		double rndDir1 = propagationParticle.refDir1[i];
		double rndDir2 = propagationParticle.refDir2[i];
		double rndTravelProbability = propagationParticle.travelProbability[i];
		double rndMatAbsorbtion = propagationParticle.matAbsorbtions[i];

		if (MoveToNextReflection(propagationParticle, rndDir1, rndDir2, rndTravelProbability, rndMatAbsorbtion, deltaT, distanceToTravel)) return;
	}

	//add new reflections if still allive
	while (propagationParticle.stateParticule == PARTICULE_STATE_ALIVE && propagationParticle.totalDistance / propagationParticle.direction.length() < confEnv.nbPasTemps)
	{
		if (propagationParticle.stateParticule == PARTICULE_STATE_ALIVE)
			Movement(propagationParticle); //Effectue un mouvement sur la distance restante
	}

}

void MLTCore::FreeParticleTranslation(CONF_PARTICULE_AGH& configurationP, const vec3& translationVector)
{
	if (configurationP.isShadowRay) reportTool->ShadowRayFreeTranslation(configurationP, configurationP.position + translationVector);
	// On prend en compte le rapprochement vers l'encombrement virtuel
	if (configurationP.currentTetra->volumeEncombrement)
		configurationP.distanceToNextEncombrementEle -= translationVector.length();
	configurationP.position += translationVector;
}

bool MLTCore::GetNextCollision(CONF_PARTICULE_MLT& configurationP, double& distance)
{
	float t;

	CONF_PARTICULE_AGH testParticle = configurationP;

	testParticle.nextModelIntersection.idface = GetTetraFaceCollision(testParticle, testParticle.direction, t);
	testParticle.nextModelIntersection.collisionPosition = testParticle.position + testParticle.direction * t;

	int maxIteration = 50000;
	int iteration = 0;

	//testParticle.currentTetra->faces[testParticle.nextModelIntersection.idface].face_scene == NULL && 
	while (iteration++ < maxIteration)
	{
		testParticle.position = testParticle.nextModelIntersection.collisionPosition;

		t_Tetra* nextTetra = testParticle.currentTetra->voisins[testParticle.nextModelIntersection.idface];
		if (!nextTetra)
		{
			if (testParticle.nextModelIntersection.idface == -1)
			{
				configurationP.energie = 0;
				if (configurationP.stateParticule == PARTICULE_STATE_ALIVE)
					configurationP.stateParticule = PARTICULE_STATE_LOST;
				return false;
			}

			distance = (testParticle.position - configurationP.position).length();
			configurationP.position = testParticle.position;
			configurationP.currentTetra = testParticle.currentTetra;
			configurationP.nextModelIntersection.collisionPosition = testParticle.position;
			configurationP.nextModelIntersection.idface = testParticle.nextModelIntersection.idface;
			//configurationP.totalDistance += distance;
			return true;
		}
		else
		{
			testParticle.currentTetra = nextTetra;
			testParticle.nextModelIntersection.idface = GetTetraFaceCollision(testParticle, testParticle.direction, t);
			testParticle.nextModelIntersection.collisionPosition = testParticle.position + testParticle.direction * t;
		}
	}

	configurationP.energie = 0;
	if (configurationP.stateParticule == PARTICULE_STATE_ALIVE)
		configurationP.stateParticule = PARTICULE_STATE_LOST;
	return false;
}


void MLTCore::MutationGenerator(double& value) const
{
	float s1 = 1. / 1024, s2 = 1. / 64; 
	float dv = s2*exp(-log(s2 / s1)*GetRandValue()); 
	if (GetRandValue() < 0.5) {
		value += dv; if (value > 1) value -= 1;
	}
	else {
		value -= dv; if (value < 0) value += 1;
	}
}
