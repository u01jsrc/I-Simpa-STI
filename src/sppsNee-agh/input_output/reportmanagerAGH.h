#include <input_output/reportmanager.h>
#include <sppsNeeAGHTypes.h>
#include <data_manager/Core_ConfigurationAGH.h>

/**
 * @file reportmanager.h
 * @brief Implémentation du gestionnaire de fichiers de rapports
 */


#ifndef __REPORT_MANAGER_AGH__
#define __REPORT_MANAGER_AGH__

/**
 * @brief Gestionnaire de fichier de sortie à déstination de l'interface PSPS
 *
 * Cette classe permet l'export de tout les fichiers de resultat de calcul.
 */
class ReportManagerAGH : public ReportManager
{

public:

	/**
	 * Constructeur de la classe
	 *
	 * @param _particlePath Chemin où enregistrer le fichier de particules
	 *
	 */
	ReportManagerAGH(t_ParamReport& _paramReport);
	~ReportManagerAGH();
	/**
	 * A appeler avant la translation d'une particule
	 */
	void ShadowRayFreeTranslation(CONF_PARTICULE_AGH& particleInfos, const vec3& nextPosition);

	formatGABE::GABE_Object* GetColStats();

	struct t_StatsAGH : public t_Stats
	{
		uentier_long partShadowRay;
		t_StatsAGH() { memset(this,0,sizeof(t_StatsAGH)); }
	} statReport;

	/**
	 * Sauvegarde le tableau de statistique des états de particules et les données de niveaux sonores globaux
	 */
	static void SaveThreadsStats(const CoreString& filename,const CoreString& filenamedBLvl,std::vector<t_sppsThreadParam>& cols,const t_ParamReport& params);
};

#endif
