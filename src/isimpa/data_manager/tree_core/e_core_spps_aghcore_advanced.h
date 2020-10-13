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

#include "first_header_include.hpp"

#include "data_manager/element.h"

#ifndef __E_CORE_SPPS_NEE_AGH_ADV__
#define __E_CORE_SPPS_NEE_AGH_ADV__

/*! \file e_core_core_tetconf.h
\brief Element permettant de paramétrer le moteur de maillage de la scène
*/

/**
\brief Element permettant de paramétrer le moteur de maillage de la scène
*/
class E_Core_SppsNee_AGH_advanced_SPPS : public Element
{
public:
	E_Core_SppsNee_AGH_advanced_SPPS(wxXmlNode* noeudCourant, Element* parent)
		:Element(parent, wxTRANSLATE("Advanced - SPPS"), Element::ELEMENT_TYPE_CORE_SPPSAGH_ADVANCED_SPPS, noeudCourant)
	{
		SetIcon(GRAPH_STATE_ALL, GRAPH_EL_CONFIGURATION);
		//Add debug mode parameter v<1.21
		if (!this->IsPropertyExist("random_seed")) {
			InitRandomSeed(this);
		}
		if (!this->IsPropertyExist("angle_filename")) {
			InitAngleStatsCalc(this);
		}
	}

	E_Core_SppsNee_AGH_advanced_SPPS(Element* parent)
		:Element(parent, wxTRANSLATE("Advanced - SPPS"), Element::ELEMENT_TYPE_CORE_SPPSAGH_ADVANCED_SPPS)
	{
		SetIcon(GRAPH_STATE_ALL, GRAPH_EL_CONFIGURATION);
		InitProperties();
	}

	wxXmlNode* SaveXMLCoreDoc(wxXmlNode* NoeudParent)
	{
		wxXmlNode* NoeudCourant = new wxXmlNode(NoeudParent, wxXML_ELEMENT_NODE, "advancedSPPS");
		return Element::SaveXMLCoreDoc(NoeudCourant);
	}
	wxXmlNode* SaveXMLDoc(wxXmlNode* NoeudParent)
	{
		wxXmlNode* thisNode = Element::SaveXMLDoc(NoeudParent);
		thisNode->SetName("advancedSPPS"); // Nom de la balise xml ( pas d'espace autorise )
		return thisNode;
	}

protected:

	void Modified(Element* eModif)
	{
		Element::Modified(eModif);
	}

	/**
	Initialisation des propriétés communes à tout les mailleurs ( fait avant l'initialisation des mailleurs spécialisés )
	*/
	void InitProperties()
	{
		InitRandomSeed(this);
		InitAngleStatsCalc(this);
	}

	void InitRandomSeed(Element* confCore) {
		confCore->AppendPropertyInteger("random_seed", wxTRANSLATE("Random seed"), 0, true, false, true);
	}

	void InitAngleStatsCalc(Element* confCore) {
		confCore->AppendPropertyText("angle_filename", "angle_filename", wxTRANSLATE("Surface incidence angle stats") + wxString(".gabe"), true, true)->Hide();
		confCore->AppendPropertyBool("normalize_angle_stats", wxTRANSLATE("Normalize angle stats"), true, true);
		confCore->AppendPropertyInteger("angle_stats_min_reflection", wxTRANSLATE("Min reflection for incidence angle record"), 0, true, false, true, 0, 0);
		confCore->AppendPropertyBool("extended_angle_stats", wxTRANSLATE("Extended angle stats"), false, true);
	}
};



class E_Core_SppsNee_AGH_advanced_NEE : public Element
{
public:
	E_Core_SppsNee_AGH_advanced_NEE(wxXmlNode* noeudCourant, Element* parent)
		:Element(parent, wxTRANSLATE("Advanced - NEE"), Element::ELEMENT_TYPE_CORE_SPPSAGH_ADVANCED_NEE, noeudCourant)
	{
		SetIcon(GRAPH_STATE_ALL, GRAPH_EL_CONFIGURATION);
		//Add debug mode parameter v<1.21
		if (!this->IsPropertyExist("random_seed")) {
			InitRandomSeed(this);
		}
		if (!this->IsPropertyExist("angle_filename")) {
			InitAngleStatsCalc(this);
		}
		if (!this->IsPropertyExist("shadowray_prob")) {
			InitShadowRayProbability(this);
		}
		if (!this->IsPropertyExist("skip_direct_sound_calc")) {
			InitSkipDirectSoundCalc(this);
		}
	}

	E_Core_SppsNee_AGH_advanced_NEE(Element* parent)
		:Element(parent, wxTRANSLATE("Advanced - NEE"), Element::ELEMENT_TYPE_CORE_SPPSAGH_ADVANCED_NEE)
	{
		SetIcon(GRAPH_STATE_ALL, GRAPH_EL_CONFIGURATION);
		InitProperties();
	}

