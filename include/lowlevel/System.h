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
#ifndef SOLARUS_SYSTEM_H
#define SOLARUS_SYSTEM_H

#include "Common.h"
#include <string>

namespace solarus {

/**
 * \brief Provides lowlevel functions and initialization.
 *
 * This class initializes all low-level features.
 */
class System {

  public:

    static void initialize(const CommandLine& args);
    static void quit();
    static void update();

    static std::string get_platform();

    static uint32_t now();
    static uint32_t get_real_time();
    static void sleep(uint32_t duration);

    static const uint32_t timestep = 10;  /**< Timestep added to the simulated time at each update. */

  private:

    static uint32_t ticks;                /**< Simulated time in milliseconds. */

};

}

#endif

