/* ----------------------------------------------------------------------
* I-SIMPA (http://i-simpa.ifsttar.fr). This file is part of I-SIMPA.
*
* I-SIMPA is a GUI for 3D numerical sound propagation modelling dedicated
* to scientific acoustic simulations.
* Copyright (C) 2007-2014 - IFSTTAR - Judicael Picaut, Nicolas Fortin
*
* I-SIMPA is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3 of the License, or
* (at your option) any later version.
* 
* I-SIMPA is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software Foundation,
* Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA or 
* see <http://ww.gnu.org/licenses/>
*
* For more information, please consult: <http://i-simpa.ifsttar.fr> or 
* send an email to i-simpa@ifsttar.fr
*
* To contact Ifsttar, write to Ifsttar, 14-20 Boulevard Newton
* Cite Descartes, Champs sur Marne F-77447 Marne la Vallee Cedex 2 FRANCE
* or write to scientific.computing@ifsttar.fr
* ----------------------------------------------------------------------*/

#include "e_scene_bdd_materiaux_propmateriau_adv.h"
#include "data_manager/appconfig.h"
#include <wx/filename.h>
#include "data_manager/e_data_file.h"
#include "last_cpp_include.hpp"

E_Scene_Bdd_Materiaux_PropertyMaterial_Adv::E_Scene_Bdd_Materiaux_PropertyMaterial_Adv( wxXmlNode* noeudCourant ,  Element* parent)
:Element(parent,"Advanced properties",Element::ELEMENT_TYPE_SCENE_BDD_MATERIAUX_PROPMATERIAU_ADVANCED,noeudCourant)
{
	_("Advanced properties");
	SetIcon(GRAPH_STATE_ALL, GRAPH_EL_CONFIGURATION);

	wxXmlNode* currentChild;
	wxString propValue;
	wxFileName storageFolder("");

	currentChild = noeudCourant->GetChildren();
	storageFolder.Assign(ApplicationConfiguration::GLOBAL_VAR.cacheFolderPath + ApplicationConfiguration::CONST_REPORT_BRDF_FOLDER_PATH + wxFileName::GetPathSeparator());
	while (currentChild != NULL)
	{
		if (currentChild->GetAttribute("eid", &propValue))
		{
			long typeEle;
			propValue.ToLong(&typeEle);
			if (typeEle == Element::ELEMENT_TYPE_FILE)
			{
				E_Data_File* dirFile = new E_Data_File(currentChild, this, storageFolder.GetPath(), _("Open BRDF file"), _("TXT files (*.TXT)|*.TXT"));
				this->AppendFils(dirFile);
			}
		}
		currentChild = currentChild->GetNext();
	}

	E_Scene_Bdd_Materiaux_PropertyMaterial_Adv* confCore = dynamic_cast<E_Scene_Bdd_Materiaux_PropertyMaterial_Adv*>(this->GetElementByType(ELEMENT_TYPE_SCENE_BDD_MATERIAUX_PROPMATERIAU_ADVANCED));
	if (!confCore->IsPropertyExist("custom_BRDF_sampling_method"))
		this->AddCustomBRDFSamplingMehtods();
	if (!confCore->IsPropertyExist("custom_BRDF_exponent"))
		this->AddCustomBRDFExpnentFactor();
}

E_Scene_Bdd_Materiaux_PropertyMaterial_Adv::E_Scene_Bdd_Materiaux_PropertyMaterial_Adv( Element* parent)
:Element(parent,"Advanced properties",Element::ELEMENT_TYPE_SCENE_BDD_MATERIAUX_PROPMATERIAU_ADVANCED)
{
	_("Advanced properties");
	this->AddCustomBRDF();
	this->AddCustomBRDFSamplingMehtods();
	this->AddCustomBRDFExpnentFactor();
}


Element* E_Scene_Bdd_Materiaux_PropertyMaterial_Adv::AppendFilsByType(ELEMENT_TYPE etypefils,const wxString& libelle)
{
	if(etypefils==Element::ELEMENT_TYPE_SCENE_BDD_MATERIAUX_PROPMATERIAU_ADVANCED)
	{
		return this->AppendFils(new E_Scene_Bdd_Materiaux_PropertyMaterial_Adv(this));
	}
	return Element::AppendFilsByType(etypefils,libelle);
}

