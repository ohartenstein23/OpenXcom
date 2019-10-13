/*
 * Copyright 2010-2016 OpenXcom Developers.
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
#include "SoldierTransformationListState.h"
#include "SoldiersState.h"
#include <climits>
#include "../Engine/Screen.h"
#include "../Engine/Game.h"
#include "../Mod/Mod.h"
#include "../Mod/RuleItem.h"
#include "../Mod/RuleSoldierTransformation.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/Options.h"
#include "../Interface/ComboBox.h"
#include "../Engine/Action.h"
#include "../Interface/TextButton.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "../Interface/TextEdit.h"
#include "../Interface/TextList.h"
#include "../Savegame/Base.h"
#include "../Savegame/ItemContainer.h"
#include "../Savegame/Soldier.h"
#include "../Savegame/SavedGame.h"
#include <algorithm>
#include "../Engine/Unicode.h"
#include "../Ufopaedia/Ufopaedia.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the Transfomrations list screen.
 * @param game Pointer to the core game.
 * @param parent Pointer to the Soldiers screen that started this list.
 * @param base Pointer to the base to get info from.
 */
SoldierTransformationListState::SoldierTransformationListState(SoldiersState *parent, Base *base, ComboBox *screenActions) : _parent(parent), _base(base), _screenActions(screenActions), _eligiblePressed(0)
{
	std::vector<RuleSoldierTransformation* > availableTransformations;
	_game->getSavedGame()->getAvailableTransformations(availableTransformations, _game->getMod(), _base);

	// Create objects
	_window = new Window(this, 320, 200, 0, 0);
	_txtTitle = new Text(168, 17, 16, 8);
    _txtProject = new Text(120, 9, 8, 40);
    _txtNumber = new Text(70, 17, 164, 32);
    _txtSoldierNumber = new Text(70, 17, 242, 32);
    _btnQuickSearch = new TextEdit(this, 48, 9, 8, 30);
	_btnHasEligible = new TextButton(120, 16, 192, 8);
    _btnCancel = new TextButton(304, 16, 8, 176);
	_cbxSoldierType = new ComboBox(this, 148, 16, 8, 158, true);
	_cbxSoldierStatus = new ComboBox(this, 148, 16, 164, 158, true);
	_lstTransformations = new TextList(288, 104, 8, 50);

	// Set palette
	setInterface("transformationList");

	add(_window, "window", "transformationList");
	add(_txtTitle, "text1", "transformationList");
    add(_txtProject, "text1", "transformationList");
    add(_txtNumber, "text1", "transformationList");
    add(_txtSoldierNumber, "text1", "transformationList");
	add(_btnQuickSearch, "button", "transformationList");
	add(_btnHasEligible, "button", "transformationList");
	add(_btnCancel, "button", "transformationList");
	add(_cbxSoldierType, "button", "transformationList");
	add(_cbxSoldierStatus, "button", "transformationList");
	add(_lstTransformations, "list", "transformationList");

	centerAllSurfaces();

	// Set up objects
	setWindowBackground(_window, "transformationList");

    _btnHasEligible->setText(tr("STR_SHOWING_ALL_TRANSFORMATIONS"));
	_btnHasEligible->onMouseClick((ActionHandler)&SoldierTransformationListState::btnHasEligibleClick);

	_btnCancel->setText(tr("STR_CANCEL"));
	_btnCancel->onMouseClick((ActionHandler)&SoldierTransformationListState::btnCancelClick);
	_btnCancel->onKeyboardPress((ActionHandler)&SoldierTransformationListState::btnCancelClick, Options::keyCancel);
	_btnCancel->onKeyboardPress((ActionHandler)&SoldierTransformationListState::btnCancelClick, Options::keyOk);

    std::vector<std::string> availableOptions;
    availableOptions.clear();
    availableOptions.push_back("STR_SELECT_SOLDIER_TYPE");
    for (auto& i : _game->getMod()->getSoldiersList())
    {
        availableOptions.push_back(i);
    }
    
    _cbxSoldierType->setOptions(availableOptions, true);
    _cbxSoldierType->setSelected(0);
    _cbxSoldierType->onChange((ActionHandler)&SoldierTransformationListState::cbxSoldierTypeChange);

    availableOptions.clear();
    availableOptions.push_back("STR_SELECT_SOLDIER_STATUS");
    availableOptions.push_back("STR_HEALTHY_SOLDIERS");
    availableOptions.push_back("STR_WOUNDED_SOLDIERS");
    availableOptions.push_back("STR_DEAD_SOLDIERS");

    _cbxSoldierStatus->setOptions(availableOptions, true);
	_cbxSoldierStatus->setSelected(0);
	_cbxSoldierStatus->onChange((ActionHandler)&SoldierTransformationListState::cbxSoldierStatusChange);

	_txtTitle->setBig();
	_txtTitle->setAlign(ALIGN_LEFT);
	_txtTitle->setText(tr("STR_SOLDIER_TRANSFORMATION_LIST"));

	_txtProject->setAlign(ALIGN_LEFT);
	_txtProject->setText(tr("STR_TRANSFORMATION_NAME"));

	_txtNumber->setAlign(ALIGN_CENTER);
	_txtNumber->setText(tr("STR_NUMBER_OF_TRANSFORMATIONS_BY_MATERIALS_AVAILABLE"));

	_txtSoldierNumber->setAlign(ALIGN_CENTER);
	_txtSoldierNumber->setText(tr("STR_NUMBER_OF_TRANSFORMATIONS_BY_SOLDIERS_ELIGIBLE"));

	_lstTransformations->setColumns(3, 186, 10, 78);
    _lstTransformations->setAlign(ALIGN_RIGHT, 1);
    _lstTransformations->setAlign(ALIGN_RIGHT, 2);
	_lstTransformations->setSelectable(true);
	_lstTransformations->setBackground(_window);
	//_lstTransformations->setMargin(8);
	_lstTransformations->onMouseClick((ActionHandler)&SoldierTransformationListState::lstTransformationsClick);
    _lstTransformations->onMousePress((ActionHandler)&SoldierTransformationListState::lstTransformationsMousePress);

	_btnQuickSearch->setText(""); // redraw
	_btnQuickSearch->onEnter((ActionHandler)&SoldierTransformationListState::btnQuickSearchApply);
	_btnQuickSearch->setVisible(false);

	_btnCancel->onKeyboardRelease((ActionHandler)&SoldierTransformationListState::btnQuickSearchToggle, Options::keyToggleQuickSearch);

    initList();
}

