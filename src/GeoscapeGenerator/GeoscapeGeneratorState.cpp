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

#include <sstream>
#include <string>
#include "GeoscapeGeneratorState.h"
#include "GeoscapeGenerator.h"
#include "TextureOrderState.h"
#include "../Interface/TextButton.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "../Interface/TextEdit.h"
#include "../Interface/ComboBox.h"
#include "../Interface/Slider.h"
#include "../Interface/Frame.h"
#include "../Engine/Action.h"
#include "../Engine/Exception.h"
#include "../Engine/Game.h"
#include "../Engine/Language.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/Options.h"
#include "../Engine/RNG.h"
#include "../Engine/Screen.h"
#include "../Mod/Mod.h"
#include "../Mod/RuleGlobe.h"
#include "../Mod/Texture.h"
#include "../Savegame/SavedGame.h"

namespace OpenXcom
{

GeoscapeGeneratorState::GeoscapeGeneratorState()
{
	// Create objects for the interface window
	_window = new Window(this, 320, 200, 0, 0, POPUP_BOTH);
	_txtTitle = new Text(320, 17, 0, 9);

	_txtSeed = new Text(95, 10, 8, 28);
	_frameSeed = new Frame(105, 14, 13, 38);
	_edtSeed = new TextEdit(this, 100, 10, 18, 41);

	_txtNumCircles = new Text(100, 10, 8, 54);
	_frameNumCircles = new Frame(32, 14, 13, 64);
	_edtNumCircles = new TextEdit(this, 22, 10, 18, 67);

	_txtWaterThreshold = new Text(100, 10, 8, 80);
	_frameWaterThreshold = new Frame(32, 14, 13, 90);
	_edtWaterThreshold = new TextEdit(this, 22, 10, 18, 93);

	_btnTextureAltitude = new TextButton(100, 16, 8, 106);

	_txtPolesThreshold = new Text(100, 10, 8, 124);
	_framePolesThreshold = new Frame(32, 14, 13, 134);
	_edtPolesThreshold = new TextEdit(this, 22, 10, 18, 137);

	_btnTexturePoles = new TextButton(100, 16, 8, 150);

	_frameErrorMessages = new Frame(160, 144, 150, 28);
	_txtError = new Text(150, 134, 155, 33);

	_btnOk = new TextButton(100, 16, 8, 176);
	_btnClear = new TextButton(100, 16, 110, 176);
	_btnCancel = new TextButton(100, 16, 212, 176);

	// Set palette for menu screen
	setInterface("geoGeneratorMenu");

	// Add objects to screen
	add(_window, "window", "geoGeneratorMenu");
	add(_txtTitle, "heading", "geoGeneratorMenu");

	add(_txtSeed, "text", "geoGeneratorMenu");
	add(_frameSeed, "frames", "geoGeneratorMenu");
	add(_edtSeed, "text", "geoGeneratorMenu");

	add(_txtNumCircles, "text", "geoGeneratorMenu");
	add(_frameNumCircles, "frames", "geoGeneratorMenu");
	add(_edtNumCircles, "text", "geoGeneratorMenu");

	add(_txtWaterThreshold, "text", "geoGeneratorMenu");
	add(_frameWaterThreshold, "frames", "geoGeneratorMenu");
	add(_edtWaterThreshold, "text", "geoGeneratorMenu");

	add(_btnTextureAltitude, "button2", "geoGeneratorMenu");

	add(_txtPolesThreshold, "text", "geoGeneratorMenu");
	add(_framePolesThreshold, "frames", "geoGeneratorMenu");
	add(_edtPolesThreshold, "text", "geoGeneratorMenu");

	add(_btnTexturePoles, "button2", "geoGeneratorMenu");

	add(_txtError, "text", "geoGeneratorMenu");
	add(_frameErrorMessages, "frames", "geoGeneratorMenu");

	add(_btnOk, "button2", "geoGeneratorMenu");
	add(_btnClear, "button2", "geoGeneratorMenu");
	add(_btnCancel, "button2", "geoGeneratorMenu");

	centerAllSurfaces();

	// Set up objects
	_window->setBackground(_game->getMod()->getSurface("BACK01.SCR"));

	_txtTitle->setAlign(ALIGN_CENTER);
	_txtTitle->setBig();
	_txtTitle->setText(tr("STR_GEOSCAPE_GENERATOR"));

	_txtSeed->setText(tr("STR_RNG_SEED"));
	_frameSeed->setThickness(1);
	_edtSeed->onChange((ActionHandler)&GeoscapeGeneratorState::edtSeedChange);
	_edtSeed->setConstraint(TEC_NUMERIC_POSITIVE);

	_txtNumCircles->setText(tr("STR_NUMBER_OF_CIRCLES"));
	_frameNumCircles->setThickness(1);
	_edtNumCircles->onChange((ActionHandler)&GeoscapeGeneratorState::edtNumCirclesChange);
	_edtNumCircles->setConstraint(TEC_NUMERIC_POSITIVE);

	_txtWaterThreshold->setText(tr("STR_WATER_LEVEL_THRESHOLD"));
	_frameWaterThreshold->setThickness(1);
	_edtWaterThreshold->onChange((ActionHandler)&GeoscapeGeneratorState::edtWaterThresholdChange);
	_edtWaterThreshold->setConstraint(TEC_NUMERIC_POSITIVE);

	_btnTextureAltitude->setText(tr("STR_GLOBE_TEXTURE_LIST_BY_ALTITUDE"));
	_btnTextureAltitude->onMouseClick((ActionHandler)&GeoscapeGeneratorState::btnTextureAltitudeClick);

	_txtPolesThreshold->setText(tr("STR_POLES_LATITUDE_THRESHOLD"));
	_framePolesThreshold->setThickness(1);
	_edtPolesThreshold->onChange((ActionHandler)&GeoscapeGeneratorState::edtPolesThresholdChange);
	_edtPolesThreshold->setConstraint(TEC_NUMERIC_POSITIVE);

	_btnTexturePoles->setText(tr("STR_GLOBE_TEXTURE_LIST_FOR_POLES"));
	_btnTexturePoles->onMouseClick((ActionHandler)&GeoscapeGeneratorState::btnTexturePolesClick);

	_txtError->setWordWrap(true);
	_txtError->setText(tr("STR_GEOSCAPE_GENERATOR_ERRORS"));
	_frameErrorMessages->setThickness(3);

	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&GeoscapeGeneratorState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&GeoscapeGeneratorState::btnOkClick, Options::keyOk);

