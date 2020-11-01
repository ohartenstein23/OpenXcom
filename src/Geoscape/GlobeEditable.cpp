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
#include "GlobeEditable.h"
#include <algorithm>
#include "../fmath.h"
#include "../Engine/Action.h"
#include "../Engine/SurfaceSet.h"
#include "../Engine/Timer.h"
#include "../Mod/Mod.h"
#include "../Mod/Polygon.h"
#include "../Mod/Polyline.h"
#include "../Engine/FastLineClip.h"
#include "../Engine/Game.h"
#include "../Engine/RNG.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/GameTime.h"
#include "../Mod/RuleCountry.h"
#include "../Interface/Text.h"
#include "../Engine/LocalizedText.h"
#include "../Mod/RuleRegion.h"
#include "../Savegame/Region.h"
#include "../Mod/City.h"
#include "../Engine/ShaderMove.h"
#include "../Engine/ShaderRepeat.h"
#include "../Engine/Options.h"
#include "../Engine/Language.h"
#include "../Mod/RuleGlobe.h"
#include "../Mod/Texture.h"
#include "../Interface/Cursor.h"
#include "../Engine/Screen.h"

namespace OpenXcom
{

// pre-globe material cut from here

/**
 * Sets up a globe with the specified size and position, capable of being edited
 * @param game Pointer to core game.
 * @param cenX X position of the center of the globe.
 * @param cenY Y position of the center of the globe.
 * @param width Width in pixels.
 * @param height Height in pixels.
 * @param x X position in pixels.
 * @param y Y position in pixels.
 */
GlobeEditable::GlobeEditable(Game* game, int cenX, int cenY, int width, int height, int x, int y) : Globe(game, cenX, cenY, width, height, x, y), _game(game), _cenX(cenX), _cenY(cenY), _rotLon(0.0), _rotLat(0.0), _blink(-1),
	_isMouseScrolling(false), _isMouseScrolled(false), _xBeforeMouseScrolling(0), _yBeforeMouseScrolling(0), _lonBeforeMouseScrolling(0.0), _latBeforeMouseScrolling(0.0), _mouseScrollingStartTime(0), _totalMouseMoveX(0), _totalMouseMoveY(0), _mouseMovedOverThreshold(false)
{
	//_rules = game->getMod()->getGlobe();
	//_texture = new SurfaceSet(*_game->getMod()->getSurfaceSet("TEXTURE.DAT"));
	//_markerSet = _game->getMod()->getSurfaceSet("GlobeMarkers");

	//_countries = new Surface(width, height, x, y);
	//_markers = new Surface(width, height, x, y);
	//_radars = new Surface(width, height, x, y);
	//_clipper = new FastLineClip(x, x+width, y, y+height);

	// Animation timers
	//_blinkTimer = new Timer(100);
	//_blinkTimer->onTimer((SurfaceHandler)&Globe::blink);
	//_blinkTimer->start();
	//_rotTimer = new Timer(10);
	//_rotTimer->onTimer((SurfaceHandler)&Globe::rotate);

	//_cenLon = _game->getSavedGame()->getGlobeLongitude();
	//_cenLat = _game->getSavedGame()->getGlobeLatitude();
	//_zoom = _game->getSavedGame()->getGlobeZoom();
	//_zoomOld = _zoom;

	//cachePolygons();

    _drawShade = false;
}

/**
 * Deletes the contained surfaces.
 */
GlobeEditable::~GlobeEditable()
{
	//delete _blinkTimer;
	//delete _rotTimer;
	//delete _countries;
	//delete _markers;
	//delete _texture;
	//delete _radars;
	//delete _clipper;

	//for (std::list<Polygon*>::iterator i = _cacheLand.begin(); i != _cacheLand.end(); ++i)
	//{
	//	delete *i;
	//}
}

/**
 * Keeps the animation timers running.
 */
void GlobeEditable::think()
{
	Globe::think();
}

/**
 * Draws the whole globe, part by part.
 * @param drawShade whether or not to draw the shade
 */
void GlobeEditable::draw()
{
	if (_redraw)
	{
		Globe::cachePolygons();
	}
	Surface::draw();

    if (_drawShade)
    {
        Globe::drawOcean();
    }
    else
    {
        Globe::drawOceanNoShade();
    }

	Globe::drawLand();
	//drawRadars();
	//drawFlights();
    if (_drawShade)
    {
    	Globe::drawShadow();
    }
	Globe::drawMarkers();
    Globe::drawPolygonMarkers();
	Globe::drawDetail();
}

/**
 * Sets whether or not we're going to draw the shade on the globe
 * @param drawShade whether to draw shade
 */
void GlobeEditable::setDrawShade(bool drawShade)
{
    _drawShade = drawShade;
}

}
