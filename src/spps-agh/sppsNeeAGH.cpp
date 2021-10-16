// sppsNantes.cpp�: d�finit le point d'entr�e pour l'application console.
//
#ifdef _DEBUG
	#define _CRTDBG_MAP_ALLOC
	#include <stdlib.h>
	#ifdef _WIN32
		#include <crtdbg.h>
	#endif
	#include <input_output/gabe/gabe.h>
	#define __USE_MULTITHREAD__ 0
#else
	#define __USE_MULTITHREAD__ 1
#endif

#ifdef _PROFILE_
	#undef __USE_MULTITHREAD__
	#define __USE_MULTITHREAD__ 0
#endif

#include <coreinitialisation.h>
#include "CalculationCoreAGH.h"
#include "tools/dotdistribution.h"
#include "sppsInitialisation.h"
#include "calculation_cores/NextEventEstimationCore.h"
#include "calculation_cores/MLTCore.h"

#if __USE_MULTITHREAD__
	#include <boost/thread/thread.hpp>
	#include <boost/bind.hpp>

	boost::mutex amutex; /*!< Variable permettant la synchronisation des processus */
#endif
	// Print warning if particle lost is superior than this ratio
const double PARTICLE_LOST_WARNING_RATIO = 0.05;
bool verbose_mode = false;

/**
 * @brief Structure contenant tout les outils vou�s au calcul de propagation
 */
struct t_ToolBox
{
	CalculationCoreSPPS* calculationTool;
	ReportManagerAGH* outputTool;
	Core_ConfigurationAGH* configurationTool;
	t_Mesh* sceneMesh;
	t_TetraMesh* tetraMesh;
	progressionInfo* mainProgressionOutput;
	CalculationCoreSPPS::CONF_CALCULATION_AGH confCalc;
};

/**
 * Execute le calcul pour une source donn�
 * @param applicationTools Panel d'objets permettant le calcul de propagation
 * @param sourceInfo Informations sur la source � utiliser pour l'�mission
 * @param confPartFrame Informations de bases pour toutes les particules
 */
