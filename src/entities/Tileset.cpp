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
#include "entities/Tileset.h"
#include "entities/SimpleTilePattern.h"
#include "entities/AnimatedTilePattern.h"
#include "entities/SelfScrollingTilePattern.h"
#include "entities/TimeScrollingTilePattern.h"
#include "entities/ParallaxScrollingTilePattern.h"
#include "lowlevel/FileTools.h"
#include "lowlevel/Surface.h"
#include "lowlevel/Debug.h"
#include "lua/LuaTools.h"
#include <lua.hpp>
#include <sstream>

namespace solarus {

const std::string Tileset::ground_names[] = {
  "empty",
  "traversable",
  "wall",
  "low_wall",
  "wall_top_right",
  "wall_top_left",
  "wall_bottom_left",
  "wall_bottom_right",
  "wall_top_right_water",
  "wall_top_left_water",
  "wall_bottom_left_water",
  "wall_bottom_right_water",
  "deep_water",
  "shallow_water",
  "grass",
  "hole",
  "ice",
  "ladder",
  "prickles",
  "lava",
  ""  // Sentinel.
};

/**
 * \brief Constructor.
 * \param id id of the tileset to create
 */
Tileset::Tileset(const std::string& id):
  id(id),
  tiles_image(nullptr),
  entities_image(nullptr) {
}

/**
 * \brief Destructor.
 */
Tileset::~Tileset() {
  if (is_loaded()) {
    unload(); // destroy the tiles
  }
}

/**
 * \brief Returns the id of this tileset.
 * \return the tileset id
 */
const std::string& Tileset::get_id() {
  return id;
}

/**
 * \brief Adds a tile pattern to this tileset.
 *
 * This function is called by load().
 *
 * \param id id of this tile pattern
 * \param tile_pattern the tile pattern to add
 */
void Tileset::add_tile_pattern(const std::string& id, TilePattern *tile_pattern) {
  tile_patterns[id] = tile_pattern;
}

/**
 * \brief Loads the tileset from its file by creating all tile patterns.
 */
void Tileset::load() {

  // open the tileset file
  std::string file_name = std::string("tilesets/") + id + ".dat";

  lua_State* l = luaL_newstate();
  size_t size;
  char* buffer;
  FileTools::data_file_open_buffer(file_name, &buffer, &size);
  int load_result = luaL_loadbuffer(l, buffer, size, file_name.c_str());
  FileTools::data_file_close_buffer(buffer);

  if (load_result != 0) {
    Debug::die(std::string("Failed to load tileset file '")
        + file_name + "': " + lua_tostring(l, -1));
    lua_pop(l, 1);
  }

  lua_pushlightuserdata(l, this);
  lua_setfield(l, LUA_REGISTRYINDEX, "tileset");
  lua_register(l, "background_color", l_background_color);
  lua_register(l, "tile_pattern", l_tile_pattern);
  if (lua_pcall(l, 0, 0, 0) != 0) {
    Debug::die(std::string("Failed to load tileset file '")
        + file_name + "': " + lua_tostring(l, -1));
    lua_pop(l, 1);
  }

  lua_close(l);

  // load the tileset images
  file_name = std::string("tilesets/") + id + ".tiles.png";
  tiles_image = Surface::create(file_name, Surface::DIR_DATA);
  RefCountable::ref(tiles_image);

  file_name = std::string("tilesets/") + id + ".entities.png";
  entities_image = Surface::create(file_name, Surface::DIR_DATA);
  RefCountable::ref(entities_image);
}

/**
 * \brief Destroys the tile patterns and frees the memory used
 * by the tileset image.
 */
void Tileset::unload() {

  for (const auto& kvp: tile_patterns) {
    delete kvp.second;
  }
  tile_patterns.clear();

  RefCountable::unref(tiles_image);
  tiles_image = nullptr;

  RefCountable::unref(entities_image);
  entities_image = nullptr;
}

/**
 * \brief Returns the background color of this tileset.
 * \return the background color
 */
Color& Tileset::get_background_color() {
  return background_color;
}

/**
 * \brief Returns whether this tileset is loaded.
 * \return true if this tileset is loaded
 */
bool Tileset::is_loaded() {
  return tiles_image != nullptr;
}

/**
 * \brief Returns the image containing the tiles of this tileset.
 * \return the tiles image
 */
Surface& Tileset::get_tiles_image() {
  return *tiles_image;
}

/**
 * \brief Returns the image containing the skin-dependent dynamic entities for this tileset.
 * \return the image containing the skin-dependent dynamic entities for this tileset
 */
Surface& Tileset::get_entities_image() {
  return *entities_image;
}

/**
 * \brief Returns a tile pattern from this tileset.
 * \param id id of the tile pattern to get
 * \return the tile pattern with this id
 */
TilePattern& Tileset::get_tile_pattern(const std::string& id) {

  TilePattern* tile_pattern =  tile_patterns[id];
  if (tile_pattern == nullptr) {
    std::ostringstream oss;
    oss << "No such tile pattern in tileset '" << get_id() << "': " << id;
    Debug::die(oss.str());
  }
  return *tile_pattern;
}

/**
 * \brief Changes the tiles images, the entities images and the background
 * color of this tileset.
 * \param other_id Id of another tileset whose images and background color
 * will be copied into this tileset.
 */
void Tileset::set_images(const std::string& other_id) {

  // Load the new tileset to take its images.
  Tileset tmp_tileset(other_id);
  tmp_tileset.load();

  RefCountable::unref(tiles_image);
  tiles_image = &tmp_tileset.get_tiles_image();
  RefCountable::ref(tiles_image);

  RefCountable::unref(entities_image);
  entities_image = &tmp_tileset.get_entities_image();
  RefCountable::ref(entities_image);

  background_color = tmp_tileset.get_background_color();
}

/**
 * \brief Function called by Lua to set the background color of the tileset.
 *
 * - Argument 1 (table): background color (must be an array of 3 integers).
 *
 * \param l The Lua context that is calling this function.
 * \return Number of values to return to Lua.
 */
int Tileset::l_background_color(lua_State* l) {

  lua_getfield(l, LUA_REGISTRYINDEX, "tileset");
  void* p = lua_touserdata(l, -1);
  Tileset* tileset = static_cast<Tileset*>(p);
  lua_pop(l, 1);

  luaL_checktype(l, 1, LUA_TTABLE);
  lua_rawgeti(l, 1, 1);
  lua_rawgeti(l, 1, 2);
  lua_rawgeti(l, 1, 3);
  Color color(luaL_checkint(l, -3),
    luaL_checkint(l, -2),
    luaL_checkint(l, -1));
  lua_pop(l, 3);

  tileset->background_color = color;

  return 0;
}

/**
 * \brief Function called by Lua to add a tile pattern to the tileset.
 *
 * - Argument 1 (table): A table describing the tile pattern to create.
 *
 * \param l The Lua context that is calling this function.
 * \return Number of values to return to Lua.
 */
int Tileset::l_tile_pattern(lua_State* l) {

  lua_getfield(l, LUA_REGISTRYINDEX, "tileset");
  Tileset* tileset = static_cast<Tileset*>(lua_touserdata(l, -1));
  lua_pop(l, 1);

  int x[] = { -1, -1, -1, -1 };
  int y[] = { -1, -1, -1, -1 };

  const std::string& id = LuaTools::check_string_field(l, 1, "id");
  const Ground ground = LuaTools::check_enum_field<Ground>(l, 1, "ground", ground_names);
  const int width = LuaTools::check_int_field(l, 1, "width");
  const int height = LuaTools::check_int_field(l, 1, "height");
  const std::string& scrolling = LuaTools::opt_string_field(l, 1, "scrolling", "");

  int i = 0, j = 0;
  lua_settop(l, 1);
  lua_getfield(l, 1, "x");
  if (lua_isnumber(l, 2)) {
    // Single frame.
    x[0] = luaL_checkint(l, 2);
    i = 1;
  }
  else {
    // Multi-frame.
    lua_pushnil(l);
    while (lua_next(l, 2) != 0 && i < 4) {
      x[i] = luaL_checkint(l, 4);
      ++i;
      lua_pop(l, 1);
    }
  }
  lua_pop(l, 1);
  Debug::check_assertion(lua_gettop(l) == 1, "Invalid stack when parsing tile pattern");

  lua_getfield(l, 1, "y");
  if (lua_isnumber(l, 2)) {
    // Single frame.
    y[0] = luaL_checkint(l, 2);
    j = 1;
  }
  else {
    // Multi-frame.
    lua_pushnil(l);
    while (lua_next(l, 2) != 0 && j < 4) {
      y[j] = luaL_checkint(l, 4);
      ++j;
      lua_pop(l, 1);
    }
  }
  lua_pop(l, 1);
  Debug::check_assertion(lua_gettop(l) == 1, "Invalid stack when parsing tile pattern");

  // Check data.
  if (i != 1 && i != 3 && i != 4) {
    LuaTools::arg_error(l, 1, "Invalid number of frames for x");
  }
  if (j != 1 && j != 3 && j != 4) {
    LuaTools::arg_error(l, 1, "Invalid number of frames for y");
  }
  if (i != j) {
    LuaTools::arg_error(l, 1, "The length of x and y must match");
  }

  // Create the tile pattern.
  TilePattern* tile_pattern = nullptr;
  if (i == 1) {
    // Single frame.
    if (scrolling.empty()) {
      tile_pattern = new SimpleTilePattern(ground, x[0], y[0], width, height);
    }
    else if (scrolling == "parallax") {
      tile_pattern = new ParallaxScrollingTilePattern(ground, x[0], y[0], width, height);
    }
    else if (scrolling == "self") {
      tile_pattern = new SelfScrollingTilePattern(ground, x[0], y[0], width, height);
    }
  }
  else {
    // Multi-frame.
    if (scrolling == "self") {
      LuaTools::arg_error(l, 1, "Multi-frame is not supported for self-scrolling tiles");
    }
    bool parallax = scrolling == "parallax";
    AnimatedTilePattern::AnimationSequence sequence = (i == 3) ?
        AnimatedTilePattern::ANIMATION_SEQUENCE_012 : AnimatedTilePattern::ANIMATION_SEQUENCE_0121;
    tile_pattern = new AnimatedTilePattern(ground, sequence, width, height,
        x[0], y[0], x[1], y[1], x[2], y[2], parallax);
  }

  tileset->add_tile_pattern(id, tile_pattern);

  return 0;
}

}

