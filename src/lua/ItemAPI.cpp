/*
 * Copyright (C) 2006-2012 Christopho, Solarus - http://www.solarus-games.org
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
#include "lua/LuaContext.h"
#include "entities/Pickable.h"
#include "entities/Hero.h"
#include "entities/MapEntities.h"
#include "movements/Movement.h"
#include "EquipmentItem.h"
#include "Equipment.h"
#include "Game.h"
#include "Map.h"
#include "InventoryItem.h"
#include "Sprite.h"
#include "lowlevel/Debug.h"
#include "lowlevel/StringConcat.h"
#include <lua.hpp>

const std::string LuaContext::item_module_name = "sol.item";

/**
 * @brief Initializes the item features provided to Lua.
 */
void LuaContext::register_item_module() {

  static const luaL_Reg methods[] = {
      { "get_name", item_api_get_name },
      { "get_game", item_api_get_game },
      { "get_map", item_api_get_map },
      { "get_savegame_variable", item_api_get_savegame_variable },
      { "set_savegame_variable", item_api_set_savegame_variable },
      { "get_amount_savegame_variable", item_api_get_amount_savegame_variable },
      { "set_amount_savegame_variable", item_api_set_amount_savegame_variable },
      { "is_obtainable", item_api_is_obtainable },
      { "set_obtainable", item_api_set_obtainable },
      { "is_assignable", item_api_is_assignable },
      { "set_assignable", item_api_set_assignable },
      { "get_can_disappear", item_api_get_can_disappear },
      { "set_can_disappear", item_api_set_can_disappear },
      { "get_brandish_when_picked", item_api_get_brandish_when_picked },
      { "set_brandish_when_picked", item_api_set_brandish_when_picked },
      { "get_shadow", item_api_get_shadow },
      { "set_shadow", item_api_set_shadow },
      { "get_sound_when_picked", item_api_get_sound_when_picked },
      { "set_sound_when_picked", item_api_set_sound_when_picked },
      { "get_sound_when_brandished", item_api_get_sound_when_brandished },
      { "set_sound_when_brandished", item_api_set_sound_when_brandished },
      { "has_variant", item_api_has_variant },
      { "get_variant", item_api_get_variant },
      { "set_variant", item_api_set_variant },
      { "has_amount", item_api_has_amount },
      { "get_amount", item_api_get_amount },
      { "set_amount", item_api_set_amount },
      { "add_amount", item_api_add_amount },
      { "remove_amount", item_api_remove_amount },
      { "get_max_amount", item_api_get_max_amount },
      { "set_max_amount", item_api_set_max_amount },
      { "set_finished", item_api_set_finished },
      { NULL, NULL }
  };
  static const luaL_Reg metamethods[] = {
      { "__gc", userdata_meta_gc },
      { "__newindex", userdata_meta_newindex_as_table },
      { "__index", userdata_meta_index_as_table },
      { NULL, NULL }
  };
  register_type(item_module_name, methods, metamethods);
}

/**
 * @brief Returns whether a value is a userdata of type item.
 * @param l A Lua context.
 * @param index An index in the stack.
 * @return true if the value at this index is an item.
 */
bool LuaContext::is_item(lua_State* l, int index) {
  return is_userdata(l, index, item_module_name);
}

/**
 * @brief Checks that the userdata at the specified index of the stack is an
 * equipment item and returns it.
 * @param l A Lua context.
 * @param index An index in the stack.
 * @return The equipment item.
 */
EquipmentItem& LuaContext::check_item(lua_State* l, int index) {
  return static_cast<EquipmentItem&>(check_userdata(l, index, item_module_name));
}

/**
 * @brief Pushes an equipment item userdata onto the stack.
 * @param l A Lua context.
 * @param item An item.
 */
void LuaContext::push_item(lua_State* l, EquipmentItem& item) {

  push_userdata(l, item);
}

/**
 * @brief Implementation of \ref lua_api_item_get_name.
 * @param l The Lua context that is calling this function.
 * @return Number of values to return to Lua.
 */
int LuaContext::item_api_get_name(lua_State* l) {

  EquipmentItem& item = check_item(l, 1);

  push_string(l, item.get_name());
  return 1;
}

/**
 * @brief Implementation of \ref lua_api_item_get_game.
 * @param l The Lua context that is calling this function.
 * @return Number of values to return to Lua.
 */
