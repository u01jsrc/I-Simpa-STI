#include "sppsNeeAGHTypes.h"	//les types de données sont déclaré ici
						// Il est déconseillé d'utiliser d'autre types que ceux déclaré dans ce fichier dans le programme
#include "input_output/reportmanagerAGH.h"
#include "data_manager/Core_ConfigurationAGH.h"
#include "CalculationCore.h"
#include <list>
#ifndef __CALC_CORE_AGH__
#define __CALC_CORE_AGH__

/**
 * @file CalculationCoreSPPS.h
 * @brief Moteur de calcul basé sur le calcul particule par particule
 */


//void printVec(vec3 inf);
/**
 * @brief Moteur de calcul basé sur le calcul particule par particule
 *
 * Ce moteur de calcul utilise le maillage du modèle afin d'optimiser le temps de recherche de collision.
 */
class CalculationCoreSPPS : public CalculationCore
{
public:
	/**
	 * @brief Paramètres du calcul de propagation acoustique
	 *
	 * Structure de données donnant les informations sur les paramètres globaux de calcul
	 */
	struct CONF_CALCULATION_AGH : CONF_CALCULATION
	{
		std::list<CONF_PARTICULE_AGH> duplicatedParticles; /*!< Particules à calculer par la suite */
	};

protected:
ReportManagerAGH *reportTool;
Core_ConfigurationAGH *configurationTool;
CONF_CALCULATION_AGH& confEnv;

public:
	bool doDirectSoundCalculation;
	/**
	 * @brief Constructeur du moteur de calcul.
	 * 
	 * @param _pVertices Tableau de points du modèle
	 * @param _pGroups Tableau de faces du modèle
	 * @param intersTool Outils de calcul d'intersection
	 * @param confEnv Configuration générale de l'environnement
	 * @param _reportTool Classe d'enregistrements des résultats de calculs.
	 * @param _configurationTool Classe de gestion de configuration
	 */
	CalculationCoreSPPS(t_Mesh& _sceneMesh,t_TetraMesh& _sceneTetraMesh,CONF_CALCULATION_AGH &confEnv, Core_ConfigurationAGH &_configurationTool,ReportManagerAGH* _reportTool);
	/**
	 * Execute le calcul pour une particule
	 * @param configurationP Configuration de la particule
	 * @return Vrai si le calcul c'est effectué avec succès 
	 */
	virtual bool Run(CONF_PARTICULE_AGH configurationP);
	void CalculateDirectSound(CONF_PARTICULE_AGH shadowRay, t_Source& sourceInfo, float distancePerTimeStep);
	virtual ~CalculationCoreSPPS() {}
protected:
	virtual void Movement(CONF_PARTICULE_AGH &configurationP);

	/**
	* Test visability for particule "configurationP" and the point TargetPosition
	*
	* @param configurationP particle to be tested
	* @param TargetPosition destination
	*/
	bool VisabilityTest(CONF_PARTICULE_AGH &configurationP, vec3 &TargetPosition);

	/**
	* Translation d'une particule dans un milieu libre de collisions avec la scène
	*/
	virtual void FreeParticleTranslation(CONF_PARTICULE_AGH &configurationP,const vec3 &translationVector);
};

#endif