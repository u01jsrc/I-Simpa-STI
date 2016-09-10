//#include "data_manager/base_core_configuration.h"
#include "data_manager/core_configuration.h"

/**
 * @file core_configuration.h
 * @brief Gestion des données de configuration
 *
 * Cette classe permet d'acceder aux informations de la scène de facon optimisé et simple
 */

#ifndef __CORE_CONFIGURATION_AGH__
#define __CORE_CONFIGURATION_AGH__

/**
 * @brief Gestion des données de configuration
 *
 * Cette classe permet d'acceder aux informations de la scène de facon optimisé et simple
 */
class Core_ConfigurationAGH : public Core_Configuration
{
public:
	/**
	 * Initialisation des paramètres du coeur de calcul à partir d'un fichier XML
	 * @param xmlFilePath Chemin du fichier XML
	 */
	Core_ConfigurationAGH( CoreString xmlFilePath );
	/**
	 * Destructeur
	 */
	~Core_ConfigurationAGH( );
	/////////////////////////////////////////////////////////
	//	Trés rapide
	/////////////////////////////////////////////////////////
	//inline decimal* FastGetConfigValue(FPROP propertyIndex){return (tabFloatProp+propertyIndex); }
	//inline CoreString* FastGetConfigValue(SPROP propertyIndex){return (tabStringProp+propertyIndex); }
	//inline entier* FastGetConfigValue(IPROP propertyIndex){return (tabIntProp+propertyIndex); }
	//
	//inline decimal* FastGetConfigValue(BASE_FPROP propertyIndex){return (tabFloatProp+propertyIndex); }
	//inline CoreString* FastGetConfigValue(BASE_SPROP propertyIndex){return (tabStringProp+propertyIndex); }
	//inline entier* FastGetConfigValue(BASE_IPROP propertyIndex){return (tabIntProp+propertyIndex); }

private:
	//void SetConfigInformation(FPROP propertyIndex,decimal valeur);
	//void SetConfigInformation(SPROP propertyIndex,CoreString valeur);
	//void SetConfigInformation(IPROP propertyIndex,entier valeur);
	//void SetConfigInformation(BASE_FPROP propertyIndex,decimal valeur);
	//void SetConfigInformation(BASE_SPROP propertyIndex,CoreString valeur);
	//void SetConfigInformation(BASE_IPROP propertyIndex,entier valeur);
	

};

#endif