/**
 * cleans up dynamic state
 */
SoldierTransformationListState::~SoldierTransformationListState()
{
    
}

/**
 * Shows the soldiers in a list at specified offset/scroll.
 */
void SoldierTransformationListState::initList()
{
	_lstTransformations->clearList();
    _transformationIndices.clear();

	std::string searchString = _btnQuickSearch->getText();
	Unicode::upperCase(searchString);

	std::vector<RuleSoldierTransformation* > availableTransformations;
	_game->getSavedGame()->getAvailableTransformations(availableTransformations, _game->getMod(), _base);

    int currentIndex = -1;
    ItemContainer * itemContainer (_base->getStorageItems());
    for(auto transformationRule : availableTransformations)
    {
        ++currentIndex;

        // quick search
        if (!searchString.empty())
        {
            std::string transformationName = tr(transformationRule->getName());
            Unicode::upperCase(transformationName);
            if (transformationName.find(searchString) == std::string::npos)
            {
                continue;
            }
        }

        if (_cbxSoldierType->getSelected() != 0 &&
            std::find(transformationRule->getAllowedSoldierTypes().begin(), transformationRule->getAllowedSoldierTypes().end(), _game->getMod()->getSoldiersList().at(_cbxSoldierType->getSelected() - 1)) == transformationRule->getAllowedSoldierTypes().end()
        )
            continue;

        switch (_cbxSoldierStatus->getSelected())
        {
            case 1:
                if (!transformationRule->isAllowingAliveSoldiers())
                    continue;
                break;
            
            case 2:
                if (!transformationRule->isAllowingWoundedSoldiers())
                    continue;
                break;
            
            case 3:
                if (!transformationRule->isAllowingDeadSoldiers())
                    continue;
                break;
            
            default:
                break;
        }

        std::ostringstream col1;
        std::ostringstream col2;

        // supplies calculation
        int projectsPossible = 10; // max
        if (transformationRule->getCost() > 0)
        {
            int byFunds = _game->getSavedGame()->getFunds() / transformationRule->getCost();
            projectsPossible = std::min(projectsPossible, byFunds);
        }
        for (auto item : transformationRule->getRequiredItems())
        {
            RuleItem *itemRule = _game->getMod()->getItem(item.first);
            projectsPossible = std::min(projectsPossible, itemContainer->getItem(itemRule->getType()) / item.second);
        }
        if (projectsPossible <= 0)
        {
            col1 << '-';
        }
        else
        {
            if (projectsPossible < 10)
            {
                col1 << projectsPossible;
            }
            else
            {
                col1 << '+';
            }
        }

        int eligibleSoldiers = 0;
        for (auto& soldier : *_base->getSoldiers())
        {
            if (soldier->isEligibleForTransformation(transformationRule))
            {
                ++eligibleSoldiers;
            }
        }
        for (auto& deadMan : *_game->getSavedGame()->getDeadSoldiers())
        {
            if (deadMan->isEligibleForTransformation(transformationRule))
            {
                ++eligibleSoldiers;
            }
        }
        
        if (eligibleSoldiers <= 0)
        {
            if (_eligiblePressed != 0)
                continue;

            col2 << '-';
        }
        else
        {
            if (eligibleSoldiers < 10)
            {
                col2 << eligibleSoldiers;
            }
            else
            {
                col2 << '+';
            }
        }

        _lstTransformations->addRow(3, tr(transformationRule->getName()).c_str(), col1.str().c_str(), col2.str().c_str());
        _transformationIndices.push_back(currentIndex);
    }

	_lstTransformations->draw();
}

