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
#include "hero/StreamState.h"
#include "hero/FreeState.h"
#include "hero/HeroSprites.h"
#include "entities/Stream.h"
#include "movements/TargetMovement.h"
#include "movements/PathMovement.h"
#include "Game.h"
#include "Sprite.h"

namespace solarus {

/**
 * \brief Constructor.
 * \param hero The hero controlled by this state.
 * \param stream The stream to take.
 */
Hero::StreamState::StreamState(
    Hero& hero, Stream& stream):
  State(hero, "stream"),
  stream(stream),
  snapping(false) {

}

/**
 * \brief Destructor.
 */
Hero::StreamState::~StreamState() {

}

/**
 * \brief Starts this state.
 * \param previous_state the previous state
 */
void Hero::StreamState::start(const State* previous_state) {

  State::start(previous_state);

  get_sprites().set_animation_stopped_normal();

  // first, snap the hero to the center of the stream.
  snapping = true;
  Hero& hero = get_hero();
  hero.set_movement(new TargetMovement(
      &stream, 0, 0, stream.get_speed(), true));
}

/**
 * \brief Stops this state.
 * \param next_state the next state
 */
void Hero::StreamState::stop(const State* next_state) {

  State::stop(next_state);

  get_hero().clear_movement();
}

/**
 * \brief Updates this state.
 */
void Hero::StreamState::update() {

  State::update();

  if (is_suspended()) {
    return;
  }

  Hero& hero = get_hero();
  if (snapping && hero.get_movement()->is_finished()) {

    // the hero is now exactly placed on the stream: start the stream's movement
    snapping = false;
    std::string path = "  ";
    path[0] = path[1] = '0' + stream.get_direction();
    hero.clear_movement();
    hero.set_movement(new PathMovement(path, stream.get_speed(), false, false, false));
  }
  else {

    // see if the stream's movement is finished
    if (hero.get_movement()->is_finished() || !hero.on_stream) {

      hero.set_state(new FreeState(hero));
    }
    else {
      // update the sprites direction
      int keys_direction8 = get_commands().get_wanted_direction8();
      int movement_direction8 = stream.get_direction();

      int animation_direction = get_sprites().get_animation_direction(keys_direction8, movement_direction8);
      if (animation_direction != get_sprites().get_animation_direction()
          && animation_direction != -1) {
        get_sprites().set_animation_direction(animation_direction);
      }
    }

    hero.on_stream = false;
  }
}

/**
 * \brief Returns whether the hero ignores the effect of teletransporters in this state.
 * \return true if the hero ignores the effect of teletransporters in this state
 */
bool Hero::StreamState::can_avoid_teletransporter() const {
  return true; // ignore the teletransporter until the stream is finished
}

/**
 * \copydoc Hero::State::can_avoid_stream
 */
bool Hero::StreamState::can_avoid_stream(const Stream& stream) const {
  return true;
}

/**
 * \copydoc Hero::State::can_start_sword
 */
bool Hero::StreamState::can_start_sword() const {
  return stream.get_allow_attack();
}

/**
 * \copydoc Hero::State::can_start_item
 */
bool Hero::StreamState::can_start_item(EquipmentItem& item) const {
  return stream.get_allow_item();
}

}

