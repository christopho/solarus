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
#ifndef SOLARUS_VIDEO_MODE_H
#define SOLARUS_VIDEO_MODE_H

#include "Common.h"
#include "lowlevel/Rectangle.h"
#include <string>

namespace solarus {

/**
 * \brief Represents a method to display the quest content on the screen.
 *
 * The video mode may include a scaling algorithm.
 */
class VideoMode {

  public:

    VideoMode(
        const std::string& name,
        const Rectangle& initial_window_size,
        PixelFilter* software_filter,
        Shader* hardware_filter);
    ~VideoMode();

    const std::string& get_name() const;
    const Rectangle& get_initial_window_size() const;
    PixelFilter* get_software_filter() const;
    Shader* get_hardware_filter() const;

  private:

    VideoMode(const VideoMode& other);

    const std::string name;              /**< Lua name of this video mode. */
    const Rectangle initial_window_size; /**< Default size of the window when
                                          * selecting this video mode. */

    // Pixel filter (in CPU or GPU).
    PixelFilter* software_filter;        /**< Software scaling pixel filter to use or nullptr. */
    Shader* hardware_filter;             /**< Scaling shader to use or nullptr. */

};

}

#endif

