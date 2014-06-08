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
#include "lowlevel/Rectangle.h"
#include "lowlevel/InputEvent.h"
#include "lowlevel/Video.h"

namespace solarus {

const std::string LuaContext::input_module_name = "sol.input";

/**
 * \brief Initializes the input features provided to Lua.
 */
void LuaContext::register_input_module() {

  static const luaL_Reg functions[] = {
      { "is_joypad_enabled", input_api_is_joypad_enabled },
      { "set_joypad_enabled", input_api_set_joypad_enabled },
      { "is_key_pressed", input_api_is_key_pressed },
      { "get_key_modifiers", input_api_get_key_modifiers },
      { "is_joypad_button_pressed", input_api_is_joypad_button_pressed },
      { "get_joypad_axis_state", input_api_get_joypad_axis_state },
      { "get_joypad_hat_direction", input_api_get_joypad_hat_direction },
      { "is_mouse_button_pressed", input_api_is_mouse_button_pressed },
      { "get_mouse_position", input_api_get_mouse_position },
      { NULL, NULL }
  };

  register_functions(input_module_name, functions);
}

/**
 * \brief Implementation of sol.input.is_joypad_enabled().
 * \param l The Lua context that is calling this function.
 * \return Number of values to return to Lua.
 */
int LuaContext::input_api_is_joypad_enabled(lua_State* l) {

  lua_pushboolean(l, InputEvent::is_joypad_enabled());
  return 1;
}

/**
 * \brief Implementation of sol.input.set_joypad_enabled().
 * \param l The Lua context that is calling this function.
 * \return Number of values to return to Lua.
 */
int LuaContext::input_api_set_joypad_enabled(lua_State* l) {

  bool joypad_enabled = true;
  if (lua_gettop(l) >= 2) {
    joypad_enabled = lua_toboolean(l, 2);
  }

  InputEvent::set_joypad_enabled(joypad_enabled);

  return 0;
}

/**
 * \brief Implementation of sol.input.is_key_pressed().
 * \param l The Lua context that is calling this function.
 * \return Number of values to return to Lua.
 */
int LuaContext::input_api_is_key_pressed(lua_State* l) {

  const std::string& key_name = luaL_checkstring(l, 1);
  InputEvent::KeyboardKey key = InputEvent::get_keyboard_key_by_name(key_name);

  if (key == InputEvent::KEY_NONE) {
    LuaTools::arg_error(l, 1, std::string(
        "Unknown keyboard key name: '") + key_name + "'");
  }

  lua_pushboolean(l, InputEvent::is_key_down(key));
  return 1;
}

/**
 * \brief Implementation of sol.input.get_key_modifiers().
 * \param l The Lua context that is calling this function.
 * \return Number of values to return to Lua.
 */
int LuaContext::input_api_get_key_modifiers(lua_State* l) {

  const bool shift = InputEvent::is_shift_down();
  const bool control = InputEvent::is_control_down();
  const bool alt = InputEvent::is_alt_down();
  const bool caps_lock = InputEvent::is_caps_lock_on();
  const bool num_lock = InputEvent::is_num_lock_on();

  lua_newtable(l);
  if (shift) {
    lua_pushboolean(l, 1);
    lua_setfield(l, -2, "shift");
  }
  if (control) {
    lua_pushboolean(l, 1);
    lua_setfield(l, -2, "control");
  }
  if (alt) {
    lua_pushboolean(l, 1);
    lua_setfield(l, -2, "alt");
  }
  if (caps_lock) {
    lua_pushboolean(l, 1);
    lua_setfield(l, -2, "caps lock");
  }
  if (num_lock) {
    lua_pushboolean(l, 1);
    lua_setfield(l, -2, "num lock");
  }

  return 1;
}

/**
 * \brief Implementation of sol.input.is_joypad_button_pressed().
 * \param l The Lua context that is calling this function.
 * \return Number of values to return to Lua.
 */
int LuaContext::input_api_is_joypad_button_pressed(lua_State* l) {

  int button = luaL_checkint(l, 1);

  lua_pushboolean(l, InputEvent::is_joypad_button_down(button));
  return 1;
}

/**
 * \brief Implementation of sol.input.get_joypad_axis_state().
 * \param l The Lua context that is calling this function.
 * \return Number of values to return to Lua.
 */
int LuaContext::input_api_get_joypad_axis_state(lua_State* l) {

  int axis = luaL_checkint(l, 1);

  lua_pushinteger(l, InputEvent::get_joypad_axis_state(axis));
  return 1;
}

/**
 * \brief Implementation of sol.input.get_joypad_hat_direction().
 * \param l The Lua context that is calling this function.
 * \return Number of values to return to Lua.
 */
int LuaContext::input_api_get_joypad_hat_direction(lua_State* l) {

  int hat = luaL_checkint(l, 1);

  lua_pushinteger(l, InputEvent::get_joypad_hat_direction(hat));
  return 1;
}

/**
 * \brief Implementation of sol.input.is_mouse_button_pressed().
 * \param l The Lua context that is calling this function.
 * \return Number of values to return to Lua.
 */
int LuaContext::input_api_is_mouse_button_pressed(lua_State* l) {

  const std::string& button_name = luaL_checkstring(l, 1);
  InputEvent::MouseButton button = InputEvent::get_mouse_button_by_name(button_name);

  lua_pushboolean(l, InputEvent::is_mouse_button_down(button));
  return 1;
}

/**
 * \brief Implementation of sol.input.is_mouse_button_released().
 * \param l The Lua context that is calling this function.
 * \return Number of values to return to Lua.
 */
int LuaContext::input_api_is_mouse_button_released(lua_State* l) {

  const std::string& button_name = luaL_checkstring(l, 1);
  InputEvent::MouseButton button = InputEvent::get_mouse_button_by_name(button_name);

  lua_pushboolean(l, !InputEvent::is_mouse_button_down(button));
  return 1;
}

/**
 * \brief Implementation of sol.input.get_mouse_position().
 * \param l The Lua context that is calling this function.
 * \return Number of values to return to Lua.
 */
int LuaContext::input_api_get_mouse_position(lua_State* l) {

  const Rectangle& position = Video::get_scaled_position(InputEvent::get_mouse_position());

  lua_pushinteger(l, position.get_x());
  lua_pushinteger(l, position.get_y());
  return 2;
}

}

