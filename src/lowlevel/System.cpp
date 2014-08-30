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
#include "lowlevel/System.h"
#include "lowlevel/FileTools.h"
#include "lowlevel/Video.h"
#include "lowlevel/Color.h"
#include "lowlevel/TextSurface.h"
#include "lowlevel/Sound.h"
#include "lowlevel/Random.h"
#include "lowlevel/InputEvent.h"
#include "Sprite.h"
#include <SDL.h>
#ifdef SOLARUS_USE_APPLE_POOL
#  include "lowlevel/apple/AppleInterface.h"
#endif

namespace solarus {

uint32_t System::ticks = 0;

/**
 * \brief Initializes the basic lowlevel system.
 *
 * Initializes the audio system, the video system,
 * the data file system, etc.
 *
 * \param args Command-line arguments.
 */
void System::initialize(const CommandLine& args) {

  // initialize SDL
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);

  // files
  FileTools::initialize(args);

  // audio
  Sound::initialize(args);

  // input
  InputEvent::initialize();

  // random number generator
  Random::initialize();

  // video
  Video::initialize(args);
  Color::initialize();
  TextSurface::initialize();
  Sprite::initialize();
}

/**
 * \brief Closes the lowlevel system.
 *
 * This closes all initializations made in initialize().
 */
void System::quit() {

  Random::quit();
  InputEvent::quit();
  Sound::quit();
  Sprite::quit();
  TextSurface::quit();
  Color::quit();
  Video::quit();
  FileTools::quit();

  SDL_Quit();
}

/**
 * \brief This function is called repeatedly by the main loop.
 *
 * It calls the update function of low-level systems that need it.
 */
void System::update() {

  // Use a constant timestep here to have deterministic updates.
  ticks += timestep;
  Sound::update();
}

/**
 * \brief Returns the name of the platform.
 *
 * Possible values are "Windows", "Mac OS X", "Linux", "iOS" and "Android".
 *
 * If the correct platform name is not available, returns a string
 * beginning with the text "Unknown".
 * \return the name of the platform.
 */
std::string System::get_platform() {

  return SDL_GetPlatform();
}

/**
 * \brief Returns the number of simulated milliseconds elapsed since the
 * beginning of the program.
 *
 * Corresponds to the real time unless the system is too slow to play at
 * normal speed.
 *
 * \return The number of simulated milliseconds elapsed since the beginning
 * of the program.
 */
uint32_t System::now() {
  return ticks;
}

/**
 * \brief Returns the number of real milliseconds elapsed since the beginning of
 * the program.
 *
 * This function is not deterministic, so use it at your own risks.
 *
 * \return The number of milliseconds elapsed since the beginning of the
 * program.
 */
uint32_t System::get_real_time() {
  return SDL_GetTicks();
}

/**
 * \brief Makes the program sleep during some time.
 *
 * Due to the OS scheduling, the real delay may be longer.
 *
 * \param duration duration of the sleep in milliseconds
 */
void System::sleep(uint32_t duration) {
  SDL_Delay(duration);
}

}

