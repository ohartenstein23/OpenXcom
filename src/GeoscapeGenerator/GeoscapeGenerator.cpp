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

GeoscapeGenerator::GeoscapeGenerator()
{
	_rngSeed = RNG::getSeed();
	_numberOfCircles = 10;
}

// Cleans up the GeoscapeGenerator
GeoscapeGenerator::~GeoscapeGenerator()
{
	// delete variables to clean up here
}

// Runs the generation
void GeoscapeGenerator::generate()
{
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
	double latitude = asin(RNG::generate(-1.000, 1.000)) * M_RAD_TO_DEG;
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
// to the _greatCircles vector, then calculates all intersections with the
// previous great circles and adds them to _intersections
void GeoscapeGenerator::generateGreatCircle()
{
	bool uniqueCircle = false;
	GlobeVector normalVector;

	// Pick a random great circle by its normal vector
	while (!uniqueCircle)
	{
		normalVector = GlobeVector(randomLatitude(), randomLongitude());
		uniqueCircle = true;

		for (std::vector<GlobeVector>::iterator i = _greatCircles.begin(); i != _greatCircles.end(); ++i)
		{
			// If the normal vector is within 0.01 degrees in both angles, generate a new normal vector
			if ((std::abs((*i).lat - normalVector.lat) < 0.01)
				&& (std::abs((*i).lon - normalVector.lon) < 0.01))
			{
				uniqueCircle = false;
				break;
			}
		}
	}

	_greatCircles.push_back(normalVector);
	
	// Now calculate all the new intersections, if this isn't the first circle
	if (_greatCircles.size() > 1)
	{
		for (size_t i = 0; i < _greatCircles.size() - 1; ++i) // iterate over all previous circles
		{
			intersectGreatCircles(i, _greatCircles.size() - 1);
		}
	}
}

/**
 * Gets the two intersection points for a pair of great circles
 * Puts the intersections in the _intersections vector
 * @param circle1 Index of the first circle.
 * @param circle2 Index of the second circle
 */
void GeoscapeGenerator::intersectGreatCircles(size_t circle1, size_t circle2)
{
	// The intersection of two great circles lies along the line defined by the cross-product of their normal vectors
	GlobeVector intersection = _greatCircles.at(circle1) * _greatCircles.at(circle2);

	// Since the pair of intersections are related by an inversion through the origin (multiplication of vector by -1
	// for unit vectors), only need to store the one vector
	_intersections[std::make_pair(circle1, circle2)] = intersection;
}

// Gets a pointer to the list of great circles
std::vector<GlobeVector> *GeoscapeGenerator::getGreatCircles()
{
	return &_greatCircles;
}

// Gets a pointer to the list of intersections
std::map<std::pair<size_t, size_t>, GlobeVector> *GeoscapeGenerator::getIntersections()
{
	return &_intersections;
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
		for (std::vector<std::pair<std::pair<size_t, size_t>, int>>::iterator j = (*i)->getIntersections()->begin(); j != (*i)->getIntersections()->end(); ++j)
		{
			const std::pair<size_t, size_t> circles = (*j).first;
			std::map<std::pair<size_t, size_t>, GlobeVector>::const_iterator it = _intersections.find(circles);
			GlobeVector coordinates = (*it).second * (*j).second;
			sectionData.push_back(coordinates.lat);
			sectionData.push_back(coordinates.lon);
		}
		
		node["globe"]["polygons"].push_back(sectionData);
	}
	node["RNGSeed"] = _rngSeed;

	out << node;
	sav << out.c_str();
	sav.close();
}

}
