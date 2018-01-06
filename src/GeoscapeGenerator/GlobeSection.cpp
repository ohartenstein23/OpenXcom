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

GlobeSection::GlobeSection(GeoscapeGenerator *parent) : _parent(parent), _heightIndex(0), _textureId(-1)
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

// Gets a pointer to the list of polygon vertices
// Used after the first pass of the generator instead of the _intersections
std::vector<GlobeVector> *GlobeSection::getPolygonVertices()
{
	return &_polygonVertices;
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

// Gets the textureId of the section
int GlobeSection::getTextureId()
{
	return _textureId;
}

// Sets the heightIndex of the section
void GlobeSection::setTextureId(int textureId)
{
	_textureId = textureId;
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
	// Create a temporary new GlobeSection in case we split the current one
	GlobeSection *newSection = new GlobeSection(_parent);

	// If this is one of the first two sections, handle as a special case
	if (_intersections.size() == 0)
	{
		_intersections.push_back(std::make_pair(std::make_pair(0, 1), 1));
		_intersections.push_back(std::make_pair(std::make_pair(0, 1), -1));
		_greatCircles.insert(std::make_pair(1, 1));

		newSection->getIntersections()->push_back(std::make_pair(std::make_pair(0, 1), 1));
		newSection->getIntersections()->push_back(std::make_pair(std::make_pair(0, 1), -1));
		newSection->getGreatCircles()->insert(std::make_pair(0, _greatCircles[0]));
		newSection->getGreatCircles()->insert(std::make_pair(1, -1));

		_parent->getNewSections()->push_back(*newSection);
		delete newSection;
		return;
	}

	// Loop over this section's intersections, determining whether they're over or under the newest circle
	GlobeVector normalVector = _parent->getGreatCircles()->at(circleIndex);
	std::vector<std::pair<std::pair<size_t, size_t>, int>> intersectionsOver, intersectionsUnder;
	intersectionsOver.clear();
	intersectionsUnder.clear();
	for (std::vector<std::pair<std::pair<size_t, size_t>, int>>::iterator i = _intersections.begin(); i != _intersections.end(); ++i)
	{
		// Get the vector pointing to the intersection
		GlobeVector intersectionVector = (_parent->getIntersections()->find((*i).first)->second * (*i).second);

		// Dot product test for over or under the great circle - if the dot of the point we're testing and
		// the normal vector of the great circle is greater than 0, this means the point is 'over' the circle
		// i.e. closer to the direction of the normal vector than the inverse of the normal vector
		if (intersectionVector.dot(normalVector) < 0)
		{
			// If the point on the section's boundary is 'under' the circle, then it should go to a new section
			intersectionsUnder.push_back((*i));
		}
		else
		{
			intersectionsOver.push_back((*i));
		}
	}

	// If both intersectionsUnder and intersectionsOver have points, then we need to split this globe section
	// This means we need to find where on the boundary this circle intersects the section, and put these points in both of the sections
	if (intersectionsOver.size() != 0 && intersectionsUnder.size() != 0)
	{
		// Start by using the intersections to determine which great circles go where
		// As we're iterating over them, put the intersections into the two sections
		std::map<size_t, int> keepCircles;
		keepCircles.clear();
		std::map<size_t, int>::iterator it;
		_intersections.clear();
		for (std::vector<std::pair<std::pair<size_t, size_t>, int>>::iterator i = intersectionsOver.begin(); i != intersectionsOver.end(); ++i)
		{
			_intersections.push_back((*i));
			it = _greatCircles.find((*i).first.first);
			keepCircles.insert(*it);
			it = _greatCircles.find((*i).first.second);
			keepCircles.insert(*it);
		}

		for (std::vector<std::pair<std::pair<size_t, size_t>, int>>::iterator i = intersectionsUnder.begin(); i != intersectionsUnder.end(); ++i)
		{
			newSection->getIntersections()->push_back((*i));
			it = _greatCircles.find((*i).first.first);
			newSection->getGreatCircles()->insert(*it);
			it = _greatCircles.find((*i).first.second);
			newSection->getGreatCircles()->insert(*it);
		}

		std::vector<size_t> intersectingCircles;
		intersectingCircles.clear();
		// If a circle is included in both the current and the new section, then it intersects the new circle we're looking at
		for (std::map<size_t, int>::iterator i = keepCircles.begin(); i != keepCircles.end(); ++i)
		{
			it = newSection->getGreatCircles()->find((*i).first);
			if (it != newSection->getGreatCircles()->end())
			{
				intersectingCircles.push_back((*i).first);
			}
		}

		// Determine which direction of the new intersections is on the boundary
		for (std::vector<size_t>::iterator i = intersectingCircles.begin(); i != intersectingCircles.end(); ++i)
		{
			int intersectionParity = 1;
			GlobeVector candidateIntersection;
			candidateIntersection = _parent->getIntersections()->find(std::make_pair((*i), circleIndex))->second;

			for (std::map<size_t, int>::iterator j = _greatCircles.begin(); j != _greatCircles.end(); ++j)
			{
				if ((*j).first == (*i)) // Don't check the circle we're using for the intersection
					continue;

				GlobeVector testNormal = (_parent->getGreatCircles()->at((*j).first) * (*j).second);
				if (candidateIntersection.dot(testNormal) < 0)
				{
					intersectionParity = -1;
					break;
				}
			}

			_intersections.push_back(std::make_pair(std::make_pair((*i), circleIndex), intersectionParity));
			newSection->getIntersections()->push_back(std::make_pair(std::make_pair((*i), circleIndex), intersectionParity));
		}

		// Now that we're using _greatCircles for the proper data, copy keepCircles to the current section
		_greatCircles.clear();
		_greatCircles = keepCircles;
		_greatCircles[circleIndex] = 1;

		newSection->getGreatCircles()->insert(std::make_pair(circleIndex, -1));
		newSection->setHeightIndex(_heightIndex);
		++_heightIndex;

		_parent->getNewSections()->push_back(*newSection);
	}
	// If we don't need to split, then increase the height of this section if it was over the circle
	else if (intersectionsUnder.size() == 0)
	{
		++_heightIndex;
	}

	delete newSection;
}

// Returns the center coordinates of this globe section
GlobeVector GlobeSection::getCenterCoordinates()
{
	return _centerCoordinates;
}

// Calculate the center coordinate from the mean of the intersections
void GlobeSection::setCenterCoordinates()
{
	if (_intersections.size() == 0)
		throw Exception("GlobeSection.cpp: cannot calculate center coordinate if no intersections exist.");

	double x = 0, y = 0, z = 0;
	for (std::vector<std::pair<std::pair<size_t, size_t>, int>>::iterator i = _intersections.begin(); i != _intersections.end(); ++i)
	{
		GlobeVector currentIntersection = (_parent->getIntersections()->find((*i).first)->second * (*i).second);
		x += currentIntersection.x;
		y += currentIntersection.y;
		z += currentIntersection.z;
	}

	_centerCoordinates = GlobeVector(x, y, z); // constructor automatically normalizes length of vector to 1
}

// Splits this section into sections with 3 or 4 intersections if it's too large or has too many intersections for a globe polygon
void GlobeSection::splitIntoPolygons()
{
	// Make sure we have a center coordinate
	if (_centerCoordinates.x == 0 && _centerCoordinates.y == 0 && _centerCoordinates.z == 0) // default constructor for GlobeVector initializes to these
		setCenterCoordinates();

	// Check to see if we need to split this section
	if (_intersections.size() < 5)
	{
		GlobeSection *newSection = new GlobeSection(_parent);
		for (std::vector<std::pair<std::pair<size_t, size_t>, int>>::iterator i = _intersections.begin(); i != _intersections.end(); ++i)
		{
			GlobeVector currentIntersection = (_parent->getIntersections()->find((*i).first)->second * (*i).second);
			newSection->getPolygonVertices()->push_back(currentIntersection);
		}
		newSection->setHeightIndex(_heightIndex);
		_parent->getNewSections()->push_back(*newSection);
		delete newSection;
		return;
	}

	// Sort the intersections counter clockwise around the center coordinate
	std::vector<std::pair<size_t, double>> sortIndex;
	sortIndex.clear();
	size_t intersectionIndex = 0;

	for (std::vector<std::pair<std::pair<size_t, size_t>, int>>::iterator i = _intersections.begin(); i != _intersections.end(); ++i)
	{
		GlobeVector currentIntersection = (_parent->getIntersections()->find((*i).first)->second * (*i).second);

		// Rotate all the intersections such that the center intersection would lie along the z axis - this makes it possible to just sort by longitude
		currentIntersection = currentIntersection.rotate((GlobeVector(0, 0, 1) * _centerCoordinates), _centerCoordinates.lat);
		sortIndex.push_back(std::make_pair(intersectionIndex, currentIntersection.lon));
		++intersectionIndex;
	}

	// Sort by the longitudes of the rotated vectors
	std::sort(sortIndex.begin(), sortIndex.end(), [](std::pair<size_t, double> index1, std::pair<size_t, double> index2) {return index1.second < index2.second; });
	std::vector<GlobeVector> sortedIntersections;
	sortedIntersections.clear();
	for (std::vector<std::pair<size_t, double>>::iterator i = sortIndex.begin(); i != sortIndex.end(); ++i)
	{
		GlobeVector currentIntersection = (_parent->getIntersections()->find(_intersections.at((*i).first).first)->second * _intersections.at((*i).first).second);
		sortedIntersections.push_back(currentIntersection);
	}

	// Now that we have the sorted intersections, split the section by making tris and quads using the center coordinates and points along the perimeter of the section
	sortedIntersections.push_back(sortedIntersections.at(0));
	size_t startIndex = 0;
	size_t endIndex = 1;
	while (startIndex < sortedIntersections.size() - 1)
	{
		// Determine whether to make a tri or a quad by the angle made between the start point on the section, the center coordinate, and the end point
		// If it's too small, add one more point
		if (endIndex < sortedIntersections.size() - 1) // can only do this if we've not ran out of points
		{
			GlobeVector normalVector1 = sortedIntersections.at(startIndex) * _centerCoordinates;
			GlobeVector normalVector2 = _centerCoordinates * sortedIntersections.at(endIndex);
			double angle = normalVector1.distance(normalVector2); // measured in radians
			if (angle < (M_PI / 6)) // yes, I didn't want to include my conversions here, big deal. pi/6 = 30 degrees
				++endIndex;
		}

		GlobeSection *newSection = new GlobeSection(_parent);
		newSection->getPolygonVertices()->push_back(_centerCoordinates);
		for (size_t i = startIndex; i != endIndex + 1; ++i)
		{
			newSection->getPolygonVertices()->push_back(sortedIntersections.at(i));
		}

		newSection->setHeightIndex(_heightIndex);
		_parent->getNewSections()->push_back(*newSection);
		delete newSection;

		startIndex = endIndex;
		++endIndex;
	}	
}

}
