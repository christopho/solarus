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
#include "Drawable.h"
#include "Transition.h"
#include "movements/Movement.h"
#include "lua/LuaContext.h"
#include "lowlevel/Debug.h"
#include <lua.hpp>

namespace solarus {

/**
 * \brief Constructor.
 */
Drawable::Drawable():
  xy(),
  movement(nullptr),
  transition(nullptr),
  transition_callback_ref(LUA_REFNIL),
  lua_context(nullptr),
  suspended(false) {

}

/**
 * \brief Destructor.
 */
Drawable::~Drawable() {

  stop_transition();
  stop_movement();
}

/**
 * \brief Applies a movement to this object.
 *
 * Any previous movement is stopped.
 *
 * \param movement The movement to apply.
 */
void Drawable::start_movement(Movement& movement) {

  stop_movement();
  this->movement = &movement;
  movement.set_drawable(this);
  RefCountable::ref(&movement);

  movement.set_suspended(is_suspended());
}

/**
 * \brief Stops the movement applied to the object, if any.
 *
 * The movement is deleted unless the owner script uses it elsewhere.
 */
void Drawable::stop_movement() {

  RefCountable::unref(movement);
  movement = nullptr;
}

/**
 * \brief Returns the current movement of this drawable object.
 * \return The object's movement, or nullptr if there is no movement.
 */
Movement* Drawable::get_movement() {
  return movement;
}

/**
 * \brief Returns the coordinates of this drawable object as defined by its
 * movement.
 * \return The coordinates of this drawable object.
 */
const Rectangle& Drawable::get_xy() const {
  return xy;
}

/**
 * \brief Sets the coordinates of this drawable object.
 * \param xy The new coordinates of this drawable object.
 */
void Drawable::set_xy(const Rectangle& xy) {
  this->xy.set_xy(xy);
}

/**
 * \brief Starts a transition effect on this object.
 *
 * The transition will be automatically deleted when finished or stopped.
 * Any previous transition is stopped.
 *
 * \param transition The transition to start.
 * \param callback_ref A Lua registry ref to the function to call when
 * the transition finishes, or LUA_REFNIL.
 * \param lua_context The Lua world for the callback (or nullptr).
 */
void Drawable::start_transition(
    Transition& transition,
    int callback_ref,
    LuaContext* lua_context) {

  stop_transition();

  this->transition = &transition;
  this->transition_callback_ref = callback_ref;
  this->lua_context = lua_context;
  transition.start();
  transition.set_suspended(is_suspended());
}

/**
 * \brief Stops the transition effect applied to this object, if any.
 *
 * The transition is deleted and the Lua callback (if any) is canceled.
 */
void Drawable::stop_transition() {

  delete transition;
  transition = nullptr;

  if (lua_context != nullptr) {
    lua_context->cancel_callback(this->transition_callback_ref);
    transition_callback_ref = LUA_REFNIL;
  }
}

/**
 * \brief Returns the current transition of this drawable object.
 * \return The object's transition, or nullptr if there is no transition.
 */
Transition* Drawable::get_transition() {
  return transition;
}

/**
 * \brief Updates this object.
 *
 * This function is called repeatedly.
 * You can redefine it for your needs.
 */
void Drawable::update() {

  if (transition != nullptr) {
    transition->update();
    if (transition->is_finished()) {

      delete transition;
      transition = nullptr;

      int ref = transition_callback_ref;
      transition_callback_ref = LUA_REFNIL;

      if (lua_context != nullptr) {
        // Note that this callback may create a new transition right now.
        lua_context->do_callback(ref);
        lua_context->cancel_callback(ref);
      }
    }
  }

  if (movement != nullptr) {
    movement->update();
    if (movement != nullptr && movement->is_finished()) {
      stop_movement();
    }
  }
}

/**
 * \brief Returns whether this drawable is suspended.
 * \return \c true if this drawable is suspended.
 */
bool Drawable::is_suspended() const {
  return suspended;
}

/**
 * \brief Suspends or resumes this drawable.
 * \param suspended \c true to suspend this drawable, \c false to resume it.
 */
void Drawable::set_suspended(bool suspended) {

  if (suspended == this->suspended) {
    return;
  }

  this->suspended = suspended;

  // Suspend or resume the transition effect and the movement if any.
  if (transition != nullptr) {
    transition->set_suspended(suspended);
  }

  if (movement != nullptr) {
    movement->set_suspended(suspended);
  }
}

/**
 * \brief Draws this object, applying dynamic effects.
 * \param dst_surface the destination surface
 */
void Drawable::draw(Surface& dst_surface) {

  draw(dst_surface, Rectangle(0, 0));
}

/**
 * \brief Draws this object, applying dynamic effects.
 * \param dst_surface the destination surface
 * \param x x coordinate of where to draw
 * \param y y coordinate of where to draw
 */
void Drawable::draw(Surface& dst_surface, int x, int y) {

  draw(dst_surface, Rectangle(x, y));
}

/**
 * \brief Draws this object, applying dynamic effects.
 * \param dst_surface the destination surface
 * \param dst_position position on this surface
 * (will be added to the position obtained by previous movements)
 */
void Drawable::draw(Surface& dst_surface,
    const Rectangle& dst_position) {

  Rectangle dst_position2(dst_position);
  dst_position2.add_xy(xy);

  if (transition != nullptr) {
    draw_transition(*transition);
  }

  raw_draw(dst_surface, dst_position2);
}

/**
 * \brief Draws a subrectangle of this object, applying dynamic effects.
 * \param region The rectangle to draw in this object.
 * \param dst_surface The destination surface.
 */
void Drawable::draw_region(
    const Rectangle& region,
    Surface& dst_surface) {

  draw_region(region, dst_surface, Rectangle(0, 0));
}

/**
 * \brief Draws a subrectangle of this object, applying dynamic effects.
 * \param region The rectangle to draw in this object.
 * \param dst_surface The destination surface
 * \param dst_position Position on this surface
 * (will be added to the position obtained by previous movements).
 * The width and height of this rectangle are ignored.
 */
void Drawable::draw_region(
    const Rectangle& region,
    Surface& dst_surface,
    const Rectangle& dst_position) {

  Rectangle dst_position2(dst_position);
  dst_position2.add_xy(xy);

  if (transition != nullptr) {
    draw_transition(*transition);
  }

  raw_draw_region(region, dst_surface, dst_position2);
}

}

