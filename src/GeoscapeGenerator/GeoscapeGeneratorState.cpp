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
#include "GeoscapeGeneratorState.h"
#include "GeoscapeGenerator.h"
#include "../Engine/LocalizedText.h"
#include "../Interface/TextButton.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "../Interface/TextEdit.h"
#include "../Interface/ComboBox.h"
#include "../Interface/Slider.h"
#include "../Interface/Frame.h"
#include "../Engine/Game.h"
#include "../Engine/Language.h"
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
	_edtSeed = new TextEdit(this, 100, 10, 8, 12);

	_btnOk = new TextButton(100, 16, 8, 176);
	_btnClear = new TextButton(100, 16, 110, 176);
	_btnCancel = new TextButton(100, 16, 212, 176);

	// Set palette for menu screen
	setInterface("newBattleMenu");

	add(_window, "window", "newBattleMenu");
	add(_txtTitle, "heading", "newBattleMenu");

	add(_txtSeed, "text", "newBattleMenu");
	add(_edtSeed, "text1", "craftInfo");

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

	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&GeoscapeGeneratorState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&GeoscapeGeneratorState::btnOkClick, Options::keyOk);

	_btnClear->setText(tr("STR_CLEAR_DATA"));
	_btnClear->onMouseClick((ActionHandler)&GeoscapeGeneratorState::btnClearClick);
	//_btnClear->onKeyboardPress((ActionHandler)&GeoscapeGeneratorState::btnOkClick, Options::keyOk);

	_btnCancel->setText(tr("STR_CANCEL"));
	_btnCancel->onMouseClick((ActionHandler)&GeoscapeGeneratorState::btnCancelClick);
	_btnCancel->onKeyboardPress((ActionHandler)&GeoscapeGeneratorState::btnCancelClick, Options::keyCancel);
}

GeoscapeGeneratorState::~GeoscapeGeneratorState()
{

}

/**
 * Inits the geoscape generator interface state data
 *
 */
void GeoscapeGeneratorState::init()
{
	_rngSeed = RNG::getSeed();
	std::wostringstream ss;
	ss << "\t" << _rngSeed;
	_edtSeed->setText(ss.str());
}

/**
 * Changes the RNG seed.
 * @param action Pointer to an action.
 */
void GeoscapeGeneratorState::edtSeedChange(Action *action)
{
	/*
	if (_edtSeed->getText() == _craft->getDefaultName(_game->getLanguage()))
	{
		_craft->setName(L"");
	}
	else
	{
		_craft->setName(_edtCraft->getText());
	}
	if (action->getDetails()->key.keysym.sym == SDLK_RETURN ||
		action->getDetails()->key.keysym.sym == SDLK_KP_ENTER)
	{
		_edtCraft->setText(_craft->getName(_game->getLanguage()));
	}
	*/
}

/**
 * Starts the battle.
 * @param action Pointer to an action.
 */
void GeoscapeGeneratorState::btnOkClick(Action *)
{

}

/**
 * Resets the data for the geoscape generator
 * @param action Pointer to an action.
 */
void GeoscapeGeneratorState::btnClearClick(Action *)
{
	
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
