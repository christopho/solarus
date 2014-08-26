/*
 * Copyright (C) 2006-2014 Christopho, Solarus - http://www.solarus-games.org
 * 
 * Solarus is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Solarus is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "hero/LiftingState.h"
#include "hero/CarryingState.h"
#include "hero/HeroSprites.h"
#include "entities/MapEntities.h"
#include "entities/CarriedItem.h"
#include "lowlevel/Debug.h"
#include "Game.h"
#include "Map.h"
#include "KeysEffect.h"

namespace solarus {

/**
 * \brief Constructor.
 * \param hero The hero controlled by this state.
 * \param lifted_item The item to lift (will be destroyed automatically).
 */
Hero::LiftingState::LiftingState(Hero& hero, CarriedItem* lifted_item):
  State(hero, "lifting"),
  lifted_item(lifted_item) {

  Debug::check_assertion(lifted_item != nullptr, "Missing lifted item");
  RefCountable::ref(lifted_item);
}

/**
 * \brief Destructor.
 */
Hero::LiftingState::~LiftingState() {

  destroy_lifted_item();
}

/**
 * \brief Starts this state.
 * \param previous_state the previous state
 */
void Hero::LiftingState::start(const State* previous_state) {

  State::start(previous_state);

  // initialize the entity that will be lifted
  lifted_item->set_map(get_map());

  get_keys_effect().set_action_key_effect(KeysEffect::ACTION_KEY_THROW);
  get_sprites().set_animation_lifting();
  get_sprites().set_lifted_item(lifted_item);
  get_hero().set_facing_entity(nullptr);

  get_equipment().notify_ability_used(ABILITY_LIFT);
}

/**
 * \brief Ends this state.
 * \param next_state the next state
 */
void Hero::LiftingState::stop(const State* next_state) {

  State::stop(next_state);

  if (lifted_item != nullptr) {

    get_sprites().set_lifted_item(nullptr);

    // the lifted item is still managed by this state
    switch (next_state->get_previous_carried_item_behavior()) {

    case CarriedItem::BEHAVIOR_THROW:
      throw_item();
      break;

    case CarriedItem::BEHAVIOR_DESTROY:
      destroy_lifted_item();
      break;

    case CarriedItem::BEHAVIOR_KEEP:
      // The next state is now the owner and has incremented the refcount.
      Debug::check_assertion(lifted_item->get_refcount() > 1,
          "Invalid carried item refcount");
      RefCountable::unref(lifted_item);
      lifted_item = nullptr;
      break;
    }
    get_keys_effect().set_action_key_effect(KeysEffect::ACTION_KEY_NONE);
  }
}

/**
 * \brief Updates this state.
 */
void Hero::LiftingState::update() {

  State::update();

  lifted_item->update();

  if (!is_suspended() && !lifted_item->is_being_lifted()) { // the item has finished being lifted

    Hero& hero = get_hero();
    CarriedItem *carried_item = lifted_item;
    lifted_item = nullptr; // we do not take care of the carried item from this state anymore
    hero.set_state(new CarryingState(hero, carried_item));
  }
}

/**
 * \brief Notifies this state that the game was just suspended or resumed.
 * \param suspended true if the game is suspended
 */
void Hero::LiftingState::set_suspended(bool suspended) {

  State::set_suspended(suspended);

  if (lifted_item != nullptr) {
    lifted_item->set_suspended(suspended);
  }
}

/**
 * \brief Returns whether the hero can be hurt in this state.
 * \return true if the hero can be hurt in this state
 * \param attacker an attacker that is trying to hurt the hero
 * (or nullptr if the source of the attack is not an enemy)
 */
bool Hero::LiftingState::can_be_hurt(MapEntity* /* attacker */) const {
  return true;
}

/**
 * \brief Throws the item that is being lifted.
 *
 * This function is called when this state is interrupted by a new state,
 * e.g. when the hero is hurt while lifting an item.
 */
void Hero::LiftingState::throw_item() {

  lifted_item->throw_item(get_sprites().get_animation_direction());
  get_entities().add_entity(lifted_item);
  lifted_item = nullptr;
}

/**
 * \brief Destroys the item being lifted if any and sets it to nullptr.
 */
void Hero::LiftingState::destroy_lifted_item() {

  RefCountable::unref(lifted_item);
  lifted_item = nullptr;
}

}

