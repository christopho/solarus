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
#include "menus/LanguageScreen.h"
#include "lowlevel/Sound.h"
#include "lowlevel/FileTools.h"
#include "lowlevel/TextSurface.h"
#include "lowlevel/InputEvent.h"
#include "lowlevel/IniFile.h"
#include "MainLoop.h"
#include "CustomScreen.h"
#include "Transition.h"

const int LanguageScreen::max_visible_languages = 10;

/**
 * @brief Creates a language screen.
 */
LanguageScreen::LanguageScreen(MainLoop& main_loop):
  Screen(main_loop),
  transition(NULL),
  intermediate_surface(320, 240),
  language_codes(NULL),
  language_texts(NULL),
  cursor_position(0),
  nb_languages(0),
  finished(false) {

  if (FileTools::get_language().size() != 0) {
    // a language is already set: skip this screen
    finished = true;
  }
  else {
    std::map<std::string, std::string> language_map = FileTools::get_languages();
    nb_languages = language_map.size();
    first_visible_language = 0;
    nb_visible_languages = std::min(nb_languages, max_visible_languages);
    language_texts = new TextSurface*[nb_languages];
    language_codes = new std::string[nb_languages];
    int cursor_position = 0;
    int i = 0;

    std::map<std::string, std::string>::iterator it;
    for (it = language_map.begin(); it != language_map.end(); it++) {
      language_codes[i] = it->first;
      language_texts[i] = new TextSurface(160, 0, TextSurface::ALIGN_CENTER, TextSurface::ALIGN_MIDDLE);
      language_texts[i]->set_font("fixed");
      language_texts[i]->set_text(it->second);
      if (language_codes[i] == FileTools::get_default_language()) {
        cursor_position = i;
      }
      i++;
    }
    set_cursor_position(cursor_position);
  }

  if (nb_languages == 1) {
    // no choice: skip the language screen
    FileTools::set_language(language_codes[0]);
    finished = true;
  }
}

/**
 * @brief Destroys the language screen.
 */
LanguageScreen::~LanguageScreen() {

  delete transition;
  delete[] language_codes;
  for (int i = 0; i < nb_languages; i++) {
    delete language_texts[i];
  }
  delete[] language_texts;
}

/**
 * @brief Sets the position of the cursor.
 * @param cursor_position
 */
void LanguageScreen::set_cursor_position(int cursor_position) {

  language_texts[this->cursor_position]->set_text_color(Color::get_white());
  language_texts[cursor_position]->set_text_color(Color::get_yellow());

  if (cursor_position < first_visible_language) {
    first_visible_language = cursor_position;
  }
  if (cursor_position >= first_visible_language + max_visible_languages) {
    first_visible_language = cursor_position - max_visible_languages + 1;
  }

  int y = 120 - 8 * nb_visible_languages;
  for (int i = first_visible_language; i < first_visible_language + nb_visible_languages; i++) {
    language_texts[i]->set_y(y);
    y += 16;
  }

  this->cursor_position = cursor_position;
}

/**
 * @brief Updates this screen.
 */
void LanguageScreen::update() {

  if (finished) {
    start_next_screen();
  }

  if (transition != NULL) {
    transition->update();
    if (transition->is_finished()) {
      delete transition;
      transition = NULL;
      finished = true;
    }
  }
}

/**
 * @brief Displays this screen.
 * @param dst_surface the surface to draw
 */
void LanguageScreen::display(Surface& dst_surface) {

  intermediate_surface.fill_with_color(Color::get_black());

  for (int i = first_visible_language; i < first_visible_language + nb_visible_languages; i++) {
    language_texts[i]->display(intermediate_surface);
  }

  if (transition != NULL) {
    transition->display(intermediate_surface);
  }

  intermediate_surface.display(dst_surface);
}

/**
 * @brief This function is called by the main loop when there is an input event.
 * @param event the event to handle
 */
void LanguageScreen::notify_input(InputEvent& event) {

  static const InputEvent::KeyboardKey validation_keys[] = { InputEvent::KEY_SPACE, InputEvent::KEY_RETURN, InputEvent::KEY_NONE };

  if (transition == NULL) {

    if (event.is_direction_pressed()) {

      int direction = event.get_direction();
      if (direction == 2) {
	// up
	set_cursor_position((cursor_position - 1 + nb_languages) % nb_languages);
	Sound::play("cursor");
      }
      else if (direction == 6) {
	// down
	set_cursor_position((cursor_position + 1) % nb_languages);
	Sound::play("cursor");
      }
    }
    else if (event.is_keyboard_key_pressed(validation_keys)
	|| event.is_joypad_button_pressed()) {

	FileTools::set_language(language_codes[cursor_position]);
	transition = Transition::create(Transition::FADE, Transition::OUT);
	transition->start();
    }
  }
}

/**
 * @brief Ends the language screen and starts the first screen of the quest.
 */
void LanguageScreen::start_next_screen() {

  /* TODO write some compatibility here, not sure how to do that..
      IniFile ini("quest.dat", IniFile::READ);
      ini.set_group("info");
      const std::string& screen_name = ini.get_string_value("first_screen");
      MainLoop& main_loop = get_main_loop();
      main_loop.set_next_screen(new CustomScreen(main_loop, screen_name));
  */
}
