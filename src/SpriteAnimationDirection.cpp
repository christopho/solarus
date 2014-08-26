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
#include "SpriteAnimationDirection.h"
#include "lowlevel/PixelBits.h"
#include "lowlevel/Surface.h"
#include "lowlevel/Debug.h"
#include <sstream>

namespace solarus {

/**
 * \brief Constructor.
 * \param frames Position of each frame of the sequence in the image.
 * \param origin Coordinates of the sprite's origin point.
 */
SpriteAnimationDirection::SpriteAnimationDirection(
    const std::vector<Rectangle>& frames,
    const Rectangle& origin):
  frames(frames),
  origin(origin) {

  Debug::check_assertion(!frames.empty(), "Empty sprite direction");
}

/**
 * \brief Destructor.
 */
SpriteAnimationDirection::~SpriteAnimationDirection() {

  for (PixelBits* frame_pixel_bits: pixel_bits) {
    delete frame_pixel_bits;
  }
}

/**
 * \brief Returns the size of a frame.
 * \return The size of a frame.
 */
Rectangle SpriteAnimationDirection::get_size() const {

  Debug::check_assertion(get_nb_frames() > 0, "Invalid number of frames");
  return Rectangle(0, 0, frames[0].get_width(), frames[0].get_height());
}

/**
 * \brief Returns the number of frames in this direction.
 * \return the number of frames
 */
int SpriteAnimationDirection::get_nb_frames() const {
  return frames.size();
}

/**
 * \brief Returns the rectangle representing the specified frame on the source image.
 * \param frame a frame number
 * \return the rectangle of this frame
 */
const Rectangle& SpriteAnimationDirection::get_frame(int frame) const {

  if (frame < 0 || frame >= get_nb_frames()) {
    std::ostringstream oss;
    oss << "Invalid frame " << frame
        << ": this direction has " << get_nb_frames() << " frames";
    Debug::die(oss.str());
  }
  return frames[frame];
}

/**
 * \brief Draws a specific frame on the map.
 * \param dst_surface the surface on which the frame will be drawn
 * \param dst_position coordinates on the destination surface
 * (the origin point will be drawn at this position)
 * \param current_frame the frame to show
 * \param src_image the image from which the frame is extracted
 */
void SpriteAnimationDirection::draw(Surface& dst_surface,
    const Rectangle& dst_position, int current_frame, Surface& src_image) {

  const Rectangle& current_frame_rect = get_frame(current_frame);

  // Position of the sprite's upper left corner.
  Rectangle position_top_left(dst_position);
  position_top_left.add_xy(-origin.get_x(), -origin.get_y());
  position_top_left.set_size(current_frame_rect);

  src_image.draw_region(current_frame_rect, dst_surface, position_top_left);
}

/**
 * \brief Calculates the bit fields representing the non-transparent pixels
 * of the images in this direction.
 *
 * This method has to be called if you want a sprite having this animations
 * to be able to detect pixel-perfect collisions.
 * If the pixel-perfect collisions are already enabled, this function does nothing.
 *
 * \param src_image the surface containing the animations
 */
void SpriteAnimationDirection::enable_pixel_collisions(Surface* src_image) {

  if (!are_pixel_collisions_enabled()) {
    for (int i = 0; i < get_nb_frames(); i++) {
      pixel_bits.push_back(new PixelBits(*src_image, frames[i]));
    }
  }
}

/**
 * \brief Disables the pixel-perfect collision ability of this sprite animation direction.
 */
void SpriteAnimationDirection::disable_pixel_collisions() {

  for (PixelBits* frame_pixel_bits: pixel_bits) {
    delete frame_pixel_bits;
  }
  pixel_bits.clear();
}

/**
 * \brief Returns whether the pixel-perfect collisions are enabled for this direction.
 * \return true if the pixel-perfect collisions are enabled
 */
bool SpriteAnimationDirection::are_pixel_collisions_enabled() const {
  return !pixel_bits.empty();
}

}