int LuaContext::item_api_get_game(lua_State* l) {

  EquipmentItem& item = check_item(l, 1);

  push_game(l, item.get_savegame());
  return 1;
}

/**
 * @brief Implementation of \ref lua_api_item_get_map.
 * @param l The Lua context that is calling this function.
 * @return Number of values to return to Lua.
 */
int LuaContext::item_api_get_map(lua_State* l) {

  EquipmentItem& item = check_item(l, 1);

  Game* game = item.get_game();
  if (game != NULL) {
    push_map(l, game->get_current_map());
  }
  else {
    lua_pushnil(l);
  }
  return 1;
}

/**
 * @brief Implementation of \ref lua_api_item_get_savegame_variable.
 * @param l The Lua context that is calling this function.
 * @return Number of values to return to Lua.
 */
int LuaContext::item_api_get_savegame_variable(lua_State* l) {

  EquipmentItem& item = check_item(l, 1);

  const std::string& savegame_variable = item.get_savegame_variable();
  if (savegame_variable.empty()) {
    lua_pushnil(l);
  }
  else {
    push_string(l, savegame_variable);
  }
  return 1;
}

/**
 * @brief Implementation of \ref lua_api_item_set_savegame_variable.
 * @param l The Lua context that is calling this function.
 * @return Number of values to return to Lua.
 */
int LuaContext::item_api_set_savegame_variable(lua_State* l) {

  EquipmentItem& item = check_item(l, 1);
  std::string savegame_variable;
  if (!lua_isnil(l, 2)) {
    savegame_variable = luaL_checkstring(l, 2);
  }

  item.set_savegame_variable(savegame_variable);

  return 0;
}

/**
 * @brief Implementation of \ref lua_api_item_get_amount_savegame_variable.
 * @param l The Lua context that is calling this function.
 * @return Number of values to return to Lua.
 */
int LuaContext::item_api_get_amount_savegame_variable(lua_State* l) {

  EquipmentItem& item = check_item(l, 1);

  const std::string& amount_savegame_variable = item.get_amount_savegame_variable();
  if (amount_savegame_variable.empty()) {
    lua_pushnil(l);
  }
  else {
    push_string(l, amount_savegame_variable);
  }
  return 1;
}

/**
 * @brief Implementation of \ref lua_api_item_set_amount_savegame_variable.
 * @param l The Lua context that is calling this function.
 * @return Number of values to return to Lua.
 */
int LuaContext::item_api_set_amount_savegame_variable(lua_State* l) {

  EquipmentItem& item = check_item(l, 1);
  std::string amount_savegame_variable;
  if (lua_gettop(l) >= 2) {
    amount_savegame_variable = luaL_checkstring(l, 2);
  }

  item.set_amount_savegame_variable(amount_savegame_variable);

  return 0;
}

/**
 * @brief Implementation of \ref lua_api_item_is_obtainable.
 * @param l The Lua context that is calling this function.
 * @return Number of values to return to Lua.
 */
int LuaContext::item_api_is_obtainable(lua_State* l) {

  EquipmentItem& item = check_item(l, 1);

  lua_pushboolean(l, item.is_obtainable());
  return 1;
}

/**
 * @brief Implementation of \ref lua_api_item_set_obtainable.
 * @param l The Lua context that is calling this function.
 * @return Number of values to return to Lua.
 */
int LuaContext::item_api_set_obtainable(lua_State* l) {

  EquipmentItem& item = check_item(l, 1);
  bool obtainable = true;
  if (lua_gettop(l) >= 2) {
    obtainable = lua_toboolean(l, 2);
  }

  item.set_obtainable(obtainable);

  return 0;
}

/**
 * @brief Implementation of \ref lua_api_item_is_assignable.
 * @param l The Lua context that is calling this function.
 * @return Number of values to return to Lua.
 */
int LuaContext::item_api_is_assignable(lua_State* l) {

  EquipmentItem& item = check_item(l, 1);

  lua_pushboolean(l, item.is_assignable());
  return 1;
}

/**
 * @brief Implementation of \ref lua_api_item_set_assignable.
 * @param l The Lua context that is calling this function.
 * @return Number of values to return to Lua.
 */