void runSourceCalculation( progressOperation* parentOperation, t_ToolBox& applicationTools, t_Source& sourceInfo, CONF_PARTICULE_AGH& confPartFrame, unsigned int freq)
{
	if(!sourceInfo.currentVolume)
	{
		#if __USE_MULTITHREAD__
		boost::mutex::scoped_lock lock(amutex);
		#endif
		std::cerr<<"Unable to find the source position!";
		return;
	}
	uentier quandparticules=*applicationTools.configurationTool->FastGetConfigValue(Core_ConfigurationAGH::IPROP_QUANT_PARTICLE_CALCULATION);
	decimal rapportPartOutput=1;
	//Calcul du rapport particules � enregistrer sur le nombre de particules � calculer
	if(quandparticules>0)
		rapportPartOutput=(decimal)(*applicationTools.configurationTool->FastGetConfigValue(Core_ConfigurationAGH::IPROP_QUANT_PARTICLE_OUTPUT))/(decimal)quandparticules;
	if(!(rapportPartOutput>=0 && rapportPartOutput<=1))
		rapportPartOutput=0;
	confPartFrame.energie=sourceInfo.bandeFreqSource[confPartFrame.frequenceIndex].w_j/quandparticules;
	//Calcul de l'energie epsilon pour les particules
	confPartFrame.energie_epsilon=confPartFrame.energie*pow(10.,-(l_decimal)(*applicationTools.configurationTool->FastGetConfigValue(Core_ConfigurationAGH::FPROP_EPSILON_TRANSMISSION)));
	confPartFrame.currentTetra=sourceInfo.currentVolume;
	confPartFrame.sourceid=sourceInfo.idsource;
	decimal nomVecVitesse=applicationTools.configurationTool->GetNormVecPart(sourceInfo.Position,confPartFrame.currentTetra);
	decimal currentRapport=0;

	//Si la directivit� de la source est unidirectionnelle
	if(sourceInfo.type==SOURCE_TYPE_UNIDIRECTION)
		confPartFrame.direction=sourceInfo.Direction*nomVecVitesse;
	if (sourceInfo.type == SOURCE_TYPE_DIRECTION && sourceInfo.directivity->asDataForFrequency(freq) == false)
	{
		if (verbose_mode) { std::cout << "No directivity data for frequency: " << freq << " => skipping" << std::endl; }
		return;
	}

	int progresSteps = quandparticules;
	if (*applicationTools.configurationTool->FastGetConfigValue(Core_ConfigurationAGH::I_PROP_CALCULATION_CORE_SELLECTION) == MLT) progresSteps *= 2;

	progressOperation thisSrcOperation(parentOperation, progresSteps);
		
	//Prise en compte du d�lai de la source
	confPartFrame.pasCourant=(uentier_court)ceil(sourceInfo.sourceDelay/(*applicationTools.configurationTool->FastGetConfigValue(Core_ConfigurationAGH::FPROP_TIME_STEP)));
	confPartFrame.stateParticule=PARTICULE_STATE_ALIVE;
	if((*applicationTools.configurationTool->FastGetConfigValue(Core_ConfigurationAGH::IPROP_QUANT_TIMESTEP))>confPartFrame.pasCourant)
	{
		if(applicationTools.calculationTool->doDirectSoundCalculation)
			applicationTools.calculationTool->CalculateDirectSound(confPartFrame, sourceInfo, nomVecVitesse, freq);

		for(uentier idpart=1;idpart<=quandparticules;idpart++)
		{
			/*
			if(int(idpart/1000.)!=lastmill)
			{
				lastmill=int(idpart/1000.);
				std::cout<<"Solve "<<lastmill<<"xxx # particles."<<std::endl;
			}
			*/
			CONF_PARTICULE_AGH confPart=confPartFrame;
			if(sourceInfo.type==SOURCE_TYPE_OMNIDIRECTION)
				ParticleDistribution::GenSphereDistribution(confPart,nomVecVitesse);
			else if(sourceInfo.type==SOURCE_TYPE_XY)
				ParticleDistribution::GenXYDistribution(confPart,nomVecVitesse);
			else if(sourceInfo.type==SOURCE_TYPE_XZ)
				ParticleDistribution::GenXZDistribution(confPart,nomVecVitesse);
			else if(sourceInfo.type==SOURCE_TYPE_YZ)
				ParticleDistribution::GenYZDistribution(confPart,nomVecVitesse);
			else if (sourceInfo.type == SOURCE_TYPE_DIRECTION)
			{
				ParticleDistribution::GenSphereDistribution(confPart, nomVecVitesse);
				// at�nuation en fonction de la direction
				std::tuple<double, double> coord_sph = t_DirectivityBalloon::loudspeaker_coordinate(sourceInfo.Direction, confPart.direction);
				double phi = RadToDeg(std::get<0>(coord_sph));
				double theta = RadToDeg(std::get<1>(coord_sph));
				if (sourceInfo.directivity->asInterpolatedValue(freq, phi, theta))
				{
					double spl = sourceInfo.directivity->getInterpolatedValue(freq, phi, theta);
					confPart.energie *= pow(10, spl / 10);
				}
			}

			confPart.position=sourceInfo.Position;
			currentRapport+=rapportPartOutput;
			if(currentRapport>=1)
			{
				confPart.outputToParticleFile=true;
				currentRapport=0;
			}else{
				confPart.outputToParticleFile=false;
			}

			applicationTools.outputTool->NewParticule(confPart);
			applicationTools.calculationTool->Run(confPart);

			applicationTools.outputTool->SaveParticule();


			//Calcul des particules issues de confPart
			//std::cout<<"Solve "<<applicationTools.confCalc.duplicatedParticles.size()<<" duplicated particles."<<std::endl;
			while(!applicationTools.confCalc.duplicatedParticles.empty())
			{
				confPart=applicationTools.confCalc.duplicatedParticles.front();
				applicationTools.confCalc.duplicatedParticles.pop_front();
				applicationTools.outputTool->NewParticule(confPart);
				applicationTools.calculationTool->Run(confPart);
				applicationTools.outputTool->SaveParticule();
			}


			#if __USE_MULTITHREAD__
				boost::mutex::scoped_lock lock(amutex);
			#endif
			applicationTools.mainProgressionOutput->OutputCurrentProgression();
			//progressOperation thisPartOperation(&thisSrcOperation);
			thisSrcOperation.Next();
		}

		if(*applicationTools.configurationTool->FastGetConfigValue(Core_ConfigurationAGH::I_PROP_CALCULATION_CORE_SELLECTION)==MLT)
		{
			while (!static_cast<MLTCore*>(applicationTools.calculationTool)->SeedIsEmpty()) 
			{
				static_cast<MLTCore*>(applicationTools.calculationTool)->RunMutation();

				#if __USE_MULTITHREAD__
					boost::mutex::scoped_lock lock(amutex);
				#endif
				applicationTools.mainProgressionOutput->OutputCurrentProgression();
				thisSrcOperation.Next();
			}
		}
	}
}