	wxXmlNode* SaveXMLCoreDoc(wxXmlNode* NoeudParent)
	{
		wxXmlNode* NoeudCourant = new wxXmlNode(NoeudParent, wxXML_ELEMENT_NODE, "advancedNEE");
		return Element::SaveXMLCoreDoc(NoeudCourant);
	}
	wxXmlNode* SaveXMLDoc(wxXmlNode* NoeudParent)
	{
		wxXmlNode* thisNode = Element::SaveXMLDoc(NoeudParent);
		thisNode->SetName("advancedNEE"); // Nom de la balise xml ( pas d'espace autorise )
		return thisNode;
	}

protected:

	void Modified(Element* eModif)
	{
		Element::Modified(eModif);
	}

	/**
	Initialisation des propriétés communes à tout les mailleurs ( fait avant l'initialisation des mailleurs spécialisés )
	*/
	void InitProperties()
	{
		InitRandomSeed(this);
		InitAngleStatsCalc(this);
		InitShadowRayProbability(this);
		InitSkipDirectSoundCalc(this);
	}

	void InitRandomSeed(Element* confCore) {
		confCore->AppendPropertyInteger("random_seed", wxTRANSLATE("Random seed"), 0, true, false, true);
	}

	void InitAngleStatsCalc(Element* confCore) {
		confCore->AppendPropertyText("angle_filename", "angle_filename", wxTRANSLATE("Surface incidence angle stats") + wxString(".gabe"), true, true)->Hide();
		confCore->AppendPropertyBool("normalize_angle_stats", wxTRANSLATE("Normalize angle stats"), true, true);
		confCore->AppendPropertyInteger("angle_stats_min_reflection", wxTRANSLATE("Min reflection for incidence angle record"), 0, true, false, true, 0, 0);
		confCore->AppendPropertyBool("extended_angle_stats", wxTRANSLATE("Extended angle stats"), false, true);
	}
	void InitSkipDirectSoundCalc(Element* confCore) {
		confCore->AppendPropertyBool("skip_direct_sound_calc", wxTRANSLATE("Skip direct sound calculation"), false, true);
	}
	void InitShadowRayProbability(Element* confCore) {
		confCore->AppendPropertyDecimal("shadowray_prob", wxTRANSLATE("Shadowray probability"), 1.0f, false, 3, true, true, 1.0f, 0, true);
	}
};


class E_Core_SppsNee_AGH_advanced_MLT : public Element
{
public:
	E_Core_SppsNee_AGH_advanced_MLT(wxXmlNode* noeudCourant, Element* parent)
		:Element(parent, wxTRANSLATE("Advanced - MLT"), Element::ELEMENT_TYPE_CORE_SPPSAGH_ADVANCED_MLT, noeudCourant)
	{
		SetIcon(GRAPH_STATE_ALL, GRAPH_EL_CONFIGURATION);
		//Add debug mode parameter v<1.21
		if (!this->IsPropertyExist("random_seed")) {
			InitRandomSeed(this);
		}
		if (!this->IsPropertyExist("mutationNumber")) {
			InitBaseMLTProperties(this);
		}
	}

	E_Core_SppsNee_AGH_advanced_MLT(Element* parent)
		:Element(parent, wxTRANSLATE("Advanced - MLT"), Element::ELEMENT_TYPE_CORE_SPPSAGH_ADVANCED_MLT)
	{
		SetIcon(GRAPH_STATE_ALL, GRAPH_EL_CONFIGURATION);
		InitProperties();
	}

	wxXmlNode* SaveXMLCoreDoc(wxXmlNode* NoeudParent)
	{
		wxXmlNode* NoeudCourant = new wxXmlNode(NoeudParent, wxXML_ELEMENT_NODE, "advancedMLT");
		return Element::SaveXMLCoreDoc(NoeudCourant);
	}
	wxXmlNode* SaveXMLDoc(wxXmlNode* NoeudParent)
	{
		wxXmlNode* thisNode = Element::SaveXMLDoc(NoeudParent);
		thisNode->SetName("advancedMLT"); // Nom de la balise xml ( pas d'espace autorise )
		return thisNode;
	}

protected:

	void Modified(Element* eModif)
	{
		Element::Modified(eModif);
	}

	/**
	Initialisation des propriétés communes à tout les mailleurs ( fait avant l'initialisation des mailleurs spécialisés )
	*/
	void InitProperties()
	{
		InitRandomSeed(this);
		InitBaseMLTProperties(this);
	}

	void InitRandomSeed(Element* confCore) {
		confCore->AppendPropertyInteger("random_seed", wxTRANSLATE("Random seed"), 0, true, false, true);
	}

	void InitBaseMLTProperties(Element* confCore) {
		confCore->AppendPropertyDecimal("largeStepProb", wxTRANSLATE("Large step probability"), 0.5, false, 2, true, true, 1, 0, true);
		confCore->AppendPropertyDecimal("specularReflProbabilityMulti", wxTRANSLATE("Specular reflection probability multiplier"), 0.5, false, 3, true, true, 10, 0.01, true);
		confCore->AppendPropertyInteger("mutationNumber", wxTRANSLATE("Mutation number"), 25, true, false, true, 0, 0);
	}
};


#endif
#pragma once
