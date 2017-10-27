/*
 * Copyright 2010-2017 OpenXcom Developers.
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
#include "ForcedMovementBState.h"
#include "BattlescapeState.h"
#include "TileEngine.h"
#include "../Mod/RuleItem.h"
#include "../Savegame/BattleItem.h"
#include "../Savegame/BattleUnit.h"
#include "../Savegame/SavedBattleGame.h"
#include "../Savegame/Tile.h"

namespace OpenXcom
{

/**
 * Sets up a forced movement battle state
 * @param parent Pointer to the BattleScape.
 * @param action Pointer to the action that caused this state.
 * @param unit Pointer to the unit being moved.
 */
ForcedMovementBState::ForcedMovementBState(BattlescapeGame *parent, BattleAction action, BattleUnit *unit) : BattleState(parent, action), _unit(unit), _isTargeted(false), _isWarp(false)
{
	// TODO initialization commands here
}

/**
 * Cleans up the forced movement battle state
 */
ForcedMovementBState::~ForcedMovementBState()
{

}

/**
 * Initializes the forced movement state
 * Calculates the end point of the movement and starts the animation if necessary
 */
void ForcedMovementBState::init()
{
	// This shouldn't run without an item! (change for jumping?)
	BattleItem *item = _action.weapon;
	if (!item)
	{
		_parent->popState();
		return;
	}

	_isTargeted = item->getRules()->getForcedMovementIsTargeted();
	_isWarp = item->getRules()->getForcedMovementIsWarp();

	if (_isWarp && _action.actor == _unit) // This is a warp, just go teleport the unit!
	{
		if (!_parent->getSave()->getTile(_action.target)) // If we don't have a target, the warp is invalid
		{
			_parent->popState();
		}
		return;
	}
}

/**
 * Animates the forced movement state
 */
void ForcedMovementBState::think()
{
	// Handle teleports
	if (_isWarp && _action.actor == _unit)
	{
		_unit->getTile()->setUnit(0);
		_unit->setPosition(_action.target);
		_parent->getSave()->getTile(_action.target)->setUnit(_unit);
		_parent->getSave()->getTileEngine()->calculateLighting(LL_UNITS);
		//_parent->handleState();
	}
}

/**
 * Cancels the forced movement state
 */
void ForcedMovementBState::cancel()
{

}

}
