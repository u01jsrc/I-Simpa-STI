#include "sppsNeeAGHTypes.h"	//les types de données sont déclaré ici
						// Il est déconseillé d'utiliser d'autre types que ceux déclaré dans ce fichier dans le programme
#include "input_output/reportmanagerAGH.h"
#include "data_manager/Core_ConfigurationAGH.h"
#include <list>
#ifndef __CALC_CORE_AGH__
#define __CALC_CORE_AGH__

/**
 * @file CalculationCore.h
 * @brief Moteur de calcul basé sur le calcul particule par particule
 */


void printVec(vec3 inf);
/**
 * @brief Moteur de calcul basé sur le calcul particule par particule
 *
 * Ce moteur de calcul utilise le maillage du modèle afin d'optimiser le temps de recherche de collision.
 */
class CalculationCore
{
public:
	/**
	 * @brief Paramètres du calcul de propagation acoustique
	 *
	 * Structure de données donnant les informations sur les paramètres globaux de calcul
	 */
	struct CONF_CALCULATION
	{
		uentier nbPasTemps;	/*!< Nombre de pas de temps */
		decimal pasTemps;	/*!< Pas de temps (s) */
		std::list<CONF_PARTICULE_AGH> duplicatedParticles; /*!< Particules à calculer par la suite */
	};

protected:
t_Mesh *sceneMesh;
t_TetraMesh *sceneTetraMesh;
ReportManagerAGH *reportTool;
Core_ConfigurationAGH *configurationTool;
CONF_CALCULATION& confEnv;

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
	CalculationCore(t_Mesh& _sceneMesh,t_TetraMesh& _sceneTetraMesh,CONF_CALCULATION &confEnv, Core_ConfigurationAGH &_configurationTool,ReportManagerAGH* _reportTool);
	/**
	 * Execute le calcul pour une particule
	 * @param configurationP Configuration de la particule
	 * @return Vrai si le calcul c'est effectué avec succès 
	 */
	virtual bool Run(CONF_PARTICULE_AGH configurationP);
	void CalculateDirectSound(CONF_PARTICULE_AGH shadowRay, t_Source& sourceInfo, float distancePerTimeStep);
	virtual ~CalculationCore() {}
protected:
	virtual void Movement(CONF_PARTICULE_AGH &configurationP);
	inline decimal GetDistance(CONF_PARTICULE_AGH &configurationP);
	bool CollisionTest(CONF_PARTICULE_AGH &configurationP,uentier &faceIndex,INTERSECTION_INFO &infoIntersection, float &factDistance);
	void SetNextParticleCollision(CONF_PARTICULE_AGH &configurationP);
	
	/**
	* Test visability for particule "configurationP" and the point TargetPosition
	*
	* @param configurationP particle to be tested
	* @param TargetPosition destination
	*/
	bool VisabilityTest(CONF_PARTICULE_AGH &configurationP, vec3 &TargetPosition);

	void SetNextParticleCollisionWithObstructionElement(CONF_PARTICULE_AGH &configurationP);
	

	/**
		 * Translation d'une particule dans un milieu libre de collisions avec la scène
		 */
	virtual void FreeParticleTranslation(CONF_PARTICULE_AGH &configurationP,const vec3 &translationVector);
	/**
	 * Permet de connaitre la collision de la particule sur la face du tetrahèdre courant
	 * @return Indice de la face [1-4] (-1 si la particule n'est pas dans un tetrahèdre)
	 * @param t facteur de translationVector afin d'arrive a cette face
	 */
	entier_court GetTetraFaceCollision(CONF_PARTICULE_AGH &configurationP,vec3 &translationVector,float &t);

	void OnChangeCelerite(CONF_PARTICULE_AGH &configurationP, t_Tetra* tetra2);
	void TraverserTetra(CONF_PARTICULE_AGH &configurationP, bool& collisionResolution);
};

#endif