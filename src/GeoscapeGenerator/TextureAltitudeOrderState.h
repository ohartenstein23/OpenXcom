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
class TextList;
class Window;
class Text;
class ComboBox;
class Slider;
class Frame;
class Action;

class TextureAltitudeOrderState: public State
{
private:
	GeoscapeGeneratorState *_parent;
	Window *_window;
	TextList *_lstTextures;
	TextButton *_btnOk;

public:
	// Constructor
	TextureAltitudeOrderState(GeoscapeGeneratorState *parent);
	// Destructor
	~TextureAltitudeOrderState();

	// Handler for pressing the OK button
	void btnOkClick(Action *action);
};

}