/**
 * Execute le calcul pour une frequence donn�
 * @param applicationTools Panel d'objets permettant le calcul de propagation
 * @param freqInfo Informations sur la fr�quence � utiliser pour l'�mission
 * @param confPartFrame Informations de bases pour toutes les particules
 */
void runFrequenceCalculation(  progressOperation* parentOperation, ReportManagerAGH::t_ParamReport reportParameter, t_ToolBox applicationTools, t_sppsThreadParamAGH* threadData, CONF_PARTICULE_AGH confPartFrame)
{
	using namespace std;
	//Reserve l'espace m�moire pour cette bande de fr�quence
	InitRecepteurSBfreq(applicationTools.configurationTool->recepteur_s_List,threadData->freqInfos->freqIndex,*applicationTools.configurationTool->FastGetConfigValue(Core_ConfigurationAGH::IPROP_QUANT_TIMESTEP));
	//Initialisation du gestionnaire de sortie de donn�es
	reportParameter.freqIndex=threadData->freqInfos->freqIndex;
	reportParameter.freqValue=threadData->freqInfos->freqValue;
	if((applicationTools.configurationTool->recepteur_s_List.size()>0 || applicationTools.configurationTool->recepteur_scut_List.size()>0 ) && *applicationTools.configurationTool->FastGetConfigValue(Core_ConfigurationAGH::IPROP_OUTPUT_RECEPTEURS_SURF_BY_FREQ))
	{
		//Cr�ation du dossier pour le r�cepteur surfacique � cette fr�quence
		reportParameter._recepteur_surf_Path+=stringClass::FromInt(reportParameter.freqValue)+stringClass(" Hz/");
		reportParameter._recepteur_surf_cut_Path=reportParameter._recepteur_surf_Path;
		//Cr�ation du dossier de fr�quence
		st_mkdir(reportParameter._recepteur_surf_Path);
		//Ajout du nom du fichier � la fin
		reportParameter._recepteur_surf_Path+=*applicationTools.configurationTool->FastGetConfigValue(Core_ConfigurationAGH::SPROP_RECEPTEUR_SURFACIQUE_FILE_PATH);
		reportParameter._recepteur_surf_cut_Path+=*applicationTools.configurationTool->FastGetConfigValue(Core_ConfigurationAGH::SPROP_RECEPTEUR_SURFACIQUE_FILE_CUT_PATH);
	}

	ReportManagerAGH outputTool(reportParameter);
	applicationTools.outputTool=&outputTool;

	//sellect calculation code based on sellection
	CalculationCoreSPPS* calculationTool = nullptr;
	switch (*applicationTools.configurationTool->FastGetConfigValue(Core_ConfigurationAGH::I_PROP_CALCULATION_CORE_SELLECTION))
	{
	case CLASSIC_SPPS:
		calculationTool = new CalculationCoreSPPS(*applicationTools.sceneMesh, *applicationTools.tetraMesh, applicationTools.confCalc, *applicationTools.configurationTool, &outputTool);
		break;
	case NEXT_EVENT_ESTIMATION:
		calculationTool = new NextEventEstimationCore(*applicationTools.sceneMesh, *applicationTools.tetraMesh, applicationTools.confCalc, *applicationTools.configurationTool, &outputTool);
		break;
	case MLT:
		calculationTool = new MLTCore(*applicationTools.sceneMesh, *applicationTools.tetraMesh, applicationTools.confCalc, *applicationTools.configurationTool, &outputTool);
		break;
	default:
		calculationTool = new NextEventEstimationCore(*applicationTools.sceneMesh, *applicationTools.tetraMesh, applicationTools.confCalc, *applicationTools.configurationTool, &outputTool);
		break;
	}
	
	applicationTools.calculationTool=calculationTool;

	confPartFrame.frequenceIndex=threadData->freqInfos->freqIndex;
	//Pour chaque source
	progressOperation thisFreqOperation(parentOperation,applicationTools.configurationTool->srcList.size());

	for(std::size_t idsrc=0;idsrc<applicationTools.configurationTool->srcList.size();idsrc++)
	{
		runSourceCalculation(&thisFreqOperation,applicationTools,*applicationTools.configurationTool->srcList[idsrc],confPartFrame, threadData->freqInfos->freqValue);
	}
	//A partir d'ici les threads s'arretent puis continuent un par un.
	//Premi�re ligne de code du processus en cours
	#if __USE_MULTITHREAD__
		if(true) //Cette branche conditionnelle existe afin que l'objet lock soit d�truit � la fin du traitement de la condition
		{
			boost::mutex::scoped_lock lock(mutex);
	#endif
		//Post traitement des r�cepteurs de surface
		ReportManagerAGH::SetPostProcessSurfaceReceiver(*applicationTools.configurationTool,threadData->freqInfos->freqIndex,applicationTools.configurationTool->recepteur_s_List,*applicationTools.configurationTool->FastGetConfigValue(Core_ConfigurationAGH::FPROP_TIME_STEP));
		ReportManagerAGH::SetPostProcessCutSurfaceReceiver(*applicationTools.configurationTool,threadData->freqInfos->freqIndex,applicationTools.configurationTool->recepteur_scut_List,*applicationTools.configurationTool->FastGetConfigValue(Core_ConfigurationAGH::FPROP_TIME_STEP));
		//Sauvegarde des r�cepteurs surfacique pour cette bande de fr�quence
		if(applicationTools.configurationTool->recepteur_s_List.size()>0 &&  *applicationTools.configurationTool->FastGetConfigValue(Core_ConfigurationAGH::IPROP_OUTPUT_RECEPTEURS_SURF_BY_FREQ))
#ifdef UTILISER_MAILLAGE_OPTIMISATION
			ReportManagerAGH::SauveRecepteursSurfaciques(reportParameter._recepteur_surf_Path,threadData->freqInfos->freqIndex,applicationTools.configurationTool->recepteur_s_List,*applicationTools.tetraMesh,*applicationTools.configurationTool->FastGetConfigValue(Core_ConfigurationAGH::FPROP_TIME_STEP));
#else
			ReportManagerAGH::SauveRecepteursSurfaciques(reportParameter._recepteur_surf_Path,threadData->freqInfos->freqIndex,applicationTools.configurationTool->recepteur_s_List,*applicationTools.sceneMesh,*applicationTools.configurationTool->FastGetConfigValue(Core_ConfigurationAGH::FPROP_TIME_STEP));
#endif
		if(applicationTools.configurationTool->recepteur_scut_List.size()>0 &&  *applicationTools.configurationTool->FastGetConfigValue(Core_ConfigurationAGH::IPROP_OUTPUT_RECEPTEURS_SURF_BY_FREQ))
			ReportManagerAGH::SauveRecepteursSurfaciquesCoupe(reportParameter._recepteur_surf_cut_Path,applicationTools.configurationTool->recepteur_scut_List,*applicationTools.configurationTool->FastGetConfigValue(Core_ConfigurationAGH::FPROP_TIME_STEP),true,false,threadData->freqInfos->freqIndex);
		outputTool.SaveAndCloseParticleFile();				//Finalisation du fichier de particule
		threadData->GabeColData=outputTool.GetColStats();	//Recupere les donn�es des etats de particules
		threadData->GabeSumEnergyFreq=outputTool.GetSumEnergy();//Recupere les donn�es du niveau sonore global
		outputTool.FillWithLefData(*threadData); //Recupere les donn�es du lef (utilis� pour le calcul du LF et LFC)
		outputTool.GetAngleStats(*threadData, *applicationTools.configurationTool->FastGetConfigValue(Core_ConfigurationAGH::IPROP_NORMALIZE_ANGLE_STATS));
		cout<<"End of calculation at "<<threadData->freqInfos->freqValue<<" Hz."<<endl;

	#if __USE_MULTITHREAD__
		}
	#endif
		delete calculationTool;
}



