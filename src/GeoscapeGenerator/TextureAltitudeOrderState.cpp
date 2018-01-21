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

#include <string>
#include "GeoscapeGeneratorState.h"
#include "GeoscapeGenerator.h"
#include "TextureAltitudeOrderState.h"
#include "../Interface/TextButton.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "../Interface/TextList.h"
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

TextureAltitudeOrderState::TextureAltitudeOrderState(GeoscapeGeneratorState *parent) : _parent(parent)
{
	// Create objects for the interface window
	_screen = false;

	_window = new Window(this, 230, 140, 45, 32);
	_lstTextures = new TextList(198, 106, 53, 36);
	_btnOk = new TextButton(206, 16, 57, 145);

	// Set palette
	setInterface("newBattleMenu");

	add(_window, "window", "newBattleMenu");
	add(_btnOk, "button2", "newBattleMenu");
	add(_lstTextures, "text", "newBattleMenu");

	centerAllSurfaces();

	// Set up objects
	_window->setBackground(_game->getMod()->getSurface("BACK01.SCR"));

	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&TextureAltitudeOrderState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&TextureAltitudeOrderState::btnOkClick, Options::keyOk);
}

TextureAltitudeOrderState::~TextureAltitudeOrderState()
{

}

void TextureAltitudeOrderState::btnOkClick(Action *action)
{
	_game->popState();
}

}