int LuaContext::item_api_set_assignable(lua_State* l) {

  EquipmentItem& item = check_item(l, 1);
  bool assignable = true;
  if (lua_gettop(l) >= 2) {
    assignable = lua_toboolean(l, 2);
  }

  item.set_assignable(assignable);

  return 0;
}

/**
 * @brief Implementation of \ref lua_api_item_get_can_disappear.
 * @param l The Lua context that is calling this function.
 * @return Number of values to return to Lua.
 */
int LuaContext::item_api_get_can_disappear(lua_State* l) {

  EquipmentItem& item = check_item(l, 1);

  lua_pushboolean(l, item.get_can_disappear());
  return 1;
}

/**
 * @brief Implementation of \ref lua_api_item_set_can_disappear.
 * @param l The Lua context that is calling this function.
 * @return Number of values to return to Lua.
 */
int LuaContext::item_api_set_can_disappear(lua_State* l) {

  EquipmentItem& item = check_item(l, 1);
  bool can_disappear = true;
  if (lua_gettop(l) >= 2) {
    can_disappear = lua_toboolean(l, 2);
  }

  item.set_can_disappear(can_disappear);

  return 0;
}

/**
 * @brief Implementation of \ref lua_api_item_get_brandish_when_picked.
 * @param l The Lua context that is calling this function.
 * @return Number of values to return to Lua.
 */
int LuaContext::item_api_get_brandish_when_picked(lua_State* l) {

  EquipmentItem& item = check_item(l, 1);

  lua_pushboolean(l, item.get_brandish_when_picked());
  return 1;
}

/**
 * @brief Implementation of \ref lua_api_item_set_brandish_when_picked.
 * @param l The Lua context that is calling this function.
 * @return Number of values to return to Lua.
 */
int LuaContext::item_api_set_brandish_when_picked(lua_State* l) {

  EquipmentItem& item = check_item(l, 1);
  bool brandish_when_picked = true;
  if (lua_gettop(l) >= 2) {
    brandish_when_picked = lua_toboolean(l, 2);
  }

  item.set_brandish_when_picked(brandish_when_picked);

  return 0;
}

/**
 * @brief Implementation of \ref lua_api_item_get_shadow.
 * @param l The Lua context that is calling this function.
 * @return Number of values to return to Lua.
 */
int LuaContext::item_api_get_shadow(lua_State* l) {

  EquipmentItem& item = check_item(l, 1);

  const std::string& shadow = item.get_shadow();
  if (shadow.empty()) {
    lua_pushnil(l);
  }
  else {
    push_string(l, shadow);
  }
  return 1;
}

/**
 * @brief Implementation of \ref lua_api_item_set_shadow.
 * @param l The Lua context that is calling this function.
 * @return Number of values to return to Lua.
 */
int LuaContext::item_api_set_shadow(lua_State* l) {

  EquipmentItem& item = check_item(l, 1);
  std::string shadow;
  if (!lua_isnil(l, 2)) {
    shadow = luaL_checkstring(l, 2);
  }

  item.set_shadow(shadow);

  return 0;
}

/**
 * @brief Implementation of \ref lua_api_item_get_sound_when_picked.
 * @param l The Lua context that is calling this function.
 * @return Number of values to return to Lua.
 */
int LuaContext::item_api_get_sound_when_picked(lua_State* l) {

  EquipmentItem& item = check_item(l, 1);

  const std::string& sound_when_picked = item.get_sound_when_picked();
  if (sound_when_picked.empty()) {
    lua_pushnil(l);
  }
  else {
    push_string(l, sound_when_picked);
  }
  return 1;
}

/**
 * @brief Implementation of \ref lua_api_item_set_sound_when_picked.
 * @param l The Lua context that is calling this function.
 * @return Number of values to return to Lua.
 */
int LuaContext::item_api_set_sound_when_picked(lua_State* l) {

  EquipmentItem& item = check_item(l, 1);
  std::string sound_when_picked;
  if (!lua_isnil(l, 2)) {
    sound_when_picked = luaL_checkstring(l, 2);
  }

  item.set_sound_when_picked(sound_when_picked);

  return 0;
}

/**
 * @brief Implementation of \ref lua_api_item_get_sound_when_brandished.
 * @param l The Lua context that is calling this function.
 * @return Number of values to return to Lua.
 */
