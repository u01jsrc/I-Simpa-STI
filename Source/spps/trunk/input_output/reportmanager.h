#include <input_output/gabe/gabe.h>
#include <input_output\baseReportManager.h>
#include <sppsTypes.h>
#include <input_output/particles/part_binary.h>
#include <data_manager/core_configuration.h>
#include <list>
#include <iostream>
#include <numeric>
#include <Eigen/Dense>
#include <Eigen/Geometry>

using namespace Eigen;
/**
 * @file reportmanager.h
 * @brief Implémentation du gestionnaire de fichiers de rapports
 */


#ifndef __REPORT_MANAGER__
#define __REPORT_MANAGER__
/**
 * Certains fichiers de résultats nécessitent une sortie comprenant toutes les bande de fréquences
 * ainsi on doit stocker les données propres aux bandes de fréquence en dehors du thread de la bande sonore actuelle
 */

struct t_sppsThreadParam
{
	t_sppsThreadParam() { GabeColData=NULL;GabeSumEnergyFreq=NULL;GabeAngleData=NULL;AngleGroupNum=0;}
	t_FreqUsage* freqInfos;
	int AngleGroupNum;
	formatGABE::GABE_Object* GabeColData;
	formatGABE::GABE_Data_Float** GabeAngleData;
	formatGABE::GABE_Data_Float* GabeSumEnergyFreq;
	std::vector<formatGABE::GABE_Data_Float*> GabeSumEnergyCosPhi;		/*!< Tableau de récepteur ponctuel */
	std::vector<formatGABE::GABE_Data_Float*> GabeSumEnergyCosSqrtPhi;	/*!< Tableau de récepteur ponctuel */
	std::vector<formatGABE::GABE_Data_Float*> GabeIntensity[3];			/*!< Vecteur d'intensité en X,Y,Z \f$frac{W_{n}.Isec.\frac{\overrightarrow{Dir}}{\Vert\overrightarrow{Dir}\Vert}}{V}\f$ */
	std::vector<formatGABE::GABE_Data_Float*> GabeSlPerSrc;				/*!< Niveau sonore par source */
	std::vector<formatGABE::GABE_Data_Float*> GabeAngleInc[2];
	std::vector<l_decimal*> SrcContrib;									/*!< For each receiver, If timeStepInSourceOutput is true, contains the source contrib by time step */
	void clearMem()
	{
		for(uentier idrecp=0;idrecp<GabeSumEnergyCosPhi.size();idrecp++)
			delete GabeSumEnergyCosPhi[idrecp];
		for(uentier idrecp=0;idrecp<GabeSumEnergyCosSqrtPhi.size();idrecp++)
			delete GabeSumEnergyCosSqrtPhi[idrecp];
		for(uentier idrecp=0;idrecp<GabeSlPerSrc.size();idrecp++)
			delete GabeSlPerSrc[idrecp];
		for(short dim=0;dim<3;dim++)
			for(uentier idrecp=0;idrecp<GabeSumEnergyCosSqrtPhi.size();idrecp++)
				delete GabeIntensity[dim][idrecp];
		for(short dim=0;dim<2;dim++)
			for(uentier idrecp=0;idrecp<GabeSumEnergyCosSqrtPhi.size();idrecp++)
				delete GabeAngleInc[dim][idrecp];
		for(uentier idrecp=0;idrecp<SrcContrib.size();idrecp++) 
			delete[] SrcContrib[idrecp];
	}
};
typedef dvec3 veci_t;
class t_rp_lef
{
public:
	t_rp_lef(){Lf=NULL;Lfc=NULL;intensity=NULL;SrcContrib=NULL;theta=NULL;phi=NULL;en=NULL;}
	~t_rp_lef(){delete[] Lf;delete[] Lfc;delete[] intensity;delete[] SrcContrib;delete[] phi;delete[] theta;delete[] en;}
	void Init(const uentier& nbTimeStep,const uentier& nbsources, const bool& sourceLvlByTimeStep)
	{
		int sourceContribCols;
		if(sourceLvlByTimeStep) {
			sourceContribCols = nbsources * nbTimeStep;
		} else {
			sourceContribCols = nbsources;
		}
		Lf=new l_decimal[nbTimeStep];
		Lfc=new l_decimal[nbTimeStep];
		phi=new l_decimal[nbTimeStep];
		theta=new l_decimal[nbTimeStep];
		en=new l_decimal[nbTimeStep];
		intensity=new veci_t[nbTimeStep];
		SrcContrib=new l_decimal[sourceContribCols];
		memset(Lf,0,nbTimeStep*sizeof(l_decimal));
		memset(Lfc,0,nbTimeStep*sizeof(l_decimal));
		memset(SrcContrib,0,sourceContribCols*sizeof(l_decimal));
		memset(theta,0,nbTimeStep*sizeof(l_decimal));
		memset(phi,0,nbTimeStep*sizeof(l_decimal));
		memset(en,0,nbTimeStep*sizeof(l_decimal));
	}
	l_decimal* Lf;
	l_decimal* Lfc;
	l_decimal* SrcContrib;
	veci_t* intensity;
	l_decimal* theta;
	l_decimal* phi;
	l_decimal* en;
};

