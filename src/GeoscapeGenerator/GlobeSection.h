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

class GeoscapeGenerator;

class GlobeSection
{
private:
	GeoscapeGenerator *_parent;
	std::map<size_t, int> _greatCircles;
	std::vector<std::pair<std::pair<size_t, size_t>, int>> _intersections;
	int _heightIndex, _textureId;
	GlobeVector _centerCoordinates;
	std::vector<GlobeVector> _polygonVertices;

public:
	/// Constructor
	GlobeSection(GeoscapeGenerator *parent);
	/// Cleans up the GlobeSection.
	~GlobeSection();

	/// Gets a pointer to the map of great circles
	std::map<size_t, int> *getGreatCircles();
	/// Gets a pointer to the list of intersections
	std::vector<std::pair<std::pair<size_t, size_t>, int>> *getIntersections();
	/// Gets a pointer to the list of polygon vertices
	std::vector<GlobeVector> *getPolygonVertices();
	/// Gets the heightIndex of the section
	int getHeightIndex();
	/// Sets the heightIndex of the section
	void setHeightIndex(int heightIndex);
	/// Gets the textureId of the section
	int getTextureId();
	/// Sets the textureId of the section
	void setTextureId(int textureId);
	/// Intersects a great circle with this globe section
	void intersectWithGreatCircle(size_t circleIndex);
	/// Gets the center coordinates of the section
	GlobeVector getCenterCoordinates();
	/// Sets the center coordinate of the section from its intersections
	void setCenterCoordinates();
	/// Splits the section into polygons for saving as a globe ruleset
	void splitIntoPolygons();

};

}