	_btnClear->setText(tr("STR_RESET_DATA"));
	_btnClear->onMouseClick((ActionHandler)&GeoscapeGeneratorState::btnClearClick);
	//_btnClear->onKeyboardPress((ActionHandler)&GeoscapeGeneratorState::btnOkClick, Options::keyOk);

	_btnCancel->setText(tr("STR_CANCEL"));
	_btnCancel->onMouseClick((ActionHandler)&GeoscapeGeneratorState::btnCancelClick);
	_btnCancel->onKeyboardPress((ActionHandler)&GeoscapeGeneratorState::btnCancelClick, Options::keyCancel);

	// Create a new geoscape generator
	_geoscapeGenerator = new GeoscapeGenerator(this);
	resetData();
}

GeoscapeGeneratorState::~GeoscapeGeneratorState()
{
	delete _geoscapeGenerator;
}

/**
 * Inits the geoscape generator interface state data
 *
 */
void GeoscapeGeneratorState::init()
{
	// Called whenever focus is switched back to this state, so we don't want to reset any data here just from popping any of the sub-menus
}

/**
 * Resets the data to its defaults
 */
void GeoscapeGeneratorState::resetData()
{
	_rngSeed = RNG::getSeed();
	std::wostringstream ss;
	ss << _rngSeed;
	_edtSeed->setText(ss.str());

	_numCircles = 10;
	ss.str(std::wstring());
	ss << _numCircles;
	_edtNumCircles->setText(ss.str());

	_waterThreshold = 40;
	_edtWaterThreshold->setText(Text::formatPercentage(_waterThreshold));

	_polesThreshold = 10;
	_edtPolesThreshold->setText(Text::formatPercentage(_polesThreshold));

	_txtError->setText(tr("STR_GEOSCAPE_GENERATOR_ERRORS"));

	_texturesByAltitude.clear();
	_texturesForPoles.clear();
	Texture *texture = _game->getMod()->getGlobe()->getTexture(0);
	for (int i = 1; i < 100 && texture; ++i)
	{
		_texturesByAltitude.push_back(std::make_pair(i - 1, false)); // change 'false' to get default value for each texture from a ruleset
		_texturesForPoles.push_back(std::make_pair(i - 1, false));

		texture = _game->getMod()->getGlobe()->getTexture(i);
	}

	if (_texturesByAltitude.size() == 0 && _texturesForPoles.size() == 0)
	{
		throw Exception("GeoscapeGeneratorState: Unable to read textures from globe ruleset, lists not populated.");
	}
}

