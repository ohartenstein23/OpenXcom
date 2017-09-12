/*
 * Copyright 2010-2017 OpenXcom Developers.
 *
 * This file is part of OpenXcom.
 *
 * OpenXcom is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenXcom is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenXcom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "GeoscapeGenerator.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>
#include <set>
#include <iomanip>
#include <yaml-cpp/yaml.h>
#include "../fmath.h"
#include "../Engine/RNG.h"
#include "../Engine/Options.h"
#include "../Engine/Exception.h"
#include "../Engine/Logger.h"
#include "GlobeSection.h"

namespace OpenXcom
{

const double GeoscapeGenerator::RAD_TO_DEG = 180 / M_PI;
const double GeoscapeGenerator::DEG_TO_RAD = M_PI / 180;

GeoscapeGenerator::GeoscapeGenerator(uint64_t rngSeed, size_t numberOfCircles) : _rngSeed(rngSeed), _numberOfCircles(numberOfCircles)
{
	_greatCircles.clear();
	_intersections.clear();
	_globeSections.clear();
	_newSections.clear();

	// Pick the first great circle and create two globe sections to start the generator
	generateGreatCircle();

	GlobeSection *section1 = new GlobeSection(this);
	section1->getGreatCircles()->insert(std::pair<size_t, int>(0, 1));
	section1->setHeightIndex(1);

	GlobeSection *section2 = new GlobeSection(this);
	section2->getGreatCircles()->insert(std::pair<size_t, int>(0, -1));
	section2->setHeightIndex(-1);

	_globeSections.push_back(section1);
	_globeSections.push_back(section2);
}

// Cleans up the GeoscapeGenerator
GeoscapeGenerator::~GeoscapeGenerator()
{
	// delete variables to clean up here
}

// Runs the generation
void GeoscapeGenerator::generate()
{
	// Create all the globe sections with the fractal world generator method
	for (size_t i = 0; i < _numberOfCircles - 2; ++i)  // First circle is already picked
	{
		// Get a new great circle and try all possible intersections with current globe sections
		generateGreatCircle();
		for (std::vector<GlobeSection*>::iterator j = _globeSections.begin(); j < _globeSections.end(); ++j)
		{
			(*j)->intersectWithGreatCircle(_greatCircles.size() - 1);
		}

		// Add the newly split globe sections to the list
		for (std::vector<GlobeSection*>::iterator j = _newSections.begin(); j < _newSections.end(); ++j)
		{
			_globeSections.push_back((*j));
		}
		_newSections.clear();
	}
}

// Gets a random latitude
// Uses the arcsine to ensure a uniform point distribution over the globe
double GeoscapeGenerator::randomLatitude()
{
	double latitude = asin(RNG::generate(-1.000, 1.000)) * RAD_TO_DEG;
	return latitude;
}

// Gets a random longitude
double GeoscapeGenerator::randomLongitude()
{

	// Using only 3 decimal points since output will be formated to "%.3f"
	double longitude = RNG::generate(0.000, 359.999);
	return longitude;
}

// Picks a random great circle by the angle of the vector normal to its plane
// Ensures that the new circle is not too similar to a previous one and adds it
// to the _greatCircles vector
void GeoscapeGenerator::generateGreatCircle()
{
	bool uniqueCircle = false;
	std::pair<double, double> normalVector;

	while (!uniqueCircle)
	{
		normalVector.first = randomLatitude();
		normalVector.second = randomLongitude();
		uniqueCircle = true;

		for (std::vector<std::pair<double, double>>::iterator i = _greatCircles.begin(); i != _greatCircles.end(); ++i)
		{
			// If the normal vector is within 0.01 degrees in both angles, generate a new normal vector
			if ((std::abs((*i).first - normalVector.first) < 0.01)
				&& (std::abs((*i).second - normalVector.second) < 0.01))
			{
				uniqueCircle = false;
				break;
			}
		}
	}

	_greatCircles.push_back(normalVector);
}

/**
 * Gets the two intersection points for a pair of great circles
 * Puts the intersections in the _intersections vector and returns the indices of the intersections in that vector
 * If the intersections are too close to a previous pair, returns those indices instead
 * @param circle1 Index of the first circle.
 * @param circle2 Index of the second circle
 * @param index1 Pointer to the first index to be returned.
 * @param index2 Pointer to the second index to be returned.
 */
