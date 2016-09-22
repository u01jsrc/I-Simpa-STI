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
class E_Core_SppsNee_AGH_advanced : public Element
{
public:
	E_Core_SppsNee_AGH_advanced(wxXmlNode* noeudCourant, Element* parent)
		:Element(parent, wxTRANSLATE("Advanced"), Element::ELEMENT_TYPE_CORE_SPPSNEE_AGH_ADVANCED, noeudCourant)
	{
		SetIcon(GRAPH_STATE_ALL, GRAPH_ADVANCED_PARAMETERS);
		//Add debug mode parameter v<1.21
		if (!this->IsPropertyExist("random_seed")) {
			InitRandomSeed(this);
		}
	}

	E_Core_SppsNee_AGH_advanced(Element* parent)
		:Element(parent, wxTRANSLATE("Advanced"), Element::ELEMENT_TYPE_CORE_SPPSNEE_AGH_ADVANCED)
	{
		SetIcon(GRAPH_STATE_ALL, GRAPH_ADVANCED_PARAMETERS);
		InitProperties();
	}

	wxXmlNode* SaveXMLCoreDoc(wxXmlNode* NoeudParent)
	{
		wxXmlNode* NoeudCourant = new wxXmlNode(NoeudParent, wxXML_ELEMENT_NODE, "advanced");
		return Element::SaveXMLCoreDoc(NoeudCourant);
	}
	wxXmlNode* SaveXMLDoc(wxXmlNode* NoeudParent)
	{
		wxXmlNode* thisNode = Element::SaveXMLDoc(NoeudParent);
		thisNode->SetName("spps-agh-advanced"); // Nom de la balise xml ( pas d'espace autorise )
		return thisNode;
	}


protected:

	void Modified(Element* eModif)
	{
		Element::Modified(eModif);
	}

	void InitRandomSeed(Element* confCore) {
		confCore->AppendPropertyInteger("random_seed", wxTRANSLATE("Random seed"), 0, true, false, true);
	}

	/**
	Initialisation des propriétés communes à tout les mailleurs ( fait avant l'initialisation des mailleurs spécialisés )
	*/
	void InitProperties()
	{
		InitRandomSeed(this);
	}

};

#endif
#pragma once
