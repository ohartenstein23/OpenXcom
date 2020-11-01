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
#include "GeoscapeEditorState.h"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <climits>
#include <functional>
#include "../Engine/RNG.h"
#include "../Engine/Game.h"
#include "../Engine/Action.h"
#include "../Mod/Mod.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/Screen.h"
#include "../Engine/Sound.h"
#include "../Engine/Surface.h"
#include "../Engine/Options.h"
#include "../Engine/Collections.h"
#include "../Engine/Unicode.h"
#include "Globe.h"
#include "GlobeEditable.h"
#include "../Interface/ComboBox.h"
#include "../Interface/Text.h"
#include "../Interface/TextButton.h"
#include "../Engine/Timer.h"
#include "../Savegame/GameTime.h"
#include "../Savegame/SavedGame.h"
#include "../Menu/PauseState.h"
#include "SelectMusicTrackState.h"
#include "../Menu/ErrorMessageState.h"
#include "../Savegame/Region.h"
#include "../Savegame/Country.h"
#include "../Mod/RuleCountry.h"
#include "../Mod/RuleAlienMission.h"
#include "../Mod/RuleGlobe.h"
#include "../Engine/Exception.h"
#include "../Mod/RuleInterface.h"
#include "../fmath.h"
#include "../fallthrough.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the Geoscape screen.
 * @param game Pointer to the core game.
 */
