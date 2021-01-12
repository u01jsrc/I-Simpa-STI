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

#include <unordered_map>
#include <vector>
#include <string>
#include <Core/mathlib.h>
#include <algorithm>

#ifndef BRDF_BALLOON
#define BRDF_BALLOON

/**
* @brief Data structure for Directivity Balloon 
*/
class t_BrdfBalloon
{
private:
	/** @brief store magnitude values as attenuations[frequency][phi_i][theta_i][phi_o][theta_o] = value */
	std::unordered_map<short, std::unordered_map<short, std::unordered_map<short, std::unordered_map<short, std::unordered_map<short, float>>>>> attenuations;
	int AngleIncrement;
	std::unordered_map<short, std::unordered_map<short, std::unordered_map<short, float>>> normalizationFactor;
	std::unordered_map<short, std::unordered_map<short, std::unordered_map<short, std::vector<float>>>> pdfVector;

public:
	/** 
	* Constructor using a Parsing strategy
	* @param parsingStrategy a class implementing the DirectivityParser interface
	* @see DirectivityParser
	*/
	t_BrdfBalloon();
	~t_BrdfBalloon();

	void setValue(short freq, short sphi, short stheta, short rphi, short rtheta, float value);
	float getAttenuation(short freq, short sphi, short stheta, short rphi, short rtheta);

	void setAngleIncrement(int _AngleIncrement);

	void setNormalizationFactor(float _normalizationFactor, short sphi, short stheta, short freq);
	float getNormalizationFactor(short sphi, short stheta, short freq);

	std::vector<float> getAvalibleFrequencies();
	void getThetaPhi(const vec3 &normal, const vec3 &ray, double &theta, double &phi);
	double getEnergy(short freq, const vec3 &normal, const vec3 &inRay, const vec3 &outRay);
	float integrateEnergy(short freq, short sphi, short stheta);
	short roundToNearestAngle(double &value);

	double getDifferentialSolidAngle(double phi, double theta);
	double deg2rad(double deg);
	
	void initializePdfVector(short freq, short sphi, short stheta);

	void setPdfVectorValue(short freq, short sphi, short stheta, short rphi, short rtheta, float value);

	void normalizePdfVector(short freq, short sphi, short stheta, float normalizationFactor);

	void calculateReflectionAnglesFromPdf(short freq, const vec3& normal, const vec3& inRay, float& rphi, float& rtheta);
	
};
#endif
