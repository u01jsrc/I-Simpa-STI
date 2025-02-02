﻿/* ----------------------------------------------------------------------
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

#include "e_directivity.h"

/** \file e_directivity_user.h
\brief Classe spécialisant e_directivity

Représente une directivité personalisée utilisateur
*/

/**
\brief Classe spécialisant e_directivity

Représente une directivité personalisée utilisateur
*/

class E_Directivity_User : public E_Directivity
{
public:
	E_Directivity_User(wxXmlNode* noeudCourant, Element* parent)
		:E_Directivity(parent, wxTRANSLATE("User directivity"), Element::ELEMENT_TYPE_DIRECTIVITIES_USER, noeudCourant)
	{
		this->elementInfo.userDestroyable = true;
	}

	E_Directivity_User(Element* parent)
		:E_Directivity(parent, wxTRANSLATE("User directivity"), Element::ELEMENT_TYPE_DIRECTIVITIES_USER)
	{
		this->elementInfo.userDestroyable = true;
		InitProperties();
	}

	void OnBeginLabelEdit(wxTreeEvent& treeEvent)
	{
		// On autorise l'edition en surchargeant l'événement
	}

	void OnEndLabelEdit(wxTreeEvent& treeEvent)
	{
		if (treeEvent.GetLabel() != "")
		{
			this->elementInfo.libelleElement = treeEvent.GetLabel();
			this->Modified(this); //Indique le l'element a été modifié
		}
	}

	virtual wxXmlNode* SaveXMLDoc(wxXmlNode* NoeudParent)
	{
		wxXmlNode* thisNode = E_Directivity::SaveXMLDoc(NoeudParent);
		thisNode->SetName("directivities"); // Nom de la balise xml ( pas d'espace autorise )

		return thisNode;
	}

};
