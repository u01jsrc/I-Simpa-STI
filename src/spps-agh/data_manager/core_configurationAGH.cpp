#include "Core_ConfigurationAGH.h"
#include <iostream>
#include "input_output/brdfs/brdfParser.h"


Core_ConfigurationAGH::Core_ConfigurationAGH( CoreString xmlFilePath, bool verbose_mode)
	:Core_Configuration(xmlFilePath, verbose_mode)
{
	CXml fichierXml(xmlFilePath);

	CXmlNode* root=fichierXml.GetRoot();
	if(root)
	{
		CXmlNode* simuNode=root->GetChild("simulation");
		if(simuNode)
		{
			int calcluationCore = simuNode->GetProperty("calculation_core").ToInt();
			SetConfigInformation(I_PROP_CALCULATION_CORE_SELLECTION, simuNode->GetProperty("calculation_core").ToInt());

			switch (calcluationCore) {
			case CLASSIC_SPPS:
				LoadAdvancedSPPS(simuNode);
				break;
			case NEXT_EVENT_ESTIMATION:
				LoadAdvancedNEE(simuNode);
				break;
			case MLT:
				LoadAdvancedMLT(simuNode);
				break;
			}
		}


		CXmlNode* surfacesNode = root->GetChild("surface_absorption_enum");
		if (surfacesNode)
		{
			//Pour chaque matériaux
			std::vector<CXmlNode*>::iterator iterateurNoeuds;

			for (iterateurNoeuds = surfacesNode->GetFirstChild(); iterateurNoeuds != surfacesNode->GetLastChild(); iterateurNoeuds++)
			{			
				if ((*iterateurNoeuds)->IsPropertyExist("custom_BRDF") && (*iterateurNoeuds)->GetProperty("custom_BRDF").ToInt())
				{
					int matID = (*iterateurNoeuds)->GetProperty("id").ToInt();

					for (t_Material* material : this->materialList)
					{
						if (material->outsideMaterialIndice == matID)
						{
							uentier BRDF_sampling = (*iterateurNoeuds)->GetProperty("custom_BRDF_sampling_method").ToInt();
							material->custom_BRDF_sampling_method = BRDF_sampling;

							string brdfFile_path = *FastGetConfigValue(SPROP_CORE_WORKING_DIRECTORY);
							brdfFile_path += *FastGetConfigValue(SPROP_BRDF_FOLDER_PATH);
							brdfFile_path += (*iterateurNoeuds)->GetProperty("brdf_file");
							
							material->use_custom_BRDF = true;
							t_BrdfBalloon* material_brdf = new t_BrdfBalloon();
							txt_BrdfParser parser;
							parser.parse(brdfFile_path, material_brdf);
							material->customBrdf = material_brdf;

							vector<float> avalibleFreq = material_brdf->getAvalibleFrequencies();
							for (auto& freq : freqList) {
								if(freq->doCalculation && find(avalibleFreq.begin(), avalibleFreq.end(), freq->freqValue) == avalibleFreq.end())
								{
									freq->doCalculation = false;
									cout << "BRDF does not contain information for " << freq->freqValue << " Hz. This frequency is disabled!" << endl;
								}
							}

							break;
						}
					}
				}
			}
		}

	}
}


Core_ConfigurationAGH::~Core_ConfigurationAGH( )
{
}

void Core_ConfigurationAGH::SetConfigInformation(FPROP propertyIndex, decimal valeur)
{
	Base_Core_Configuration::SetConfigInformation((BASE_FPROP)propertyIndex, valeur);
}
void Core_ConfigurationAGH::SetConfigInformation(SPROP propertyIndex, CoreString valeur)
{
	Base_Core_Configuration::SetConfigInformation((BASE_SPROP)propertyIndex, valeur);
}
void Core_ConfigurationAGH::SetConfigInformation(IPROP propertyIndex, entier valeur)
{
	Base_Core_Configuration::SetConfigInformation((BASE_IPROP)propertyIndex, valeur);
}

void Core_ConfigurationAGH::SetConfigInformation(BASE_FPROP propertyIndex, decimal valeur)
{
	Base_Core_Configuration::SetConfigInformation(propertyIndex, valeur);
}
void Core_ConfigurationAGH::SetConfigInformation(BASE_SPROP propertyIndex, CoreString valeur)
{
	Base_Core_Configuration::SetConfigInformation(propertyIndex, valeur);
}
void Core_ConfigurationAGH::SetConfigInformation(BASE_IPROP propertyIndex, entier valeur)
{
	Base_Core_Configuration::SetConfigInformation(propertyIndex, valeur);
}

