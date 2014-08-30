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
#include "lua/LuaContext.h"
#include "lua/LuaTools.h"
#include "lowlevel/Geometry.h"
#include "lowlevel/FileTools.h"
#include "lowlevel/System.h"
#include "MainLoop.h"
#include "Settings.h"
#include <lua.hpp>
#include <sstream>
#include <cmath>

namespace solarus {

/**
 * Name of the Lua table representing the main module of Solarus.
 */
const std::string LuaContext::main_module_name = "sol.main";

/**
 * \brief Initializes the main features provided to Lua.
 */
void LuaContext::register_main_module() {

  static const luaL_Reg functions[] = {
      { "load_file", main_api_load_file },
      { "do_file", main_api_do_file },
      { "reset", main_api_reset },
      { "exit", main_api_exit },
      { "get_elapsed_time", main_api_get_elapsed_time },
      { "get_quest_write_dir", main_api_get_quest_write_dir },
      { "set_quest_write_dir", main_api_set_quest_write_dir },
      { "load_settings", main_api_load_settings },
      { "save_settings", main_api_save_settings },
      { "get_distance", main_api_get_distance },
      { "get_angle", main_api_get_angle },
      { "get_metatable", main_api_get_metatable },
      { "get_platform", main_api_get_platform },
      { NULL, NULL }
  };

  register_functions(main_module_name, functions);

  // Store sol.main in the registry to access it safely
  // from C++ (and also slightly faster).
  // After that, the engine will never rely on the existence of a global
  // value called "sol". The user can therefore do whatever he wants, including
  // renaming the sol global table to something else in the unlikely case where
  // another Lua library called "sol" is required, or if he simply does not
  // like the name "sol".

                                  // ...
  lua_getglobal(l, "sol");
                                  // ... sol
  lua_getfield(l, -1, "main");
                                  // ... sol main
  lua_setfield(l, LUA_REGISTRYINDEX, main_module_name.c_str());
                                  // ... sol
  lua_pop(l, 1);
                                  // ...
}

/**
 * \brief Pushes the sol.main table onto the stack.
 * \param l A Lua state.
 */
void LuaContext::push_main(lua_State* l) {

  lua_getfield(l, LUA_REGISTRYINDEX, main_module_name.c_str());
}

/**
 * \brief Implementation of sol.main.load_file().
 * \param l The Lua context that is calling this function.
 * \return Number of values to return to Lua.
 */
int LuaContext::main_api_load_file(lua_State *l) {

  const std::string& file_name = luaL_checkstring(l, 1);

  if (!load_file_if_exists(l, file_name)) {
    lua_pushnil(l);
  }

  return 1;
}

/**
 * \brief Implementation of sol.main.do_file().
 * \param l The Lua context that is calling this function.
 * \return Number of values to return to Lua.
 */
int LuaContext::main_api_do_file(lua_State *l) {

  const std::string& file_name = luaL_checkstring(l, 1);

  do_file(l, file_name);

  return 0;
}

/**
 * \brief Implementation of sol.main.reset().
 * \param l the Lua context that is calling this function
 * \return number of values to return to Lua
 */
int LuaContext::main_api_reset(lua_State* l) {

  get_lua_context(l).get_main_loop().set_resetting();

  return 0;
}

/**
 * \brief Implementation of sol.main.exit().
 * \param l the Lua context that is calling this function
 * \return number of values to return to Lua
 */
int LuaContext::main_api_exit(lua_State* l) {

  get_lua_context(l).get_main_loop().set_exiting();

  return 0;
}

/**
 * \brief Implementation of sol.main.get_elapsed_time().
 * \param l The Lua context that is calling this function.
 * \return Number of values to return to Lua.
 */
int LuaContext::main_api_get_elapsed_time(lua_State* l) {

  uint32_t elapsed_time = System::now();

  lua_pushinteger(l, elapsed_time);
  return 1;
}

/**
 * \brief Implementation of sol.main.get_quest_write_dir().
 * \param l the Lua context that is calling this function
 * \return number of values to return to Lua
 */
int LuaContext::main_api_get_quest_write_dir(lua_State* l) {

  const std::string& quest_write_dir = FileTools::get_quest_write_dir();

  if (quest_write_dir.empty()) {
    lua_pushnil(l);
  }
  else {
    push_string(l, quest_write_dir);
  }
  return 1;
}

/**
 * \brief Implementation of sol.main.set_quest_write_dir().
 * \param l the Lua context that is calling this function
 * \return number of values to return to Lua
 */
int LuaContext::main_api_set_quest_write_dir(lua_State* l) {

  const std::string& quest_write_dir = luaL_optstring(l, 1, "");

  FileTools::set_quest_write_dir(quest_write_dir);

  return 0;
}

/**
 * \brief Implementation of sol.main.load_settings().
 * \param l the Lua context that is calling this function
 * \return number of values to return to Lua
 */
int LuaContext::main_api_load_settings(lua_State* l) {

  std::string file_name = luaL_optstring(l, 1, "settings.dat");

  if (FileTools::get_quest_write_dir().empty()) {
    LuaTools::error(l, "Cannot load settings: no write directory was specified in quest.dat");
  }

  bool success = Settings::load(file_name);

  lua_pushboolean(l, success);
  return 1;
}

/**
 * \brief Implementation of sol.main.save_settings().
 * \param l the Lua context that is calling this function
 * \return number of values to return to Lua
 */
int LuaContext::main_api_save_settings(lua_State* l) {

  std::string file_name = luaL_optstring(l, 1, "settings.dat");

  if (FileTools::get_quest_write_dir().empty()) {
    LuaTools::error(l, "Cannot save settings: no write directory was specified in quest.dat");
  }

  bool success = Settings::save(file_name);

  lua_pushboolean(l, success);
  return 1;
}

/**
 * \brief Implementation of sol.main.get_distance().
 * \param l the Lua context that is calling this function
 * \return number of values to return to Lua
 */
int LuaContext::main_api_get_distance(lua_State* l) {

  int x1 = luaL_checkint(l, 1);
  int y1 = luaL_checkint(l, 2);
  int x2 = luaL_checkint(l, 3);
  int y2 = luaL_checkint(l, 4);

  int distance = (int) Geometry::get_distance(x1, y1, x2, y2);

  lua_pushinteger(l, distance);
  return 1;
}

/**
 * \brief Implementation of sol.main.get_angle().
 * \param l the Lua context that is calling this function
 * \return number of values to return to Lua
 */
int LuaContext::main_api_get_angle(lua_State* l) {

  int x1 = luaL_checkint(l, 1);
  int y1 = luaL_checkint(l, 2);
  int x2 = luaL_checkint(l, 3);
  int y2 = luaL_checkint(l, 4);

  double angle = Geometry::get_angle(x1, y1, x2, y2);

  lua_pushnumber(l, angle);
  return 1;
}

/**
 * \brief Implementation of sol.main.get_metatable().
 * \param l The Lua context that is calling this function.
 * \return Number of values to return to Lua.
 */
int LuaContext::main_api_get_metatable(lua_State* l) {

  const std::string& type_name = luaL_checkstring(l, 1);

  luaL_getmetatable(l, (std::string("sol.") + type_name).c_str());
  return 1;
}

/**
 * \brief Implementation of sol.main.get_metatable().
 * \param l The Lua context that is calling this function.
 * \return Number of values to return to Lua.
 */
int LuaContext::main_api_get_platform(lua_State* l) {

  const std::string& platform = System::get_platform();

  lua_pushstring(l, platform.c_str());
  return 1;
}

/**
 * \brief Calls sol.main.on_started() if it exists.
 *
 * This function is called when the engine requests Lua to show an
 * initial screen, i.e. at the beginning of the program
 * or when the program is reset.
 */
void LuaContext::main_on_started() {

  push_main(l);
  on_started();
  lua_pop(l, 1);
}

/**
 * \brief Calls sol.main.on_finished() if it exists.
 *
 * This function is called when the program is reset or stopped.
 */
void LuaContext::main_on_finished() {

  push_main(l);
  on_finished();
  remove_timers(-1);  // Stop timers associated to sol.main.
  remove_menus(-1);  // Stop menus associated to sol.main.
  lua_pop(l, 1);
}

/**
 * \brief Calls sol.main.on_update() if it exists.
 *
 * This function is called at each cycle by the main loop.
 */
void LuaContext::main_on_update() {

  push_main(l);
  on_update();
  menus_on_update(-1);
  lua_pop(l, 1);
}

/**
 * \brief Calls sol.main.on_draw() if it exists.
 * \param dst_surface The destination surface.
 */
void LuaContext::main_on_draw(Surface& dst_surface) {

  push_main(l);
  on_draw(dst_surface);
  menus_on_draw(-1, dst_surface);
  lua_pop(l, 1);
}

/**
 * \brief Notifies Lua that an input event has just occurred.
 *
 * The appropriate callback in sol.main is triggered if it exists.
 *
 * \param event The input event to handle.
 * \return \c true if the event was handled and should stop being propagated.
 */
bool LuaContext::main_on_input(const InputEvent& event) {

  bool handled = false;
  push_main(l);
  handled = on_input(event);
  if (!handled) {
    handled = menus_on_input(-1, event);
  }
  lua_pop(l, 1);
  return handled;
}

}

