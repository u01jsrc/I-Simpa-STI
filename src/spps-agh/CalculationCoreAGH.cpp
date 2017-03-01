#include "CalculationCoreAGH.h"
#include "tools/collision.h"
#include "tools/dotreflectionAGH.h"
#include "tools/brdfreflection.h"
#include "tools/dotdistribution.h"
#include <iostream>

#ifndef INTSIGN
	#define INTSIGN(x) ((x < 0) ? -1 : 1 );
#endif
#define TetraFaceTest(idFace) sommetsFace=configurationP.currentTetra->faces[idFace].indiceSommets;\
			if(configurationP.currentTetra->faces[idFace].normal.dot(translationVector)<EPSILON)\
				if(collision_manager::intersect_triangle(posPart,dirPart,scenenodes[sommetsFace.a].v,scenenodes[sommetsFace.b].v,scenenodes[sommetsFace.c].v,&t,&u,&v)==1)\
					return idFace;



CalculationCoreSPPS::CalculationCoreSPPS(t_Mesh& _sceneMesh,t_TetraMesh& _sceneTetraMesh,CONF_CALCULATION_AGH &_confEnv, Core_ConfigurationAGH &_configurationTool,ReportManagerAGH* _reportTool): 
CalculationCore(_sceneMesh, _sceneTetraMesh, _confEnv, _configurationTool, _reportTool), confEnv(_confEnv)
{
	configurationTool=&_configurationTool;
	reportTool=_reportTool;
	doDirectSoundCalculation = false;
}
//todo Fix method so it can use any source directivity 
void CalculationCoreSPPS::CalculateDirectSound(const CONF_PARTICULE_AGH& prototypeParticle, t_Source& sourceInfo,const float& distancePerTimeStep, unsigned int freq)
{
	float receiverRadius = *configurationTool->FastGetConfigValue(Core_ConfigurationAGH::FPROP_RAYON_RECEPTEURP);

	CONF_PARTICULE_AGH shadowRay = prototypeParticle;
	shadowRay.position = sourceInfo.Position;
	shadowRay.isShadowRay = true;
	shadowRay.outputToParticleFile = false;

	for each (t_Recepteur_P* receiver in configurationTool->recepteur_p_List)
	{
		vec3 toReceiver = (receiver->position - shadowRay.position);
		shadowRay.targetReceiver = receiver;
		shadowRay.direction = toReceiver;
		shadowRay.direction.normalize();
		shadowRay.direction *= distancePerTimeStep;
		
		bool isVisible = VisabilityTest(shadowRay, receiver->position);

		if (isVisible && sourceInfo.type == SOURCE_TYPE_OMNIDIRECTION) {
			double solidAngle = (M_PI*receiverRadius*receiverRadius) / (toReceiver.length()*toReceiver.length());

			//0.66 is normalization factor used to acount for not treating receiver as sphere 
			//- points far from sphere center ar treated with the same weight as ones in the middle,
			//proper weight schould be proportional to length of intersection, but it is hard to be evaluated quickly
			shadowRay.energie = (1 / (4 * M_PI))*solidAngle*0.66*sourceInfo.bandeFreqSource[shadowRay.frequenceIndex].w_j;
			confEnv.duplicatedParticles.push_back(shadowRay);
		}		
		else if (isVisible && sourceInfo.type == SOURCE_TYPE_DIRECTION) {
			double solidAngle = (M_PI*receiverRadius*receiverRadius) / (toReceiver.length()*toReceiver.length());

			std::tuple<double, double> coord_sph = t_DirectivityBalloon::loudspeaker_coordinate(sourceInfo.Direction, shadowRay.direction);
			double phi = RadToDeg(std::get<0>(coord_sph));
			double theta = RadToDeg(std::get<1>(coord_sph));
			if (sourceInfo.directivity->asInterpolatedValue(freq, phi, theta))
			{
				double spl = sourceInfo.directivity->getInterpolatedValue(freq, phi, theta);
				shadowRay.energie = sourceInfo.bandeFreqSource[shadowRay.frequenceIndex].w_j * pow(10, spl / 10);
			}

			//0.66 is normalization factor used to acount for not treating receiver as sphere 
			//- points far from sphere center ar treated with the same weight as ones in the middle,
			//proper weight schould be proportional to length of intersection, but it is hard to be evaluated quickly
			shadowRay.energie *= (1 / (4 * M_PI))*solidAngle*0.66;
			confEnv.duplicatedParticles.push_back(shadowRay);
		}
		else if (isVisible && sourceInfo.type == SOURCE_TYPE_XY && sourceInfo.Position.z == receiver->position.z) {
			double rad = (2*receiverRadius) / (toReceiver.length());

			//0.7854 is normalization factor used to acount for not treating receiver cut as circle 
			//- points far from sphere center ar treated with the same weight as ones in the middle,
			//proper weight schould be proportional to length of intersection, but it is hard to be evaluated quickly
			shadowRay.energie = (1/(2 * M_PI))*rad*0.7854*sourceInfo.bandeFreqSource[shadowRay.frequenceIndex].w_j;
			confEnv.duplicatedParticles.push_back(shadowRay);
		}
		else if (isVisible && sourceInfo.type == SOURCE_TYPE_XZ && sourceInfo.Position.y == receiver->position.y) {
			double rad = (2 * receiverRadius) / (toReceiver.length());

			//0.7854 is normalization factor used to acount for not treating receiver cut as circle 
			//- points far from sphere center ar treated with the same weight as ones in the middle,
			//proper weight schould be proportional to length of intersection, but it is hard to be evaluated quickly
			shadowRay.energie = (1 / (2 * M_PI))*rad*0.7854*sourceInfo.bandeFreqSource[shadowRay.frequenceIndex].w_j;
			confEnv.duplicatedParticles.push_back(shadowRay);
		}
		else if (isVisible && sourceInfo.type == SOURCE_TYPE_YZ && sourceInfo.Position.x == receiver->position.x) {
			double rad = (2 * receiverRadius) / (toReceiver.length());

			//0.7854 is normalization factor used to acount for not treating receiver cut as circle 
			//- points far from sphere center ar treated with the same weight as ones in the middle,
			//proper weight schould be proportional to length of intersection, but it is hard to be evaluated quickly
			shadowRay.energie = (1 / (2 * M_PI))*rad*0.7854*sourceInfo.bandeFreqSource[shadowRay.frequenceIndex].w_j;
			confEnv.duplicatedParticles.push_back(shadowRay);
		}
		else if (VisabilityTest(prototypeParticle, receiver->position) && sourceInfo.type == SOURCE_TYPE_UNIDIRECTION) {
			shadowRay.energie = sourceInfo.bandeFreqSource[shadowRay.frequenceIndex].w_j;
			confEnv.duplicatedParticles.push_back(shadowRay);
		}
		else{
			std::cout << "Receiver " << receiver->lblRp << " is not visible from source " << sourceInfo.sourceName << std::endl;
		}
	}
}