/**
 * Changes the RNG seed.
 * @param action Pointer to an action.
 */
void GeoscapeGeneratorState::edtSeedChange(Action *action)
{
	std::wostringstream ss;

	if (_edtSeed->getText().size() == 0 && (action->getDetails()->key.keysym.sym == SDLK_RETURN ||
	action->getDetails()->key.keysym.sym == SDLK_KP_ENTER))
	{
		_rngSeed = RNG::getSeed();
		ss << _rngSeed;
		_edtSeed->setText(ss.str());
		_edtSeed->setFocus(false);
	}
	else
	{
		std::wstring input = _edtSeed->getText();
		size_t newSeed = 0;

		// Try to convert input to a number. If it errors, return
		// to previous RNG seed
		try
		{
			newSeed = stol(input);

			if (action->getDetails()->key.keysym.sym == SDLK_RETURN ||
	action->getDetails()->key.keysym.sym == SDLK_KP_ENTER)
			{
				_edtSeed->setText(input);
			}

			_rngSeed = newSeed;
		}
		catch (...)
		{
			if (action->getDetails()->key.keysym.sym == SDLK_RETURN ||
	action->getDetails()->key.keysym.sym == SDLK_KP_ENTER)
			{
				_rngSeed = RNG::getSeed();
				ss << _rngSeed;
				_edtSeed->setText(ss.str());
			}
		}
	}
}

/**
 * Changes the number of circles used in the generator.
 * @param action Pointer to an action.
 */
void GeoscapeGeneratorState::edtNumCirclesChange(Action *action)
{
	std::wostringstream ss;

	if (_edtNumCircles->getText().size() == 0 && (action->getDetails()->key.keysym.sym == SDLK_RETURN ||
	action->getDetails()->key.keysym.sym == SDLK_KP_ENTER))
	{
		ss << _numCircles;
		_edtNumCircles->setText(ss.str());
		_edtNumCircles->setFocus(false);
	}
	else
	{
		std::wstring input = _edtNumCircles->getText();
		size_t newCircles = 0;

		// Try to convert input to a number.
		try
		{
			newCircles = stol(input);
			if (newCircles > 1000)
			{
				newCircles = 1000;
			}

			if (action->getDetails()->key.keysym.sym == SDLK_RETURN ||
	action->getDetails()->key.keysym.sym == SDLK_KP_ENTER)
			{
				ss << newCircles;
				_edtNumCircles->setText(ss.str());
			}

			_numCircles = newCircles;
		}
		catch (...)
		{
			if (action->getDetails()->key.keysym.sym == SDLK_RETURN ||
	action->getDetails()->key.keysym.sym == SDLK_KP_ENTER)
			{
				ss << _numCircles;
				_edtNumCircles->setText(ss.str());
			}
		}
	}
}

/**
 * Changes the threshold for water level based on the altitude.
 * @param action Pointer to an action.
 */
void GeoscapeGeneratorState::edtWaterThresholdChange(Action *action)
{
	if (_edtWaterThreshold->getText().size() == 0 && (action->getDetails()->key.keysym.sym == SDLK_RETURN ||
	action->getDetails()->key.keysym.sym == SDLK_KP_ENTER))
	{
		_edtWaterThreshold->setText(Text::formatPercentage(_waterThreshold));
		_edtWaterThreshold->setFocus(false);
	}
	else
	{
		std::wstring input = _edtWaterThreshold->getText();
		int newThreshold = _waterThreshold;

		// Try to convert input to a number. If it errors, return
		// to last good threshold
		try
		{
			newThreshold = std::min(std::max(stoi(input), 0), 100);

			if (action->getDetails()->key.keysym.sym == SDLK_RETURN ||
	action->getDetails()->key.keysym.sym == SDLK_KP_ENTER)
			{
				_edtWaterThreshold->setText(Text::formatPercentage(newThreshold));
			}

			_waterThreshold = newThreshold;
		}
		catch (...)
		{
			if (action->getDetails()->key.keysym.sym == SDLK_RETURN ||
	action->getDetails()->key.keysym.sym == SDLK_KP_ENTER)
			{
				_edtWaterThreshold->setText(Text::formatPercentage(_waterThreshold));
			}
		}
	}
}