void Core_ConfigurationAGH::SetConfigInformation(NEW_FPROP propertyIndex, decimal valeur)
{
	Base_Core_Configuration::SetConfigInformation((BASE_FPROP)propertyIndex, valeur);
}
void Core_ConfigurationAGH::SetConfigInformation(NEW_SPROP propertyIndex, CoreString valeur)
{
	Base_Core_Configuration::SetConfigInformation((BASE_SPROP)propertyIndex, valeur);
}
void Core_ConfigurationAGH::SetConfigInformation(NEW_IPROP propertyIndex, entier valeur)
{
	Base_Core_Configuration::SetConfigInformation((BASE_IPROP)propertyIndex, valeur);
}

void Core_ConfigurationAGH::LoadAdvancedSPPS(CXmlNode* simuNode) {
	CXmlNode* advancedNode = simuNode->GetChild("advancedSPPS");
	if (advancedNode)
	{
		uentier_long seed = 0;
		if (advancedNode->IsPropertyExist("random_seed")) {
			seed = advancedNode->GetProperty("random_seed").ToInt();
		}
		if (seed != 0) {
			// User define a random seed, multi-thread have to be deactivated in order to do the same computation
			SetConfigInformation(IPROP_DO_MULTITHREAD, 0);
			std::cout << "Random seed has been set; then multi-thread has been desactivated." << std::endl;
		}
		SetConfigInformation(I_PROP_RANDOM_SEED, seed);
		SetConfigInformation(IPROP_NORMALIZE_ANGLE_STATS, advancedNode->GetProperty("normalize_angle_stats").ToInt());
		SetConfigInformation(IPROP_EXTENDED_ANGLE_STATS, advancedNode->GetProperty("extended_angle_stats").ToInt());
		SetConfigInformation(IPROP_ANGLE_STATS_MIN_REFL, advancedNode->GetProperty("angle_stats_min_reflection").ToInt());;
		SetConfigInformation(SPROP_ANGLE_FILE_PATH, advancedNode->GetProperty("angle_filename"));
	}
}

void Core_ConfigurationAGH::LoadAdvancedNEE(CXmlNode* simuNode) {
	CXmlNode* advancedNode = simuNode->GetChild("advancedNEE");
	if (advancedNode)
	{
		uentier_long seed = 0;
		if (advancedNode->IsPropertyExist("random_seed")) {
			seed = advancedNode->GetProperty("random_seed").ToInt();
		}
		if (seed != 0) {
			// User define a random seed, multi-thread have to be deactivated in order to do the same computation
			SetConfigInformation(IPROP_DO_MULTITHREAD, 0);
			std::cout << "Random seed has been set; then multi-thread has been desactivated." << std::endl;
		}
		SetConfigInformation(I_PROP_RANDOM_SEED, seed);
		SetConfigInformation(IPROP_NORMALIZE_ANGLE_STATS, advancedNode->GetProperty("normalize_angle_stats").ToInt());
		SetConfigInformation(IPROP_EXTENDED_ANGLE_STATS, advancedNode->GetProperty("extended_angle_stats").ToInt());
		SetConfigInformation(IPROP_ANGLE_STATS_MIN_REFL, advancedNode->GetProperty("angle_stats_min_reflection").ToInt());
		SetConfigInformation(SPROP_ANGLE_FILE_PATH, advancedNode->GetProperty("angle_filename"));
		SetConfigInformation(IPROP_CAST_SR_TO_SURFACE_REC, advancedNode->GetProperty("SR_to_surface_reciver").ToInt());
		SetConfigInformation(FPROP_NEE_SHADOWRAY_PROB, advancedNode->GetProperty("shadowray_prob").ToFloat());
	}
}

void Core_ConfigurationAGH::LoadAdvancedMLT(CXmlNode* simuNode) {
	CXmlNode* advancedNode = simuNode->GetChild("advancedMLT");
	if (advancedNode)
	{
		uentier_long seed = 0;
		if (advancedNode->IsPropertyExist("random_seed")) {
			seed = advancedNode->GetProperty("random_seed").ToInt();
		}
		if (seed != 0) {
			// User define a random seed, multi-thread have to be deactivated in order to do the same computation
			SetConfigInformation(IPROP_DO_MULTITHREAD, 0);
			std::cout << "Random seed has been set; then multi-thread has been desactivated." << std::endl;
		}
		SetConfigInformation(I_PROP_RANDOM_SEED, seed);
		SetConfigInformation(FPROP_MLT_SPECULAR_REFL_PROB, advancedNode->GetProperty("specularReflProbabilityMulti").ToFloat());
		SetConfigInformation(FPROP_MLT_MUTATION_NUMBER, advancedNode->GetProperty("mutationNumber").ToInt());
		SetConfigInformation(FPROP_MLT_LARGE_STEP_PROB, advancedNode->GetProperty("largeStepProb").ToFloat());
	}
}


