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
#ifndef SOLARUS_STRING_RESOURCE_H
#define SOLARUS_STRING_RESOURCE_H

#include "Common.h"
#include <string>

namespace solarus {

/**
 * \brief Provides access to strings translated in the current language.
 *
 * This class provides some strings loaded from an external file containing
 * the texts in the current language.
 * This class is used only to load simple strings such as the ones displayed
 * in the menus.
 * The messages displayed in the dialog box during the game come from another
 * data file (see class DialogResource).
 */
class StringResource {

  public:

    static void initialize();
    static void quit();

    static bool exists(const std::string& key);
    static const std::string& get_string(const std::string& key);

  private:

    // we don't need to instantiate this class
    StringResource();
    ~StringResource();

};

}

#endif

