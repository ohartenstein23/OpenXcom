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
#include "../Savegame/SavedGame.h"

namespace OpenXcom
{

GeoscapeGeneratorState::GeoscapeGeneratorState()
{
	// Create objects for the interface window
	_window = new Window(this, 320, 200, 0, 0, POPUP_BOTH);
	_txtTitle = new Text(320, 17, 0, 9);

	_txtSeed = new Text(100, 10, 8, 28);
	_edtSeed = new TextEdit(this, 100, 10, 18, 40);

	_txtNumCircles = new Text(100, 10, 8, 52);
	_edtNumCircles = new TextEdit(this, 22, 10, 18, 64);

	_txtError = new Text(140, 160, 170, 28);

	_btnOk = new TextButton(100, 16, 8, 176);
	_btnClear = new TextButton(100, 16, 110, 176);
	_btnCancel = new TextButton(100, 16, 212, 176);

	// Set palette for menu screen
	setInterface("newBattleMenu");

	// Add objects to screen
	add(_window, "window", "newBattleMenu");
	add(_txtTitle, "heading", "newBattleMenu");

	add(_txtSeed, "text", "newBattleMenu");
	add(_edtSeed, "text", "newBattleMenu");

	add(_txtNumCircles, "text", "newBattleMenu");
	add(_edtNumCircles, "text", "newBattleMenu");

	add(_txtError, "text", "newBattleMenu");

	add(_btnOk, "button2", "newBattleMenu");
	add(_btnClear, "button2", "newBattleMenu");
	add(_btnCancel, "button2", "newBattleMenu");

	centerAllSurfaces();

	// Set up objects
	_window->setBackground(_game->getMod()->getSurface("BACK01.SCR"));

	_txtTitle->setAlign(ALIGN_CENTER);
	_txtTitle->setBig();
	_txtTitle->setText(tr("STR_GEOSCAPE_GENERATOR"));

	_txtSeed->setText(tr("STR_RNG_SEED"));
	_edtSeed->onChange((ActionHandler)&GeoscapeGeneratorState::edtSeedChange);

	_txtNumCircles->setText(tr("STR_NUMBER_OF_CIRCLES"));
	_edtNumCircles->onChange((ActionHandler)&GeoscapeGeneratorState::edtNumCirclesChange);

	_txtError->setWordWrap(true);
	_txtError->setText(tr("STR_GEOSCAPE_GENERATOR_ERRORS"));

	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&GeoscapeGeneratorState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&GeoscapeGeneratorState::btnOkClick, Options::keyOk);

	_btnClear->setText(tr("STR_CLEAR_DATA"));
	_btnClear->onMouseClick((ActionHandler)&GeoscapeGeneratorState::btnClearClick);
	//_btnClear->onKeyboardPress((ActionHandler)&GeoscapeGeneratorState::btnOkClick, Options::keyOk);

	_btnCancel->setText(tr("STR_CANCEL"));
	_btnCancel->onMouseClick((ActionHandler)&GeoscapeGeneratorState::btnCancelClick);
	_btnCancel->onKeyboardPress((ActionHandler)&GeoscapeGeneratorState::btnCancelClick, Options::keyCancel);

	_geoscapeGenerator = new GeoscapeGenerator(this);
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
	_rngSeed = RNG::getSeed();
	std::wostringstream ss;
	ss << _rngSeed;
	_edtSeed->setText(ss.str());

	_numCircles = 10;
	ss.str(std::wstring());
	ss << _numCircles;
	_edtNumCircles->setText(ss.str());

	ss.str(std::wstring());
	_txtError->setText(ss.str());
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
 * Starts the geoscape generator.
 * @param action Pointer to an action.
 */
void GeoscapeGeneratorState::btnOkClick(Action *)
{
	_geoscapeGenerator->init(_rngSeed, _numCircles);
	try
	{
		_geoscapeGenerator->generate();
		_txtError->setText(tr("STR_GEOSCAPE_GENERATOR_RAN_SUCCESS"));
		_geoscapeGenerator->save();
		_geoscapeGenerator->error(std::string("Successful run, writing to log."));
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
	init();
	_geoscapeGenerator->init(_rngSeed, _numCircles);
}

/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void GeoscapeGeneratorState::btnCancelClick(Action *)
{
	_game->popState();
}

}
