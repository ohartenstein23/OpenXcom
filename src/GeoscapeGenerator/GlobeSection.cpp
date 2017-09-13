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

#include "GlobeSection.h"
#include <algorithm>
#include "GeoscapeGenerator.h"
#include "../Engine/Logger.h"

namespace OpenXcom
{

GlobeSection::GlobeSection(GeoscapeGenerator *parent) : _parent(parent), _heightIndex(0)
{
	_greatCircles.clear();
	_intersections.clear();
}

// Cleans up the GlobeSection
GlobeSection::~GlobeSection()
{
	// delete variables to clean up here
}

// Gets a pointer to the map of great circles
std::map<size_t, int> *GlobeSection::getGreatCircles()
{
	return &_greatCircles;
}

// Gets a pointer to the list of intersection indexes
std::map<std::map<std::pair<size_t, size_t>, std::vector<double>>::iterator, size_t> *GlobeSection::getIntersections()
{
	return &_intersections;
}

// Gets the heightIndex of the section
int GlobeSection::getHeightIndex()
{
	return _heightIndex;
}

// Sets the heightIndex of the section
void GlobeSection::setHeightIndex(int heightIndex)
{
	_heightIndex = heightIndex;
	return;
}

/** Intersects a great circle with this globe section
 * Adds intersections and circle references as necessary, splits the section into
 * two and creates a new one if the circle intersects. Otherwise just changes the
 * height index
 * @param circleIndex The index of the great circle
 */
void GlobeSection::intersectWithGreatCircle(size_t circleIndex)
{
	std::map<std::map<std::pair<size_t, size_t>, std::vector<double>>::iterator, size_t> confirmedIntersections;

	// Loop over this section's great circles, finding intersections with the new circle that lie on the boundary
	for (std::map<size_t, int>::iterator i = _greatCircles.begin(); i != _greatCircles.end(); ++i)
	{
		// Get the pair of intersections that correspond to the current pair of circles
		std::pair<size_t, size_t> currentCircles((*i).first, circleIndex);
		std::map<std::pair<size_t, size_t>, std::vector<double>>::iterator candidateIntersections = _parent->getIntersections()->find(currentCircles);
		if (it == _parent->getIntersections()->end())
			{}// Something went horribly wrong, error handling function goes here

		
	}
}

}