wxXmlNode* E_Scene_Bdd_Materiaux_PropertyMaterial_Adv::SaveXMLCoreDoc(wxXmlNode* NoeudParent)
{
	bool containsCustomBRDF;
	Element* materialAdvProperty = this->GetElementByType(Element::ELEMENT_TYPE_SCENE_BDD_MATERIAUX_PROPMATERIAU_ADVANCED);
	if (materialAdvProperty) {
		containsCustomBRDF = materialAdvProperty->GetBoolConfig("custom_BRDF");
	}
	if (containsCustomBRDF) {
		wxFileName brdfFile = materialAdvProperty->GetFileConfig("brdf_file");
		wxFileName storageFolder(ApplicationConfiguration::GLOBAL_VAR.workingFolderPath);
		//wxFileName brdfDir = ApplicationConfiguration::CONST_REPORT_BRDF_FOLDER_PATH;
		storageFolder.AppendDir("BRDFs");
		if (!storageFolder.DirExists())
		{
			storageFolder.Mkdir();
		}
		storageFolder.SetFullName(brdfFile.GetFullName());
		wxCopyFile(brdfFile.GetFullPath(), storageFolder.GetFullPath());

	}

	Element::SaveXMLCoreDoc(NoeudParent);
	return NoeudParent;
}

wxXmlNode* E_Scene_Bdd_Materiaux_PropertyMaterial_Adv::SaveXMLDoc(wxXmlNode* NoeudParent)
{
	wxXmlNode* thisNode = Element::SaveXMLDoc(NoeudParent);
	thisNode->SetName("advanced_property"); // Nom de la balise xml ( pas d'espace autorise )

	return thisNode;
}
void E_Scene_Bdd_Materiaux_PropertyMaterial_Adv::OnRightClic(wxMenu* leMenu)
{
	//leMenu->Append(GetMenuItem(leMenu,IDEVENT_NEW_VOLUME, _("Create a volume"),"./Bitmaps/popup_new.png"));
	//leMenu->Append(GetMenuItem(leMenu,IDEVENT_BUILD_VOLUMES_FROM_TRIMESH, _("Volume auto-detect"),"./Bitmaps/popup_new.png"));
	//Element::OnRightClic(leMenu);
}

void E_Scene_Bdd_Materiaux_PropertyMaterial_Adv::AddCustomBRDF()
{
	wxFileName storageFolder(ApplicationConfiguration::GLOBAL_VAR.cacheFolderPath);
	storageFolder.AppendDir("BRDFs");
	if (!storageFolder.DirExists())
	{
		storageFolder.Mkdir();
	}

	this->AppendPropertyBool("custom_BRDF", "Use custom BRDF", false, true);
	this->AppendPropertyFile("brdf_file", wxTRANSLATE("BRDF file"), storageFolder.GetPath(), _("Open BRDF file"), _("TXT files (*.TXT)|*.TXT"), true);
	_("Use custom BRDF");
}

void  E_Scene_Bdd_Materiaux_PropertyMaterial_Adv::AddCustomBRDFSamplingMehtods() {
	std::vector<wxString> samplingMethods;
	std::vector<int> samplingMethodsIndex;
	samplingMethods.push_back("Uniform");
	samplingMethods.push_back("Lambert");
	samplingMethods.push_back("PDF");
	this->AppendPropertyList("custom_BRDF_sampling_method", wxTRANSLATE("Custom BRDF sampling"), samplingMethods, 2, false, 1, samplingMethodsIndex, true);
}

void  E_Scene_Bdd_Materiaux_PropertyMaterial_Adv::AddCustomBRDFExpnentFactor() {
	this->AppendPropertyDecimal("custom_BRDF_exponent", wxTRANSLATE("Custom BRDF exponent"), 1., false, 3, false, true, 10, 0, true);
}
