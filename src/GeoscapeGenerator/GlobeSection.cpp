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
#include <cmath>
#include "../fmath.h"
#include "GeoscapeGenerator.h"
#include "../Engine/Logger.h"

namespace OpenXcom
{

GlobeSection::GlobeSection(GeoscapeGenerator *parent) : _parent(parent), _heightIndex(0)
{

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

// Gets a pointer to the list of intersections
// This references which two great circles form the intersection and whether the intersection
// is the GlobeVector or the inverted GlobeVector (-1 * Globe Vector)
std::vector<std::pair<std::pair<size_t, size_t>, int>> *GlobeSection::getIntersections()
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
	std::vector<std::pair<std::pair<size_t, size_t>, int>> confirmedIntersections;

	// Loop over this section's great circles, finding intersections with the new circle that lie on the boundary
	for (std::map<size_t, int>::iterator i = _greatCircles.begin(); i != _greatCircles.end(); ++i)
	{
		// Get the intersections that correspond to the current pair of circles
		std::pair<size_t, size_t> currentCircles((*i).first, circleIndex);
		// If this is the first set of sections, we have no intersections yet, so add the only ones to the map
		if (_intersections.size() == 0)
		{
			confirmedIntersections.push_back(std::make_pair(currentCircles, 1));
			confirmedIntersections.push_back(std::make_pair(currentCircles, -1));
			break;
		}

		GlobeVector candidateIntersection;
		std::map<std::pair<size_t, size_t>, GlobeVector>::iterator it = _parent->getIntersections()->find(currentCircles);
		if (it != _parent->getIntersections()->end())
		{
			candidateIntersection = (*it).second;
		}
		else
		{
			// Something went horribly wrong, error handling function goes here
			Log(LOG_ERROR) << "GlobeSection.cpp: Something went horribly wrong, no candidateIntersection found.";
			Log(LOG_ERROR) << "   circleIndex = " << circleIndex << ", current section's circle = " << (*i).first;
			_parent->error();
		}
	

		// Loop over the intersections that define this section, getting which two circles intersect with the current one we're looking at
		std::vector<size_t> outerCircles;
		outerCircles.clear();
		for (std::vector<std::pair<std::pair<size_t, size_t>, int>>::iterator j = _intersections.begin(); j != _intersections.end(); ++j)
		{
			if ((*j).first.first == currentCircles.first)
			{
				outerCircles.push_back((*j).first.second);
			}
		
			if ((*j).first.second == currentCircles.first)
			{
				outerCircles.push_back((*j).first.first);
			}
		
			if (outerCircles.size() == 2)
				break;
		}

		if (outerCircles.size() != 2)
		{
			Log(LOG_ERROR) << "GlobeSection.cpp: Don't know how this happened, but can't find the circles that frame this intersection.";
			for (auto& j : outerCircles)
			{
				Log(LOG_ERROR) << "   " << j;
			}
			_parent->error();
		}
	
		// Test the two candidate intersections, determining whether they lie on the perimeter of this section
		int direction[2] = {-1, 1};
		bool testIntersections[2] = {true, true};
		for (size_t j = 0; j < 2; ++j) // loop over inverted intersection and intersection
		{
			for (size_t k = 0; k < 2; ++k) // loop over the two circles for testing
			{
				GlobeVector testIntersection = candidateIntersection * direction[j];
				GlobeVector coordinateReference(0, 0, 1); // Our reference frame is to the great circle with normal along the z direction
				testIntersection.rotate(coordinateReference * _parent->getGreatCircles()->at(outerCircles.at(k)), _parent->getGreatCircles()->at(outerCircles.at(k)).lat);
			
				// Find the testing circle in the list and get whether this section is above or below it
				int parity = 1;
				std::map<size_t, int>::iterator it = _greatCircles.find(outerCircles.at(k));
				if (it != _greatCircles.end())
				{
					parity = (*it).second;
				}
				else
				{
					// Don't know how this happened, but handle error here
					Log(LOG_ERROR) << "GlobeSection.cpp: Don't know how we got here, but couldn't find a great circle on it's own section!";
					Log(LOG_ERROR) << "   j = " << j << ", k = " << k;
					_parent->error();
				}
			
				testIntersections[j] == testIntersections[j] && ((testIntersection.z * parity) > 0);
			}
		
			if (testIntersections[j]) // We found the point, only one of the two can be valid
			{
				confirmedIntersections.push_back(std::make_pair(currentCircles, direction[j]));
				break;
			}
		}
	
		// If we've found two confirmed intersections, that's all that can intersect this section
		if (confirmedIntersections.size() == 2)
		{
			Log(LOG_INFO) << "Intersection 1: (" << confirmedIntersections.at(0).first.first << ", " << confirmedIntersections.at(0).first.second << ", " << confirmedIntersections.at(0).second << ")";
			Log(LOG_INFO) << "Intersection 2: (" << confirmedIntersections.at(1).first.first << ", " << confirmedIntersections.at(1).first.second << ", " << confirmedIntersections.at(1).second << ")";
			break;
		}
	}
	
	// If we've found intersection points, that means we need to create a new section
	if (confirmedIntersections.size() != 0)
	{
		GlobeSection *newSection = new GlobeSection(_parent);
		std::vector<std::pair<std::pair<size_t, size_t>, int>> keepIntersections;
		std::map<size_t, int> keepCircles;
		
		// Split this section's data between it and the new section
		if (_intersections.size() != 0)
		{
			for (std::vector<std::pair<std::pair<size_t, size_t>, int>>::iterator i = _intersections.begin(); i != _intersections.end(); ++i)
			{
				const std::pair<size_t, size_t> circles = (*i).first;
				std::map<std::pair<size_t, size_t>, GlobeVector>::const_iterator it = _parent->getIntersections()->find(circles);
				GlobeVector testIntersection = (*it).second * (*i).second;
				GlobeVector coordinateReference(0, 0, 1);
			
				testIntersection.rotate(coordinateReference * _parent->getGreatCircles()->at(circleIndex), _parent->getGreatCircles()->at(circleIndex).lat);
				std::map<size_t, int>::iterator jt = _greatCircles.find((*i).first.first);
				std::map<size_t, int>::iterator kt = _greatCircles.find((*i).first.second);
				if (testIntersection.z > 0)
				{
					keepIntersections.push_back(*i);
				
					if (jt != _greatCircles.end())
						keepCircles.insert((*jt)); // Only adds the data if it wasn't already there
					if (kt != _greatCircles.end())
						keepCircles.insert((*kt));				
				}
				else
				{
					newSection->getIntersections()->push_back(*i);
				
					if (jt != _greatCircles.end())
						newSection->getGreatCircles()->insert((*jt));
					if (kt != _greatCircles.end())
						newSection->getGreatCircles()->insert((*kt));
				}
			}
		
			// Push the confirmed intersections back to both this and new section
			for(std::vector<std::pair<std::pair<size_t, size_t>, int>>::iterator i = confirmedIntersections.begin(); i != confirmedIntersections.end(); ++i)
			{
				keepIntersections.push_back((*i));
				newSection->getIntersections()->push_back(*i);
			}
			_intersections.clear();
			_intersections = keepIntersections;
		
			// Add the information about the new great circle
			keepCircles.insert(std::make_pair(circleIndex, 1));
			newSection->getGreatCircles()->insert(std::make_pair(circleIndex, -1));
			_greatCircles.clear();
			_greatCircles = keepCircles;
		}
		else // special case of first intersections
		{
			// Push the confirmed intersections back to both this and new section
			for(std::vector<std::pair<std::pair<size_t, size_t>, int>>::iterator i = confirmedIntersections.begin(); i != confirmedIntersections.end(); ++i)
			{
				keepIntersections.push_back((*i));
				newSection->getIntersections()->push_back(*i);
			}
			_intersections.clear();
			_intersections = keepIntersections;
		
			// Add the information about the new great circle
			newSection->getGreatCircles()->insert(std::make_pair(0, _greatCircles[0]));
			newSection->getGreatCircles()->insert(std::make_pair(circleIndex, -1));
			_greatCircles.insert(std::make_pair(circleIndex, 1));
		}
		
		// Change the height information
		newSection->setHeightIndex(_heightIndex);
		++_heightIndex;
		
		// Push the new section to the generator's list of new sections
		_parent->getNewSections()->push_back(*newSection);

		delete newSection;
	}
	else // No intersection, only need to check if we're above or below the circle
	{
		const std::pair<size_t, size_t> circles = _intersections.at(0).first; // only need to check one point, since all should be either above or below
		std::map<std::pair<size_t, size_t>, GlobeVector>::const_iterator it = _parent->getIntersections()->find(circles);
		GlobeVector testIntersection = (*it).second * _intersections.at(0).second;
		GlobeVector coordinateReference(0, 0, 1);
		
		testIntersection.rotate(coordinateReference * _parent->getGreatCircles()->at(circleIndex), _parent->getGreatCircles()->at(circleIndex).lat);
		if (testIntersection.z > 0)
		{
			++_heightIndex;
		}
	}
}

}
