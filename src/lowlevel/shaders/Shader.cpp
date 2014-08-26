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
#include "lowlevel/shaders/Shader.h"
#include "lowlevel/System.h"
#include "lowlevel/FileTools.h"
#include "lowlevel/Video.h"
#include "lua/LuaContext.h"
#include "lua/LuaTools.h"

namespace solarus {

std::string Shader::sampler_type = "";
std::string Shader::shading_language_version = "";
int Shader::display_time = 0;


/**
 * \brief Constructor.
 * \param shader_name The name of the shader to load.
 */
Shader::Shader(const std::string& shader_name):
    shader_name(shader_name),
    default_window_scale(1.0),
    is_shader_valid(true) {
}
  
/**
 * \brief Destructor.
 */
Shader::~Shader() {
}

/**
 * \brief Set the shading language version string.
 * \param version The shading language version.
 */
void Shader::set_shading_language_version(const std::string& version) {

  shading_language_version = version;
}
  
/**
 * \brief Get the sampler type as string.
 * \return The sampler type.
 */
const std::string& Shader::get_sampler_type() {

  return sampler_type;
}
  
/**
 * \brief Reset the displaying time.
 */
void Shader::reset_time() {

  display_time = 0;
}
  
/**
 * \brief Get the name of the shader, which is also the name of the related video mode.
 * \return The name of the shader.
 */
const std::string& Shader::get_name() {

  return shader_name;
}
  
/**
 * \brief Get the scale to apply on the quest size to get the final default size of the related video mode.
 * \return The window scale.
 */
double Shader::get_default_window_scale() {

  return default_window_scale;
}

/**
 * \brief Check if the engine shader context is marked as compatible by the shader script.
 * \return True if the engine shader context is not been explicitely set as not compatible with the shader script .
 */
bool Shader::is_valid() {

  return is_shader_valid;
}
  
/**
 * \brief Draws the quest surface on the screen in a shader-allowed context.
 * It will perform the render using the OpenGL API directly.
 * \param quest_surface the surface to render on the screen
 */
void Shader::render(Surface& /* quest_surface */) {
  
  display_time += System::timestep;
}

/**
 * \brief Load all files from the corresponding shader, depending on the driver and shader names.
 * Parse the Lua file and compile GLSL others.
 * \param shader_name The name of the shader to load.
 */
void Shader::load(const std::string& shader_name) {
    
  const std::string shader_path =
      "shaders/videomodes/" + shader_name;
    
  // Parse the lua file
  load_lua_file(shader_path);
}

/**
 * \brief Dummy method used to call the static lua callback for a specific shader implementation.
 * \param l The lua state.
 */
void Shader::register_callback(lua_State* /* l */) {
}
  
/**
 * \brief Load and parse the Lua file of the requested shader.
 * \param path The path to the lua file, relative to the data folder.
 */
void Shader::load_lua_file(const std::string& path) {
    
  lua_State* l = luaL_newstate();
  luaL_openlibs(l);  // FIXME don't open the libs
  size_t size;
  char* buffer;
    
  FileTools::data_file_open_buffer(path, &buffer, &size);
  int load_result = luaL_loadbuffer(l, buffer, size, path.c_str());
    
  if (load_result != 0) {
    // Syntax error in the lua file.
    Debug::die(std::string("Failed to load ") + path + " : " + lua_tostring(l, -1));
  }
  else {
    // Register the callback and send string parameters to the lua script.
    register_callback(l);
    lua_pushstring(l, Video::get_rendering_driver_name().c_str());
    lua_pushstring(l, shading_language_version.c_str());
    lua_pushstring(l, sampler_type.c_str());
      
    if (lua_pcall(l, 3, 0, 0) != 0) {
        
      // Runtime error.
      Debug::die(std::string("Failed to parse ") + path + " : " + lua_tostring(l, -1));
      lua_pop(l, 1);
    }
  }

  FileTools::data_file_close_buffer(buffer);
  lua_close(l);
}

}