class t_angle_energy
{
private:
	double t,y;
public:
	int angle;
	bool extended;
	std::vector<double> energy;
	std::vector<double> correction;
	t_angle_energy(){}
	void fill_empty_data(bool mode)
	{	
		extended=mode;
		if(!extended){
			for(int j = 0; j < 90; j++)
			{
				energy.push_back(0);
				correction.push_back(0);
			}
		}else{
			for(int j = 0; j < 90*360; j++)
			{
				energy.push_back(0);
				correction.push_back(0);
			}
		}
	}	
	void calc_angle(CONF_PARTICULE& particleInfos, t_cFace face)
	{
		if(!extended){
			vec3 normal=face.normal;
			vec3 dir=particleInfos.direction;

			angle=(int)(acos(normal.dot(dir)/(dir.length()*normal.length()))*180.0/M_PI);
		
			if(angle>89){angle=89;}	//probably not needed
			else if(angle<0){angle=0;}	//probably not needed
		}else{
			Vector3f normal(face.normal.x,face.normal.y,face.normal.z);
			Vector3f dir(particleInfos.direction.x,particleInfos.direction.y,particleInfos.direction.z);
			Vector3f target(0,0,1);
			Vector3f cross,res;
			Matrix3f I,R,v;
			I.setIdentity(3,3);

			dir=dir/dir.norm();
			normal=normal/normal.norm();
			cross=normal.cross(target);
			
			if(cross.norm()>0.00000001){
				v<<0,-cross(2), cross(1),
					cross(2), 0 ,-cross(0),
					-cross(1), cross(0),0;


				R=I+v+v*v*(1-normal.dot(target))/cross.norm();
				dir=R*dir;
			}else{
				dir*=-1;
			}
			
			//std::cout << "normal = " << normal << std::endl;
			//std::cout << "dir = " << dir << std::endl;
			//std::cout << "target = " << target << std::endl;
			//std::cout << "I = " << I << std::endl;
			//std::cout << "cross = " << cross << std::endl;
			//std::cout << "v = " << v << std::endl;
			//std::cout << "R = " << R << std::endl;
			//std::cout << "dir = " << dir << std::endl;

			int phi=atan2(dir(1),dir(0))*180/M_PI;
			int theta=acos(dir(2)/dir.norm())*180/M_PI;

			//std::cout << "theta = " << theta << std::endl;
			//std::cout << "phi = " << phi << std::endl;

			if(phi>179)
				phi=179;

			angle=90*(phi+180)+theta;
		}
	}
	void add_eng(CONF_PARTICULE& particleInfos)
	{
		y=particleInfos.energie-correction[angle];
		t=energy[angle]+y;
		correction[angle]=(t-energy[angle])-y;
		energy[angle]=t;
		//energy[angle]+=particleInfos.energie;
	}
	void calc_energy_density(){
		if(!extended){
			for(int i=0;i<90;i++){
				energy[i]=energy[i]/(-2*M_PI*(cos((i+1)*M_PI/180)-cos((i)*M_PI/180)));
			}
		}else{
			for(int j=0;j<360;j++){
				for(int i=0;i<90;i++){
					energy[j*90+i]=energy[j*90+i]/(-2*M_PI/360*(cos((i+1)*M_PI/180)-cos((i)*M_PI/180)));
				}
			}
		}
	}
	void normalize_energy_density(){
		double sum=accumulate(energy.begin(),energy.end(),0.0);
		if(!extended){
			for(int i=0;i<90;i++){
				energy[i]=energy[i]*90/sum;
			}
		}else{
			for(int i=0;i<90*360;i++){
				energy[i]=energy[i]*90*360/sum;
			}
		}
	}
};


