/*
 * Copyright 2010-2018 OpenXcom Developers.
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
#include "GeoscapeGeneratorState.h"
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

GeoscapeGenerator::GeoscapeGenerator(GeoscapeGeneratorState *parent)
{
	_rngSeed = RNG::getSeed();
}

// Cleans up the GeoscapeGenerator
GeoscapeGenerator::~GeoscapeGenerator()
{
	// delete variables to clean up here
}

void GeoscapeGenerator::init(size_t seed, size_t numCircles)
{
	_greatCircles.clear();
	_intersections.clear();
	_globeSections.clear();
	_newSections.clear();

	RNG::setSeed(seed);
	_numberOfCircles = numCircles;
}

// Runs the generation
void GeoscapeGenerator::generate()
{
	// Pick the first great circle and create two globe sections to start the generator
	generateGreatCircle();

	GlobeSection section1 = GlobeSection(this);
	section1.getGreatCircles()->insert(std::pair<size_t, int>(0, 1));
	section1.setHeightIndex(1);

	GlobeSection section2 = GlobeSection(this);
	section2.getGreatCircles()->insert(std::pair<size_t, int>(0, -1));

	_globeSections.push_back(section1);
	_globeSections.push_back(section2);
	
	// Create all the globe sections with the fractal world generator method
	for (size_t i = 0; i < _numberOfCircles - 1; ++i)  // First circle is already picked
	{
		// Get a new great circle and try all possible intersections with current globe sections
		generateGreatCircle();
		for (std::vector<GlobeSection>::iterator j = _globeSections.begin(); j != _globeSections.end(); ++j)
		{
			try
			{
				(*j).intersectWithGreatCircle(_greatCircles.size() - 1);
			}
			catch (Exception& errorMsg)
			{
				error(errorMsg);
			}
		}

		// Add the newly split globe sections to the list
		for (std::vector<GlobeSection>::iterator j = _newSections.begin(); j != _newSections.end(); ++j)
		{
			_globeSections.push_back((*j));
		}
		_newSections.clear();
	}

	// Now that we have the sections, split them into viable polygons for the globe ruleset
	for (std::vector<GlobeSection>::iterator i = _globeSections.begin(); i != _globeSections.end(); ++i)
	{
		try
		{
			(*i).splitIntoPolygons();
		}
		catch (Exception& errorMsg)
		{
			error(errorMsg);
		}
	}

	// Convert the polygon sections' height data into globe textures
	// Start by sorting the sections according to height
	std::sort(_newSections.begin(), _newSections.end(), [](GlobeSection section1, GlobeSection section2) {return section1.getHeightIndex() < section2.getHeightIndex(); });

	// Fill up polygons with textures - first set of sections up to the chosen water percentage shouldn't change from -1
	double waterPercentage = 0.4f;
	for (size_t i = 0; i < _newSections.size(); ++i)
	{
		double percent = (double) i / (double) (_newSections.size() -1);
		if (percent > waterPercentage)
		{
			_newSections.at(i).setTextureId(0);
		}
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
			// If the normal vector is within 0.01 arc length, generate a new normal vector
			if (normalVector.distance((*i)) < 0.01)
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
std::vector<GlobeSection> *GeoscapeGenerator::getGlobeSections()
{
	return &_globeSections;
}

// Gets the list of globe sections added by the latest great circle intersections
std::vector<GlobeSection> *GeoscapeGenerator::getNewSections()
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

	for (auto i : _newSections)
	{
		if (i.getTextureId() == -1)
			continue;

		std::vector<double> sectionData;
		sectionData.clear();
		sectionData.push_back(i.getTextureId());
		for (auto j = i.getPolygonVertices()->begin(); j != i.getPolygonVertices()->end(); ++j)
		{
			sectionData.push_back((*j).lat);
			sectionData.push_back((*j).lon);
		}
		
		node["globe"]["polygons"].push_back(sectionData);
	}
	node["RNGSeed"] = _rngSeed;

	for (auto i : _greatCircles)
	{
		std::vector<double> coords = {i.x, i.y, i.z, i.lat, i.lon};
		node["globe"]["greatCircles"].push_back(coords);
	}

	out << node;
	sav << out.c_str();
	sav.close();
}

// Outputs all info to log on error
void GeoscapeGenerator::error(Exception errorMsg) const
{
	Log(LOG_ERROR) << "GeoscapeGenerator encountered an error, printing all info.";
	Log(LOG_ERROR) << " Great Circles:";
	for (auto i : _greatCircles)
	{
		i.writeToLog();
	}
	Log(LOG_ERROR) << " Intersections:";
	for (auto i : _intersections)
	{
		Log(LOG_ERROR) << "  (" << i.first.first << ", " << i.first.second << ")";
		i.second.writeToLog();
	}
	Log(LOG_ERROR) << " GlobeSections:";
	size_t sectionNumber = 0;
	for (auto i : _globeSections)
	{
		Log(LOG_ERROR) << "  Section " << sectionNumber << ":";
		sectionNumber++;
		for (auto j = i.getGreatCircles()->begin(); j != i.getGreatCircles()->end(); ++j)
		{
			Log(LOG_ERROR) << "   (" << (*j).first << ", " << (*j).second << ")";
		}
		for (auto j = i.getIntersections()->begin(); j != i.getIntersections()->end(); ++j)
		{
			Log(LOG_ERROR) << "   (" << (*j).first.first << ", " << (*j).first.second << ", " << (*j).second << ")";
		}
	}
	Log(LOG_ERROR) << " NewSections:";
	sectionNumber = 0;
	for (auto i : _newSections)
	{
		Log(LOG_ERROR) << "  Section " << sectionNumber << ":";
		sectionNumber++;
		for (auto j = i.getGreatCircles()->begin(); j != i.getGreatCircles()->end(); ++j)
		{
			Log(LOG_ERROR) << "   (" << (*j).first << ", " << (*j).second << ")";
		}
		for (auto j = i.getIntersections()->begin(); j != i.getIntersections()->end(); ++j)
		{
			Log(LOG_ERROR) << "   (" << (*j).first.first << ", " << (*j).first.second << ", " << (*j).second << ")";
		}
	}
	std::ostringstream ss;
	ss << errorMsg.what() << "\nError in geoscape generator. All data written to openxcom.log";
	throw Exception(ss.str());
}

}