GeoscapeEditorState::GeoscapeEditorState() : _pause(true)
{
	int screenWidth = Options::baseXGeoscape;
	int screenHeight = Options::baseYGeoscape;

	// Create objects
	Surface *hd = _game->getMod()->getSurface("ALTGEOBORD.SCR");
	_bg = new Surface(hd->getWidth(), hd->getHeight(), 0, 0);
	_sideLine = new Surface(64, screenHeight, screenWidth - 64, 0);
	_sidebar = new Surface(64, 200, screenWidth - 64, screenHeight / 2 - 100);

	_globe = new GlobeEditable(_game, (screenWidth-64)/2, screenHeight/2, screenWidth-64, screenHeight, 0, 0);
	_bg->setX((_globe->getWidth() - _bg->getWidth()) / 2);
	_bg->setY((_globe->getHeight() - _bg->getHeight()) / 2);

	_btnOptions = new TextButton(63, 11, screenWidth-63, screenHeight/2-52);

    _btnPause = new TextButton(20, 13, screenWidth-63, screenHeight/2+26);
    _btnSlow = new TextButton(20, 13, screenWidth-42, screenHeight/2+26);
    _btnFast = new TextButton(20, 13, screenWidth-21, screenHeight/2+26);
    _btnStop = new TextButton(20, 13, screenWidth-63, screenHeight/2+40);
    _btnReset = new TextButton(20, 13, screenWidth-42, screenHeight/2+40);
    _btnTime = new TextButton(20, 13, screenWidth-21, screenHeight/2+40);

	_btnRotateLeft = new InteractiveSurface(12, 10, screenWidth-61, screenHeight/2+76);
	_btnRotateRight = new InteractiveSurface(12, 10, screenWidth-37, screenHeight/2+76);
	_btnRotateUp = new InteractiveSurface(13, 12, screenWidth-49, screenHeight/2+62);
	_btnRotateDown = new InteractiveSurface(13, 12, screenWidth-49, screenHeight/2+87);
	_btnZoomIn = new InteractiveSurface(23, 23, screenWidth-25, screenHeight/2+56);
	_btnZoomOut = new InteractiveSurface(13, 17, screenWidth-20, screenHeight/2+82);

	int height = (screenHeight - Screen::ORIGINAL_HEIGHT) / 2 + 10;
	_sideTop = new TextButton(63, height, screenWidth-63, _sidebar->getY() - height - 1);
	_sideBottom = new TextButton(63, height, screenWidth-63, _sidebar->getY() + _sidebar->getHeight() + 1);

	_timeSpeed = _btnStop;
	_gameTimer = new Timer(Options::geoClockSpeed);

	_txtDebug = new Text(200, 32, 0, 0);

	// Set palette
	setInterface("geoscape");

	add(_bg);
	add(_sideLine);
	add(_sidebar);
	add(_globe);

	add(_btnOptions, "button", "geoscape");

	add(_btnPause, "button", "geoscape");
	add(_btnSlow, "button", "geoscape");
	add(_btnFast, "button", "geoscape");
	add(_btnStop, "button", "geoscape");
	add(_btnReset, "button", "geoscape");
	add(_btnTime, "button", "geoscape");

	add(_btnRotateLeft);
	add(_btnRotateRight);
	add(_btnRotateUp);
	add(_btnRotateDown);
	add(_btnZoomIn);
	add(_btnZoomOut);

	add(_sideTop, "button", "geoscape");
	add(_sideBottom, "button", "geoscape");

	add(_txtDebug, "text", "geoscape");

	// Set up objects
	Surface *geobord = _game->getMod()->getSurface("GEOBORD.SCR");
	geobord->setX(_sidebar->getX() - geobord->getWidth() + _sidebar->getWidth());
	geobord->setY(_sidebar->getY());
	_sidebar->copy(geobord);
	_game->getMod()->getSurface("ALTGEOBORD.SCR")->blitNShade(_bg, 0, 0);

	_sideLine->drawRect(0, 0, _sideLine->getWidth(), _sideLine->getHeight(), 15);

	_btnOptions->initText(_game->getMod()->getFont("FONT_GEO_BIG"), _game->getMod()->getFont("FONT_GEO_SMALL"), _game->getLanguage());
	_btnOptions->setText(tr("STR_OPTIONS_UC"));
	_btnOptions->onMouseClick((ActionHandler)&GeoscapeEditorState::btnOptionsClick);
	_btnOptions->onKeyboardPress((ActionHandler)&GeoscapeEditorState::btnOptionsClick, Options::keyGeoOptions);
	_btnOptions->setGeoscapeButton(true);

	_btnPause->initText(_game->getMod()->getFont("FONT_GEO_BIG"), _game->getMod()->getFont("FONT_GEO_SMALL"), _game->getLanguage());
	_btnPause->setBig();
	_btnPause->setText("||");
	_btnPause->setGroup(&_timeSpeed);
	_btnPause->onKeyboardPress((ActionHandler)&GeoscapeEditorState::btnTimerClick, Options::keyGeoSpeed1);
	_btnPause->setGeoscapeButton(true);

	_btnSlow->initText(_game->getMod()->getFont("FONT_GEO_BIG"), _game->getMod()->getFont("FONT_GEO_SMALL"), _game->getLanguage());
	_btnSlow->setBig();
	_btnSlow->setText(">");
	_btnSlow->setGroup(&_timeSpeed);
	_btnSlow->onKeyboardPress((ActionHandler)&GeoscapeEditorState::btnTimerClick, Options::keyGeoSpeed2);
	_btnSlow->setGeoscapeButton(true);

	_btnFast->initText(_game->getMod()->getFont("FONT_GEO_BIG"), _game->getMod()->getFont("FONT_GEO_SMALL"), _game->getLanguage());
	_btnFast->setBig();
	_btnFast->setText(">>");
	_btnFast->setGroup(&_timeSpeed);
	_btnFast->onKeyboardPress((ActionHandler)&GeoscapeEditorState::btnTimerClick, Options::keyGeoSpeed3);
	_btnFast->setGeoscapeButton(true);

	_btnStop->initText(_game->getMod()->getFont("FONT_GEO_BIG"), _game->getMod()->getFont("FONT_GEO_SMALL"), _game->getLanguage());
	_btnStop->setBig();
	_btnStop->setText("[]");
	_btnStop->setGroup(&_timeSpeed);
	_btnStop->onKeyboardPress((ActionHandler)&GeoscapeEditorState::btnTimerClick, Options::keyGeoSpeed4);
	_btnStop->setGeoscapeButton(true);

	_btnReset->initText(_game->getMod()->getFont("FONT_GEO_BIG"), _game->getMod()->getFont("FONT_GEO_SMALL"), _game->getLanguage());
	_btnReset->setBig();
	_btnReset->setText("@");
	_btnReset->setGroup(&_timeSpeed);
	_btnReset->onKeyboardPress((ActionHandler)&GeoscapeEditorState::btnTimerClick, Options::keyGeoSpeed5);
	_btnReset->setGeoscapeButton(true);

	_btnTime->initText(_game->getMod()->getFont("FONT_GEO_BIG"), _game->getMod()->getFont("FONT_GEO_SMALL"), _game->getLanguage());
	_btnTime->setBig();
	_btnTime->onKeyboardPress((ActionHandler)&GeoscapeEditorState::btnTimeClick, Options::keyGeoSpeed6);
	_btnTime->setGeoscapeButton(true);

	_sideBottom->setGeoscapeButton(true);
	_sideTop->setGeoscapeButton(true);

	_btnRotateLeft->onMousePress((ActionHandler)&GeoscapeEditorState::btnRotateLeftPress);
	_btnRotateLeft->onMouseRelease((ActionHandler)&GeoscapeEditorState::btnRotateLeftRelease);
	_btnRotateLeft->onKeyboardPress((ActionHandler)&GeoscapeEditorState::btnRotateLeftPress, Options::keyGeoLeft);
	_btnRotateLeft->onKeyboardRelease((ActionHandler)&GeoscapeEditorState::btnRotateLeftRelease, Options::keyGeoLeft);

	_btnRotateRight->onMousePress((ActionHandler)&GeoscapeEditorState::btnRotateRightPress);
	_btnRotateRight->onMouseRelease((ActionHandler)&GeoscapeEditorState::btnRotateRightRelease);
	_btnRotateRight->onKeyboardPress((ActionHandler)&GeoscapeEditorState::btnRotateRightPress, Options::keyGeoRight);
	_btnRotateRight->onKeyboardRelease((ActionHandler)&GeoscapeEditorState::btnRotateRightRelease, Options::keyGeoRight);

	_btnRotateUp->onMousePress((ActionHandler)&GeoscapeEditorState::btnRotateUpPress);
	_btnRotateUp->onMouseRelease((ActionHandler)&GeoscapeEditorState::btnRotateUpRelease);
	_btnRotateUp->onKeyboardPress((ActionHandler)&GeoscapeEditorState::btnRotateUpPress, Options::keyGeoUp);
	_btnRotateUp->onKeyboardRelease((ActionHandler)&GeoscapeEditorState::btnRotateUpRelease, Options::keyGeoUp);

	_btnRotateDown->onMousePress((ActionHandler)&GeoscapeEditorState::btnRotateDownPress);
	_btnRotateDown->onMouseRelease((ActionHandler)&GeoscapeEditorState::btnRotateDownRelease);
	_btnRotateDown->onKeyboardPress((ActionHandler)&GeoscapeEditorState::btnRotateDownPress, Options::keyGeoDown);
	_btnRotateDown->onKeyboardRelease((ActionHandler)&GeoscapeEditorState::btnRotateDownRelease, Options::keyGeoDown);

	_btnZoomIn->onMouseClick((ActionHandler)&GeoscapeEditorState::btnZoomInLeftClick, SDL_BUTTON_LEFT);
	_btnZoomIn->onMouseClick((ActionHandler)&GeoscapeEditorState::btnZoomInRightClick, SDL_BUTTON_RIGHT);
	_btnZoomIn->onKeyboardPress((ActionHandler)&GeoscapeEditorState::btnZoomInLeftClick, Options::keyGeoZoomIn);

	_btnZoomOut->onMouseClick((ActionHandler)&GeoscapeEditorState::btnZoomOutLeftClick, SDL_BUTTON_LEFT);
	_btnZoomOut->onMouseClick((ActionHandler)&GeoscapeEditorState::btnZoomOutRightClick, SDL_BUTTON_RIGHT);
	_btnZoomOut->onKeyboardPress((ActionHandler)&GeoscapeEditorState::btnZoomOutLeftClick, Options::keyGeoZoomOut);

	_gameTimer->onTimer((StateHandler)&GeoscapeEditorState::timeAdvance);
	_gameTimer->start();

	// debug helpers
	//{
	//	std::vector<std::string> regionList;
	//	regionList.push_back("All regions");
	//	for (auto r : *_game->getSavedGame()->getRegions())
	//	{
	//		regionList.push_back(r->getRules()->getType());
	//	}
	//	_cbxRegion->setOptions(regionList, false);
	//	_cbxRegion->setVisible(false);
	//	_cbxRegion->onChange((ActionHandler)&GeoscapeEditorState::cbxRegionChange);

	//	std::vector<std::string> zoneList;
	//	zoneList.push_back("All zones");
	//	for (int z = 0; z < 20; ++z)
	//	{
	//		zoneList.push_back(std::to_string(z));
	//	}
	//	_cbxZone->setOptions(zoneList, false);
	//	_cbxZone->setVisible(false);
	//	_cbxZone->onChange((ActionHandler)&GeoscapeEditorState::cbxZoneChange);
	//}

	timeDisplay();
}