/**
 * @brief Gestionnaire de fichier de sortie à déstination de l'interface PSPS
 *
 * Cette classe permet l'export de tout les fichiers de resultat de calcul.
 */
class ReportManager : public BaseReportManager
{
protected:
	std::vector<t_rp_lef> lst_rp_lef;
public:
	/**
	 * @brief Structure décrivant les paramètres d'un objet de rapport
	 */
	struct t_ParamReport{
		uentier nbParticles;
		uentier nbTimeStep;
		uentier freqIndex;
		uentier freqValue;
		decimal timeStep;
		stringClass _particlePath;
		stringClass _particleFileName;
		stringClass _recepteur_surf_Path;
		stringClass _recepteur_surf_cut_Path;
		stringClass working_Path;
		t_TetraMesh* tetraModel;
		t_Mesh* sceneModel;
		Core_Configuration* configManager;
		bool NormalizeAngleStats;
	};
	std::vector <t_angle_energy> angle_energy;	//zmiana!!!
private:
	struct t_collision_history
	{
		vec3 collisionCoordinate;
		vec3 incidentVector;
		vec3 faceNormal;
		entier reflexionOrder;
		decimal energy;
		t_collision_history()
			: reflexionOrder(0)
		{
		}
		t_collision_history(const vec3& _faceNormal,const entier& _reflexionOrder,const vec3& _collisionCoordinate,const vec3& _incidentVector,const decimal& _energy)
			:faceNormal(_faceNormal),reflexionOrder(_reflexionOrder),collisionCoordinate(_collisionCoordinate),incidentVector(_incidentVector),energy(_energy)
		{

		}
	};

	t_ParamReport paramReport;
	std::fstream* particleFile;
	std::fstream* particleCSVFile;
	uentier_long lastParticuleFileHeaderInfo;
	binaryFHeader enteteSortie;
	uentier_long nbPasDeTempsMax;
	uentier_long realNbParticle;
	uentier nbSource;
	bool timeStepInSourceOutput;
	entier firstTimeStep;
	std::vector<binaryPTimeStep> positionsCurrentParticule;
	std::list<t_collision_history> collisionHistory; /*!< Garde l'historique des collisions si les particules sont sauvegardées*/
	l_decimal* tabEnergyByTimeStep; /*!< Tableau de l'energie global en fonction du temps */


