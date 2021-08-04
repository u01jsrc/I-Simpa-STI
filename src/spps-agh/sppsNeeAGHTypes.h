/* ---------------------------------------------------------------------------------------------------------
* This code is based on sppsNantes 2.2.1 by Judicael Picaut and Nicolas Fortin - IFSTTAR
*
* It is modified for resarch and educational purposes by
* Wojciech Binek, AGH University of Science and Technology, Cracow, Poland
*
* All added features are experimental and may require evaluation!
* There is no warranty that they produce right results.
*
* This project is a basis for developement of new calculation methods based on original SPPS code
* ---------------------------------------------------------------------------------------------------------------*/

/* ---------------------------------------------------------------------------------------------------------
*changelog 10.09.2016:
*	- Fixed: Next Event Estimation Code - passed evaluation
*	- New: New calculation code - MLT (Kelemen style) - intial version
*	
*	- TODO: ALL - Full integration with source directivity options
*	- TODO: MLT - Fix for models with more than one receiver - wrong weight calculation
*	- TODO: MLT - Configuration via I-SIMPA interface, not hardcoded values
*	- TODO: MLT - Initial ray direction mutations
*	- TODO: MLT - Compliance with all diffusion models (currently only Lambert + Specular)
* ---------------------------------------------------------------------------------------------------------------*/

//#include "coreTypes.h"
#include "sppsTypes.h"
#include <list>

#ifndef SPPSNEE_AGH_TYPES
#define SPPSNEE_AGH_TYPES



#define SPPSNEE_AGH_VERSION "SPPS-AGH 0.36 (16.06.2021). Based on SPPS Nantes 2.2.1 version ocober 2020"
#define __USE_BOOST_RANDOM_GENERATOR__
#define UTILISER_MAILLAGE_OPTIMISATION


/**
 * @brief Informations propres à une particule
 *
 * Structure de données donnant les informations sur une particule
 */
struct CONF_PARTICULE_AGH : public CONF_PARTICULE
{
	bool isShadowRay;							/*!< Defines if the particle is main ray or shadow ray	*/
	bool record;								/*!< Sets if particle should be recorded by reportmanager (shadow rays are allways recorded) */
	t_Recepteur_P* targetReceiver;
	CONF_PARTICULE_AGH() { targetReceiver = NULL; isShadowRay = false; record = false; sourceid = 0; currentTetra = NULL; distanceToNextEncombrementEle = 0.f; stateParticule = PARTICULE_STATE_ALIVE; elapsedTime = 0.; reflectionOrder = 0; }
};

/**
* 
*
* Particle definition for MLT calculation
*/
struct CONF_PARTICULE_MLT : public CONF_PARTICULE_AGH
{
	std::vector<double> matAbsorbtions;				/*!< Defines if the particle is main ray or shadow ray	*/
	std::vector<double> travelProbability;				
	std::vector<double> refDir1;
	std::vector<double> refDir2;
	std::list<CONF_PARTICULE_AGH> shadowRays;

	double totalDistance = 0;
	double totalProbability = 1;
	double weight = 0;
	int reflectionsNum = 0;
	int creationTime = 0;
	int largeStep = 0;

	struct initialState
	{
		vec3 direction;
		t_Tetra* currentTetra;
		vec3 position;
		int creationTime;
		double energie;
		PARTICULE_STATE stateParticule;

	};

	initialState iniState;

	CONF_PARTICULE_MLT() {targetReceiver = NULL; isShadowRay = false; sourceid = 0; currentTetra = NULL; distanceToNextEncombrementEle = 0.f; stateParticule = PARTICULE_STATE_ALIVE; elapsedTime = 0.; reflectionOrder = 0; }
	CONF_PARTICULE_MLT(CONF_PARTICULE_AGH particleAGH)
	{
		targetReceiver = particleAGH.targetReceiver; 
		isShadowRay = particleAGH.isShadowRay; 
		sourceid = particleAGH.sourceid; 
		currentTetra = particleAGH.currentTetra; 
		distanceToNextEncombrementEle = particleAGH.distanceToNextEncombrementEle; 
		stateParticule = particleAGH.stateParticule;
		elapsedTime = particleAGH.elapsedTime;
		reflectionOrder = particleAGH.reflectionOrder;

		energie = particleAGH.energie;
		direction = particleAGH.direction;
		elapsedTime = particleAGH.elapsedTime;
		energie = particleAGH.energie;
		pasCourant = particleAGH.pasCourant;
		energie_epsilon = particleAGH.energie_epsilon;
		frequenceIndex = particleAGH.frequenceIndex;
		position = particleAGH.position;
		nextModelIntersection = particleAGH.nextModelIntersection;
		outputToParticleFile = false;
		creationTime = particleAGH.pasCourant;
	}
	void SetInitialState(CONF_PARTICULE_MLT input_particle)
	{
		iniState.direction = input_particle.direction;
		iniState.currentTetra = input_particle.currentTetra;
		iniState.position = input_particle.position;
		iniState.creationTime = input_particle.creationTime;
		iniState.energie = input_particle.energie;
		iniState.stateParticule = input_particle.stateParticule;
	}
	void ResetToInitialState()
	{
		direction = iniState.direction;
		currentTetra = iniState.currentTetra;
		position = iniState.position;
		creationTime = iniState.creationTime;
		energie = iniState.energie;
		stateParticule = iniState.stateParticule;
	}
};


#endif