int LuaContext::item_api_get_sound_when_brandished(lua_State* l) {

  EquipmentItem& item = check_item(l, 1);

  const std::string& sound_when_brandished = item.get_sound_when_brandished();
  if (sound_when_brandished.empty()) {
    lua_pushnil(l);
  }
  else {
    push_string(l, sound_when_brandished);
  }
  return 1;
}

/**
 * @brief Implementation of \ref lua_api_item_set_sound_when_brandished.
 * @param l The Lua context that is calling this function.
 * @return Number of values to return to Lua.
 */
int LuaContext::item_api_set_sound_when_brandished(lua_State* l) {

  EquipmentItem& item = check_item(l, 1);
  std::string sound_when_brandished;
  if (!lua_isnil(l, 2)) {
    sound_when_brandished = luaL_checkstring(l, 2);
  }

  item.set_sound_when_picked(sound_when_brandished);

  return 0;
}

/**
 * @brief Implementation of \ref lua_api_item_has_variant.
 * @param l The Lua context that is calling this function.
 * @return Number of values to return to Lua.
 */
int LuaContext::item_api_has_variant(lua_State* l) {

  EquipmentItem& item = check_item(l, 1);
  int variant = 1;
  if (lua_gettop(l) >= 2) {
    variant = int(luaL_checkinteger(l, 2));
  }

  lua_pushboolean(l, item.get_variant() >= variant);
  return 1;
}

/**
 * @brief Implementation of \ref lua_api_item_get_variant.
 * @param l The Lua context that is calling this function.
 * @return Number of values to return to Lua.
 */
int LuaContext::item_api_get_variant(lua_State* l) {

  EquipmentItem& item = check_item(l, 1);

  if (!item.is_saved()) {
    luaL_error(l, (StringConcat()
        << "Item '" << item.get_name() << "' is not saved").c_str());
  }

  lua_pushinteger(l, item.get_variant());
  return 1;
}

/**
 * @brief Implementation of \ref lua_api_item_set_variant.
 * @param l The Lua context that is calling this function.
 * @return Number of values to return to Lua.
 */
int LuaContext::item_api_set_variant(lua_State* l) {

  EquipmentItem& item = check_item(l, 1);
  int variant = int(luaL_checkinteger(l, 2));

  if (!item.is_saved()) {
    luaL_error(l, (StringConcat()
        << "Item '" << item.get_name() << "' is not saved").c_str());
  }

  item.set_variant(variant);

  return 0;
}

/**
 * @brief Implementation of \ref lua_api_item_has_amount.
 * @param l The Lua context that is calling this function.
 * @return Number of values to return to Lua.
 */
int LuaContext::item_api_has_amount(lua_State* l) {

  EquipmentItem& item = check_item(l, 1);
  if (lua_gettop(l) >= 2) {
    int amount = int(luaL_checkinteger(l, 2));
    if (!item.has_amount()) {
      luaL_error(l, (StringConcat() <<
          "Item '" << item.get_name() << "' has no amount").c_str());
    }
    lua_pushboolean(l, item.get_amount() >= amount);
  }
  else {
    lua_pushboolean(l, item.has_amount());
  }
  return 1;
}

/**
 * @brief Implementation of \ref lua_api_item_get_amount.
 * @param l The Lua context that is calling this function.
 * @return Number of values to return to Lua.
 */
int LuaContext::item_api_get_amount(lua_State* l) {

  EquipmentItem& item = check_item(l, 1);

  if (!item.has_amount()) {
    lua_pushnil(l);
  }
  else {
    lua_pushinteger(l, item.get_amount());
  }
  return 1;
}

/**
 * @brief Implementation of \ref lua_api_item_set_amount.
 * @param l The Lua context that is calling this function.
 * @return Number of values to return to Lua.
 */
int LuaContext::item_api_set_amount(lua_State* l) {

  EquipmentItem& item = check_item(l, 1);
  int amount = int(luaL_checkinteger(l, 2));

  if (!item.has_amount()) {
    luaL_error(l, (StringConcat() <<
        "Item '" << item.get_name() << "' has no amount").c_str());
  }

  item.set_amount(amount);

  return 0;
}

/**
 * @brief Implementation of \ref lua_api_item_add_amount.
 * @param l The Lua context that is calling this function.
 * @return Number of values to return to Lua.
 */
