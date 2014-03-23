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
#ifndef SOLARUS_HERO_INVENTORY_ITEM_STATE_H
#define SOLARUS_HERO_INVENTORY_ITEM_STATE_H

#include "hero/State.h"
#include "EquipmentItemUsage.h"

namespace solarus {

/**
 * \brief The state "using equipment item" of the hero.
 */
class Hero::UsingItemState: public Hero::State {

  public:

    UsingItemState(Hero& hero, EquipmentItem& item);
    ~UsingItemState();

    void start(const State* previous_state);
    void update();

    virtual bool is_using_item() const;
    virtual EquipmentItemUsage& get_item_being_used();
    virtual bool can_avoid_stream(const Stream& stream) const;

  private:

    EquipmentItemUsage item_usage;     /**< Info about using this equipment item. */

};

}

#endif

