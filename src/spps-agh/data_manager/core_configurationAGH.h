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
	enum NEW_SPROP
	{
		SPROP_ANGLE_FILE_PATH = 70,
	};

	enum NEW_FPROP
	{
		FPROP_MLT_LARGE_STEP_PROB = 70,
		FPROP_MLT_SPECULAR_REFL_PROB,
		FPROP_MLT_MUTATION_NUMBER,
		FPROP_NEE_SHADOWRAY_PROB,
	};

	enum NEW_IPROP
	{
		I_PROP_CALCULATION_CORE_SELLECTION = 70,
		IPROP_NORMALIZE_ANGLE_STATS,
		IPROP_EXTENDED_ANGLE_STATS,
		IPROP_ANGLE_STATS_MIN_REFL,					/*!< Min reflection order for angle stats calculation*/
		IPROP_CAST_SR_TO_SURFACE_REC
	};
	/**
	 * Initialisation des paramètres du coeur de calcul à partir d'un fichier XML
	 * @param xmlFilePath Chemin du fichier XML
	 */
	Core_ConfigurationAGH( CoreString xmlFilePath, bool verbose_mode = false);
	/**
	 * Destructeur
	 */
	~Core_ConfigurationAGH( );
	
	inline decimal* FastGetConfigValue(FPROP propertyIndex) { return (tabFloatProp + propertyIndex); }
	inline CoreString* FastGetConfigValue(SPROP propertyIndex) { return (tabStringProp + propertyIndex); }
	inline entier* FastGetConfigValue(IPROP propertyIndex) { return (tabIntProp + propertyIndex); }

	inline decimal* FastGetConfigValue(BASE_FPROP propertyIndex) { return (tabFloatProp + propertyIndex); }
	inline CoreString* FastGetConfigValue(BASE_SPROP propertyIndex) { return (tabStringProp + propertyIndex); }
	inline entier* FastGetConfigValue(BASE_IPROP propertyIndex) { return (tabIntProp + propertyIndex); }

	inline decimal* FastGetConfigValue(NEW_FPROP propertyIndex) { return (tabFloatProp + propertyIndex); }
	inline CoreString* FastGetConfigValue(NEW_SPROP propertyIndex) { return (tabStringProp + propertyIndex); }
	inline entier* FastGetConfigValue(NEW_IPROP propertyIndex) { return (tabIntProp + propertyIndex); }

	void SetConfigInformation(FPROP propertyIndex, decimal valeur);
	void SetConfigInformation(SPROP propertyIndex, CoreString valeur);
	void SetConfigInformation(IPROP propertyIndex, entier valeur);
	void SetConfigInformation(BASE_FPROP propertyIndex, decimal valeur);
	void SetConfigInformation(BASE_SPROP propertyIndex, CoreString valeur);
	void SetConfigInformation(BASE_IPROP propertyIndex, entier valeur);
	void SetConfigInformation(NEW_FPROP propertyIndex, decimal valeur);
	void SetConfigInformation(NEW_SPROP propertyIndex, CoreString valeur);
	void SetConfigInformation(NEW_IPROP propertyIndex, entier valeur);

	void LoadAdvancedSPPS(CXmlNode * simuNode);
	void LoadAdvancedNEE(CXmlNode * simuNode);
	void LoadAdvancedMLT(CXmlNode * simuNode);
};

#endif