/**
 * Brings up the menu for selecting the order in which textures get assigned to relative altitudes
 * @param action Pointer to an action.
 */
void GeoscapeGeneratorState::btnTextureAltitudeClick(Action *)
{
	_game->pushState(new TextureOrderState(this, &_texturesByAltitude));
}

/**
 * Changes the threshold for water level based on the altitude.
 * @param action Pointer to an action.
 */
void GeoscapeGeneratorState::edtPolesThresholdChange(Action *action)
{
	if (_edtPolesThreshold->getText().size() == 0 && (action->getDetails()->key.keysym.sym == SDLK_RETURN ||
	action->getDetails()->key.keysym.sym == SDLK_KP_ENTER))
	{
		_edtPolesThreshold->setText(Text::formatPercentage(_polesThreshold));
		_edtPolesThreshold->setFocus(false);
	}
	else
	{
		std::wstring input = _edtPolesThreshold->getText();
		int newThreshold = _polesThreshold;

		// Try to convert input to a number. If it errors, return
		// to last good threshold
		try
		{
			newThreshold = std::min(std::max(stoi(input), 0), 100);

			if (action->getDetails()->key.keysym.sym == SDLK_RETURN ||
				action->getDetails()->key.keysym.sym == SDLK_KP_ENTER)
			{
				_edtPolesThreshold->setText(Text::formatPercentage(newThreshold));
			}

			_polesThreshold = newThreshold;
		}
		catch (...)
		{
			if (action->getDetails()->key.keysym.sym == SDLK_RETURN ||
	action->getDetails()->key.keysym.sym == SDLK_KP_ENTER)
			{
				_edtPolesThreshold->setText(Text::formatPercentage(_polesThreshold));
			}
		}
	}
}

/**
 * Brings up the menu for selecting the order in which textures get assigned to polar regions
 * @param action Pointer to an action.
 */
void GeoscapeGeneratorState::btnTexturePolesClick(Action *)
{
	_game->pushState(new TextureOrderState(this, &_texturesForPoles));
}

/**
 * Starts the geoscape generator.
 * @param action Pointer to an action.
 */
void GeoscapeGeneratorState::btnOkClick(Action *)
{
	// Pass all the data from this menu to the generator
	_geoscapeGenerator->init(_rngSeed, _numCircles, _waterThreshold, _polesThreshold);

	bool assignedTextures = false;
	for (auto &i : _texturesByAltitude)
	{
		if (i.second)
		{
			_geoscapeGenerator->getTexturesByAltitude()->push_back(i.first);
			assignedTextures = true;
		}
	}
	if (!assignedTextures)
	{
		_txtError->setText(L"Error: You must choose which globe textures to assign by altitude!");
		return;
	}

	assignedTextures = false;
	for (auto &i : _texturesForPoles)
	{
		if (i.second)
		{
			_geoscapeGenerator->getTexturesForPoles()->push_back(i.first);
			assignedTextures = true;
		}
	}
	if (!assignedTextures && _polesThreshold != 0)
	{
		_txtError->setText(L"Error: You must choose which globe textures to assign for the poles!");
		return;
	}

	// Run the generator!
	try
	{
		_geoscapeGenerator->generate();
		_txtError->setText(tr("STR_GEOSCAPE_GENERATOR_RAN_SUCCESS"));
		_geoscapeGenerator->save();
	}
	catch (Exception &errorMsg)
	{
		std::wstring errorStr;
		errorStr.assign(errorMsg.what(), errorMsg.what() + strlen(errorMsg.what()));
		_txtError->setText(errorStr);
	}
}

/**
 * Resets the data for the geoscape generator
 * @param action Pointer to an action.
 */
void GeoscapeGeneratorState::btnClearClick(Action *)
{
	resetData();
	_geoscapeGenerator->init(_rngSeed, _numCircles, _waterThreshold, _polesThreshold);
}

/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void GeoscapeGeneratorState::btnCancelClick(Action *)
{
	_game->popState();
}

/**
 * Gets a pointer to the geoscape generator
 */
GeoscapeGenerator *GeoscapeGeneratorState::getGeoscapeGenerator()
{
	return _geoscapeGenerator;
}

}
