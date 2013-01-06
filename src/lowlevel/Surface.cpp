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
#include "lowlevel/Surface.h"
#include "lowlevel/Color.h"
#include "lowlevel/Rectangle.h"
#include "lowlevel/FileTools.h"
#include "lowlevel/Debug.h"
#include "lowlevel/StringConcat.h"
#include "lua/LuaContext.h"
#include "Transition.h"
#include <SDL_image.h>

/**
 * @brief Creates an empty surface with the specified size.
 * @param width the width in pixels
 * @param height the height in pixels
 */
Surface::Surface(int width, int height):
  Drawable(),
  internal_surface_created(true) {

  this->internal_surface = SDL_CreateRGBSurface(
      SDL_SWSURFACE, width, height, SOLARUS_COLOR_DEPTH, 0, 0, 0, 0);
}

/**
 * @brief Creates a empty surface with the specified size.
 * @param size The size in pixels.
 */
Surface::Surface(const Rectangle& size):
  Drawable(),
  internal_surface_created(true) {

  this->internal_surface = SDL_CreateRGBSurface(
      SDL_HWSURFACE, size.get_width(), size.get_height(), SOLARUS_COLOR_DEPTH, 0, 0, 0, 0);
}

/**
 * @brief Creates a surface from the specified image file name.
 * @param file_name name of the image file to load, relative to the base directory specified
 * @param base_directory the base directory to use
 */
Surface::Surface(const std::string& file_name, ImageDirectory base_directory):
  Drawable(),
  internal_surface_created(true) {

  std::string prefix = "";
  bool language_specific = false;

  if (base_directory == DIR_SPRITES) {
    prefix = "sprites/";
  }
  else if (base_directory == DIR_LANGUAGE) {
    language_specific = true;
    prefix = "images/";
  }
  std::string prefixed_file_name = prefix + file_name;

  size_t size;
  char* buffer;
  FileTools::data_file_open_buffer(prefixed_file_name, &buffer, &size, language_specific);
  SDL_RWops* rw = SDL_RWFromMem(buffer, int(size));
  this->internal_surface = IMG_Load_RW(rw, 0);
  FileTools::data_file_close_buffer(buffer);
  SDL_RWclose(rw);

  Debug::check_assertion(internal_surface != NULL, StringConcat() << "Cannot load image '" << prefixed_file_name << "'");
}

/**
 * @brief Creates a surface form the specified SDL surface.
 *
 * This constructor must be used only by lowlevel classes that manipulate directly
 * SDL dependent surfaces.
 *
 * @param internal_surface the internal surface data (the destructor will not free it)
 */
Surface::Surface(SDL_Surface* internal_surface):
  Drawable(),
  internal_surface(internal_surface),
  internal_surface_created(false) {

}

/**
 * @brief Copy constructor.
 * @param other a surface to copy
 */
Surface::Surface(const Surface& other):
  Drawable(),
  internal_surface(SDL_ConvertSurface(other.internal_surface,
      other.internal_surface->format, other.internal_surface->flags)),
  internal_surface_created(true) {

}

/**
 * @brief Destructor.
 */
Surface::~Surface() {

  if (internal_surface_created) {
    SDL_FreeSurface(internal_surface);
  }
}

/**
 * @brief Returns the width of the surface.
 * @return the width in pixels
 */
int Surface::get_width() const {
  return internal_surface->w;
}

/**
 * @brief Returns the height of the surface.
 * @return the height in pixels
 */
int Surface::get_height() const {
  return internal_surface->h;
}

/**
 * @brief Returns the size of this surface.
 * @return the size of this surface
 */
const Rectangle Surface::get_size() const {
  
  return Rectangle(0, 0, get_width(), get_height());
}

/**
 * @brief Returns the transparency color of this surface.
 *
 * Pixels in that color will not be drawn.
 *
 * @return The transparency color.
 */
Color Surface::get_transparency_color() {

  return Color(internal_surface->format->colorkey);
}

/**
 * @brief Sets the transparency color of this surface.
 *
 * Pixels in that color will not be drawn.
 *
 * @param color The transparency color to set.
 */
void Surface::set_transparency_color(const Color& color) {

  SDL_SetColorKey(internal_surface, SDL_SRCCOLORKEY, color.get_internal_value());
}

/**
 * @brief Sets the opacity of this surface.
 * @param opacity the opacity (0 to 255)
 */