/**
 * Deletes timers.
 */
GeoscapeEditorState::~GeoscapeEditorState()
{
	delete _gameTimer;
}

/**
 * Handle blitting of Geoscape.
 */
void GeoscapeEditorState::blit()
{
	State::blit();
}

/**
 * Handle key shortcuts.
 * @param action Pointer to an action.
 */
void GeoscapeEditorState::handle(Action *action)
{
    State::handle(action);

	if (action->getDetails()->type == SDL_KEYDOWN)
	{
		// "ctrl-d" - enable debug mode
		if (Options::debug && action->getDetails()->key.keysym.sym == SDLK_d && (SDL_GetModState() & KMOD_CTRL) != 0)
		{
			//_game->getSavedGame()->setDebugMode();
			//if (_game->getSavedGame()->getDebugMode())
			//{
			//	_txtDebug->setText("DEBUG MODE");
			//}
			//else
			//{
			//	_txtDebug->setText("");
			//}
			//_cbxRegion->setVisible(_game->getSavedGame()->getDebugMode());
			//_cbxZone->setVisible(_game->getSavedGame()->getDebugMode());
		}
        
		// quick save and quick load
		//if (!_game->getSavedGame()->isIronman())
		//{
		//	if (action->getDetails()->key.keysym.sym == Options::keyQuickSave)
		//	{
		//		popup(new SaveGameState(OPT_GEOSCAPE, SAVE_QUICK, _palette));
		//	}
		//	else if (action->getDetails()->key.keysym.sym == Options::keyQuickLoad)
		//	{
		//		popup(new LoadGameState(OPT_GEOSCAPE, SAVE_QUICK, _palette));
		//	}
		//}
	}
}