int MainProcess(int argc, char* argv[])
{

	using namespace std;

	cout<<SPPSNEE_AGH_VERSION<<endl;
	//**************************************************
	//Initialisation
	t_ToolBox applicationToolBox;
	applicationToolBox.mainProgressionOutput=NULL;
	formatCoreBIN::ioModel modelEntree;
	t_Mesh sceneMesh;
	t_TetraMesh sceneTetraMesh;
	applicationToolBox.sceneMesh=&sceneMesh;
	applicationToolBox.tetraMesh=&sceneTetraMesh;

	//**************************************************
	//Verification des arguments
	string pathFichier;
	bool asParam = false;
	bool asFilePath = false;

	if(argc>1)
	{
		for(int idarg=1; idarg<argc; idarg++)
		{
			string arg = argv[idarg];

			if (arg == "-v" && !asFilePath){ // params need to be before the filepath
				verbose_mode = true;
				asParam = true;
			}
			else if (pathFichier.empty()) {
				pathFichier.append(arg);
				asFilePath = true;
			}
			else {
				pathFichier.append(" ");
				pathFichier.append(arg);
			}
		}
	}

	if (!asFilePath)
	{
		cout << "The path of the XML configuration file must be specified!" << endl;
		return 1;
	}

	//**************************************************
	// 1: Lire le fichier XML
	if (verbose_mode) { cout << "XML configuration file is currently loading..." << endl; }
	Core_ConfigurationAGH configManager(pathFichier, verbose_mode);
	applicationToolBox.configurationTool=&configManager;
	if (verbose_mode) { cout << "XML configuration file has been loaded." << endl; }

	//**************************************************
	// 2: Initialisation des variables
	CoreString workingDir=*configManager.FastGetConfigValue(Core_ConfigurationAGH::SPROP_CORE_WORKING_DIRECTORY);
	CoreString sceneMeshPath=*configManager.FastGetConfigValue(Core_ConfigurationAGH::SPROP_MODEL_FILE_PATH);
	sceneMeshPath=workingDir+sceneMeshPath;
	// Chargement du Random SEED
	entier seedValue = *configManager.FastGetConfigValue(Core_ConfigurationAGH::I_PROP_RANDOM_SEED);
	if(seedValue!=0) {
		SetRandSeed((uint32_t)seedValue);
	}
	else {
		SetRandSeed(static_cast<unsigned int>(std::time(0)));
	}

	//**************************************************
	// 3: Chargement du mod�le
	if(!initMesh(sceneMesh,workingDir,sceneMeshPath,configManager, verbose_mode))
		return 1;

	//**************************************************
	// 4: Chargement du maillage
	if(!initTetraMesh(workingDir+*configManager.FastGetConfigValue(Core_ConfigurationAGH::SPROP_TETRAHEDRALIZATION_FILE_PATH),sceneMesh,configManager.freqList.size(),sceneTetraMesh,configManager, verbose_mode))
		return 1;

	// Attach the neighboring tetrahedron with all punctual receivers depending of punctual receiver radius.
	ExpandPunctualReceiverTetrahedronLocalisation(&sceneTetraMesh, &configManager.recepteur_p_List, configManager);
	TranslateSourceAtTetrahedronVertex(configManager.srcList, &sceneTetraMesh);
	if (!CheckSourcePosition(configManager.srcList, &sceneMesh)) {
		std::cerr << _("A sound source position is intersecting with the 3D model, move the sound source inside the room") << std::endl;
		return 1;
	}
	//**************************************************
	// 5: Instancier param�tre gestionnaire de sortie de donn�es
	ReportManagerAGH::t_ParamReport reportParameter;
	reportParameter._particleFileName=*configManager.FastGetConfigValue(Core_ConfigurationAGH::SPROP_PARTICULE_FILE_PATH);
	reportParameter._particlePath=workingDir+*configManager.FastGetConfigValue(Core_ConfigurationAGH::SPROP_PARTICULE_FOLDER_PATH);
	reportParameter._recepteur_surf_Path=workingDir+*configManager.FastGetConfigValue(Core_ConfigurationAGH::SPROP_RECEPTEUR_SURFACIQUE_FOLDER_PATH);
	reportParameter.nbParticles=*configManager.FastGetConfigValue(Core_ConfigurationAGH::IPROP_QUANT_PARTICLE_OUTPUT)*configManager.srcList.size();
	reportParameter.nbTimeStep=*configManager.FastGetConfigValue(Core_ConfigurationAGH::IPROP_QUANT_TIMESTEP); //(int)((*configManager.FastGetConfigValue(Core_ConfigurationAGH::FPROP_SIMULATION_TIME))/(*configManager.FastGetConfigValue(Core_ConfigurationAGH::FPROP_TIME_STEP)));
	reportParameter.timeStep=*configManager.FastGetConfigValue(Core_ConfigurationAGH::FPROP_TIME_STEP);
	reportParameter.tetraModel=&sceneTetraMesh;
	reportParameter.configManager=&configManager;
	reportParameter.sceneModel=&sceneMesh;
	reportParameter.working_Path=workingDir;

	//Cr�ation du dossier contenant les recepteurs surfaciques
	if(configManager.recepteur_s_List.size()>0 || configManager.recepteur_scut_List.size()>0 )
		st_mkdir(reportParameter._recepteur_surf_Path);


	//**************************************************
	// 6: Instancer les threads

	#if __USE_MULTITHREAD__
		boost::thread_group threads;
	#endif

	//**************************************************
	// 7: Executer les threads de calculs

	CalculationCoreSPPS::CONF_CALCULATION_AGH confCalc;
	confCalc.nbPasTemps=reportParameter.nbTimeStep;
	confCalc.pasTemps=(*configManager.FastGetConfigValue(Core_ConfigurationAGH::FPROP_TIME_STEP));
	applicationToolBox.calculationTool=NULL;
	applicationToolBox.confCalc=confCalc;

	CONF_PARTICULE_AGH particuleFrame;

	progressionInfo progOutputManager(*configManager.FastGetConfigValue(Core_ConfigurationAGH::IPROP_QUANT_BFREQ_TO_CALCULATE)); //gestionnaire d'affichage de progression
	applicationToolBox.mainProgressionOutput=&progOutputManager;

	std::vector<t_sppsThreadParamAGH> threadsData(configManager.freqList.size());


	for(std::size_t idfreq=0;idfreq<configManager.freqList.size();idfreq++)
	{
		if(configManager.freqList[idfreq]->doCalculation)
		{
			threadsData.at(idfreq).freqInfos=configManager.freqList[idfreq];

			#if __USE_MULTITHREAD__
			if(*configManager.FastGetConfigValue(Core_ConfigurationAGH::IPROP_DO_MULTITHREAD))
				threads.add_thread(new boost::thread(boost::bind(&runFrequenceCalculation,progOutputManager.GetMainOperation(),reportParameter,applicationToolBox,&threadsData.at(idfreq),particuleFrame)));
			else
			#endif
			runFrequenceCalculation(progOutputManager.GetMainOperation(),reportParameter,applicationToolBox,&threadsData.at(idfreq),particuleFrame);
		}
	}
	#if __USE_MULTITHREAD__
		uentier workingThreads=threads.size();
		if(*configManager.FastGetConfigValue(Core_ConfigurationAGH::IPROP_DO_MULTITHREAD))
			threads.join_all();
	#endif

	cout<<"End of calculation."<<endl;

	//**************************************************
	// 8: Une fois tout les threads de calculs ferm�s on compile les fichiers de resultats
	reportCompilation(configManager,workingDir);

	//cast extended results to oryginal class
	std::vector<t_sppsThreadParam> threadDataCast;
	for (t_sppsThreadParamAGH& element : threadsData)
	{
		threadDataCast.push_back(element);
	}

	ReportManagerAGH::SaveThreadsStats(workingDir+*configManager.FastGetConfigValue(Core_ConfigurationAGH::SPROP_STATS_FILE_PATH),workingDir+*configManager.FastGetConfigValue(Core_ConfigurationAGH::SPROP_CUMUL_FILE_PATH),threadsData,reportParameter);

	if (verbose_mode) { cout << "Saving Angle of incidence statistics..." << endl; }
	ReportManagerAGH::SaveAngleStats(workingDir + *configManager.FastGetConfigValue(Core_ConfigurationAGH::SPROP_ANGLE_FILE_PATH), workingDir + *configManager.FastGetConfigValue(Core_Configuration::SPROP_CUMUL_FILE_PATH), threadsData, reportParameter, *configManager.FastGetConfigValue(Core_ConfigurationAGH::IPROP_EXTENDED_ANGLE_STATS));
	if (verbose_mode) { cout << "End of save of Angle of incidence statistics." << endl; }

	if (verbose_mode) { cout << "Saving Ponctual Receiver Advanced Parameters..." << endl; }
	ReportManagerAGH::SaveRecpAcousticParamsAdvance(*configManager.FastGetConfigValue(Core_ConfigurationAGH::SPROP_ADV_PONCTUAL_RECEIVER_FILE_PATH), threadDataCast,reportParameter);
	if (verbose_mode) { cout << "End of save of Ponctual Receiver Advanced Parameters." << endl; }

	if (verbose_mode) { cout << "Saving Ponctual Receiver Intensity..." << endl; }
	ReportManagerAGH::SaveRecpIntensity("Punctual receiver intensity.gabe", threadDataCast,reportParameter);
	if (verbose_mode) { cout << "End of save of Ponctual Receiver intensity." << endl; }

	if (verbose_mode) { cout << "Saving sound level for each Ponctual Receiver per source..." << endl; }
	ReportManagerAGH::SaveSoundLevelBySource("Sound level per source.recps", threadDataCast,reportParameter);
	if (verbose_mode) { cout << "End of save sound level for each Ponctual Receiver per source." << endl; }
	st_mkdir(workingDir + *configManager.FastGetConfigValue(Core_Configuration::SPROP_RECEPTEUR_SURFACIQUE_FOLDER_PATH));
	stringClass globalRecSurfPath = workingDir + *configManager.FastGetConfigValue(Core_Configuration::SPROP_RECEPTEUR_SURFACIQUE_FOLDER_PATH) + "Global" + st_path_separator();
	//Cr�ation du dossier Global
	st_mkdir(globalRecSurfPath);
	stringClass globalSurfCutPath=globalRecSurfPath+*configManager.FastGetConfigValue(Core_ConfigurationAGH::SPROP_RECEPTEUR_SURFACIQUE_FILE_CUT_PATH);
	globalRecSurfPath+=*configManager.FastGetConfigValue(Core_ConfigurationAGH::SPROP_RECEPTEUR_SURFACIQUE_FILE_PATH);
	if (verbose_mode) { cout << "Saving Global Surface Receiver Data..." << endl; }
	#ifndef _PROFILE_
		#ifdef UTILISER_MAILLAGE_OPTIMISATION
			ReportManagerAGH::SauveGlobalRecepteursSurfaciques(globalRecSurfPath,configManager.recepteur_s_List,sceneTetraMesh,*configManager.FastGetConfigValue(Core_ConfigurationAGH::FPROP_TIME_STEP));
		#else
			ReportManagerAGH::SauveGlobalRecepteursSurfaciques(globalRecSurfPath,configManager.recepteur_s_List,sceneMesh,*configManager.FastGetConfigValue(Core_ConfigurationAGH::FPROP_TIME_STEP));
		#endif
		ReportManagerAGH::SauveRecepteursSurfaciquesCoupe(globalSurfCutPath,configManager.recepteur_scut_List,*configManager.FastGetConfigValue(Core_ConfigurationAGH::FPROP_TIME_STEP));
	#endif
		if (verbose_mode) { cout << "End of save Global Surface Receiver Data." << endl; }

	// Check lost particle in error and show a warning if there is too many lost particles
	long totalLost = 0l;
	long total = 0l;
	for (std::size_t idfreq = 0; idfreq<configManager.freqList.size(); idfreq++)
	{
		if (configManager.freqList[idfreq]->doCalculation)
		{
			const t_sppsThreadParamAGH& tdata = threadsData.at(idfreq);
			totalLost += tdata.particleStats.partLost + tdata.particleStats.partLoop;
			total += tdata.particleStats.partTotal;
		}
	}
	if ((double)totalLost / (double)total > PARTICLE_LOST_WARNING_RATIO) {
		fprintf(stderr, _("Warning %il particles has been in error on %il particles. The computation result may be wrong, please check the particles statitics file for more details."), totalLost, total);
	}
	//**************************************************
	// 9: Lib�re l'espace m�moire
	for(std::size_t idfreq=0;idfreq<threadsData.size();idfreq++)
		threadsData.at(idfreq).clearMem();


	return 0;
}

int main(int argc, char* argv[])
{

	MainProcess(argc,argv);

	#ifdef _WIN32
	#ifdef _DEBUG
		_CrtDumpMemoryLeaks(); //Affiche les fuites m�moires
	#endif
	#endif

	return 0;
}