void GeoscapeGenerator::intersectGreatCircles(size_t circle1, size_t circle2, size_t *index1, size_t *index2)
{
	// Get the angles from the normal vectors in radians
	double theta[2] = {_greatCircles.at(circle1).first * DEG_TO_RAD, _greatCircles.at(circle2).first * DEG_TO_RAD};
	double phi[2] = {_greatCircles.at(circle1).second * DEG_TO_RAD, _greatCircles.at(circle2).second * DEG_TO_RAD};

	// Get the normal vectors from the circles in cartesian coordinates
	double circle1Vector[3] = {cos(theta[0]) * cos(phi[0]),
				cos(theta[0]) * sin(phi[0]),
				sin(theta[0])};
	double circle2Vector[3] = {cos(theta[1]) * cos(phi[1]),
				cos(theta[1]) * sin(phi[1]),
				sin(theta[1])};

	// Get the vector of the line on which the circles intersect in Cartesian coordinates
	// (Intersection of two planes is along the cross-product of their normal vectors)
	double intersectionVector[3] = {circle1Vector[1] * circle2Vector[2] - circle1Vector[2] * circle2Vector[1],
				circle1Vector[2] * circle2Vector[0] - circle1Vector[0] * circle2Vector[2],
				circle1Vector[0] * circle2Vector[1] - circle1Vector[1] * circle2Vector[0]};
	double normalization = 0;
	for (size_t i = 0; i < 3; ++i)
	{
		normalization += intersectionVector[i] * intersectionVector[i];
	}

	// Get the angles from the Cartesian coordinates (should be 180 degrees or inverted across the origin from each other)
	// Latitude
	double z = intersectionVector[2] / normalization;
	if (z > 1) // just in case of floating point errors making this outside the range [-1, 1]
	{
		theta[0] = 90.000;
	}
	else if (z < -1)
	{
		theta[0] = -90.000;
	}
	else
	{
		theta[0] = asin(intersectionVector[2] / normalization) * RAD_TO_DEG;
	}
	theta[1] = -theta[0];

	// Longitude
	if (intersectionVector[1] > 0)
	{
		phi[0] = atan2(intersectionVector[1], intersectionVector[0]) * RAD_TO_DEG;
		phi[1] = phi[0] + 180.000;
	}
	else
	{
		phi[1] = atan2(-intersectionVector[1], -intersectionVector[0]) * RAD_TO_DEG;
		phi[0] = phi[1] + 180.000;
	}

	// Check to see if these calculated intersections are close enough to another to just use that one
	// The intersecting circles have to be the same too, otherwise we want a unique reference to this pair of circles
	bool indexSet[2] = {false, false};
	for (size_t i = 0; i < _intersections.size(); ++i)
	{
		for (size_t j = 0; j < 2; ++j)
		{
			if (!indexSet[j]
				&& (std::abs(_intersections.at(i).coordinates.first - theta[j]) < 0.01)
				&& (std::abs(_intersections.at(i).coordinates.second - phi[j]) < 0.01)
				&& (_intersections.at(i).circles.first == circle1 || _intersections.at(i).circles.second == circle1)
				&& (_intersections.at(i).circles.first == circle2 || _intersections.at(i).circles.second == circle2))
			{
				if (!j) // j = 0, circle 1
				{
					(*index1) = i;
				}
				else // j = 1, circle 2
				{
					(*index2) = i;
				}

				indexSet[j] = true;
			}
		}

		if (indexSet[0] && indexSet[1]) // We found both already
		{
			break;
		}
	}

	// Add new intersections to the list if they weren't found already
	if (!indexSet[0])
	{
		GreatCircleIntersection newIntersection(std::make_pair(theta[0], phi[0]), std::make_pair(circle1, circle2));
		_intersections.push_back(newIntersection);
		(*index1) = _intersections.size() - 1;
	}

	if (!indexSet[1])
	{
		GreatCircleIntersection newIntersection(std::make_pair(theta[1], phi[1]), std::make_pair(circle1, circle2));
		_intersections.push_back(newIntersection);
		(*index2) = _intersections.size() - 1;
	}
}

// Gets a pointer to the list of great circles
std::vector<std::pair<double, double>> *GeoscapeGenerator::getGreatCircles()
{
	return &_greatCircles;
}

// Gets a pointer to the list of intersections
std::vector<GreatCircleIntersection> *GeoscapeGenerator::getIntersections()
{
	return &_intersections;
}

/** Rotates a point on the globe according to the normal vector of a great circle
 * @param circle Index of the great circle
 * @param latitude Pointer to the rotated latitude
 * @param longitude Pointer to the rotated longitude
 */