/**
 * Updates the timer display and resets the palette
 * since it's bound to change on other screens.
 */
void GeoscapeEditorState::init()
{
	State::init();
	//timeDisplay();

	_globe->onMouseClick((ActionHandler)&GeoscapeEditorState::globeClick);
	_globe->onMouseOver(0);
	_globe->rotateStop();
	_globe->setFocus(true);
	_globe->draw();

    _game->getMod()->playMusic("GMGEO");
}

/**
 * Runs the game timer and handles popups.
 */
void GeoscapeEditorState::think()
{
	State::think();

	//if (!_pause)
	//{
		// Handle timers
		_gameTimer->think(this, 0);
	//}
}

/**
 * Updates the Geoscape clock with the latest
 * game time and date in human-readable format. (+Funds)
 */
//void GeoscapeEditorState::timeDisplay()
//{
	//std::ostringstream ss;
	//ss << std::setfill('0') << std::setw(2) << _game->getSavedGame()->getTime()->getSecond();
	//_txtSec->setText(ss.str());

	//std::ostringstream ss2;
	//ss2 << std::setfill('0') << std::setw(2) << _game->getSavedGame()->getTime()->getMinute();
	//_txtMin->setText(ss2.str());

	//std::ostringstream ss3;
	//ss3 << _game->getSavedGame()->getTime()->getHour();
	//_txtHour->setText(ss3.str());
