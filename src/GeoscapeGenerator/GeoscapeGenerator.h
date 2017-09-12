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

#include <vector>
#include <list>
#include <map>

namespace OpenXcom
{

class GlobeSection;

struct GreatCircleIntersection
{
	std::pair<double, double> coordinates;
	std::pair<size_t, size_t> circles;

	/// Default constructor
	GreatCircleIntersection() : coordinates{}, circles{}
		{ }

	// Constructor initializing the variables
	GreatCircleIntersection(std::pair<double, double> coords, std::pair<size_t, size_t> circs) : coordinates(coords), circles(circs)
		{ }
};

class GeoscapeGenerator
{
private:
	static const double RAD_TO_DEG;
	static const double DEG_TO_RAD;

	uint64_t _rngSeed;
	size_t _numberOfCircles;
	std::vector<std::pair<double, double>> _greatCircles; // toil and trouble
	std::vector<GreatCircleIntersection> _intersections;
	std::vector<GlobeSection*> _globeSections, _newSections;
public:
	/// Constructor
	GeoscapeGenerator(uint64_t rngSeed, size_t numberOfCircles);
	/// Cleans up the GeoscapeGenerator
	~GeoscapeGenerator();

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
	/// Gets a pointer to the list of great circles
	std::vector<std::pair<double, double>> *getGreatCircles();
	/// Gets a pointer to the list of intersections
	std::vector<GreatCircleIntersection> *getIntersections();
	/// Rotates a point on the globe according to a great circle
	void rotatePointOnSphere(size_t circle, double *latitude, double *longitude);
	/// Gets the list of globe sections
	std::vector<GlobeSection*> *getGlobeSections();
	/// Gets the list of globe sections added by the latest great circle intersections
	std::vector<GlobeSection*> *getNewSections();
	/// Saves the result of the geoscape generator
	void save() const;

};

}
