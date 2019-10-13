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
#include "SoldierSortUtil.h"

namespace OpenXcom
{

class TextButton;
class Window;
class Text;
class TextEdit;
class TextList;
class ComboBox;
class Base;
class Soldier;
class SoldiersState;

/**
 * Soldiers screen that lets the player
 * manage all the soldiers in a base.
 */
class SoldierTransformationListState : public State
{
private:
    SoldiersState *_parent;
	Base *_base;
    ComboBox *_screenActions;
	TextButton *_btnHasEligible, *_eligiblePressed, *_btnCancel;
	Window *_window;
	Text *_txtTitle, *_txtProject, *_txtNumber, *_txtSoldierNumber;
	ComboBox *_cbxSoldierType, *_cbxSoldierStatus;
    TextEdit *_btnQuickSearch;
	TextList *_lstTransformations;
    std::vector<int> _transformationIndices;
public:
	/// Creates the Transformation list State.
	SoldierTransformationListState(SoldiersState *parent, Base *base, ComboBox *screenActions);
	/// Cleans up the Transformation list state.
	~SoldierTransformationListState();
    /// Refreshes the list of transformations.
    void initList();
    /// Toggles the quick search
    void btnQuickSearchToggle(Action *action);
    /// Applies the quick search
    void btnQuickSearchApply(Action *action);
	/// Handler for changing the soldier type combo box.
	void cbxSoldierTypeChange(Action *action);
	/// Handler for changing the soldier status combo box.
	void cbxSoldierStatusChange(Action *action);
    /// Handler for toggling showing transformations with eligible soldiers and all transformations
    void btnHasEligibleClick(Action *action);
    /// Closes the transformations list without selecting one.
    void btnCancelClick(Action *action);
	/// Handler for clicking the Transformations list.
	void lstTransformationsClick(Action *action);
    /// Handler for m-clicking the Transformations list.
	void lstTransformationsMousePress(Action *action);
};

}
