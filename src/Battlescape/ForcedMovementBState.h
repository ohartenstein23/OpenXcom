#pragma once
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
#include "BattleState.h"
#include "Position.h"

namespace OpenXcom
{

class BattlescapeGame;
class BattleAction;
class BattleUnit;
class Tile;

/**
 * Runs a forced movement on a unit
 */
class ForcedMovementBState : public BattleState
{
private:
	BattleUnit *_unit;
	bool _isTargeted, _isWarp;
	int _unitSize;
	Position _targetPosition;
	bool _fallAtEnd, _movementFinished;
public:
	/// Creates a new ForcedMovementBState class.
	ForcedMovementBState(BattlescapeGame *parent, BattleAction action, BattleUnit *unit);
        /// Cleans up the ForcedMovementBState.
        ~ForcedMovementBState();
	/// Initializes the state.
	void init();
	/// Handles a cancel request.
	void cancel();
	/// Runs state functionality every cycle.
	void think();
	/// Checks whether or not a target endpoint is valid.
	bool validateTarget();
};

}
