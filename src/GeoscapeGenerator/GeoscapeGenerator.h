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

#include <vector>
#include <list>
#include <map>
#include "GlobeVector.h"

namespace OpenXcom
{

class GeoscapeGeneratorState;
class GlobeSection;
class Exception;

class GeoscapeGenerator
{
private:
	static const double RAD_TO_DEG;
	static const double DEG_TO_RAD;

	uint64_t _rngSeed;
	size_t _numberOfCircles;
	int _waterThreshold, _polesThreshold;
	std::vector<GlobeVector> _greatCircles;
	std::map<std::pair<size_t, size_t>, GlobeVector> _intersections;
	std::vector<GlobeSection> _globeSections;
	std::vector<GlobeSection> _newSections;
	std::vector<int> _texturesByAltitude, _texturesForPoles;
public:
	/// Constructor
	GeoscapeGenerator(GeoscapeGeneratorState *parent);
	/// Cleans up the GeoscapeGenerator
	~GeoscapeGenerator();

	/// Inputs the data for the GeoscapeGenerator
	void init(size_t seed, size_t numCircles, int waterThreshold, int polesThreshold);

	/// Runs the GeoscapeGenerator
	void generate();

	/// Gets a random latitude
	double randomLatitude();
	/// Gets a random longitude
	double randomLongitude();
	/// Picks a random great circle
	void generateGreatCircle();
	/// Gets the two intersection points for a pair of great circles
	void intersectGreatCircles(size_t circle1, size_t circle2);
	/// Gets the rotation of a point to the frame of reference of a great circle
	std::vector<GlobeVector> *getGreatCircles();
	/// Gets a pointer to the list of intersections
	std::map<std::pair<size_t, size_t>, GlobeVector> *getIntersections();
	/// Gets the list of globe sections
	std::vector<GlobeSection> *getGlobeSections();
	/// Gets the list of globe sections added by the latest great circle intersections
	std::vector<GlobeSection> *getNewSections();
	/// Gets the ordered list of texture ids to paint on the globe by altitude
	std::vector<int> *getTexturesByAltitude();
	/// Gets the ordered list of texture ids to paint around the poles going outwards
	std::vector<int> *getTexturesForPoles();
	/// Saves the result of the geoscape generator
	void save() const;
	/// Saves information and throws an exception in case of an error.
	void error(Exception errorMsg) const;

};

}
