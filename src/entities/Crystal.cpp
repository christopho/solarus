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
#include "entities/Crystal.h"
#include "entities/Hero.h"
#include "Game.h"
#include "Map.h"
#include "Sprite.h"
#include "SpriteAnimationSet.h"
#include "KeysEffect.h"
#include "lowlevel/FileTools.h"
#include "lowlevel/System.h"
#include "lowlevel/Random.h"
#include "lowlevel/Sound.h"
#include <lua.hpp>

namespace solarus {

/**
 * \brief Creates a new crystal.
 * \param name name of the entity to create
 * \param layer layer of the entity to create on the map
 * \param x x coordinate of the entity to create
 * \param y y coordinate of the entity to create
 */
Crystal::Crystal(const std::string& name, Layer layer, int x, int y):
  Detector(COLLISION_SPRITE | COLLISION_OVERLAPPING | COLLISION_FACING,
      name, layer, x, y, 16, 16),
  state(false),
  next_possible_hit_date(System::now()) {

  set_origin(8, 13);
  set_optimization_distance(2000);  // Because of bombs and arrows on the crystal.
  create_sprite("entities/crystal", true);
  star_sprite = new Sprite("entities/star");
  twinkle();
}

/**
 * \brief Destructor.
 */
Crystal::~Crystal() {
  delete star_sprite;
}

/**
 * \brief Returns the type of entity.
 * \return the type of entity
 */
EntityType Crystal::get_type() const {
  return ENTITY_CRYSTAL;
}

/**
 * \copydoc MapEntity::notify_creating
 */
void Crystal::notify_creating() {

  Detector::notify_creating();

  bool state = get_game().get_crystal_state();
  if (state != this->state) {

    this->state = state;
    get_sprite().set_current_animation(state ? "blue_lowered" : "orange_lowered");
  }
}

/**
 * \brief Returns whether this entity is an obstacle for another one.
 * \param other another entity
 * \return true if this entity is an obstacle for the other one
 */
bool Crystal::is_obstacle_for(MapEntity& other) {
  return other.is_crystal_obstacle(*this);
}

/**
 * \brief This function is called when another entity collides with this crystal.
 * \param entity_overlapping the other entity
 * \param collision_mode the collision mode that detected the collision
 */
void Crystal::notify_collision(MapEntity& entity_overlapping, CollisionMode collision_mode) {
  entity_overlapping.notify_collision_with_crystal(*this, collision_mode);
}

/**
 * \brief Notifies this entity that another sprite is overlapping it.
 *
 * This function is called by check_collision(MapEntity*, Sprite*) when another entity's
 * sprite overlaps a sprite of this detector.
 *
 * \param other_entity the entity overlapping this detector
 * \param other_sprite the sprite of other_entity that is overlapping this detector
 * \param this_sprite the sprite of this detector that is overlapping the other entity's sprite
 */
void Crystal::notify_collision(MapEntity& other_entity, Sprite& other_sprite, Sprite& /* this_sprite */) {
  other_entity.notify_collision_with_crystal(*this, other_sprite);
}

/**
 * \copydoc Detector::notify_action_command_pressed
 */
bool Crystal::notify_action_command_pressed() {

  if (get_hero().is_free() &&
      get_keys_effect().get_action_key_effect() == KeysEffect::ACTION_KEY_LOOK
  ) {
    get_keys_effect().set_action_key_effect(KeysEffect::ACTION_KEY_NONE);

    // start a dialog
    get_game().start_dialog("_crystal", LUA_REFNIL, LUA_REFNIL);
    return true;
  }

  return false;
}

/**
 * \brief Activates the crystal if the delay since the last activation allows it.
 * \param entity_activating the entity that activates this crystal
 */
void Crystal::activate(MapEntity& entity_activating) {

  bool recently_activated = false;
  for (MapEntity* entity: entities_activating) {
    recently_activated  = (entity == &entity_activating);
  }

  uint32_t now = System::now();
  if (!recently_activated || now >= next_possible_hit_date) {
    Sound::play("switch");
    get_game().change_crystal_state();
    next_possible_hit_date = now + 1000;
    entities_activating.push_back(&entity_activating);
  }
}

/**
 * \brief Makes a star twinkle on the crystal at a random position.
 */
void Crystal::twinkle() {

  star_xy.set_xy(Random::get_number(3, 13), Random::get_number(3, 13));
  star_sprite->restart_animation();
}

/**
 * \brief Updates the entity.
 */
void Crystal::update() {

  if (!is_suspended()) {
    bool state = get_game().get_crystal_state();
    if (state != this->state) {

      this->state = state;
      get_sprite().set_current_animation(state ? "blue_lowered" : "orange_lowered");
    }

    star_sprite->update();
    if (star_sprite->is_animation_finished()) {
      twinkle();
    }

    uint32_t now = System::now();
    if (now >= next_possible_hit_date) {
      entities_activating.clear();
    }
  }

  MapEntity::update();
}

/**
 * \brief Draws the entity on the map.
 *
 * This is a redefinition of MapEntity::draw_on_map() to also draw the twinkling star
 * which has a special position.
 */
void Crystal::draw_on_map() {

  if (!is_drawn()) {
    return;
  }

  // draw the crystal
  MapEntity::draw_on_map();

  // draw the star
  if (is_drawn()) {
    get_map().draw_sprite(*star_sprite, get_top_left_x() + star_xy.get_x(), get_top_left_y() + star_xy.get_y());
  }
}

/**
 * \brief Suspends or resumes the entity.
 * \param suspended true to suspend the entity, false to resume it
 */
void Crystal::set_suspended(bool suspended) {

  MapEntity::set_suspended(suspended);

  if (!suspended) {
    next_possible_hit_date += System::now() - get_when_suspended();
  }
}

}

