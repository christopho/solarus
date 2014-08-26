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
#include "Transition.h"
#include "TransitionImmediate.h"
#include "TransitionFade.h"
#include "TransitionScrolling.h"
#include "lowlevel/Debug.h"
#include "lowlevel/System.h"

namespace solarus {

const std::string Transition::style_names[] = {
  "immediate",
  "fade",
  "scrolling",
  ""  // Sentinel.
};

/**
 * \brief Creates a transition effect.
 * \param direction direction of the transition effect (in or out)
 */
Transition::Transition(Transition::Direction direction):
  game(nullptr),
  direction(direction),
  previous_surface(nullptr),
  suspended(false),
  when_suspended(0) {

}

/**
 * \brief Destructor.
 */
Transition::~Transition() {
}

/**
 * \brief Creates a transition effect with the specified type and direction.
 * \param style style of the transition: Transition::IMMEDIATE, Transition::FADE, etc.
 * \param direction Direction of the transition.
 * \param dst_surface The surface that will receive the transition
 * (used by some kinds of transitions).
 * \param game The current game if any (used by some kinds of transitions).
 * \return the transition created
 */
Transition* Transition::create(
    Transition::Style style,
    Transition::Direction direction,
    Surface& dst_surface,
    Game* game) {

  Transition* transition = nullptr;

  switch (style) {

  case Transition::IMMEDIATE:
    transition = new TransitionImmediate(direction);
    break;

  case Transition::FADE:
    transition = new TransitionFade(direction, dst_surface);
    break;

  case Transition::SCROLLING:
    transition = new TransitionScrolling(direction);
    break;
  }

  transition->game = game;

  return transition;
}

/**
 * \brief Returns the current game.
 *
 * Some transition effects need a game to run.
 *
 * \return The current game or nullptr.
 */
Game* Transition::get_game() const {
  return game;
}

/**
 * \brief Returns the direction of this transition effect.
 * \returns the direction of this transition effect.
 */
Transition::Direction Transition::get_direction() const {
  return direction;
}

/**
 * \brief Returns the surface to show during the closing transition
 * that was played before this opening transition.
 * \return The previous surface or nullptr.
 */
Surface* Transition::get_previous_surface() const {
  return previous_surface;
}

/**
 * \brief Indicates the surface that was shown during the closing transition
 * that was played before this opening transition.
 * \param previous_surface The previous surface or nullptr.
 */
void Transition::set_previous_surface(Surface* previous_surface) {

  Debug::check_assertion(previous_surface == nullptr
      || get_direction() != TRANSITION_CLOSING,
      "Cannot show a previous surface with an closing transition effect");

  this->previous_surface = previous_surface;
}

/**
 * \brief Returns whether this transition effect needs the previous surface.
 * \return false
 */
bool Transition::needs_previous_surface() const {
  return false;
}

/**
 * \brief Returns whether this transition is currently suspended.
 * \return true if this transition is suspended.
 */
bool Transition::is_suspended() const {
  return suspended;
}

/**
 * \brief Suspends or resumes this transition.
 * \param suspended true to suspend it, false to resume it.
 */
void Transition::set_suspended(bool suspended) {

  if (suspended != this->suspended) {
    this->suspended = suspended;
    if (suspended) {
      when_suspended = System::now();
    }
    notify_suspended(suspended);
  }
}

/**
 * \brief Returns the date when this transition was suspended if it is.
 * \return The date when this transition was suspended or 0.
 */
uint32_t Transition::get_when_suspended() const {
  return when_suspended;
}

}