//}

/**
 * Advances the game timer according to
 * the timer speed set, and calls the respective
 * triggers. The timer always advances in "5 secs"
 * cycles, regardless of the speed, otherwise it might
 * skip important steps. Instead, it just keeps advancing
 * the timer until the next speed step (eg. the next day
 * on 1 Day speed) or until an event occurs, since updating
 * the screen on each step would become cumbersomely slow.
 */
void GeoscapeEditorState::timeAdvance()
{
	int timeSpan = 0;
    _pause = false;
    _globe->setDrawShade(true);
	if (_timeSpeed == _btnSlow)
	{
		//if (Options::oxceGeoSlowdownFactor > 1)
		//{
		//	_slowdownCounter--;
		//	if (_slowdownCounter > 0)
		//	{
		//		// wait
		//		_globe->draw();
		//		return;
		//	}
		//	else
		//	{
		//		_slowdownCounter = Clamp(Options::oxceGeoSlowdownFactor, 2, 100);
		//	}
		//}
		timeSpan = 12; //1; 1 min
	}
	else if (_timeSpeed == _btnFast)
	{
		timeSpan = 12 * 5 * 6; // 30 min
	}
    else
    {
        _pause = true;
        if (_timeSpeed != _btnPause)
        {
            _globe->setDrawShade(false);

            if (_timeSpeed == _btnReset)
            {
                delete _game->getSavedGame()->getTime();
                _game->getSavedGame()->setTime(GameTime(0, 0, 0, 0, 0, 0, 0));
            }
        }
    }


	for (int i = 0; i < timeSpan && !_pause; ++i)
	{
		TimeTrigger trigger;
		trigger = _game->getSavedGame()->getTime()->advance();
		//switch (trigger)
		//{
		//case TIME_1MONTH:
		//	time1Month();
		//	FALLTHROUGH;
		//case TIME_1DAY:
		//	time1Day();
		//	FALLTHROUGH;
		//case TIME_1HOUR:
		//	time1Hour();
		//	FALLTHROUGH;
		//case TIME_30MIN:
		//	time30Minutes();
		//	FALLTHROUGH;
		//case TIME_10MIN:
		//	time10Minutes();
		//	FALLTHROUGH;
		//case TIME_5SEC:
		//	time5Seconds();
		//}
	}

	timeDisplay();
	_globe->draw();
}

/**
 * Updates the clock
 */
void GeoscapeEditorState::timeDisplay()
{
	std::ostringstream ss;
	ss << std::setfill('0') << std::setw(2) << _game->getSavedGame()->getTime()->getMinute();

	std::ostringstream ss2;
	ss2 << _game->getSavedGame()->getTime()->getHour();

	_btnTime->setText(ss2.str() + ":" + ss.str());
}

/**
 * Returns a pointer to the Geoscape globe for
 * access by other substates.
 * @return Pointer to globe.
 */
GlobeEditable *GeoscapeEditorState::getGlobe() const
{
	return _globe;
}

/**
 * Processes any left-clicks on globe markers,
 * or right-clicks to scroll the globe.
 * @param action Pointer to an action.
 */

