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

#include <input_output/brdfs/brdfParser.h>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/regex.hpp>
#ifdef WIN32
#include "input_output/pugixml/src/pugixml.hpp"
#endif // WIN32

using namespace std;

bool txt_BrdfParser::parse(std::string filePath, t_BrdfBalloon *balloon)
{
	#ifdef WIN32
		ifstream file(pugi::as_wide(filePath), ifstream::in);
	#else
		ifstream file(filePath, ifstream::in);
	#endif // WIN
	if (!file.is_open()) {
		cout << "BRDF file not found : File not open" << endl;
		return false;
	}
	else
	{
		std::cout << "Parse BRDF file: " << filePath << std::endl;
		string line;
		short currentFrequency = 0, Stheta, Sphi, Rtheta, Rphi = 0;
		double domega;
		double energyIntegral = 0;
		//double theta_min_value, theta_max_value; // regardless of phi, thetha for 0 and 180° must be constant
		//bool thetaBoundDefined = false;
		vector< string > tokens;
		boost::smatch headerMatch;
		boost::regex Re_notNumber("[^0-9]");
		boost::regex Re_angleHeader("f=([0-9]*) Hz, Stheta=([0-9]*), Sphi=([0-9]*)$");
		std::getline(file, line);
		if (boost::contains(line, "ANGLE INCREMENT"))
		{
			boost::split(tokens, line, boost::is_any_of(" "));
			txt_BrdfParser::ANGLE_INCREMENT = stod(tokens[2]);
			std::cout << "Found angle increment: " << txt_BrdfParser::ANGLE_INCREMENT << " deg" << std::endl;
		}
		else 
		{
			std::cout << "Angle increment not found!!! Assumed 5 deg " << std::endl;
			txt_BrdfParser::ANGLE_INCREMENT = 5;
			file.seekg(0, ios::beg);
		}

		balloon->setAngleIncrement(txt_BrdfParser::ANGLE_INCREMENT);

		while (std::getline(file, line))
		{
			if (!line.empty() && line[0] != ';') // not a comment
			{
				// check header
				if (boost::regex_match(line, headerMatch, Re_angleHeader))
				{
					// Skip match 0, it contains full string
					currentFrequency = stoi(headerMatch[1].str());
					Stheta = stod(headerMatch[2].str());
					Sphi = stod(headerMatch[3].str());
					Rtheta = 0;
					energyIntegral = 0;
					balloon->initializePdfVector(currentFrequency, Sphi, Stheta);
					continue;
				}
				boost::trim(line);
				boost::split(tokens, line, boost::is_any_of(" ")); // split, 60% of cpu time
				if (!tokens.empty())
				{
					// data, no missing value allowed, 40% of cpu time
					if (tokens.size()*txt_BrdfParser::ANGLE_INCREMENT == 360 && currentFrequency > 0)
					{
						//boost::erase_all_regex(tokens[0], Re_notNumber);
		//				double phi = stod(tokens[0]);

						for (auto i = 0; i < tokens.size(); i++)
						{
							Rphi = i * txt_BrdfParser::ANGLE_INCREMENT;
							double value = stod(tokens[i]);
							value = value * value;
							balloon->setValue(currentFrequency, Sphi, Stheta, Rphi, Rtheta, value);

							domega = this->getDifferentialSolidAngle(Rphi, Rtheta);
							energyIntegral += domega * value;
							balloon->setPdfVectorValue(currentFrequency, Sphi, Stheta, Rphi, Rtheta, energyIntegral);
							//energyIntegral += domega * 1;
						}
						Rtheta += txt_BrdfParser::ANGLE_INCREMENT;

						if (Rtheta == 90)
						{
							balloon->setNormalizationFactor(1. / energyIntegral, Sphi, Stheta, currentFrequency);
							balloon->normalizePdfVector(currentFrequency, Sphi, Stheta, 1. / energyIntegral);
							//cout << "Parsed BRDF for: " << currentFrequency << Sphi << Stheta << endl;
						}
					}
				}
			}
			tokens.clear();
		}

		file.close();
		std::cout << "Parsing BRDF finished." << std::endl;
	}
	return true;
}

double txt_BrdfParser::getDifferentialSolidAngle(double phi, double theta) 
{
	double dtheta = this->ANGLE_INCREMENT / 2.;
	double theta0, theta1;
	if (theta == 0) 
	{
		theta0 = 0;
		theta1 = dtheta;
	}
	else if (theta == 90 - ANGLE_INCREMENT) 
	{
		theta0 = theta-dtheta;
		theta1 = theta + 2*dtheta;
	}
	else 
	{
		theta0 = theta - dtheta;
		theta1 = theta + dtheta;
	}
	return deg2rad(ANGLE_INCREMENT)*(cos(deg2rad(theta0)) - cos(deg2rad(theta1)));
}

inline double txt_BrdfParser::deg2rad(double deg)
{
	return deg * M_PI / 180.;
}