void Surface::set_opacity(int opacity) {

  // SDL has a special handling of the alpha value 128
  // which doesn't work well with my computer
  if (opacity == 128) {
    opacity = 127;
  }

  SDL_SetAlpha(internal_surface, SDL_SRCALPHA, opacity);
}

/**
 * @brief Restricts drawing on this surface to a subarea.
 *
 * Sets a subarea of the surface where the next drawings will be restricted to
 * when this surface is used as the destination of blitting.
 * A zero-sized rectangle means that drawings are not restricted to a subarea of the surface.
 * The rectangle specified may be partially outside this rectangle
 * (then it will be resized to fit inside).
 *
 * @param clipping_rectangle a subarea of the rectangle to restrict the drawing to
 */
void Surface::set_clipping_rectangle(const Rectangle& clipping_rectangle) {

  if (clipping_rectangle.get_width() == 0) {
    SDL_SetClipRect(internal_surface, NULL);
  }
  else {
    Rectangle copy = clipping_rectangle;
    SDL_SetClipRect(internal_surface, copy.get_internal_rect());
  }
}

/**
 * @brief Fills the entire surface with the specified color.
 * @param color a color
 */
void Surface::fill_with_color(Color& color) {
  SDL_FillRect(internal_surface, NULL, color.get_internal_value());
}

/**
 * @brief Fills a rectangle of this surface with the specified color.
 * @param color a color
 * @param where the rectangle to fill
 */
void Surface::fill_with_color(Color& color, const Rectangle& where) {
  Rectangle where2 = where;
  SDL_FillRect(internal_surface, where2.get_internal_rect(), color.get_internal_value());
}

/**
 * @brief Blits this surface on another surface.
 * @param dst_surface the destination surface
 * @param dst_position coordinates on the destination surface
 */
void Surface::raw_draw(Surface& dst_surface,
    const Rectangle& dst_position) {

  Rectangle dst_position2(dst_position);
  SDL_BlitSurface(internal_surface, NULL, dst_surface.internal_surface,
      dst_position2.get_internal_rect());
}

/**
 * @brief Draws a transition effect on this drawable object.
 * @param transition The transition effect to apply.
 */
void Surface::draw_transition(Transition& transition) {
  transition.draw(*this);
}

/**
 * @brief Blits a region of this surface on another surface.
 *
 * The top-left corner of the source subarea will be blitted on the other's surface top-left corner.
 *
 * @param src_position the subrectangle of this surface to pick
 * @param dst_surface the destination surface
 */
void Surface::draw_region(const Rectangle& src_position, Surface& dst_surface) {

  Rectangle src_position2(src_position);
  SDL_BlitSurface(internal_surface, src_position2.get_internal_rect(),
      dst_surface.internal_surface, NULL);
}

/**
 * @brief Blits a region of this surface on a specified location of another surface.
 * @param src_position the subrectangle of this surface to pick
 * @param dst_surface the destination surface
 * @param dst_position the destination position where the current surface will be blitted on dst
 */
void Surface::draw_region(const Rectangle &src_position, Surface& dst_surface,
    const Rectangle &dst_position) {

  Rectangle src_position2(src_position);
  Rectangle dst_position2(dst_position);
  SDL_BlitSurface(internal_surface, src_position2.get_internal_rect(),
      dst_surface.internal_surface, dst_position2.get_internal_rect());
}

/**
 * @brief Returns the SDL surface encapsulated by this object.
 *
 * This method should be used only by low-level classes.
 *
 * @return the SDL surface encapsulated
 */
SDL_Surface* Surface::get_internal_surface() {
  return internal_surface;
}

/**
 * @brief Returns the mapped 32bits pixel from internal SDL_PixelFormat to dst_format.
 *
 * This method should be used only by low-level classes, and after lock source internal_surface.
 *
 * @param idx_pixel the index of the pixel to convert
 * @param dst_format the destination format
 * @return the mapped 32bits pixel
 */
uint32_t Surface::get_mapped_pixel(int idx_pixel, SDL_PixelFormat* dst_format) {

  uint8_t r, g, b, a;
  SDL_GetRGBA(((uint32_t*) internal_surface->pixels)[idx_pixel], internal_surface->format, &r, &g, &b, &a);
  return SDL_MapRGBA(dst_format, r, g, b, a);
}

/**
 * @brief Returns the name identifying this type in Lua.
 * @return the name identifying this type in Lua
 */
const std::string& Surface::get_lua_type_name() const {
  return LuaContext::surface_module_name;
}