/**
* Quick search toggle.
* @param action Pointer to an action.
*/
void SoldierTransformationListState::btnQuickSearchToggle(Action *action)
{
	if (_btnQuickSearch->getVisible())
	{
		_btnQuickSearch->setText("");
		_btnQuickSearch->setVisible(false);
		btnQuickSearchApply(action);
	}
	else
	{
		_btnQuickSearch->setVisible(true);
		_btnQuickSearch->setFocus(true);
	}
}

/**
* Quick search.
* @param action Pointer to an action.
*/
void SoldierTransformationListState::btnQuickSearchApply(Action *)
{
	initList();
}

/**
 * Filters the list of transformations by soldier type
 * @param action Pointer to an action
 */
void SoldierTransformationListState::cbxSoldierTypeChange(Action *action)
{
	initList();
}

/**
 * Filters the list of transformations by soldier status
 * @param action Pointer to an action
 */
void SoldierTransformationListState::cbxSoldierStatusChange(Action *action)
{
	initList();
}

/**
 * Toggles showing transformations with eligible soldiers and all transformations
 * @param action Pointer to an action.
 */
void SoldierTransformationListState::btnHasEligibleClick(Action *)
{
    if (_eligiblePressed == 0) 
    {
        _eligiblePressed = _btnHasEligible;
        _btnHasEligible->setText(tr("STR_SHOWING_TRANSFORMATIONS_WITH_ELIGIBLE_SOLDIERS"));
        _btnHasEligible->setGroup(&_eligiblePressed);
    }
    else
    {
        _eligiblePressed = 0;
        _btnHasEligible->setText(tr("STR_SHOWING_ALL_TRANSFORMATIONS"));
        _btnHasEligible->setGroup(0);
    }

    initList();
}

/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void SoldierTransformationListState::btnCancelClick(Action *)
{
    _screenActions->setSelected(0);
	_game->popState();
}

/**
 * Shows the selected soldier's info.
 * @param action Pointer to an action.
 */
void SoldierTransformationListState::lstTransformationsClick(Action *action)
{
    int transformationIndex = _transformationIndices.at(_lstTransformations->getSelectedRow());

    _screenActions->setSelected(_screenActions->getSelected() + transformationIndex + 1);
    _parent->init();

    _game->popState();
}

/**
 * Brings up pedia articles for selected transformation
 * @param action Pointer to an action.
 */
void SoldierTransformationListState::lstTransformationsMousePress(Action *action)
{
 	if (action->getDetails()->button.button == SDL_BUTTON_MIDDLE)
	{
        std::vector<RuleSoldierTransformation* > availableTransformations;
	    _game->getSavedGame()->getAvailableTransformations(availableTransformations, _game->getMod(), _base);
        int transformationIndex = _transformationIndices.at(_lstTransformations->getSelectedRow());
		std::string articleId = availableTransformations.at(transformationIndex)->getName();
		Ufopaedia::openArticle(_game, articleId);
	}
}

}
