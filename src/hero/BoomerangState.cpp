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
#include "hero/BoomerangState.h"
#include "hero/FreeState.h"
#include "hero/HeroSprites.h"
#include "entities/MapEntities.h"
#include "entities/Boomerang.h"
#include "lowlevel/Geometry.h"
#include "Game.h"
#include "GameCommands.h"
#include "Map.h"

namespace solarus {

/**
 * \brief Constructor.
 * \param hero the hero controlled by this state
 * \param max_distance maximum distance of the movement in pixels
 * \param speed speed of the movement in pixels per second
 * \param tunic_preparing_animation animation name of the hero's tunic sprite
 * when preparing the boomerang
 * \param sprite_name animation set id that represents the boomerang
 */
Hero::BoomerangState::BoomerangState(
    Hero& hero,
    int max_distance,
    int speed,
    const std::string& tunic_preparing_animation,
    const std::string& sprite_name):
  State(hero, "boomerang"),
  direction_pressed8(-1),
  max_distance(max_distance),
  speed(speed),
  tunic_preparing_animation(tunic_preparing_animation),
  sprite_name(sprite_name) {

}

/**
 * \brief Destructor.
 */
Hero::BoomerangState::~BoomerangState() {
}

/**
 * \brief Starts this state.
 * \param previous_state the previous state
 */
void Hero::BoomerangState::start(const State* previous_state) {

  State::start(previous_state);

  if (get_map().get_entities().is_boomerang_present()) {
    Hero& hero = get_hero();
    hero.set_state(new FreeState(hero));
  }
  else {
    get_sprites().set_animation_boomerang(tunic_preparing_animation);
    this->direction_pressed8 = get_commands().get_wanted_direction8();
  }
}

/**
 * \brief Updates this state.
 */
void Hero::BoomerangState::update() {

  State::update();

  Hero& hero = get_hero();
  if (hero.is_animation_finished()) {

    if (direction_pressed8 == -1) {
      // the player can press the diagonal arrows before or after the boomerang key
      direction_pressed8 = get_commands().get_wanted_direction8();
    }

    int boomerang_direction8;
    if (direction_pressed8 == -1 || direction_pressed8 % 2 == 0) {
      boomerang_direction8 = get_sprites().get_animation_direction() * 2;
    }
    else {
      boomerang_direction8 = direction_pressed8;
    }
    double angle = Geometry::degrees_to_radians(boomerang_direction8 * 45);
    get_entities().add_entity(new Boomerang(hero, max_distance, speed, angle, sprite_name));

    hero.set_state(new FreeState(hero));
  }
}

/**
 * \copydoc Hero::State::can_avoid_stream
 */
bool Hero::BoomerangState::can_avoid_stream(const Stream& stream) const {
  return true;
}

}