bool CalculationCoreSPPS::Run(CONF_PARTICULE_AGH configurationP)
{
	decimal densite_proba_absorption_atmospherique=configurationTool->freqList[configurationP.frequenceIndex]->densite_proba_absorption_atmospherique;
	SetNextParticleCollision(configurationP);						//1er test de collision
	SetNextParticleCollisionWithObstructionElement(configurationP);	// Test de collision avec objet virtuel encombrant
	//Au premier pas de temps il faut enregistrer l'energie de la particule dans la maille courante
	//configurationP.stateParticule=PARTICULE_STATE_ALIVE;
	reportTool->ParticuleGoToNextTetrahedra(configurationP,configurationP.currentTetra);
	while(configurationP.stateParticule==PARTICULE_STATE_ALIVE && configurationP.pasCourant<confEnv.nbPasTemps)
	{
		//Test d'absorption atmosphérique
		if(*configurationTool->FastGetConfigValue(Core_ConfigurationAGH::IPROP_DO_CALC_ABS_ATMO) && !configurationP.isShadowRay)
		{
			//Test de méthode de calcul
			if(*configurationTool->FastGetConfigValue(Core_ConfigurationAGH::IPROP_ENERGY_CALCULATION_METHOD))
			{ //Energetique
				configurationP.energie*=densite_proba_absorption_atmospherique;
				if(configurationP.energie<=configurationP.energie_epsilon)
				{
					configurationP.stateParticule=PARTICULE_STATE_ABS_ATMO;
				}
			}else{
				//Aléatoire
				if(GetRandValue()>=densite_proba_absorption_atmospherique)
				{
					configurationP.energie=0;	// La particule est détruite
					configurationP.stateParticule=PARTICULE_STATE_ABS_ATMO;
				}
			}
		}

		if(configurationP.stateParticule==PARTICULE_STATE_ALIVE)
			Movement(configurationP);	//Effectue un mouvement sur la distance restante


		//Fin du pas de temps, la particule a effectué aucune ou plusieurs collisions
		if(configurationP.stateParticule==PARTICULE_STATE_ALIVE)
		{
			reportTool->RecordTimeStep(configurationP);
			if(configurationP.currentTetra->z!=-1)								//Si le milieu n'est pas homogène
			{
				OnChangeCelerite(configurationP,configurationP.currentTetra);	//On calcul le changement de direction dû au gradiant de célérité
				SetNextParticleCollision(configurationP);
			}
			configurationP.pasCourant++;
		}
	}
	switch(configurationP.stateParticule)
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

void CalculationCoreSPPS::Movement(CONF_PARTICULE_AGH &configurationP)
{
	CalculationCore::Movement(configurationP);
}

void CalculationCoreSPPS::FreeParticleTranslation(CONF_PARTICULE_AGH &configurationP, const vec3 &translationVector)
{
	CalculationCore::FreeParticleTranslation(configurationP, translationVector);
}

bool CalculationCoreSPPS::VisabilityTest(const CONF_PARTICULE_AGH &configurationP, vec3 &TargetPosition)
{
	float obst_dist;
	float t;
	float rec_dist = (configurationP.position - TargetPosition).length();

	CONF_PARTICULE_AGH testParticle = configurationP;

	testParticle.nextModelIntersection.idface = GetTetraFaceCollision(testParticle, testParticle.direction, t);
	testParticle.nextModelIntersection.collisionPosition = testParticle.position + testParticle.direction*t;
	obst_dist = configurationP.direction.length()*t;

	int maxIteration = rec_dist/configurationP.direction.length() + 20;
	int iteration = 0;

	while (testParticle.currentTetra->faces[testParticle.nextModelIntersection.idface].face_scene == NULL && iteration++<maxIteration)
	{
		testParticle.position = testParticle.nextModelIntersection.collisionPosition;

		t_Tetra* nextTetra = testParticle.currentTetra->voisins[testParticle.nextModelIntersection.idface];
		if (!nextTetra)
		{
			return false;
		}
		else
		{
			testParticle.currentTetra = nextTetra;
			testParticle.nextModelIntersection.idface = GetTetraFaceCollision(testParticle, testParticle.direction, t);
			testParticle.nextModelIntersection.collisionPosition = testParticle.position + testParticle.direction*t;
			obst_dist += testParticle.direction.length()*t;

			if (rec_dist < obst_dist)
			{
				return true;
			}
		}
	}

	return false;
}

