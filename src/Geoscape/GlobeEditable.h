#pragma once
/*
 * Copyright 2010-2020 OpenXcom Developers.
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
#include "../Engine/InteractiveSurface.h"
#include "../Engine/FastLineClip.h"
#include "Cord.h"
#include "Globe.h"

namespace OpenXcom
{

class Game;
class Polygon;
class SurfaceSet;
class Timer;
class Target;
class LocalizedText;
class RuleGlobe;

/**
 * Interactive globe view of the world.
 * Takes a flat world map made out of land polygons with
 * polar coordinates and renders it as a 3D-looking globe
 * with cartesian coordinates that the player can interact with.
 */
class GlobeEditable : public Globe
{
private:

    Game *_game;
	RuleGlobe *_rules;
	Sint16 _cenX, _cenY;
	double _cenLon, _cenLat, _rotLon, _rotLat;
	int _blink;

	bool _isMouseScrolling, _isMouseScrolled;
	int _xBeforeMouseScrolling, _yBeforeMouseScrolling;
	double _lonBeforeMouseScrolling, _latBeforeMouseScrolling;
	Uint32 _mouseScrollingStartTime;
	int _totalMouseMoveX, _totalMouseMoveY;
	bool _mouseMovedOverThreshold;

    bool _drawShade;

public:
	static Uint8 OCEAN_COLOR;
	static bool OCEAN_SHADING;
	static Uint8 COUNTRY_LABEL_COLOR;
	static Uint8 LINE_COLOR;
	static Uint8 CITY_LABEL_COLOR;
	static Uint8 BASE_LABEL_COLOR;

	/// Creates a new globe at the specified position and size.
	GlobeEditable(Game* game, int cenX, int cenY, int width, int height, int x = 0, int y = 0);
	/// Cleans up the globe.
	~GlobeEditable();
	/// Handles the timers.
	void think() override;
    /// Draws the whole globe, part by part (overrides Globe::draw)
    void draw() override;

    /// Sets whether or not we're going to draw the shade on the globe
    void setDrawShade(bool drawShade);
};

}
