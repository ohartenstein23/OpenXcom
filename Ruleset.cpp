/*
 * Copyright 2010 Daniel Albano
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
#include "Ruleset.h"

/**
 * Creates a ruleset with blank sets of rules.
 */
Ruleset::Ruleset() : _names(), _facilities(), _crafts()
{
}

/**
 * Deletes all the contained rules from memory.
 */
Ruleset::~Ruleset()
{
	for (vector<SoldierNamePool*>::iterator i = _names.begin(); i != _names.end(); i++)
	{
		delete *i;
	}
	for (map<LangString, RuleBaseFacility*>::iterator i = _facilities.begin(); i != _facilities.end(); i++)
	{
		delete i->second;
	}
	for (map<LangString, RuleCraft*>::iterator i = _crafts.begin(); i != _crafts.end(); i++)
	{
		delete i->second;
	}
}

/**
 * Generates a brand new blank saved game.
 * @param diff Difficulty for the save.
 * @return New saved game.
 */
SavedGame *Ruleset::newSave(GameDifficulty diff)
{
	SavedGame *save = new SavedGame(diff);

	return save;
}

/**
 * Returns the list of soldier name pools.
 * @return Pointer to soldier name pool list.
 */
vector<SoldierNamePool*> *Ruleset::getPools()
{
	return &_names;
}

/**
 * Returns the rules for the specified base facility.
 * @param id Facility type.
 * @return Rules for the facility.
 */
RuleBaseFacility *Ruleset::getBaseFacility(LangString id)
{
	return _facilities[id];
}

/**
 * Returns the rules for the specified craft.
 * @param id Craft type.
 * @return Rules for the craft.
 */
RuleCraft *Ruleset::getCraft(LangString id)
{
	return _crafts[id];
}