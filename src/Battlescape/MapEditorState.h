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
 * along with OpenXcom.  If not, see <http:///www.gnu.org/licenses/>.
 */
#include "../Engine/State.h"
#include "Position.h"

#include <vector>
#include <string>

namespace OpenXcom
{

class Surface;
class Map;
class ImageButton;
class BattlescapeButton;
class InteractiveSurface;
class Text;
class TextButton;
class TextEdit;
class SavedBattleGame;
class Timer;
class MapEditor;

/**
 * Interface screen for the Map Editor
 * Very similar to a pared-down BattlescapeState
 */
class MapEditorState : public State
{
private:
	Map *_map;
	Timer *_animTimer, *_gameTimer;
	SavedBattleGame *_save;
    TextButton *_btnOptions;
	Text *_txtDebug, *_txtTooltip;
    int _tooltipDefaultColor;
	bool _firstInit;
	bool _isMouseScrolling, _isMouseScrolled;
	int _xBeforeMouseScrolling, _yBeforeMouseScrolling;
	Position _mapOffsetBeforeMouseScrolling;
	Uint32 _mouseScrollingStartTime;
	int _totalMouseMoveX, _totalMouseMoveY;
	bool _mouseMovedOverThreshold;
	bool _mouseOverIcons;
	std::string _currentTooltip;
	Position _cursorPosition;
	bool _autosave;
	Uint8 _indicatorTextColor, _indicatorGreen, _indicatorBlue, _indicatorPurple;
	MapEditor *_editor;
	Text *_txtSelectedIndex, *_txtEditRegister;
	TextEdit *_edtSelectedIndex;
	TextButton *_tileSelection, *_panelTileSelection, *_tileSelectionPageCount;
	TextButton *_tileSelectionLeftArrow, *_tileSelectionRightArrow;
	std::vector<InteractiveSurface*> _tileSelectionGrid;
	int _tileSelectionColumns, _tileSelectionRows, _tileSelectionCurrentPage, _tileSelectionLastPage;
public:
	static const int DEFAULT_ANIM_SPEED = 100;
	/// Creates the Map Editor state.
	MapEditorState(MapEditor *editor);
	/// Cleans up the Map Editor state.
	~MapEditorState();
	/// Initializes the MapEditorState.
	void init() override;
	/// Runs the timers.
	void think() override;
	/// Handler for moving mouse over the map.
	void mapOver(Action *action);
	/// Handler for pressing the map.
	void mapPress(Action *action);
	/// Handler for clicking the map.
	void mapClick(Action *action);
	/// Handler for entering with mouse to the map surface.
	void mapIn(Action *action);
    /// Handler for clicking the options button
    void btnOptionsClick(Action *action);
	/// Handler for clicking the Map Up button.
	//void btnMapUpClick(Action *action);
	/// Handler for clicking the Map Down button.
	//void btnMapDownClick(Action *action);
	/// Handler for clicking the [SelectMusicTrack] button.
	void btnSelectMusicTrackClick(Action *action);;
	/// Animates map objects on the map
	void animate();
	/// Handles the battle game state.
	void handleState();
	/// Sets the state timer interval.
	void setStateInterval(Uint32 interval);
	/// Gets game.
	Game *getGame() const;
	/// Gets map.
	Map *getMap() const;
	/// Show debug message.
	void debug(const std::string &message);
	/// Show warning message.
	void warning(const std::string &message);
	/// Handles keypresses.
	void handle(Action *action) override;
	/// Clears mouse-scrolling state.
	void clearMouseScrollingState();
	/// Returns a pointer to the battlegame, in case we need its functions.
	//BattlescapeGame *getBattleGame();
	/// Handler for the mouse moving over the icons, disables the tile selection cube.
	void mouseInIcons(Action *action);
	/// Handler for the mouse going out of the icons, enabling the tile selection cube.
	void mouseOutIcons(Action *action);
	/// Checks if the mouse is over the icons.
	bool getMouseOverIcons() const;
	/// Handler for showing tooltip.
	void txtTooltipIn(Action *action);
	/// Handler for hiding tooltip.
	void txtTooltipOut(Action *action);
	/// Update the resolution settings, we just resized the window.
	void resize(int &dX, int &dY) override;
	/// Move the mouse back to where it started after we finish drag scrolling.
	void stopScrolling(Action *action);
	/// Autosave next turn.
	//void autosave();
	/// Gets the pointer to the map editor
	MapEditor *getMapEditor();
	/// Changes the tile index selected for the map editor
	void edtSelectedIndexChange(Action *action);
	/// Toggles the tile selection UI
	void tileSelectionClick(Action *action);
	/// Draws the tile sprites on the selection grid
	void drawTileSelectionGrid();
	/// Moves the tile selection UI left one page
	void tileSelectionLeftArrowClick(Action *action);
	/// Moves the tile selection UI right one page
	void tileSelectionRightArrowClick(Action *action);
	/// Selects the tile from the tile selection UI
	void tileSelectionGridClick(Action *action);
	/// Handles mouse wheel scrolling for the tile selectionUI
	void tileSelectionMousePress(Action *action);
	/// Draws a tile sprite on a given surface
	bool drawTileSpriteOnSurface(Surface *surface, int index);
};

}