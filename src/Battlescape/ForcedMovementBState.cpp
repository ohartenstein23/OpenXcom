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
#include "Camera.h"
#include "Explosion.h"
#include "Map.h"
#include "Pathfinding.h"
#include "TileEngine.h"
#include "UnitFallBState.h"
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
ForcedMovementBState::ForcedMovementBState(BattlescapeGame *parent, BattleAction action, BattleUnit *unit) : BattleState(parent, action), _unit(unit), _isTargeted(false), _isWarp(false), _unitSize(0), _targetPosition(-1, -1, -1), _movementFinished(false)
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
 * Does a lot of validation too
 */
void ForcedMovementBState::init()
{
	// Make sure we have a unit, otherwise this shouldn't run!
	if (!_unit)
	{
		_parent->popState();
		return;
	}

	_unitSize = _unit->getArmor()->getSize() - 1;

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
			return;
		}
		else
		{
			_targetPosition = _action.target;
		}
	}

	if (!validateTarget())
	{
		_parent->popState();
		return;
	}

	if (!_action.haveTU())
	{
		_action.result = "STR_NOT_ENOUGH_TIME_UNITS";
		_parent->popState();
		return;
	}
	else
	{
		_action.spendTU();
	}

	if (_isWarp)
	{
		int frame = Mod::EXPLOSION_OFFSET;
		int sound = Mod::SMALL_EXPLOSION;

		frame = _action.weapon->getRules()->getHitAnimation();
		if (_action.weapon->getRules()->getExplosionHitSound());
		{
			sound = _action.weapon->getRules()->getExplosionHitSound();
		}

		if (_parent->getDepth() > 0)
		{
			frame -= Explosion::EXPLODE_FRAMES;
		}

		Explosion *explosion = new Explosion(_unit->getPosition().toVexel(), frame, false, true);
		// add the explosion on the map
		_parent->getMap()->getExplosions()->push_back(explosion);
		_parent->setStateInterval(BattlescapeState::DEFAULT_ANIM_SPEED/2);
		// explosion sound
		_parent->playSound(sound);
		//_parent->getMap()->getCamera()->centerOnPosition(_center.toTile(), false);
	}

	return;
}

/**
 * Animates the forced movement state
 */
void ForcedMovementBState::think()
{
	// Start by animating any start explosion
	if (!_parent->getMap()->getExplosions()->empty()) //!_parent->getMap()->getBlastFlash() && 
	{
		for(std::list<Explosion*>::iterator explosion = _parent->getMap()->getExplosions()->begin(); explosion != _parent->getMap()->getExplosions()->end();)
		{

			if (!(*explosion)->animate())
			{
				delete (*explosion);
				explosion = _parent->getMap()->getExplosions()->erase(explosion);
				if (_parent->getMap()->getExplosions()->empty())
				{
					break;
				}
			}
			else
			{
				++explosion;
			}
		}
	}
	else if (!_movementFinished) // If the start explosion is done, we can move the unit
	{
		_movementFinished = true;

		// Handle teleports
		if (_isWarp && _action.actor == _unit)
		{
			// Move the unit
			for (int x = _unitSize; x >= 0; x--)
			{
				for (int y = _unitSize; y >= 0; y--)
				{
					_parent->getSave()->getTile(_unit->getPosition() + Position(x, y, 0))->setUnit(0);
				}
			}

			_unit->setPosition(_targetPosition);

			for (int x = _unitSize; x >= 0; x--)
			{
				for (int y = _unitSize; y >= 0; y--)
				{
					_parent->getSave()->getTile(_unit->getPosition() + Position(x,y,0))->setUnit(_unit, _parent->getSave()->getTile(_unit->getPosition() + Position(x,y,-1)));
				}
			}

			_parent->getSave()->getTile(_targetPosition)->setUnit(_unit);
			_parent->getSave()->getTileEngine()->calculateLighting(LL_UNITS);
			_parent->getSave()->getTileEngine()->calculateFOV(_unit, 2, false);

			int frame = Mod::EXPLOSION_OFFSET;
			int sound = Mod::SMALL_EXPLOSION;

			frame = _action.weapon->getRules()->getHitAnimation();
			if (_action.weapon->getRules()->getExplosionHitSound());
			{
				sound = _action.weapon->getRules()->getExplosionHitSound();
			}

			if (_parent->getDepth() > 0)
			{
				frame -= Explosion::EXPLODE_FRAMES;
			}

			Explosion *explosion = new Explosion(_unit->getPosition().toVexel(), frame, false, true);
			// add the explosion on the map
			_parent->getMap()->getExplosions()->push_back(explosion);
			_parent->setStateInterval(BattlescapeState::DEFAULT_ANIM_SPEED/2);
			// explosion sound
			_parent->playSound(sound);
			_parent->getMap()->getCamera()->centerOnPosition(_unit->getPosition(), false);
		}
	}
	else // Both movement and all animations are done, pop the state
	{
		_parent->popState();
		if (_fallAtEnd)
		{
			_parent->getSave()->addFallingUnit(_unit);
			_parent->statePushFront(new UnitFallBState(_parent));
		}
		else if (_isWarp && _action.actor == _unit)
		{
			// Let reaction fire happen at the end of warps
			_parent->getSave()->getTileEngine()->checkReactionFire(_unit, _action);
		}
	}

	return;
}

