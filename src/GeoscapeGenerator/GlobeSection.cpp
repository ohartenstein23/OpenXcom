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

#include "GlobeSection.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>
#include "../fmath.h"
#include "GeoscapeGenerator.h"
#include "../Engine/Exception.h"
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
	// If this is one of the first two sections, handle as a special case
	if (_intersections.size() == 0)
	{
		_intersections.push_back(std::make_pair(std::make_pair(0, 1), 1));
		_intersections.push_back(std::make_pair(std::make_pair(0, 1), -1));
		_greatCircles.insert(std::make_pair(1, 1));

		GlobeSection *newSection = new GlobeSection(_parent);
		newSection->getIntersections()->push_back(std::make_pair(std::make_pair(0, 1), 1));
		newSection->getIntersections()->push_back(std::make_pair(std::make_pair(0, 1), -1));
		newSection->getGreatCircles()->insert(std::make_pair(0, _greatCircles[0]));
		newSection->getGreatCircles()->insert(std::make_pair(1, -1));

		_parent->getNewSections()->push_back(*newSection);
		delete newSection;
		return;
	}

	std::vector<std::pair<std::pair<size_t, size_t>, int>> confirmedIntersections;

	// Loop over this section's great circles, finding intersections with the new circle that lie on the boundary
	for (std::map<size_t, int>::iterator i = _greatCircles.begin(); i != _greatCircles.end(); ++i)
	{
		// Get the intersections that correspond to the current pair of circles
		std::pair<size_t, size_t> currentCircles((*i).first, circleIndex);

		GlobeVector candidateIntersection;
		std::map<std::pair<size_t, size_t>, GlobeVector>::iterator it = _parent->getIntersections()->find(currentCircles);
		if (it != _parent->getIntersections()->end())
		{
			candidateIntersection = (*it).second;
		}
		else
		{
			std::ostringstream ss;
			ss << "GlobeSection.cpp: no candidateIntersection found for circle pair (" << currentCircles.first << ", " << currentCircles.second << ").";
			throw Exception(ss.str());
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
			std::ostringstream ss;
			ss << "GlobeSection.cpp: No bounding circles found.\n currentCircles = (" << currentCircles.first << ", " << currentCircles.second << ")\n";
			ss << " outerCircles = ";
			for (auto i : outerCircles)
			{
				ss << i << " ";
			}
			throw Exception(ss.str());
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
				GlobeVector rotationAxis = coordinateReference * _parent->getGreatCircles()->at(outerCircles.at(k));
				testIntersection = testIntersection.rotate(rotationAxis, _parent->getGreatCircles()->at(outerCircles.at(k)).lat);

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
					std::ostringstream ss;
					ss << "GlobeSection.cpp: Don't know how we got here, but couldn't find a great circle on it's own section!\n" << "   j = " << j << ", k = " << k;
					throw Exception(ss.str());
				}

				testIntersections[j] = testIntersections[j] && ((testIntersection.z * parity) > 0);
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
			break;
		}
	}
	
	// If we've found a pair of intersection points, that means we need to create a new section
	if (confirmedIntersections.size() == 2)
	{
		GlobeSection *newSection = new GlobeSection(_parent);
		std::vector<std::pair<std::pair<size_t, size_t>, int>> keepIntersections;
		std::map<size_t, int> keepCircles;
		
		// Split this section's data between it and the new section
		for (std::vector<std::pair<std::pair<size_t, size_t>, int>>::iterator i = _intersections.begin(); i != _intersections.end(); ++i)
		{
			const std::pair<size_t, size_t> circles = (*i).first;
			std::map<std::pair<size_t, size_t>, GlobeVector>::const_iterator it = _parent->getIntersections()->find(circles);
			if (it == _parent->getIntersections()->end())
			{
				std::ostringstream ss;
				ss << "GlobeSection.cpp: Could not find intersection in parent, circles = (" << circles.first << ", " << circles.second << ")";
				throw Exception(ss.str());
			}
			GlobeVector testIntersection = (*it).second * (*i).second;
			GlobeVector coordinateReference(0, 0, 1);
			Log(LOG_INFO) << "Testing intersection";
			testIntersection.writeToLog();
			_parent->getGreatCircles()->at(circleIndex).writeToLog();

			GlobeVector rotationAxis = coordinateReference * _parent->getGreatCircles()->at(circleIndex);
			rotationAxis.writeToLog();
			testIntersection = testIntersection.rotate(rotationAxis, _parent->getGreatCircles()->at(circleIndex).lat);
			std::map<size_t, int>::iterator jt = _greatCircles.find((*i).first.first);
			std::map<size_t, int>::iterator kt = _greatCircles.find((*i).first.second);
			testIntersection.writeToLog();
			if (testIntersection.z > 0)
			{
				keepIntersections.push_back(*i);
			
				if (jt != _greatCircles.end())
					keepCircles.insert((*jt)); // Only adds the data if it wasn't already there
				if (kt != _greatCircles.end())
					keepCircles.insert((*kt));
				Log(LOG_INFO) << "Kept";				
			}
			else if (testIntersection.z < 0)
			{
				newSection->getIntersections()->push_back(*i);
			
				if (jt != _greatCircles.end())
					newSection->getGreatCircles()->insert((*jt));
				if (kt != _greatCircles.end())
					newSection->getGreatCircles()->insert((*kt));
				Log(LOG_INFO) << "Sent";
			}
			else
			{
				throw Exception("GlobeVector.cpp: Trying to split intersection found on newest circle!");
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
		
		testIntersection = testIntersection.rotate(coordinateReference * _parent->getGreatCircles()->at(circleIndex), _parent->getGreatCircles()->at(circleIndex).lat);
		if (testIntersection.z > 0)
		{
			++_heightIndex;
		}
	}
}

}
