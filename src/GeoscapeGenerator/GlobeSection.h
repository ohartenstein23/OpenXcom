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
	int _heightIndex;
	GlobeVector _centerCoordinates;

public:
	/// Constructor
	GlobeSection(GeoscapeGenerator *parent);
	/// Cleans up the GlobeSection.
	~GlobeSection();

	/// Gets a pointer to the map of great circles
	std::map<size_t, int> *getGreatCircles();
	/// Gets a pointer to the list of intersections
	std::vector<std::pair<std::pair<size_t, size_t>, int>> *getIntersections();
	/// Gets the heightIndex of the section
	int getHeightIndex();
	/// Sets the heightIndex of the section
	void setHeightIndex(int heightIndex);
	/// Intersects a great circle with this globe section
	void intersectWithGreatCircle(size_t circleIndex);

};

}