/**
 * Cancels the forced movement state
 */
void ForcedMovementBState::cancel()
{
	//TODO: implement skipping the animation, but not the action
}

/**
 * Validates the target location for the movement
 * Also tests if the unit should fall at the end
 * @return True for valid location
 */
bool ForcedMovementBState::validateTarget()
{
	// Check if this action is range- and path-limited
	int distance = sqrt(_parent->getTileEngine()->distanceSq(_action.actor->getPosition(), _action.target, true));
	bool validPath = true;

	if (_action.weapon->getRules()->getMaxRange() < distance)
	{
		_action.result = "STR_OUT_OF_RANGE";
		return false;
	}

	if (_action.actor == _unit && _action.weapon->getRules()->getForcedMovementRequiresPath())
	{
		int pathingType = _action.weapon->getRules()->getForcedMovementPathingType();
		MovementType movementType = _action.actor->getMovementType();

		if (pathingType != -1)
		{
			switch (pathingType)
			{
				case 0:
					_action.actor->setMovementType(MT_WALK);
					break;
				case 1:
					_action.actor->setMovementType(MT_FLY);
					break;
				case 2:
					_action.actor->setMovementType(MT_SLIDE);
					break;
				default:
					break;
			}
		}

		_parent->getSave()->getPathfinding()->calculate(_action.actor, _action.target);
		
		if (_parent->getSave()->getPathfinding()->getStartDirection() == -1)
		{
			_action.result = "STR_UNABLE_TO_MOVE_HERE";
			validPath = false;
		}

		if (_action.weapon->getRules()->getForcedMovementRangeType() == 1)
		{
			distance = _parent->getSave()->getPathfinding()->getTotalTUCost();
		}

		_parent->getSave()->getPathfinding()->abortPath();
		_action.actor->setMovementType(movementType);

		if (!validPath)
		{
			return false;
		}
	}

	if (_action.actor == _unit && _action.weapon->getRules()->getForcedMovementRangeType() != -1)
	{
		int range = _action.weapon->getRules()->getPowerBonus(_action.actor);

		if (range < distance)
		{
			_action.result = "STR_OUT_OF_RANGE";
			return false;
		}
	}

	// Check if we need LOS to the target position
	if (_action.weapon->getRules()->isLOSRequired() && !_parent->getTileEngine()->isTileInLOS(&_action, _parent->getSave()->getTile(_targetPosition)))
	{
		_action.result = "STR_LINE_OF_SIGHT_REQUIRED";
		return false;
	}

	// Do we need to check for falling from the target location?
	_fallAtEnd = ((_targetPosition.z != 0) && (_unit->getMovementType() != MT_FLY));

	for (int x = _unitSize; x >= 0; x--)
	{
		for (int y = _unitSize; y >= 0; y--)
		{
			// Make sure the location is in bounds and nothing blocks being there
			Position positionToCheck = _targetPosition + Position(x, y, 0);
			Tile* tileToCheck = _parent->getSave()->getTile(positionToCheck);
			if (!tileToCheck || (tileToCheck->getUnit() && tileToCheck->getUnit() != _unit) ||
				tileToCheck->getTUCost(O_OBJECT, _unit->getMovementType()) == 255 ||
				(tileToCheck->getMapData(O_OBJECT) && tileToCheck->getMapData(O_OBJECT)->getBigWall() && tileToCheck->getMapData(O_OBJECT)->getBigWall() <= 3))
			{
				_action.result = "STR_UNABLE_TO_MOVE_HERE";
				return false;
			}

			// Check to see if the unit will fall from the target location
			if (_fallAtEnd)
			{
				Tile *tileBelow = _parent->getSave()->getTile(positionToCheck + Position(0, 0, -1));
				if (!_parent->getSave()->getTile(positionToCheck)->hasNoFloor(tileBelow))
				{
					_fallAtEnd = false;
				}
			}
		}
	}

	// Extra test for large units
	if (_unitSize > 0)
	{
		_parent->getSave()->getPathfinding()->setUnit(_unit);
		for (int dir = 2; dir <= 4; ++dir)
		{
			if (_parent->getSave()->getPathfinding()->isBlocked(_parent->getSave()->getTile(_targetPosition), 0, dir, 0))
			{
				_action.result = "STR_UNABLE_TO_MOVE_HERE";
				return false;
			}
		}
	}

	return true;
}

}
