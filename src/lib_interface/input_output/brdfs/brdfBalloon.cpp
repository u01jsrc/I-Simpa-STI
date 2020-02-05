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

#include <input_output/brdfs/brdfBalloon.h>
#include <iostream>

using namespace std;

t_BrdfBalloon::t_BrdfBalloon()
{
	AngleIncrement = 0;
}

t_BrdfBalloon::~t_BrdfBalloon()
{
}

void t_BrdfBalloon::setValue(short freq, short sphi, short stheta, short rphi, short rtheta, float value) {
	attenuations[freq][sphi][stheta][rphi][rtheta] = value;
}

float t_BrdfBalloon::getAttenuation(short freq, short sphi, short stheta, short rphi, short rtheta)
{
	return attenuations[freq][sphi][stheta][rphi][rtheta];
}

void t_BrdfBalloon::setAngleIncrement(int _AngleIncrement) {
	AngleIncrement = _AngleIncrement;
}

void t_BrdfBalloon::setNormalizationFactor(float _normalizationFactor, short sphi, short stheta, short freq) {
	normalizationFactor[freq][sphi][stheta] = _normalizationFactor;
}

float t_BrdfBalloon::getNormalizationFactor(short sphi, short stheta, short freq)
{
	return normalizationFactor[freq][sphi][stheta];
}

vector<float> t_BrdfBalloon::getAvalibleFrequencies()
{
	vector<float> freq;
	for (auto const& element : attenuations) {
		freq.push_back(element.first);
	}
	return freq;
}

void t_BrdfBalloon::getThetaPhi(const vec3 & _normal, const vec3 & ray, double & theta, double & phi)
{
	vec3 normal(_normal.x, _normal.y, _normal.z);
	vec3 dir(ray.x, ray.y, ray.z);
	vec3 target(0, 0, 1);
	Matrix3 R;

	R.calculateRotationMatrix(normal, target);
	dir = R * dir;

	phi = atan2(dir.y, dir.x) * 180 / M_PI;
	theta = acos(dir.z / dir.length()) * 180 / M_PI;

	if (phi < 0) phi += 360;
}

double t_BrdfBalloon::getEnergy(short freq, const vec3 & normal, const vec3 & inRay, const vec3 & outRay)
{
	double Stheta, Sphi, Rtheta, Rphi;
	short  Stheta_r, Sphi_r, Rtheta_r, Rphi_r;

	getThetaPhi(normal, -inRay, Stheta, Sphi);
	if (Stheta > 90) 
		std::cout << "Stheta > 90" << std::endl;

	getThetaPhi(normal, outRay, Rtheta, Rphi);
	if (Rtheta > 90)
		std::cout << "Rtheta > 90" << std::endl;

	Stheta_r = roundToNearestAngle(Stheta);
	Sphi_r = roundToNearestAngle(Sphi);
	Rtheta_r = roundToNearestAngle(Rtheta);
	Rphi_r = roundToNearestAngle(Rphi);

	if (Sphi_r > 360 - AngleIncrement)
		Sphi_r -= 0;
	if (Stheta_r > 90 - AngleIncrement)
		Stheta_r = 90 - AngleIncrement;
	if (Rphi_r > 360 - AngleIncrement)
		Rphi_r -= 0;
	if (Rtheta_r > 90 - AngleIncrement)
		Rtheta_r = 90 - AngleIncrement;

	return getAttenuation(freq, Sphi_r, Stheta_r, Rphi_r, Rtheta_r) * getNormalizationFactor(Sphi_r, Stheta_r, freq);
}

short t_BrdfBalloon::roundToNearestAngle(double & value)
{
	return ((short(value) + AngleIncrement - 1) / AngleIncrement) * AngleIncrement;
}

