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
#include "hero/JumpingState.h"
#include "hero/FreeState.h"
#include "hero/CarryingState.h"
#include "hero/HeroSprites.h"
#include "entities/MapEntities.h"
#include "movements/JumpMovement.h"
#include "lowlevel/Sound.h"
#include "lowlevel/Debug.h"
#include "Game.h"
#include "Map.h"

namespace solarus {

/**
 * \brief Constructor.
 * \param hero The hero controlled by this state.
 * \param direction8 Direction of the jump (0 to 7).
 * \param distance Distance of the jump in pixels.
 * \param ignore_obstacles \c true to make the movement ignore obstacles.
 * \param with_sound \c true to play the "jump" sound.
 * \param movement_delay Delay between each one-pixel move in the jump
 * movement (0: default).
 */
Hero::JumpingState::JumpingState(
    Hero& hero,
    int direction8,
    int distance,
    bool ignore_obstacles,
    bool with_sound,
    uint32_t movement_delay):
  State(hero, "jumping"),
  carried_item(NULL) {

  if (get_previous_carried_item_behavior() == CarriedItem::BEHAVIOR_KEEP) {
    // Keep the carried item of the previous state.
    carried_item = hero.get_carried_item();
    if (carried_item != NULL) {
      RefCountable::ref(carried_item);
    }
  }

  this->movement = new JumpMovement(direction8, distance, 0, ignore_obstacles);
  this->direction8 = direction8;
  this->with_sound = with_sound;
}

/**
 * \brief Destructor.
 */
Hero::JumpingState::~JumpingState() {
  destroy_carried_item();
}

/**
 * \brief Starts this state.
 * \param previous_state the previous state
 */
void Hero::JumpingState::start(const State* previous_state) {

  State::start(previous_state);

  // update the sprites
  HeroSprites& sprites = get_sprites();
  sprites.set_animation_direction8(direction8);

  if (carried_item == NULL) {
    sprites.set_animation_jumping();
  }
  else {
    sprites.set_animation_walking_carrying();
    sprites.set_lifted_item(carried_item);
  }

  // jump
  get_hero().set_movement(movement);

  if (with_sound) {
    Sound::play("jump");
  }
}

/**
 * \brief Stops this state.
 * \param next_state the next state
 */
void Hero::JumpingState::stop(const State* next_state) {

  State::stop(next_state);

  get_hero().clear_movement();

  if (carried_item != NULL) {

    switch (next_state->get_previous_carried_item_behavior()) {

    case CarriedItem::BEHAVIOR_THROW:
      carried_item->throw_item(get_sprites().get_animation_direction());
      get_entities().add_entity(carried_item);
      carried_item = NULL;
      get_sprites().set_lifted_item(NULL);
      break;

    case CarriedItem::BEHAVIOR_DESTROY:
      destroy_carried_item();
      get_sprites().set_lifted_item(NULL);
      break;

    case CarriedItem::BEHAVIOR_KEEP:
      // The next state is now the owner and has incremented the refcount.
      Debug::check_assertion(carried_item->get_refcount() > 1,
          "Invalid carried item refcount");
      RefCountable::unref(carried_item);
      carried_item = NULL;
      break;

    default:
      Debug::die("Invalid carried item behavior");
    }
  }
}

/**
 * \brief Changes the map.
 * \param map the new map
 */
void Hero::JumpingState::set_map(Map& map) {

  State::set_map(map);

  // the hero may go to another map while jumping and carrying an item
  if (carried_item != NULL) {
    carried_item->set_map(map);
  }
}

/**
 * \brief Updates this state.
 */
void Hero::JumpingState::update() {

  State::update();

  if (carried_item != NULL) {
    carried_item->update();
  }

  if (movement->is_finished()) {
    get_hero().start_state_from_ground();
  }
}

/**
 * \brief Notifies this state that the game was just suspended or resumed.
 * \param su1spended true if the game is suspended
 */
void Hero::JumpingState::set_suspended(bool suspended) {

  State::set_suspended(suspended);

  if (carried_item != NULL) {
    carried_item->set_suspended(suspended);
  }
}

/**
 * \brief Notifies this state that the layer has changed.
 */
void Hero::JumpingState::notify_layer_changed() {

  if (carried_item != NULL) {
    carried_item->set_layer(get_hero().get_layer());
  }
}

/**
 * \brief Returns the direction of the hero's movement as defined by the controls applied by the player
 * and the movements allowed is the current state.
 *
 * If he is not moving, -1 is returned.
 * This direction may be different from the real movement direction because of obstacles.
 *
 * \return the hero's wanted direction between 0 and 7, or -1 if he is stopped
 */
int Hero::JumpingState::get_wanted_movement_direction8() const {
  return get_sprites().get_animation_direction8();
}

/**
 * \brief Returns whether the game over sequence can start in the current state.
 * \return true if the game over sequence can start in the current state
 */
bool Hero::JumpingState::can_start_gameover_sequence() const {
  return false;
}

/**
 * \brief Returns whether the hero is touching the ground in the current state.
 * \return true if the hero is touching the ground in the current state
 */
bool Hero::JumpingState::is_touching_ground() const {
  return false;
}

/**
 * \brief Returns whether the hero ignores the effect of deep water in this state.
 * \return true if the hero ignores the effect of deep water in the current state
 */
bool Hero::JumpingState::can_avoid_deep_water() const {
  return true;
}

/**
 * \brief Returns whether the hero ignores the effect of holes in this state.
 * \return true if the hero ignores the effect of holes in the current state
 */
bool Hero::JumpingState::can_avoid_hole() const {
  return true;
}

/**
 * \brief Returns whether the hero ignores the effect of ice in this state.
 * \return \c true if the hero ignores the effect of ice in the current state.
 */
bool Hero::JumpingState::can_avoid_ice() const {
  return true;
}

/**
 * \brief Returns whether the hero ignores the effect of lava in this state.
 * \return true if the hero ignores the effect of lava in the current state
 */
bool Hero::JumpingState::can_avoid_lava() const {
  return true;
}

/**
 * \brief Returns whether the hero ignores the effect of prickles in this state.
 * \return true if the hero ignores the effect of prickles in the current state
 */
bool Hero::JumpingState::can_avoid_prickle() const {
  return true;
}

/**
 * \brief Returns whether the hero ignores the effect of teletransporters in this state.
 * \return true if the hero ignores the effect of teletransporters in this state
 */
bool Hero::JumpingState::can_avoid_teletransporter() const {
  return true;
}

/**
 * \copydoc Hero::State::can_avoid_stream
 */
bool Hero::JumpingState::can_avoid_stream(const Stream& stream) const {
  return true;
}

/**
 * \brief Returns whether some stairs are considered as obstacle in this state.
 * \param stairs some stairs
 * \return true if the stairs are obstacle in this state
 */
bool Hero::JumpingState::is_stairs_obstacle(const Stairs& stairs) const {
  // allow to jump over stairs covered by water
  return get_hero().get_ground_below() != GROUND_DEEP_WATER;
}

/**
 * \brief Returns whether a sensor is considered as an obstacle in this state.
 * \param sensor a sensor
 * \return true if the sensor is an obstacle in this state
 */
bool Hero::JumpingState::is_sensor_obstacle(const Sensor& sensor) const {

  return false;
}

/**
 * \copydoc Hero::State::is_separator_obstacle
 */
bool Hero::JumpingState::is_separator_obstacle(
    const Separator& separator) const {
  return true;
}

/**
 * \brief Returns whether the hero ignores the effect of sensors in this state.
 * \return true if the hero ignores the effect of sensors in this state
 */
bool Hero::JumpingState::can_avoid_sensor() const {
  return false;
}

/**
 * \brief Returns whether the hero ignores the effect of switches in this state.
 * \return true if the hero ignores the effect of switches in this state
 */
bool Hero::JumpingState::can_avoid_switch() const {
  return true;
}

/**
 * \brief Returns whether the hero can be hurt in this state.
 * \return true if the hero can be hurt in this state
 * \param attacker an attacker that is trying to hurt the hero
 * (or NULL if the source of the attack is not an enemy)
 */
bool Hero::JumpingState::can_be_hurt(MapEntity* attacker) const {
  return false;
}

/**
 * \brief Returns the item currently carried by the hero in this state, if any.
 * \return the item carried by the hero, or NULL
 */
CarriedItem* Hero::JumpingState::get_carried_item() const {
  return carried_item;
}

/**
 * \brief Destroys the item carried if any and sets it to NULL.
 */
void Hero::JumpingState::destroy_carried_item() {

  RefCountable::unref(carried_item);
  carried_item = NULL;
}

/**
 * \brief Returns the action to do with an item previously carried by the hero when this state starts.
 * \return the action to do with a previous carried item when this state starts
 */
CarriedItem::Behavior Hero::JumpingState::get_previous_carried_item_behavior() const {

  return CarriedItem::BEHAVIOR_KEEP;
}

}

