//#include "data_manager/base_core_configuration.h"
#include "data_manager/core_configuration.h"
//#include "data_manager/core_configuration.cpp"

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
class Core_Configuration_AGH : public Core_Configuration
{
public:
	/**
	 * Enumeration of new int properites
	 */
	enum NEW_IPROP
	{
		I_PROP_CALCULATION_CORE_SELLECTION			/*!< Sellect calculation core*/
	};

	/**
	 * Initialisation des paramètres du coeur de calcul à partir d'un fichier XML
	 * @param xmlFilePath Chemin du fichier XML
	 */
	Core_Configuration_AGH( CoreString xmlFilePath );
	/**
	 * Destructeur
	 */
	~Core_Configuration_AGH( );

	inline entier* FastGetConfigValue(NEW_IPROP propertyIndex) { return (tabIntProp + propertyIndex); }

	inline decimal* FastGetConfigValue(FPROP propertyIndex) { return (tabFloatProp + propertyIndex); }
	inline CoreString* FastGetConfigValue(SPROP propertyIndex) { return (tabStringProp + propertyIndex); }
	inline entier* FastGetConfigValue(IPROP propertyIndex) { return (tabIntProp + propertyIndex); }

	inline decimal* FastGetConfigValue(BASE_FPROP propertyIndex) { return (tabFloatProp + propertyIndex); }
	inline CoreString* FastGetConfigValue(BASE_SPROP propertyIndex) { return (tabStringProp + propertyIndex); }
	inline entier* FastGetConfigValue(BASE_IPROP propertyIndex) { return (tabIntProp + propertyIndex); }
	/////////////////////////////////////////////////////////
	//	Trés rapide
	/////////////////////////////////////////////////////////
private:

	void SetConfigInformation(NEW_IPROP propertyIndex, entier valeur);

	
};

#endif