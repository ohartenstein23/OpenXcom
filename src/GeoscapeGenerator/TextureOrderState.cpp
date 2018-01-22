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

#include <climits>
#include <sstream>
#include <string>
#include "GeoscapeGeneratorState.h"
#include "GeoscapeGenerator.h"
#include "TextureOrderState.h"
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

TextureOrderState::TextureOrderState(GeoscapeGeneratorState *parent, std::vector<std::pair<int, bool> > *textures) : _parent(parent), _textures(textures)
{
	// Create objects for the interface window
	_screen = false;

	_window = new Window(this, 96, 144, 112, 28);
	_lstTextures = new TextList(70, 106, 128, 36);
	_btnOk = new TextButton(70, 16, 128, 145);

	// Set palette
	setInterface("geoGeneratorMenu");

	add(_window, "window", "geoGeneratorMenu");
	add(_btnOk, "button2", "geoGeneratorMenu");
	add(_lstTextures, "text", "geoGeneratorMenu");

	centerAllSurfaces();

	// Set up objects
	_window->setBackground(_game->getMod()->getSurface("BACK01.SCR"));

	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&TextureOrderState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&TextureOrderState::btnOkClick, Options::keyOk);

	_lstTextures->setArrowColumn(33, ARROW_VERTICAL);
	_lstTextures->setColumns(2, 32, 16);
	_lstTextures->setAlign(ALIGN_RIGHT, 3);
	_lstTextures->setSelectable(true);
	_lstTextures->setBackground(_window);
	_lstTextures->setMargin(8);
	_lstTextures->onMouseClick((ActionHandler)&TextureOrderState::lstTexturesClick);
	_lstTextures->onLeftArrowClick((ActionHandler)&TextureOrderState::lstTexturesLeftArrowClick);
	_lstTextures->onRightArrowClick((ActionHandler)&TextureOrderState::lstTexturesRightArrowClick);
	drawTextureList();
}

TextureOrderState::~TextureOrderState()
{

}

/**
 * Redraws the list of textures
 */
void TextureOrderState::drawTextureList()
{
	_lstTextures->clearList();
	size_t row = 0;
	for (auto &i : *_textures)
	{
		std::wostringstream ss;
		ss << i.first;
		_lstTextures->addRow(1, ss.str().c_str());

		if (!i.second)
			_lstTextures->setRowColor(row, _lstTextures->getSecondaryColor());

		++row;
	}
}

/**
 * Handles clicking on the list of textures
 * Swaps a texture from used state (true) to unused state (false)
 * @param action Pointer to an action.
 */
void TextureOrderState::lstTexturesClick(Action *action)
{
	double mouseX = action->getAbsoluteXMouse();
	if (mouseX >= _lstTextures->getArrowsLeftEdge() && mouseX < _lstTextures->getArrowsRightEdge())
		return;

	unsigned int row = _lstTextures->getSelectedRow();
	if (action->getDetails()->button.button == SDL_BUTTON_LEFT && row < _textures->size())
	{
		// swap the state of the selected row
		_textures->at(row).second = _textures->at(row).second ? false : true;
		_lstTextures->setRowColor(row, _textures->at(row).second ? _lstTextures->getColor() : _lstTextures->getSecondaryColor());
	}
}

/**
 * Reorders the texture list up
 * @param action Pointer to an action.
 */
void TextureOrderState::lstTexturesLeftArrowClick(Action *action)
{
	unsigned int row = _lstTextures->getSelectedRow();
	if (row > 0)
	{
		if (action->getDetails()->button.button == SDL_BUTTON_LEFT)
		{
			moveTextureUp(action, row);
		}
		else if (action->getDetails()->button.button == SDL_BUTTON_RIGHT)
		{
			moveTextureUp(action, row, true);
		}
	}
}

/**
 * Handles the moving of textures up on the list
 * @param action Pointer to an action.
 * @param row Selected row.
 * @param max Move the texture all the way to the top?
 */
void TextureOrderState::moveTextureUp(Action *action, unsigned int row, bool max)
{
	std::pair<int, bool> rowData = _textures->at(row);
	if (max)
	{
		_textures->erase(_textures->begin() + row);
		_textures->insert(_textures->begin(), rowData);
	}
	else
	{
		_textures->at(row) = _textures->at(row - 1);
		_textures->at(row - 1) = rowData;
		if (row != _lstTextures->getScroll())
		{
			SDL_WarpMouse(action->getLeftBlackBand() + action->getXMouse(), action->getTopBlackBand() + action->getYMouse() - static_cast<Uint16>(8 * action->getYScale()));
		}
		else
		{
			_lstTextures->scrollUp(false);
		}
	}

	size_t scroll = _lstTextures->getScroll();
	drawTextureList();
	if (scroll)
		_lstTextures->scrollTo(scroll);
}

/**
 * Reorders the texture list down
 * @param action Pointer to an action.
 */
void TextureOrderState::lstTexturesRightArrowClick(Action *action)
{
	unsigned int row = _lstTextures->getSelectedRow();
	if (0 < _textures->size() && INT_MAX >= _textures->size() && row < _textures->size() - 1)
	{
		if (action->getDetails()->button.button == SDL_BUTTON_LEFT)
		{
			moveTextureDown(action, row);
		}
		else if (action->getDetails()->button.button == SDL_BUTTON_RIGHT)
		{
			moveTextureDown(action, row, true);
		}
	}
}

/**
 * Handles the moving of textures down on the list
 * @param action Pointer to an action.
 * @param row Selected row.
 * @param max Move the texture all the way to the bottom?
 */
void TextureOrderState::moveTextureDown(Action *action, unsigned int row, bool max)
{
	std::pair<int, bool> rowData = _textures->at(row);
	if (max)
	{
		_textures->erase(_textures->begin() + row);
		_textures->insert(_textures->end(), rowData);
	}
	else
	{
		_textures->at(row) = _textures->at(row + 1);
		_textures->at(row + 1) = rowData;
		if (row != _lstTextures->getVisibleRows() - 1 + _lstTextures->getScroll())
		{
			SDL_WarpMouse(action->getLeftBlackBand() + action->getXMouse(), action->getTopBlackBand() + action->getYMouse() + static_cast<Uint16>(8 * action->getYScale()));
		}
		else
		{
			_lstTextures->scrollDown(false);
		}
	}

	drawTextureList();
}

void TextureOrderState::btnOkClick(Action *action)
{
	_game->popState();
}

}
