/*
 * Copyright (C) 2006-2013 Christopho, Solarus - http://www.solarus-games.org
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
#ifndef SOLARUS_NOMAIN

#include "MainLoop.h"
#include <iostream>
#include <SDL.h>  // Necessary on some systems for SDLMain.

static void print_help(int argc, char** argv);

/**
 * \brief Usual entry point of the program.
 *
 * Usage: solarus [options] [quest_path]
 *
 * The quest path is the name of a directory that contains either the data
 * directory ("data") or the data archive ("data.solarus").
 * If the quest path is not specified, it is set to the preprocessor constant
 * DEFAULT_QUEST, which is the current directory "." by default.
 * In all cases, this quest path is relative to the working directory,
 * or to the solarus executable directory if no quest is found in the working
 * directory.
 *
 * The following options are supported:
 *   -help               shows a help message
 *   -no-audio           disables sounds and musics
 *   -no-video           disables displaying (used for unitary tests)
 *   -quest-size=<width>x<height>         sets the size of the drawing area (if compatible with the quest)
 *
 * \param argc number of command-line arguments
 * \param argv command-line arguments
 */
int main(int argc, char** argv) {

  std::cout << "Solarus " << SOLARUS_VERSION << std::endl;

  // check the -help option
  bool help = false;
  for (int i = 1; i < argc && !help; ++i) {
    const std::string arg = argv[i];
    help = (arg == std::string("-help"));
  }

  if (help) {
    // print a help message
    print_help(argc, argv);
  }
  else {
    // run the window
    MainLoop(argc, argv).run();
  }

  return 0;
}

/**
 * \brief Prints the usage of the program.
 * \param argc number of command-line arguments
 * \param argv command-line arguments
 */
static void print_help(int argc, char **argv) {

  const std::string& binary_name = (argc > 0) ? argv[0] : "solarus";
  std::cout << "Usage: " << binary_name << " [options] [quest_path]"
    << std::endl << std::endl
    << "The quest path is the name of a directory that contains either the data"
    << std::endl
#ifdef GCWZERO
    << "directory or the data archive (data.solarus.zip) of the game to run."
#else
    << "directory or the data archive (data.solarus) of the game to run."
#endif
    << std::endl
    << "If the quest path is not specified, the default directory will be: '"
    << SOLARUS_DEFAULT_QUEST << "'."
    << std::endl
    << std::endl
    << "Options:"
    << std::endl
    << "  -help               shows this help message and exits"
    << std::endl
    << "  -no-audio           disables sounds and musics"
    << std::endl
    << "  -no-video           disables displaying (may be useful for automated tests)"
    << std::endl
    << "  -quest-size=<width>x<height>         sets the size of the drawing area (if compatible with the quest)"
    << std::endl;
}

#endif

