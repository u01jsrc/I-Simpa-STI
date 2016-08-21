/* ---------------------------------------------------------------------------------------------------------
* This code is based on sppsNantes 2.1.4 by Judicael Picaut and Nicolas Fortin - IFSTTAR
*
* It is modified for resarch and educational purposes by
* Wojciech Binek, AGH University of Science and Technology, Cracow, Poland
*
* All added features are experimental and may require evaluation!
* There is no warranty that they produce right results.
*
* This calculation code implements path tracing with next event estimation method based on oryginal SPPS code
* ---------------------------------------------------------------------------------------------------------------*/

//#include "coreTypes.h"
#include "sppsTypes.h"

#ifndef SPPSNEE_AGH_TYPES
#define SPPSNEE_AGH_TYPES



#define SPPSNEE_AGH_VERSION "Spps NEE AGH 0.1 (22.04.2016). Based on SPPS Nantes 2.1.4 version december 13 2013"
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
	t_Recepteur_P* targetReceiver;
	CONF_PARTICULE_AGH() { targetReceiver = NULL; isShadowRay = false; sourceid = 0; currentTetra = NULL; distanceToNextEncombrementEle = 0.f; stateParticule = PARTICULE_STATE_ALIVE; elapsedTime = 0.; reflectionOrder = 0; }
};


#endif
