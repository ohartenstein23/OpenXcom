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
 * along with OpenXcom.  If not, see <http:///www.gnu.org/licenses/>.
 */
#include "../Engine/State.h"
#include <list>

namespace OpenXcom
{

class Surface;
class GlobeEditable;
class TextButton;
class InteractiveSurface;
class Text;
class ComboBox;
class Timer;
class MissionSite;

/**
 * Geoscape screen which shows an overview of
 * the world and lets the player manage the game.
 */
class GeoscapeEditorState : public State
{
private:
	Surface *_bg, *_sideLine, *_sidebar;
	GlobeEditable *_globe;
    TextButton *_btnOptions;
    TextButton *_btnPause, *_btnSlow, *_btnFast, *_btnStop, *_btnReset, *_btnTime, *_timeSpeed;
	TextButton *_sideTop, *_sideBottom;
	InteractiveSurface *_btnRotateLeft, *_btnRotateRight, *_btnRotateUp, *_btnRotateDown, *_btnZoomIn, *_btnZoomOut;
	Timer *_gameTimer;
	bool _pause;
	Text *_txtDebug;

public:
	/// Creates the Geoscape state.
	GeoscapeEditorState();
	/// Cleans up the Geoscape state.
	~GeoscapeEditorState();
	/// Handle keypresses.
	void handle(Action *action) override;
	/// Updates the palette and timer.
	void init() override;
	/// Runs the timer.
	void think() override;
	/// Advances the game timer.
	void timeAdvance();
    /// Updates the clock
    void timeDisplay();
	/// Gets the Geoscape globe.
	GlobeEditable *getGlobe() const;
	/// Handler for clicking the globe.
	void globeClick(Action *action);
	/// Handler for clicking the [SelectMusicTrack] button.
	void btnSelectMusicTrackClick(Action *action);
	/// Handler for clicking the Options button.
	void btnOptionsClick(Action *action);
	/// Handler for pressing the Rotate Left arrow.
	void btnRotateLeftPress(Action *action);
	/// Handler for releasing the Rotate Left arrow.
	void btnRotateLeftRelease(Action *action);
	/// Handler for pressing the Rotate Right arrow.
	void btnRotateRightPress(Action *action);
	/// Handler for releasing the Rotate Right arrow.
	void btnRotateRightRelease(Action *action);
	/// Handler for pressing the Rotate Up arrow.
	void btnRotateUpPress(Action *action);
	/// Handler for releasing the Rotate Up arrow.
	void btnRotateUpRelease(Action *action);
	/// Handler for pressing the Rotate Down arrow.
	void btnRotateDownPress(Action *action);
	/// Handler for releasing the Rotate Down arrow.
	void btnRotateDownRelease(Action *action);
	/// Handler for left-clicking the Zoom In icon.
	void btnZoomInLeftClick(Action *action);
	/// Handler for right-clicking the Zoom In icon.
	void btnZoomInRightClick(Action *action);
	/// Handler for left-clicking the Zoom Out icon.
	void btnZoomOutLeftClick(Action *action);
	/// Handler for right-clicking the Zoom Out icon.
	void btnZoomOutRightClick(Action *action);
	/// Blit method - renders the state.
	void blit() override;
	/// Handler for clicking the timer button.
	void btnTimerClick(Action *action);
    /// Handler for clicking the set time button.
    void btnTimeClick(Action *action);
	/// Update the resolution settings, we just resized the window.
	void resize(int &dX, int &dY) override;
};

}