void GeoscapeGenerator::rotatePointOnSphere(size_t circle, double *latitude, double *longitude)
{
	// Convert angle vectors to Cartesian coordinates
	double inputVector[3] = {cos((*longitude) * DEG_TO_RAD) * cos((*latitude) * DEG_TO_RAD),
				sin((*longitude) * DEG_TO_RAD) * cos((*latitude) * DEG_TO_RAD),
				sin((*latitude) * DEG_TO_RAD)};

	double thetaNormal = _greatCircles.at(circle).first * DEG_TO_RAD;
	double phiNormal = _greatCircles.at(circle).second * DEG_TO_RAD;
	double normalVector[3] = {cos(phiNormal) * cos(thetaNormal),
				sin(phiNormal) * cos(thetaNormal),
				sin(thetaNormal)};

	// Create matrix for rotating the points in Cartesian coordinates
	double rotationVector[3] = {-normalVector[1], normalVector[0], 0};
	double rotationAngle = M_PI_2 - thetaNormal;
	double rotationMatrix[3][3] = {0};

	rotationMatrix[0][0] = cos(rotationAngle) + (1 - cos(rotationAngle)) * rotationVector[0] * rotationVector[0];
	rotationMatrix[0][1] = (1 - cos(rotationAngle)) * rotationVector[0] * rotationVector[1];
	rotationMatrix[0][2] = rotationVector[1];
	rotationMatrix[1][0] = (1 - cos(rotationAngle)) * rotationVector[0] * rotationVector[1];
	rotationMatrix[1][1] = cos(rotationAngle) + (1 - cos(rotationAngle)) * rotationVector[1] * rotationVector[1];
	rotationMatrix[1][2] = -rotationVector[0];
	rotationMatrix[2][0] = -rotationVector[1];
	rotationMatrix[2][1] = rotationVector[0];
	rotationMatrix[2][2] = cos(rotationAngle);

	double outputVector[3] = {0};
	for (size_t i = 0; i < 3; ++i)
	{
		for (size_t j = 0; j < 3; ++j)
		{
			outputVector[i] += rotationMatrix[i][j] * inputVector[j];
		}
	}

	double normalization = 0;
	for (size_t i = 0; i < 3; ++i)
	{
		normalization += outputVector[i] * outputVector[i];
	}

	double z = outputVector[2] / normalization;
	if (z > 1)
	{
		(*latitude) = M_PI;
	}
	else if (z < 1)
	{
		(*latitude) = -1 * M_PI;
	}
	else
	{
		(*latitude) = asin(outputVector[2] / normalization);
	}

	if (outputVector[1] > 0)
	{
		(*longitude) = atan2(outputVector[1], outputVector[0]);
	}
	else
	{
		(*longitude) = atan2(outputVector[1], outputVector[0]) + 2 * M_PI;
	}

	(*latitude) = (*latitude) * RAD_TO_DEG;
	(*longitude) = (*longitude) * RAD_TO_DEG;
}

// Gets the list of globe sections
std::vector<GlobeSection*> *GeoscapeGenerator::getGlobeSections()
{
	return &_globeSections;
}

// Gets the list of globe sections added by the latest great circle intersections
std::vector<GlobeSection*> *GeoscapeGenerator::getNewSections()
{
	return &_newSections;
}

// Saves the result of the geoscape generator
void GeoscapeGenerator::save() const
{
	std::string filename = "geoscapeGeneratorOutput.yml";
	std::string s = Options::getMasterUserFolder() + filename;
	std::ofstream sav(s.c_str());
	if (!sav)
	{
		throw Exception("Failed to save " + filename);
	}

	YAML::Emitter out;
	out << YAML::BeginDoc;
	YAML::Node node;

	for (std::vector<GlobeSection*>::const_iterator i = _globeSections.begin(); i != _globeSections.end(); ++i)
	{
		std::vector<double> sectionData;
		sectionData.clear();
		sectionData.push_back((*i)->getHeightIndex());
		for (std::vector<GreatCircleIntersection*>::iterator j = (*i)->getIntersections()->begin(); j != (*i)->getIntersections()->end(); ++j)
		{
			sectionData.push_back((*j)->coordinates.second);
			sectionData.push_back((*j)->coordinates.first);
		
		node["globe"]["polygons"].push_back(sectionData);
		}
	}
	node["RNGSeed"] = _rngSeed;

	out << node;
	sav << out.c_str();
	sav.close();
}

}
