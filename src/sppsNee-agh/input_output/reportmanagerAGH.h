#include <input_output/reportmanager.h>
#include <sppsNeeAGHTypes.h>
#include <data_manager/Core_ConfigurationAGH.h>

/**
 * @file reportmanager.h
 * @brief Implémentation du gestionnaire de fichiers de rapports
 */


#ifndef __REPORT_MANAGER_AGH__
#define __REPORT_MANAGER_AGH__


struct t_StatsAGH : public t_Stats
{
	uentier_long partShadowRay;
	t_StatsAGH() { memset(this, 0, sizeof(t_StatsAGH)); }
};

struct t_sppsThreadParamAGH : public t_sppsThreadParam
{
	std::vector<formatGABE::GABE_Data_Float*> GabeAngleData;

	t_sppsThreadParamAGH() : t_sppsThreadParam(){};
	void clearMem()
	{
		t_sppsThreadParam::clearMem();
		for (uentier idrecp = 0; idrecp < GabeAngleData.size(); idrecp++)
			delete GabeAngleData[idrecp];
	}
};

class t_angle_energy
{
private:
	double t, y;
public:
	int angle;
	bool extended;
	std::vector<double> energy;
	std::vector<double> correction;
	t_angle_energy() {}
	void fill_empty_data(bool mode)
	{
		extended = mode;
		if (!extended) {
			energy.resize(90);
			correction.resize(90);
		}
		else {
			energy.resize(90 * 360);
			correction.resize(90 * 360);
		}
	}
	void calc_angle(CONF_PARTICULE& particleInfos, t_cFace face)
	{
		if (!extended) {
			vec3 normal = face.normal;
			vec3 dir = particleInfos.direction;

			angle = (int)(acos(normal.dot(dir) / (dir.length()*normal.length()))*180.0 / M_PI);

			if (angle>89) { angle = 89; }	//probably not needed
			else if (angle<0) { angle = 0; }	//probably not needed
		}
		else {
			vec3 normal(face.normal.x, face.normal.y, face.normal.z);
			vec3 dir(particleInfos.direction.x, particleInfos.direction.y, particleInfos.direction.z);
			vec3 target(0, 0, 1);
			Matrix3 R;

			R.calculateRotationMatrix(normal, target);
			dir = R*dir;

			int phi = atan2(dir.y, dir.x) * 180 / M_PI;
			int theta = acos(dir.z / dir.length()) * 180 / M_PI;

			if (phi>179)
				phi = 179;

			angle = 90 * (phi + 180) + theta;
		}
	}
	void add_energy(CONF_PARTICULE& particleInfos)
	{
		y = particleInfos.energie - correction[angle];
		t = energy[angle] + y;
		correction[angle] = (t - energy[angle]) - y;
		energy[angle] = t;
		//energy[angle]+=particleInfos.energie;
	}
	void calc_energy_density() {
		if (!extended) {
			for (int i = 0; i<90; i++) {
				energy[i] = energy[i] / (-2 * M_PI*(cos((i + 1)*M_PI / 180) - cos((i)*M_PI / 180)));
			}
		}
		else {
			for (int j = 0; j<360; j++) {
				for (int i = 0; i<90; i++) {
					energy[j * 90 + i] = energy[j * 90 + i] / (-2 * M_PI / 360 * (cos((i + 1)*M_PI / 180) - cos((i)*M_PI / 180)));
				}
			}
		}
	}
	void normalize_energy_density() {
		double sum = 0.0;
		for (double& n : energy)
			sum += n;
		//double sum = accumulate(energy.begin(), energy.end(), 0.0);
		if (!extended) {
			for (int i = 0; i<90; i++) {
				energy[i] = energy[i] * 90 / sum;
			}
		}
		else {
			for (int i = 0; i<90 * 360; i++) {
				energy[i] = energy[i] * 90 * 360 / sum;
			}
		}
	}
};

/**
 * @brief Gestionnaire de fichier de sortie à déstination de l'interface PSPS
 *
 * Cette classe permet l'export de tout les fichiers de resultat de calcul.
 */
class ReportManagerAGH : public ReportManager
{
protected:
	std::vector <t_angle_energy> surfIncidenceAngleEnergy;
public:
	t_StatsAGH statReport;

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

	bool GetAngleStats(t_sppsThreadParamAGH& data, bool NormalizeAngleStats);

	formatGABE::GABE_Object* GetColStats();

	/**
	* Une particule est entrée en collision avec une face du modèle
	* L'appel doit se faire avant la recherche de la prochaine collision
	*/
	void ParticuleCollideWithSceneMesh(CONF_PARTICULE& particleInfos) override;
	void ParticuleCollideWithSceneMesh(CONF_PARTICULE_AGH& particleInfos);

	/**
	 * Sauvegarde le tableau de statistique des états de particules et les données de niveaux sonores globaux
	 */
	static void SaveThreadsStats(const CoreString& filename,const CoreString& filenamedBLvl,std::vector<t_sppsThreadParamAGH>& cols,const t_ParamReport& params);

	/**
	* Save information about angle energy relation
	*/
	static void SaveAngleStats(const CoreString& filename, const CoreString& filenamedBLvl, std::vector<t_sppsThreadParamAGH>& cols, const t_ParamReport& params, bool extended);
};

#endif
