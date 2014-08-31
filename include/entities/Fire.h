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
#ifndef SOLARUS_FIRE_H
#define SOLARUS_FIRE_H

#include "Common.h"
#include "entities/Detector.h"

namespace solarus {

/**
 * \brief Represents some fire on the map.
 *
 * Fire can interact with entities.
 */
class Fire: public Detector {

  public:

    Fire(const std::string& name, Layer layer, const Rectangle& xy);
    ~Fire();

    EntityType get_type() const;
    bool can_be_obstacle() const;

    // state
    void update();

    // collisions
    void notify_collision(MapEntity& other_entity, Sprite& other_sprite, Sprite& this_sprite);

};

}

#endif

