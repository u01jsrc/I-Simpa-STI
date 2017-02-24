#include "NextEventEstimationCore.h"
#include "tools/collision.h"
#include "tools/dotreflectionAGH.h"
#include "tools/brdfreflection.h"
#include "tools/dotdistribution.h"
#include <iostream>

NextEventEstimationCore::NextEventEstimationCore(t_Mesh& _sceneMesh, t_TetraMesh& _sceneTetraMesh, CONF_CALCULATION_AGH &_confEnv, Core_ConfigurationAGH &_configurationTool, ReportManagerAGH* _reportTool)
	: CalculationCoreSPPS(_sceneMesh, _sceneTetraMesh, _confEnv, _configurationTool, _reportTool) 
{
	doDirectSoundCalculation = true;
};


void NextEventEstimationCore::Movement(CONF_PARTICULE_AGH &configurationP)
{
	decimal deltaT = *configurationTool->FastGetConfigValue(Core_ConfigurationAGH::FPROP_TIME_STEP);
	decimal distanceSurLePas = configurationP.direction.length();
	decimal celeriteLocal = distanceSurLePas / deltaT;
	decimal faceDirection;
	bool collisionResolution = true; //On test de nouveau la collision dans le pas de temps courant si cette valeur est à vrai
	int iteration = 0;
	decimal distanceCollision = 0.f;
	decimal distanceToTravel = 0.f;
	while (collisionResolution && configurationP.stateParticule == PARTICULE_STATE_ALIVE)
	{
		iteration++;
		collisionResolution = false;
		//Si il y a collision avec une face (avec prise en compte de la distance parcourue)
		distanceCollision = (configurationP.nextModelIntersection.collisionPosition - configurationP.position).length();
		distanceToTravel = celeriteLocal*(deltaT - configurationP.elapsedTime);

		//Test de collision avec un élément de l'encombrement entre la position de la particule et une face du tetrahèdre courant.
		if (*configurationTool->FastGetConfigValue(Core_ConfigurationAGH::IPROP_DO_CALC_ENCOMBREMENT) && distanceToTravel >= configurationP.distanceToNextEncombrementEle && distanceCollision>configurationP.distanceToNextEncombrementEle && configurationP.currentTetra->volumeEncombrement)
		{
			//Collision avec un élément virtuel de l'encombrement courant

			//Test d'absorption

			if (*configurationTool->FastGetConfigValue(Core_ConfigurationAGH::IPROP_ENERGY_CALCULATION_METHOD))
			{
				//Energétique
				configurationP.energie *= (1 - configurationP.currentTetra->volumeEncombrement->encSpectrumProperty[configurationP.frequenceIndex].alpha);
				if (configurationP.energie <= configurationP.energie_epsilon)
				{
					configurationP.stateParticule = PARTICULE_STATE_ABS_ENCOMBREMENT;
					return;
				}
			}
			else {
				//Aléatoire
				if (GetRandValue() <= configurationP.currentTetra->volumeEncombrement->encSpectrumProperty[configurationP.frequenceIndex].alpha)
				{
					//Absorbé
					configurationP.energie = 0.f;
					configurationP.stateParticule = PARTICULE_STATE_ABS_ENCOMBREMENT;
					return;
				}
			}
			//N'est pas absorbé

			//On incrémente le temps de parcourt entre la position avant et aprés collision avec l'encombrement virtuel
			configurationP.elapsedTime += configurationP.distanceToNextEncombrementEle / celeriteLocal;
			//On place la particule sur la position de collision
			FreeParticleTranslation(configurationP, (configurationP.direction / configurationP.direction.length())*configurationP.distanceToNextEncombrementEle);
			collisionResolution = true;
			//On change la direction de la particule en fonction de la loi de distribution
			vec3 newDir;
			switch (configurationP.currentTetra->volumeEncombrement->encSpectrumProperty[configurationP.frequenceIndex].law_diffusion)
			{
			case DIFFUSION_LAW_UNIFORM:
				ParticleDistribution::GenSphereDistribution(configurationP, configurationP.direction.length());
				break;
			case DIFFUSION_LAW_REFLEXION_UNIFORM:
				newDir = ReflectionLawsAGH::FittingUniformReflection(configurationP.direction);
				newDir.normalize();
				configurationP.direction = newDir*configurationP.direction.length();
				break;
			case DIFFUSION_LAW_REFLEXION_LAMBERT:
				newDir = ReflectionLawsAGH::FittingLambertReflection(configurationP.direction);
				newDir.normalize();
				configurationP.direction = newDir*configurationP.direction.length();
				break;

			};
			//Calcul du nouveau point de collision
			SetNextParticleCollision(configurationP);
			SetNextParticleCollisionWithObstructionElement(configurationP);
		}
		else if (distanceCollision <= distanceToTravel) // && configurationP.nextModelIntersection.idface!=-1
		{
			//Enregistrement de l'énergie passé à la paroi
			reportTool->ParticuleCollideWithSceneMesh(configurationP);

			vec3 vecTranslation = configurationP.nextModelIntersection.collisionPosition - configurationP.position;
			//On incrémente le temps de parcourt entre la position avant et aprés collision
			configurationP.elapsedTime += (vecTranslation / configurationP.direction.length()).length()*deltaT;

			//On place la particule sur la position de collision
			FreeParticleTranslation(configurationP, vecTranslation);

			// Récuperation de l'information de la face
			t_cFace* faceInfo = NULL;

#ifdef UTILISER_MAILLAGE_OPTIMISATION
			faceInfo = configurationP.currentTetra->faces[configurationP.nextModelIntersection.idface].face_scene;

			//test de passage d'un tétraèdre à un autre

			//Vrai si la paroi est anormalement orientée
			bool doInvertNormal(false);
			if (faceInfo)
			{
				faceDirection = configurationP.direction.dot(faceInfo->normal);
				doInvertNormal = (faceDirection <= -BARELY_EPSILON);
			}
			//On traverse la paroi du tetrahedre si (pas de résolution de collision si)
			//	- Ce n'est pas une surface du modèle
			//  - (ou) Elle n'est pas orientée vers nous et le matériau n'affecte les surfaces sur une orientation
			//  - (ou) Cette surface est un encombrement et qu'un autre volume nous attend derrière
			if (!faceInfo || ((faceInfo->faceEncombrement || (!(faceInfo->faceMaterial->doubleSidedMaterialEffect) && doInvertNormal)) && configurationP.currentTetra->voisins[configurationP.nextModelIntersection.idface]))
			{
				TraverserTetra(configurationP, collisionResolution);
			}
			else 
			{
#else
			faceInfo = &sceneMesh->pfaces[configurationP.nextModelIntersection.idface];
			///////////////////////////////////
			// Test de passage d'un milieu libre à un milieu encombré (et inversement)
			if (!faceInfo->faceEncombrement)
			{
#endif

				//On stocke le materiau dans la variable materialInfo
				t_Material_BFreq* materialInfo = &(*faceInfo).faceMaterial->matSpectrumProperty[configurationP.frequenceIndex];


				//Tirage aléatoire pour le test d'absorption
				if (*configurationTool->FastGetConfigValue(Core_ConfigurationAGH::IPROP_DO_CALC_CHAMP_DIRECT))
				{
					//Particule absorbée
					if (configurationP.stateParticule == PARTICULE_STATE_ALIVE)
						configurationP.stateParticule = PARTICULE_STATE_ABS_SURF;
					configurationP.energie = 0.f;
					return;
				}
				else 
				{
					if (*configurationTool->FastGetConfigValue(Core_ConfigurationAGH::IPROP_ENERGY_CALCULATION_METHOD))
					{
						//Methode énérgétique, particule en collision avec la paroi
						//Particule courante = (1-alpha)*epsilon
						//Si l'absorption est totale la particule est absorbée si tau=0
						if (materialInfo->absorption == 1) //Pas de duplication possible de la particule (forcement non réfléchie)
						{
							if (configurationP.stateParticule == PARTICULE_STATE_ALIVE)
								configurationP.stateParticule = PARTICULE_STATE_ABS_SURF;
							configurationP.energie = 0.;
							return;
						}
						else 
						{
							if (materialInfo->absorption != 0) //Pas de duplication possible de la particule (forcement réfléchie)
								configurationP.energie *= (1 - materialInfo->absorption);
						}
					}
					else {
						//Test d'absorption en aléatoire
						if (GetRandValue() <= materialInfo->absorption)
						{
							//Particule absorbée
							if (configurationP.stateParticule == PARTICULE_STATE_ALIVE)
								configurationP.stateParticule = PARTICULE_STATE_ABS_SURF;
							configurationP.energie = 0.;
							return;
						}
					}
				}
				if (configurationP.energie <= configurationP.energie_epsilon)
				{
					if (configurationP.stateParticule == PARTICULE_STATE_ALIVE)
						configurationP.stateParticule = PARTICULE_STATE_ABS_SURF;
					configurationP.energie = 0.;
					return;
				}

				// Choix de la méthode de reflexion en fonction de la valeur de diffusion
				vec3 nouvDirection;
				vec3 faceNormal;
				if (!doInvertNormal)
					faceNormal = -faceInfo->normal;
				else
					faceNormal = faceInfo->normal;

				//Calculate and cast shadow rays
				GenerateShadowRays(configurationP, materialInfo, faceInfo, deltaT, distanceToTravel, &confEnv.duplicatedParticles);

				//Get direction for diffuse or specular part based on material info
				if (materialInfo->diffusion == 1 || GetRandValue()<materialInfo->diffusion)
				{
					nouvDirection = ReflectionLawsAGH::SolveDiffusePart(configurationP.direction, *materialInfo, faceNormal);
				}
				else 
				{
					nouvDirection = ReflectionLawsAGH::SolveSpecularPart(configurationP.direction, *materialInfo, faceNormal);
				}

				//Calcul de la nouvelle direction de réflexion (en reprenant la célérité de propagation du son)
				configurationP.direction = nouvDirection*distanceSurLePas;
				collisionResolution = true;
				SetNextParticleCollision(configurationP);
			}
		}


		if (iteration>1000)
		{
			//Elle est détruite et l'utilisateur en sera informé
			if (configurationP.stateParticule == PARTICULE_STATE_ALIVE)
				configurationP.stateParticule = PARTICULE_STATE_LOOP;
			configurationP.energie = 0;
			return;
		}
	}

	if (configurationP.elapsedTime == 0.f)
	{   //Aucune collision sur le pas de temps courant
		FreeParticleTranslation(configurationP, configurationP.direction);
	}
	else {
		//Il y a eu une ou plusieurs collisions sur le pas de temps courant
		FreeParticleTranslation(configurationP, configurationP.direction*((deltaT - configurationP.elapsedTime) / deltaT));
		configurationP.elapsedTime = 0; //remise du compteur à 0
	}
}


void NextEventEstimationCore::FreeParticleTranslation(CONF_PARTICULE_AGH &configurationP, const vec3 &translationVector)
{
	if(configurationP.isShadowRay) reportTool->ShadowRayFreeTranslation(configurationP, configurationP.position + translationVector);
	// On prend en compte le rapprochement vers l'encombrement virtuel
	if (configurationP.currentTetra->volumeEncombrement)
		configurationP.distanceToNextEncombrementEle -= translationVector.length();
	configurationP.position += translationVector;
}

void NextEventEstimationCore::GenerateShadowRays(CONF_PARTICULE_AGH& particle, t_Material_BFreq* materialInfo, t_cFace* faceInfo, double deltaT, double distanceToTravel, std::list<CONF_PARTICULE_AGH>* shadowRays, double* probability)
{
	//Calculate and cast shadow rays
	for each (t_Recepteur_P* receiver in configurationTool->recepteur_p_List)
	{
		CONF_PARTICULE_AGH shadowRay = particle;
		vec3 newDirection, toReceiver;

		toReceiver = receiver->position - shadowRay.position;
		newDirection = toReceiver;
		newDirection.normalize();
		shadowRay.direction = newDirection * distanceToTravel;

		if (VisabilityTest(shadowRay, receiver->position))
		{
			shadowRay.targetReceiver = receiver;
			shadowRay.isShadowRay = true;

			float energy = BRDFs::SolveBRDFReflection(*materialInfo, faceInfo->normal, shadowRay, particle.direction, configurationTool);
			shadowRay.energie *= energy;

			//fast forward particle to receiver surrounding
			int timeStepNum = (toReceiver.length() - ((deltaT - particle.elapsedTime) / deltaT) - *configurationTool->FastGetConfigValue(Core_ConfigurationAGH::FPROP_RAYON_RECEPTEURP)) / distanceToTravel;

			decimal densite_proba_absorption_atmospherique = configurationTool->freqList[particle.frequenceIndex]->densite_proba_absorption_atmospherique;
			shadowRay.position = shadowRay.position + shadowRay.direction * (timeStepNum + (deltaT - particle.elapsedTime) / deltaT);
			shadowRay.elapsedTime = 0;
			shadowRay.pasCourant += timeStepNum + 1; //+1 for current time step

			if (*configurationTool->FastGetConfigValue(Core_ConfigurationAGH::IPROP_QUANT_TIMESTEP) < particle.pasCourant)
				break;

			shadowRay.currentTetra = &sceneTetraMesh->tetraedres[receiver->indexTetra];
			shadowRay.energie *= pow(densite_proba_absorption_atmospherique, timeStepNum);

			shadowRays->push_back(shadowRay);
			if(probability != nullptr) *probability *= energy;
		}
	}
}