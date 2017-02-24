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
	Core_ConfigurationAGH( CoreString xmlFilePath, bool verbose_mode = false);
	/**
	 * Destructeur
	 */
	~Core_ConfigurationAGH( );
};

#endif