void GeoscapeEditorState::globeClick(Action *action)
{
	int mouseX = (int)floor(action->getAbsoluteXMouse()), mouseY = (int)floor(action->getAbsoluteYMouse());

	// Clicking markers on the globe
	//if (action->getDetails()->button.button == SDL_BUTTON_LEFT)
	//{
	//	std::vector<Target*> v = _globe->getTargets(mouseX, mouseY, false, 0);
	//	if (!v.empty())
	//	{
	//		_game->pushState(new MultipleTargetsState(v, 0, this));
	//	}
	//}

	if (_game->getSavedGame()->getDebugMode())
	{
		double lon, lat;
		int texture, shade;
		_globe->cartToPolar(mouseX, mouseY, &lon, &lat);
		double lonDeg = lon / M_PI * 180, latDeg = lat / M_PI * 180;
		_globe->getPolygonTextureAndShade(lon, lat, &texture, &shade);
		std::ostringstream ss;
		ss << "rad: " << lon << ", " << lat << std::endl;
		ss << "deg: " << lonDeg << ", " << latDeg << std::endl;
		ss << "texture: " << texture << ", shade: " << shade << std::endl;

		_txtDebug->setText(ss.str());
	}
}

/**
 * Opens the jukebox.
 * @param action Pointer to an action.
 */
void GeoscapeEditorState::btnSelectMusicTrackClick(Action *)
{
	_game->pushState(new SelectMusicTrackState(SMT_GEOSCAPE));
}

/**
 * Opens the Options window.
 * @param action Pointer to an action.
 */
void GeoscapeEditorState::btnOptionsClick(Action *)
{
	_game->pushState(new PauseState(OPT_GEOSCAPE));
}

/**
 * Starts rotating the globe to the left.
 * @param action Pointer to an action.
 */
void GeoscapeEditorState::btnRotateLeftPress(Action *)
{
	_globe->rotateLeft();
}

/**
 * Stops rotating the globe to the left.
 * @param action Pointer to an action.
 */
void GeoscapeEditorState::btnRotateLeftRelease(Action *)
{
	_globe->rotateStopLon();
}

/**
 * Starts rotating the globe to the right.
 * @param action Pointer to an action.
 */
void GeoscapeEditorState::btnRotateRightPress(Action *)
{
	_globe->rotateRight();
}

/**
 * Stops rotating the globe to the right.
 * @param action Pointer to an action.
 */
void GeoscapeEditorState::btnRotateRightRelease(Action *)
{
	_globe->rotateStopLon();
}

/**
 * Starts rotating the globe upwards.
 * @param action Pointer to an action.
 */
void GeoscapeEditorState::btnRotateUpPress(Action *)
{
	_globe->rotateUp();
}

/**
 * Stops rotating the globe upwards.
 * @param action Pointer to an action.
 */
void GeoscapeEditorState::btnRotateUpRelease(Action *)
{
	_globe->rotateStopLat();
}

/**
 * Starts rotating the globe downwards.
 * @param action Pointer to an action.
 */
void GeoscapeEditorState::btnRotateDownPress(Action *)
{
	_globe->rotateDown();
}

/**
 * Stops rotating the globe downwards.
 * @param action Pointer to an action.
 */
void GeoscapeEditorState::btnRotateDownRelease(Action *)
{
	_globe->rotateStopLat();
}

/**
 * Zooms into the globe.
 * @param action Pointer to an action.
 */
void GeoscapeEditorState::btnZoomInLeftClick(Action *)
{
	_globe->zoomIn();
}

/**
 * Zooms the globe maximum.
 * @param action Pointer to an action.
 */
void GeoscapeEditorState::btnZoomInRightClick(Action *)
{
	_globe->zoomMax();
}

/**
 * Zooms out of the globe.
 * @param action Pointer to an action.
 */