int LuaContext::item_api_add_amount(lua_State* l) {

  EquipmentItem& item = check_item(l, 1);
  int amount = int(luaL_checkinteger(l, 2));

  if (!item.has_amount()) {
    luaL_error(l, (StringConcat() <<
        "Item '" << item.get_name() << "' has no amount").c_str());
  }

  item.set_amount(item.get_amount() + amount);

  return 0;
}

/**
 * @brief Implementation of \ref lua_api_item_remove_amount.
 * @param l The Lua context that is calling this function.
 * @return Number of values to return to Lua.
 */
int LuaContext::item_api_remove_amount(lua_State* l) {

  EquipmentItem& item = check_item(l, 1);
  int amount = int(luaL_checkinteger(l, 2));

  if (!item.has_amount()) {
    luaL_error(l, (StringConcat() <<
        "Item '" << item.get_name() << "' has no amount").c_str());
  }

  item.set_amount(item.get_amount() - amount);

  return 0;
}

/**
 * @brief Implementation of \ref lua_api_item_get_max_amount.
 * @param l The Lua context that is calling this function.
 * @return Number of values to return to Lua.
 */
int LuaContext::item_api_get_max_amount(lua_State* l) {

  EquipmentItem& item = check_item(l, 1);

  if (!item.has_amount()) {
    luaL_error(l, (StringConcat() <<
        "Item '" << item.get_name() << "' has no amount").c_str());
  }

  lua_pushinteger(l, item.get_max_amount());
  return 1;
}

/**
 * @brief Implementation of \ref lua_api_item_set_max_amount.
 * @param l The Lua context that is calling this function.
 * @return Number of values to return to Lua.
 */
int LuaContext::item_api_set_max_amount(lua_State* l) {

  EquipmentItem& item = check_item(l, 1);
  int max_amount = int(luaL_checkinteger(l, 2));

  if (!item.has_amount()) {
    luaL_error(l, (StringConcat() <<
        "Item '" << item.get_name() << "' has no amount").c_str());
  }

  item.set_max_amount(max_amount);

  return 0;
}

/**
 * @brief Implementation of \ref lua_api_item_set_finished.
 * @param l The Lua context that is calling this function.
 * @return Number of values to return to Lua.
 */
int LuaContext::item_api_set_finished(lua_State* l) {

  EquipmentItem& item = check_item(l, 1);

  // retrieve the inventory item from the hero
  Hero& hero = item.get_game()->get_hero();
  if (hero.is_using_inventory_item()) {  // Do nothing if the script has already changed the hero's state.

    InventoryItem& inventory_item = hero.get_current_inventory_item();
    Debug::check_assertion(inventory_item.get_name() == item.get_name(),
        StringConcat() << "Trying to finish inventory item '" << item.get_name()
        << "' but the current inventory item is '" << inventory_item.get_name() << "'");
    inventory_item.set_finished();
  }

  return 0;
}

/**
 * @brief Calls the on_update() method of a Lua equipment item.
 * @param item An equipment item.
 */
void LuaContext::item_on_update(EquipmentItem& item) {

  push_item(l, item);
  on_update();
  lua_pop(l, 1);
}

/**
 * @brief Calls the on_suspended() method of a Lua equipment item.
 * @param item An equipment item.
 * @param suspended true if the game is suspended.
 */
void LuaContext::item_on_suspended(EquipmentItem& item, bool suspended) {

  push_item(l, item);
  on_suspended(suspended);
  lua_pop(l, 1);
}

/**
 * @brief Calls the on_created() method of a Lua equipment item.
 * @param item An equipment item.
 */
void LuaContext::item_on_created(EquipmentItem& item) {

  push_item(l, item);
  on_created();
  lua_pop(l, 1);
}

/**
 * @brief Calls the on_started() method of a Lua equipment item.
 * @param item An equipment item.
 */
void LuaContext::item_on_started(EquipmentItem& item) {

  push_item(l, item);
  on_started();
  lua_pop(l, 1);
}

/**
 * @brief Calls the on_finished() method of a Lua equipment item.
 * @param item An equipment item.
 */
void LuaContext::item_on_finished(EquipmentItem& item) {

  push_item(l, item);
  on_finished();
  remove_timers(-1);  // Stop timers and menus associated to this item.
  remove_menus(-1);
  lua_pop(l, 1);
}