	void writeParticleFile();
	void CloseLastParticleHeader();
	void CloseLastParticleFileHeader();
public:
	/**
	 * Constructeur de la classe
	 *
	 * @param _particlePath Chemin où enregistrer le fichier de particules
	 *
	 */
	ReportManager(t_ParamReport& _paramReport);
	~ReportManager();
	/**
	 * Méthode permettant d'enregistrer les information de la particule lors du passage du pas de temps
	 * @param particleInfos Informations de la particule au moment du nouveau pas de temps
	 */
	void RecordTimeStep(CONF_PARTICULE& particleInfos);
	/**
	 * Une nouvelle particule va être sauvegardé dans le fichier de particules
	 */
	void NewParticule(CONF_PARTICULE& particleInfos);
	/**
	 * Enregistre dans le fichier les données de la particule
	 */
	void SaveParticule();
	/**
	 * Une particule passe du volume d'un tétraèdre à un autre
	 * @param particleInfos Particule avant le passage
	 * @param nextTetra Nouveau tétraèdre
	 */
	void ParticuleGoToNextTetrahedra(CONF_PARTICULE& particleInfos,t_Tetra* nextTetra);

	/**
	 * A appeler avant la translation d'une particule
	 */
	void ParticuleFreeTranslation(CONF_PARTICULE& particleInfos, const vec3& nextPosition);
	/**
	 * Une particule est entrée en collision avec une face du modèle
	 * L'appel doit se faire avant la recherche de la prochaine collision
	 */
	void ParticuleCollideWithSceneMesh(CONF_PARTICULE& particleInfos);

	void SaveAndCloseParticleFile();

	formatGABE::GABE_Object* GetColStats();

	bool GetAngleStats(t_sppsThreadParam& data, bool NormalizeAngleStats);


	void FillWithLefData(t_sppsThreadParam& data);

	/**
	 * Exporte le cumul du niveau sonore
	 */
	formatGABE::GABE_Data_Float* GetSumEnergy();
	struct t_Stats
	{
		uentier_long partLost;
		uentier_long partAbsAtmo;
		uentier_long partAbsSurf;
		uentier_long partAbsEncombrement;
		uentier_long partLoop;
		uentier_long partAlive;
		uentier_long partTotal;
		double sumaric_time;
		double sumaric_reflections;
		t_Stats() { memset(this,0,sizeof(t_Stats)); }
	} statReport;

	/**
	 * Sauvegarde le tableau de statistique des états de particules et les données de niveaux sonores globaux
	 */
	static void SaveThreadsStats(const CoreString& filename,const CoreString& filenamedBLvl,std::vector<t_sppsThreadParam>& cols,const t_ParamReport& params);
	
	/**
	 * Save information about angle energy relation
	 */
	static void SaveAngleStats(const CoreString& filename,const CoreString& filenamedBLvl,std::vector<t_sppsThreadParam>& cols,const t_ParamReport& params, bool extended);
	
	/**
	 * Sauvegarde le tableau contenants les données servant aux calcul des paramètres acoustiques avancées tels que la tenue acoustique G ou la fraction d'énergie latérale précoce LF et LFC
	 */
	static void SaveRecpAcousticParamsAdvance(const CoreString& filename,std::vector<t_sppsThreadParam>& cols,const t_ParamReport& params);
	/**
	 * Save of angle of incidence for receiver
	 */
	static void SaveIncidenceAngle(const CoreString& filename,std::vector<t_sppsThreadParam>& cols,const t_ParamReport& params);

	/**
	 * Sauvegarde le tableau contenants les données de vecteurs d'intensité
	 */
	static void SaveRecpIntensity(const CoreString& filename,std::vector<t_sppsThreadParam>& cols,const t_ParamReport& params);
	/**
	 * Sauvegarde le tableau contenants les données de niveau sonore en fonction des sources
	 */
	static void SaveSoundLevelBySource(const CoreString& filename,std::vector<t_sppsThreadParam>& cols,const t_ParamReport& params);


	static void SetPostProcessSurfaceReceiver(Core_Configuration& coreConfig ,const std::size_t& freqIndex,std::vector<r_Surf*>& surface_receiver_list,const float& timeStep);
	static void SetPostProcessCutSurfaceReceiver(Core_Configuration& coreConfig ,const std::size_t& freqIndex,std::vector<r_SurfCut*>& surface_receiver_list,const float& timeStep);
};

#endif
