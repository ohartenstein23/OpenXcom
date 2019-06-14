#pragma once
/*
 * Copyright 2010-2019 OpenXcom Developers.
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
#include "../Engine/State.h"
#include <vector>
#include <string>

namespace OpenXcom
{

class TextButton;
class TextList;
class Window;
class Text;

class MapEditorMenuState : public State
{
private :
	Window *_window;
	Text *_txtTitle, *_txtSelectedMap, *_txtSelectedMapTerrain;
	TextButton *_btnOk, *_btnCancel;
    TextList *_lstMaps;
    std::vector< std::pair<std::string, std::string> > _mapsList;
    int _selectedMap;

public :
    /// Creates the Map Editor Menu State
    MapEditorMenuState();
    /// Cleans up the Map Editor Menu State
    ~MapEditorMenuState();
    /// Initializes the data in the Map Editor Menu
    void init();
    /// Populates the list of available maps
    void populateMapsList();
    /// Handles clicking on the list of available maps
    void lstMapsClick(Action *action);
    /// Starts the Map Editor
    void btnOkClick(Action *action);
    /// Returns to the Main Menue
    void btnCancelClick(Action *action);

};

}