/**
 * @brief Calls the on_map_changed() method of a Lua equipment item.
 * @param item An equipment item.
 * @param map A map.
 */
void LuaContext::item_on_map_changed(EquipmentItem& item, Map& map) {

  push_item(l, item);
  on_map_changed(map);
  lua_pop(l, 1);
}

/**
 * @brief Calls the on_pickable_created() method of a Lua equipment item.
 * @param item An equipment item.
 * @param pickable The instance of pickable item that has just appeared.
 */
void LuaContext::item_on_pickable_created(EquipmentItem& item,
    Pickable& pickable) {

  push_item(l, item);
  on_pickable_created(pickable);
  lua_pop(l, 1);
}

/**
 * @brief Calls the on_pickable_movement_changed() method of a Lua equipment item.
 * @param item An equipment item.
 * @param pickable The instance of pickable item whose movement has changed.
 * @param movement The movement.
 */
void LuaContext::item_on_pickable_movement_changed(EquipmentItem& item,
    Pickable& pickable, Movement& movement) {

  push_item(l, item);
  on_pickable_movement_changed(pickable, movement);
  lua_pop(l, 1);
}

/**
 * @brief Calls the on_obtaining() method of a Lua equipment item.
 * @param item An equipment item.
 * @param treasure The treasure being obtained.
 */
void LuaContext::item_on_obtaining(EquipmentItem& item, const Treasure& treasure) {

  push_item(l, item);
  on_obtaining(treasure);
  lua_pop(l, 1);
}

/**
 * @brief Calls the on_obtained() method of a Lua equipment item.
 * @param item An equipment item.
 * @param treasure The treasure just obtained.
 */
void LuaContext::item_on_obtained(EquipmentItem& item, const Treasure& treasure) {

  push_item(l, item);
  on_obtained(treasure);
  lua_pop(l, 1);
}

/**
 * @brief Calls the on_variant_changed() method of a Lua equipment item.
 * @param item An equipment item.
 * @param variant The possession state.
 */
void LuaContext::item_on_variant_changed(EquipmentItem& item, int variant) {

  push_item(l, item);
  on_variant_changed(variant);
  lua_pop(l, 1);
}

/**
 * @brief Calls the on_amount_changed() method of a Lua equipment item.
 * @param item An equipment item.
 * @param amount The amount of this item.
 */
void LuaContext::item_on_amount_changed(EquipmentItem& item, int amount) {

  push_item(l, item);
  on_amount_changed(amount);
  lua_pop(l, 1);
}

/**
 * @brief Calls the on_using() method of a Lua equipment item.
 * @param item An equipment item.
 */
void LuaContext::item_on_using(EquipmentItem& item) {

  push_item(l, item);
  on_using();
  lua_pop(l, 1);
}

/**
 * @brief Calls the on_ability_used() method of a Lua equipment item.
 * @param item An equipment item.
 * @param ability_name The ability just used.
 */
void LuaContext::item_on_ability_used(EquipmentItem& item, const std::string& ability_name) {

  push_item(l, item);
  on_ability_used(ability_name);
  lua_pop(l, 1);
}

/**
 * @brief Calls the on_npc_interaction() method of a Lua equipment item.
 * @param item An equipment item.
 * @param npc An NPC.
 */
void LuaContext::item_on_npc_interaction(EquipmentItem& item, NPC& npc) {

  push_item(l, item);
  on_npc_interaction(npc);
  lua_pop(l, 1);
}

/**
 * @brief Calls the on_npc_interaction_item() method of a Lua equipment item.
 * @param item The equipment item linked to the NPC.
 * @param npc An NPC.
 * @param item_used The equipment item used.
 * @return true if an interaction occurred.
 */
bool LuaContext::item_on_npc_interaction_item(EquipmentItem& item, NPC& npc,
    EquipmentItem& item_used) {

  push_item(l, item);
  bool result = on_npc_interaction_item(npc, item_used);
  lua_pop(l, 1);
  return result;
}

/**
 * @brief Calls the on_npc_collision_fire() method of a Lua equipment item.
 * @param item An equipment item.
 * @param npc An NPC.
 */
void LuaContext::item_on_npc_collision_fire(EquipmentItem& item, NPC& npc) {

  push_item(l, item);
  on_npc_collision_fire(npc);
  lua_pop(l, 1);
}

