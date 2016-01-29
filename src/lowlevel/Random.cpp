/*
 * Copyright (C) 2006-2016 Christopho, Solarus - http://www.solarus-games.org
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
#include "solarus/lowlevel/Random.h"
#include <ctime>
#include <random>

#if defined(SOLARUS_OSX) || defined(SOLARUS_IOS)
#  define thread_local
#endif

namespace Solarus {
namespace Random {

/**
 * \brief Initializes the random number generator.
 */
void initialize() {
  // nothing to do
}

/**
 * \brief Uninitializes the random number generator.
 */
void quit() {
  // nothing to do
}

/**
 * \brief Returns a random integer number in [0, x[ with a uniform distribution.
 *
 * This is equivalent to: Random::get_number(0, x).
 *
 * \param x the superior bound
 * \return a random integer number in [0, x[
 */
int get_number(unsigned int x) {
  return get_number(0, x);
}

/**
 * \brief Returns a random integer number in [x, y[ with a uniform distribution.
 * \param x the inferior bound
 * \param y the superior bound
 * \return a random integer number in [x, y[
 */
int get_number(int x, int y) {

  // Initialize the engine and the distribution
  //
  // The engine is not initialized with std::random_device
  // because not every main platform support non-deterministic
  // random numbers generation yet.
  //
  // The variables are thread_local so that every thread has
  // its own to avoid thread-safety problems while still being
  // more efficient than a mutex-based solution
  //
  // These variables need to be known in the function but not
  // needed outside of it, so it is easier to have them as
  // thread_local function-local variables (one instance per
  // thread, initialized once, like a static variable) rather
  // than maintaining them in the body of a class.
  //
  thread_local std::mt19937 engine(std::time(nullptr));
  thread_local std::uniform_int_distribution<int> dist{};

  // Type of the parameters of the distribution
  using param_type = std::uniform_int_distribution<int>::param_type;

  // Get a random number in [x, y[
  return dist(engine, param_type{x, y-1});
}

}
}
