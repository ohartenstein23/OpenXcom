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

#include "../Engine/State.h"
#include <vector>
#include <string>

namespace OpenXcom
{

class GeoscapeGenerator;
class TextButton;
class TextEdit;
class Window;
class Text;
class ComboBox;
class Slider;
class Frame;

class GeoscapeGeneratorState : public State
{
private:
	Window *_window;
	Text *_txtTitle, *_txtSeed;
	TextButton *_btnOk, *_btnClear, *_btnCancel;
	TextEdit *_edtSeed;

	size_t _rngSeed;

public:
	/// Constructor
	GeoscapeGeneratorState();
	/// Cleans up the GeoscapeGeneratorState
	~GeoscapeGeneratorState();
	/// Initializes the data for the geoscape generator.
	void init();
	/// Handler for changing the RNG seed using the text editor.
	void edtSeedChange(Action *action);
	/// Handler for clicking the OK button.
	void btnOkClick(Action *action);
	/// Handler for clicking the Clear button.
	void btnClearClick(Action *action);
	/// Handler for clicking the Cancel button.
	void btnCancelClick(Action *action);

};

}
