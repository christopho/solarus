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
#include <cstring>  // memcpy
#include <cmath>
#include <sstream>
#include "lowlevel/Sound.h"
#include "lowlevel/Music.h"
#include "lowlevel/FileTools.h"
#include "lowlevel/Debug.h"
#include "QuestResourceList.h"
#include "CommandLine.h"

namespace solarus {

ALCdevice* Sound::device = nullptr;
ALCcontext* Sound::context = nullptr;
bool Sound::initialized = false;
bool Sound::sounds_preloaded = false;
float Sound::volume = 1.0;
std::list<Sound*> Sound::current_sounds;
std::map<std::string, Sound> Sound::all_sounds;
ov_callbacks Sound::ogg_callbacks = {
    cb_read,
    nullptr,
    nullptr,
    nullptr
};

/**
 * \brief Creates a new Ogg Vorbis sound.
 * \param sound_id id of the sound: name of a .ogg file in the sounds subdirectory,
 * without the extension (.ogg is added automatically)
 */
Sound::Sound(const std::string& sound_id):
  id(sound_id),
  buffer(AL_NONE) {

}

/**
 * \brief Destroys the sound.
 */
Sound::~Sound() {

  if (is_initialized() && buffer != AL_NONE) {

    // stop the sources where this buffer is attached
    for (ALuint source: sources) {
      alSourceStop(source);
      alSourcei(source, AL_BUFFER, 0);
      alDeleteSources(1, &source);
    }
    alDeleteBuffers(1, &buffer);
    current_sounds.remove(this);
  }
}

/**
 * \brief Initializes the audio (music and sound) system.
 *
 * This method should be called when the application starts.
 * If the argument -no-audio is provided, this function has no effect and
 * there will be no sound.
 *
 * \param args Command-line arguments.
 */
void Sound::initialize(const CommandLine& args) {

  // Check the -no-audio option.
  const bool disable = args.has_argument("-no-audio");
  if (disable) {
    return;
  }

  // Initialize OpenAL.

  device = alcOpenDevice(nullptr);
  if (!device) {
    Debug::error("Cannot open audio device");
    return;
  }

  ALCint attr[] = { ALC_FREQUENCY, 32000, 0 }; // 32 KHz is the SPC output sampling rate
  context = alcCreateContext(device, attr);
  if (!context) {
    Debug::error("Cannot create audio context");
    alcCloseDevice(device);
    return;
  }
  if (!alcMakeContextCurrent(context)) {
    Debug::error("Cannot activate audio context");
    alcDestroyContext(context);
    alcCloseDevice(device);
    return;
  }

  alGenBuffers(0, AL_NONE);  // Necessary on some systems to avoid errors with the first sound loaded.

  initialized = true;
  set_volume(100);

  // initialize the music system
  Music::initialize();
}

/**
 * \brief Closes the audio (music and sound) system.
 *
 * This method should be called when exiting the application.
 */
void Sound::quit() {

  if (is_initialized()) {

    // uninitialize the music subsystem
    Music::quit();

    // clear the sounds
    all_sounds.clear();

    // uninitialize OpenAL

    alcMakeContextCurrent(nullptr);
    alcDestroyContext(context);
    context = nullptr;
    alcCloseDevice(device);
    device = nullptr;

    initialized = false;
  }
}

/**
 * \brief Returns whether the audio (music and sound) system is initialized.
 * \return true if the audio (music and sound) system is initilialized
 */
bool Sound::is_initialized() {
  return initialized;
}

/**
 * \brief Loads and decodes all sounds listed in the game database.
 */
void Sound::load_all() {

  if (is_initialized() && !sounds_preloaded) {

    const std::vector<QuestResourceList::Element>& sound_elements =
        QuestResourceList::get_elements(QuestResourceList::RESOURCE_SOUND);
    for (const auto& kvp: sound_elements) {
      const std::string& sound_id = kvp.first;

      all_sounds[sound_id] = Sound(sound_id);
      all_sounds[sound_id].load();
    }

    sounds_preloaded = true;
  }
}

/**
 * \brief Returns whether a sound exists.
 * \param sound_id id of the sound to test
 * \return true if the sound exists
 */
bool Sound::exists(const std::string& sound_id) {

  std::ostringstream oss;
  oss << "sounds/" << sound_id << ".ogg";
  return FileTools::data_file_exists(oss.str());
}

/**
 * \brief Starts playing the specified sound.
 * \param sound_id id of the sound to play
 */
void Sound::play(const std::string& sound_id) {

  if (all_sounds.find(sound_id) == all_sounds.end()) {
    all_sounds[sound_id] = Sound(sound_id);
  }

  all_sounds[sound_id].start();
}

/**
 * \brief Returns the current volume of sound effects.
 * \return the volume (0 to 100)
 */
int Sound::get_volume() {

  return (int) (volume * 100.0 + 0.5);
}

/**
 * \brief Sets the volume of sound effects.
 * \param volume the new volume (0 to 100)
 */
void Sound::set_volume(int volume) {

  volume = std::min(100, std::max(0, volume));
  Sound::volume = volume / 100.0;
}

/**
 * \brief Updates the audio (music and sound) system.
 *
 * This function is called repeatedly by the game.
 */
void Sound::update() {

  // update the playing sounds
  std::list<Sound*> sounds_to_remove;
  for (Sound* sound: current_sounds) {
    if (!sound->update_playing()) {
      sounds_to_remove.push_back(sound);
    }
  }

  for (Sound* sound: sounds_to_remove) {
    current_sounds.remove(sound);
  }

  // also update the music
  Music::update();
}

/**
 * \brief Updates this sound when it is playing.
 * \return true if the sound is still playing, false if it is finished.
 */
bool Sound::update_playing() {

  // See if this sound is still playing.
  const auto it = sources.begin();
  if (it == sources.end()) {
    return false;
  }

  ALuint source = *it;
  ALint status;
  alGetSourcei(source, AL_SOURCE_STATE, &status);

  if (status != AL_PLAYING) {
    sources.pop_front();
    alSourcei(source, AL_BUFFER, 0);
    alDeleteSources(1, &source);
  }

  return !sources.empty();
}

/**
 * \brief Loads and decodes the sound into memory.
 */
void Sound::load() {

  if (alGetError() != AL_NONE) {
    Debug::error("Previous audio error not cleaned");
  }

  std::string file_name = std::string("sounds/" + id);
  if (id.find(".") == std::string::npos) {
    file_name += ".ogg";
  }

  // Create an OpenAL buffer with the sound decoded by the library.
  buffer = decode_file(file_name);

  // buffer is now AL_NONE if there was an error.
}

/**
 * \brief Plays the sound.
 * \return true if the sound was loaded successfully, false otherwise
 */
bool Sound::start() {

  bool success = false;

  if (is_initialized()) {

    if (buffer == AL_NONE) { // first time: load and decode the file
      load();
    }

    if (buffer != AL_NONE) {

      // create a source
      ALuint source;
      alGenSources(1, &source);
      alSourcei(source, AL_BUFFER, buffer);
      alSourcef(source, AL_GAIN, volume);

      // play the sound
      int error = alGetError();
      if (error != AL_NO_ERROR) {
        std::ostringstream oss;
        oss << "Cannot attach buffer " << buffer
            << " to the source to play sound '" << id << "': error " << error;
        Debug::error(oss.str());
        alDeleteSources(1, &source);
      }
      else {
        sources.push_back(source);
        current_sounds.remove(this); // to avoid duplicates
        current_sounds.push_back(this);
        alSourcePlay(source);
        error = alGetError();
        if (error != AL_NO_ERROR) {
          std::ostringstream oss;
          oss << "Cannot play sound '" << id << "': error " << error;
          Debug::error(oss.str());
        }
        else {
          success = true;
        }
      }
    }
  }
  return success;
}

/**
 * \brief Loads the specified sound file and decodes its content into an OpenAL buffer.
 * \param file_name name of the file to open
 * \return the buffer created, or AL_NONE if the sound could not be loaded
 */
ALuint Sound::decode_file(const std::string& file_name) {

  ALuint buffer = AL_NONE;

  if (!FileTools::data_file_exists(file_name)) {
    Debug::error(std::string("Cannot find sound file '") + file_name + "'");
    return AL_NONE;
  }

  // load the sound file
  SoundFromMemory mem;
  mem.loop = false;
  mem.position = 0;
  FileTools::data_file_open_buffer(file_name, &mem.data, &mem.size);

  OggVorbis_File file;
  int error = ov_open_callbacks(&mem, &file, nullptr, 0, ogg_callbacks);

  if (error) {
    std::ostringstream oss;
    oss << "Cannot load sound file '" << file_name
        << "' from memory: error " << error;
    Debug::error(oss.str());
  }
  else {

    // read the encoded sound properties
    vorbis_info* info = ov_info(&file, -1);
    ALsizei sample_rate = ALsizei(info->rate);

    ALenum format = AL_NONE;
    if (info->channels == 1) {
      format = AL_FORMAT_MONO16;
    }
    else if (info->channels == 2) {
      format = AL_FORMAT_STEREO16;
    }

    if (format == AL_NONE) {
      Debug::error(std::string("Invalid audio format for sound file '")
          + file_name + "'");
    }
    else {
      // decode the sound with vorbisfile
      std::vector<char> samples;
      int bitstream;
      long bytes_read;
      long total_bytes_read = 0;
      const int buffer_size = 4096;
      char samples_buffer[buffer_size];
      do {
        bytes_read = ov_read(&file, samples_buffer, buffer_size, 0, 2, 1, &bitstream);
        if (bytes_read < 0) {
          std::ostringstream oss;
          oss << "Error while decoding ogg chunk in sound file '"
              << file_name << "': " << bytes_read;
          Debug::error(oss.str());
        }
        else {
          total_bytes_read += bytes_read;
          if (format == AL_FORMAT_STEREO16) {
            samples.insert(samples.end(), samples_buffer, samples_buffer + bytes_read);
          }
          else {
            // mono sound files make no sound on some machines
            // workaround: convert them on-the-fly into stereo sounds
            // TODO find a better solution
            for (int i = 0; i < bytes_read; i += 2) {
              samples.insert(samples.end(), samples_buffer + i, samples_buffer + i + 2);
              samples.insert(samples.end(), samples_buffer + i, samples_buffer + i + 2);
            }
            total_bytes_read += bytes_read;
          }
        }
      }
      while (bytes_read > 0);

      // copy the samples into an OpenAL buffer
      alGenBuffers(1, &buffer);
      if (alGetError() != AL_NO_ERROR) {
          Debug::error("Failed to generate audio buffer");
      }
      alBufferData(buffer,
          AL_FORMAT_STEREO16,
          reinterpret_cast<ALshort*>(&samples[0]),
          ALsizei(total_bytes_read),
          sample_rate);
      ALenum error = alGetError();
      if (error != AL_NO_ERROR) {
        std::ostringstream oss;
        oss << "Cannot copy the sound samples of '"
            << file_name << "' into buffer " << buffer
            << ": error " << error;
        Debug::error(oss.str());
        buffer = AL_NONE;
      }
    }
    ov_clear(&file);
  }

  FileTools::data_file_close_buffer(mem.data);

  return buffer;
}

/**
 * \brief Loads an encoded sound from memory.
 *
 * This function respects the prototype specified by libvorbisfile.
 *
 * \param ptr pointer to a buffer to load
 * \param size size
 * \param nb_bytes number of bytes to load
 * \param datasource source of the data to read
 * \return number of bytes loaded
 */
size_t Sound::cb_read(void* ptr, size_t /* size */, size_t nb_bytes, void* datasource) {

  SoundFromMemory* mem = (SoundFromMemory*) datasource;

  if (mem->position >= mem->size) {
    if (mem->loop) {
      mem->position = 0;
    }
    else {
      return 0;
    }
  }
  else if (mem->position + nb_bytes >= mem->size) {
    nb_bytes = mem->size - mem->position;
  }

  memcpy(ptr, mem->data + mem->position, nb_bytes);
  mem->position += nb_bytes;

  return nb_bytes;
}

}

