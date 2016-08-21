#include "core_configuration_AGH.h"
#include <iostream>


Core_Configuration_AGH::Core_Configuration_AGH( CoreString xmlFilePath )
	:Core_Configuration(xmlFilePath, false)
{
	CXml fichierXml(xmlFilePath);
	//LoadCfgFile( fichierXml );

	#ifdef _PROFILE_
		SetConfigInformation(SPROP_CORE_WORKING_DIRECTORY,"tmp\\");
	#endif
	#ifdef _DEBUG
		//SetConfigInformation(SPROP_CORE_WORKING_DIRECTORY,"tmp\\");
	#endif
	CXmlNode* root=fichierXml.GetRoot();
	if(root)
	{

		CXmlNode* simuNode=root->GetChild("simulation");
		if(simuNode)
		{
			SetConfigInformation(I_PROP_CALCULATION_CORE_SELLECTION, simuNode->GetProperty("calculation_core").ToInt());

			CXmlNode* advancedNode = simuNode->GetChild("advanced");
			if (advancedNode)
			{
				uentier_long seed = 0;
				if (advancedNode->IsPropertyExist("random_seed")) {
					seed = advancedNode->GetProperty("random_seed").ToInt();
				}
				if (seed != 0) {
					// User define a random seed, multi-thread have to be deactivated in order to do the same computation
					Core_Configuration::SetConfigInformation(IPROP_DO_MULTITHREAD, 0);
					std::cout << "Random seed has been set; then multi-thread has been desactivated." << std::endl;
				}
				Core_Configuration::SetConfigInformation(I_PROP_RANDOM_SEED, seed);
			}
		}
	}
}

Core_Configuration_AGH::~Core_Configuration_AGH( )
{
}

void Core_Configuration_AGH::SetConfigInformation(NEW_IPROP propertyIndex, entier valeur)
{
	Base_Core_Configuration::SetConfigInformation((BASE_IPROP)propertyIndex, valeur);
}