void GeoscapeEditorState::btnZoomOutLeftClick(Action *)
{
	_globe->zoomOut();
}

/**
 * Zooms the globe minimum.
 * @param action Pointer to an action.
 */
void GeoscapeEditorState::btnZoomOutRightClick(Action *)
{
	_globe->zoomMin();
}

/**
 * Handler for clicking on a timer button.
 * @param action pointer to the mouse action.
 */
void GeoscapeEditorState::btnTimerClick(Action *action)
{
	SDL_Event ev;
	ev.type = SDL_MOUSEBUTTONDOWN;
	ev.button.button = SDL_BUTTON_LEFT;
	Action a = Action(&ev, 0.0, 0.0, 0, 0);
	action->getSender()->mousePress(&a, this);
}

/**
 * Handler for clicking the set time button.
 * @param action pointer to an action
 */
void GeoscapeEditorState::btnTimeClick(Action *action)
{
    // do something
}

/**
 * Updates the scale.
 * @param dX delta of X;
 * @param dY delta of Y;
 */
void GeoscapeEditorState::resize(int &dX, int &dY)
{
	if (_game->getSavedGame()->getSavedBattle())
		return;
	dX = Options::baseXResolution;
	dY = Options::baseYResolution;
	int divisor = 1;
	double pixelRatioY = 1.0;

	if (Options::nonSquarePixelRatio)
	{
		pixelRatioY = 1.2;
	}
	switch (Options::geoscapeScale)
	{
	case SCALE_SCREEN_DIV_6:
		divisor = 6;
		break;
	case SCALE_SCREEN_DIV_5:
		divisor = 5;
		break;
	case SCALE_SCREEN_DIV_4:
		divisor = 4;
		break;
	case SCALE_SCREEN_DIV_3:
		divisor = 3;
		break;
	case SCALE_SCREEN_DIV_2:
		divisor = 2;
		break;
	case SCALE_SCREEN:
		break;
	default:
		dX = 0;
		dY = 0;
		return;
	}

	Options::baseXResolution = std::max(Screen::ORIGINAL_WIDTH, Options::displayWidth / divisor);
	Options::baseYResolution = std::max(Screen::ORIGINAL_HEIGHT, (int)(Options::displayHeight / pixelRatioY / divisor));

	dX = Options::baseXResolution - dX;
	dY = Options::baseYResolution - dY;

	_globe->resize();

	for (std::vector<Surface*>::const_iterator i = _surfaces.begin(); i != _surfaces.end(); ++i)
	{
		if (*i != _globe)
		{
			(*i)->setX((*i)->getX() + dX);
			(*i)->setY((*i)->getY() + dY/2);
		}
	}

	_bg->setX((_globe->getWidth() - _bg->getWidth()) / 2);
	_bg->setY((_globe->getHeight() - _bg->getHeight()) / 2);

	int height = (Options::baseYResolution - Screen::ORIGINAL_HEIGHT) / 2 + 10;
	_sideTop->setHeight(height);
	_sideTop->setY(_sidebar->getY() - height - 1);
	_sideBottom->setHeight(height);
	_sideBottom->setY(_sidebar->getY() + _sidebar->getHeight() + 1);

	_sideLine->setHeight(Options::baseYResolution);
	_sideLine->setY(0);
	_sideLine->drawRect(0, 0, _sideLine->getWidth(), _sideLine->getHeight(), 15);
}


//void GeoscapeEditorState::cbxRegionChange(Action *)
//{
//	int index = _cbxRegion->getSelected();
//	if (index < 1)
//	{
//		_game->getSavedGame()->debugRegion = nullptr;
//	}
//	else
//	{
//		_game->getSavedGame()->debugRegion = (*_game->getSavedGame()->getRegions())[index-1];
//	}
//}

//void GeoscapeEditorState::cbxZoneChange(Action *)
//{
//	_game->getSavedGame()->debugZone = _cbxZone->getSelected();
//}

}