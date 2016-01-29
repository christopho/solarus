/*
 * Copyright (C) 2006-2016 Christopho, Solarus - http://www.solarus-games.org
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
#include "solarus/lowlevel/Debug.h"
#include "solarus/lowlevel/Logger.h"
#include "solarus/lowlevel/QuestFiles.h"
#include "solarus/lua/LuaContext.h"
#include "solarus/Arguments.h"
#include "solarus/CurrentQuest.h"
#include <physfs.h>
#include <fstream>
#include <cstdlib>  // exit(), mkstemp(), tmpnam()
#include <cstdio>   // remove()
#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif

#if defined(SOLARUS_OSX) || defined(SOLARUS_IOS)
#   include "solarus/lowlevel/apple/AppleInterface.h"
#endif

namespace Solarus {

std::string QuestFiles::quest_path;
std::string QuestFiles::solarus_write_dir;
std::string QuestFiles::quest_write_dir;
std::vector<std::string> QuestFiles::temporary_files;

/**
 * \brief Initializes the file tools.
 * \param args Command-line arguments.
 */
void QuestFiles::initialize(const Arguments& args) {

  const std::string& program_name = args.get_program_name().c_str();
  if (program_name.empty()) {
    PHYSFS_init(nullptr);
  }
  else {
    PHYSFS_init(program_name.c_str());
  }

  // Set the quest path, by default as defined during the build process.
  quest_path = SOLARUS_DEFAULT_QUEST;

  // If a quest command-line argument was specified, use it instead.
  const std::vector<std::string>& options = args.get_arguments();
  if (!options.empty()
      && !options.back().empty()
      && options.back()[0] != '-') {
    // The last parameter is not an option: it is the quest path.
    quest_path = options.back();
  }

  Logger::info("Opening quest '" + quest_path + "'");

  // Now, quest_path may be the path defined as command-line argument,
  // the path defined during the build process, or the current directory
  // if nothing was specified.

  std::string dir_quest_path = quest_path + "/data";
  std::string archive_quest_path_1 = quest_path + "/data.solarus";
  std::string archive_quest_path_2 = quest_path + "/data.solarus.zip";

  const std::string& base_dir = PHYSFS_getBaseDir();
  PHYSFS_addToSearchPath(dir_quest_path.c_str(), 1);   // data directory
  PHYSFS_addToSearchPath(archive_quest_path_1.c_str(), 1); // data.solarus archive
  PHYSFS_addToSearchPath(archive_quest_path_2.c_str(), 1); // data.solarus.zip archive
  PHYSFS_addToSearchPath((base_dir + "/" + dir_quest_path).c_str(), 1);
  PHYSFS_addToSearchPath((base_dir + "/" + archive_quest_path_1).c_str(), 1);
  PHYSFS_addToSearchPath((base_dir + "/" + archive_quest_path_2).c_str(), 1);

  // Check the existence of a quest at this location.
  if (!QuestFiles::data_file_exists("quest.dat")) {
    Debug::error("No quest was found in the directory '" + quest_path +
        "'. To specify your quest's path, run: " +
        (program_name.empty() ? std::string("solarus-run") : program_name) +
        " path/to/quest");
    std::exit(EXIT_SUCCESS);
  }

  // Set the engine root write directory.
  set_solarus_write_dir(SOLARUS_WRITE_DIR);
}

/**
 * \brief Quits the file tools.
 */
void QuestFiles::quit() {

  remove_temporary_files();

  quest_path = "";
  solarus_write_dir = "";
  quest_write_dir = "";

  PHYSFS_deinit();
}

/**
 * \brief Returns the path of the quest, relative to the current directory.
 * \return Path of the data/ directory, the data.solarus archive or the
 * data.solarus.zip archive, relative to the current directory.
 */
const std::string& QuestFiles::get_quest_path() {
  return quest_path;
}

/**
 * \brief Returns the physical location of a data file.
 *
 * This function is not often necessary since the whole point of this class
 * is to access files independently of their real file location
 * (in the quest write directory, in the data directory or in the data archive).
 *
 * \param file_name The file to look for.
 * \return Where it is actually located, or LOCATION_NONE if not found.
 */
QuestFiles::DataFileLocation QuestFiles::data_file_get_location(
    const std::string& file_name) {

  const char* path_ptr = PHYSFS_getRealDir(file_name.c_str());
  std::string path = path_ptr == nullptr ? "" : path_ptr;
  if (path.empty()) {
    // File does not exist.
    return LOCATION_NONE;
  }

  if (!get_quest_write_dir().empty() && path == PHYSFS_getWriteDir()) {
    return LOCATION_WRITE_DIRECTORY;
  }

  if (path.rfind("data") == path.size() - 4) {
    return LOCATION_DATA_DIRECTORY;
  }

  if (path.rfind("data.solarus") == path.size() - 12
      || path.rfind("data.solarus.zip") == path.size() - 16) {
    return LOCATION_DATA_ARCHIVE;
  }

  Debug::die(std::string("Unexpected search path element: " + path));
}

/**
 * \brief Returns whether a file exists in the quest data directory or
 * in Solarus write directory.
 * \param file_name A file name relative to the quest data directory,
 * to the current language directory or to Solarus write directory.
 * \param language_specific true if the file is relative to the current
 * language directory.
 * \return true if this file exists.
 */
bool QuestFiles::data_file_exists(const std::string& file_name,
    bool language_specific) {

  std::string full_file_name;
  if (language_specific) {
    if (CurrentQuest::get_language().empty()) {
      return false;
    }
    full_file_name = std::string("languages/") +
        CurrentQuest::get_language() + "/" + file_name;
  }
  else {
    full_file_name = file_name;
  }
  return PHYSFS_exists(full_file_name.c_str());
}

/**
 * \brief Opens a data file an loads its content into memory.
 * \param file_name Name of the file to open.
 * \param language_specific \c true if the file is specific to the current language.
 * \return The content of the file.
 */
std::string QuestFiles::data_file_read(
    const std::string& file_name,
    bool language_specific
) {
  std::string full_file_name;
  if (language_specific) {
    Debug::check_assertion(!CurrentQuest::get_language().empty(),
        std::string("Cannot open language-specific file '") + file_name
        + "': no language was set"
    );
    full_file_name = std::string("languages/") +
        CurrentQuest::get_language() + "/" + file_name;
  }
  else {
    full_file_name = file_name;
  }

  // open the file
  Debug::check_assertion(PHYSFS_exists(full_file_name.c_str()),
      std::string("Data file '") + full_file_name + "' does not exist"
  );
  PHYSFS_file* file = PHYSFS_openRead(full_file_name.c_str());
  Debug::check_assertion(file != nullptr,
      std::string("Cannot open data file '") + full_file_name + "'"
  );

  // load it into memory
  size_t size =  static_cast<size_t>(PHYSFS_fileLength(file));
  std::vector<char> buffer(size);

  PHYSFS_read(file, buffer.data(), 1, (PHYSFS_uint32) size);
  PHYSFS_close(file);

  return std::string(buffer.data(), size);
}

/**
 * \brief Saves a buffer into a data file.
 * \param file_name Name of the file to write, relative to Solarus write directory.
 * \param buffer The buffer to save.
 *
 */
void QuestFiles::data_file_save(
    const std::string& file_name,
    const std::string& buffer
) {
  // open the file to write
  PHYSFS_file* file = PHYSFS_openWrite(file_name.c_str());
  if (file == nullptr) {
    Debug::die(std::string("Cannot open file '") + file_name
        + "' for writing: " + PHYSFS_getLastError()
    );
  }

  // save the memory buffer
  if (PHYSFS_write(file, buffer.data(), (PHYSFS_uint32) buffer.size(), 1) == -1) {
    Debug::die(std::string("Cannot write file '") + file_name + "': "
        + PHYSFS_getLastError());
  }
  PHYSFS_close(file);
}

/**
 * \brief Removes a file from the write directory.
 * \param file_name Name of the file to delete, relative to the Solarus
 * write directory.
 * \return \c true in case of success.
 */
bool QuestFiles::data_file_delete(const std::string& file_name) {

  if (!PHYSFS_delete(file_name.c_str())) {
    return false;
  }

  return true;
}

/**
 * \brief Creates a directory in the write directory.
 * \param dir_name Name of the directory to delete, relative to the Solarus
 * write directory.
 * \return \c true in case of success.
 */
bool QuestFiles::data_file_mkdir(const std::string& dir_name) {

  if (!PHYSFS_mkdir(dir_name.c_str())) {
    return false;
  }

  return true;
}

/**
 * \brief Enumerate files of a directory.
 *
 * Symbolic links are never returned.
 *
 * \param dir_path Name of the directory to list, relative to the quest data
 * directory.
 * \param list_files Whether regular files should be included in the result.
 * \param list_directories Whether directories should be included in the result.
 * \return The files in this directory.
 */
std::vector<std::string> QuestFiles::data_files_enumerate(
    const std::string& dir_path,
    bool list_files,
    bool list_directories
) {

  std::vector<std::string> result;

  if (data_file_exists(dir_path)) {
    char** files = PHYSFS_enumerateFiles(dir_path.c_str());

    for (char** file = files; *file != nullptr; file++) {
      bool is_directory = PHYSFS_isDirectory((dir_path + "/" + *file).c_str());

      if (!PHYSFS_isSymbolicLink(*file)
          && ((list_files && !is_directory)
              || (list_directories && is_directory)))
        result.push_back(std::string(*file));
    }

    PHYSFS_freeList(files);
  }

  return result;
}

/**
 * \brief Returns the directory where the engine can write files.
 * \returns The directory where the engine can write files, relative to the
 * base write directory.
 */
const std::string& QuestFiles::get_solarus_write_dir() {
  return solarus_write_dir;
}

/**
 * \brief Sets the directory where the engine can write files.
 *
 * Initially, this directory is set to the preprocessor constant
 * SOLARUS_WRITE_DIR (by default ".solarus").
 * You normally don't need to change this, it should have been set correctly
 * at compilation time to a value that depends on the target system.
 *
 * \param solarus_write_dir The directory where the engine can write files,
 * relative to the base write directory.
 */
void QuestFiles::set_solarus_write_dir(const std::string& solarus_write_dir) {

  // This setting never changes at runtime.
  // Allowing to change it would be complex and we don't need that.
  Debug::check_assertion(QuestFiles::solarus_write_dir.empty(),
      "The Solarus write directory is already set");

  QuestFiles::solarus_write_dir = solarus_write_dir;

  // First check that we can write in a directory.
  if (!PHYSFS_setWriteDir(get_base_write_dir().c_str())) {
     Debug::die(std::string("Cannot write in user directory '")
         + get_base_write_dir().c_str() + "': " + PHYSFS_getLastError());
  }

  // Create the directory.
  PHYSFS_mkdir(solarus_write_dir.c_str());

  const std::string& full_write_dir = get_base_write_dir() + "/" + solarus_write_dir;
  if (!PHYSFS_setWriteDir(full_write_dir.c_str())) {
    Debug::die(std::string("Cannot set Solarus write directory to '")
        + full_write_dir + "': " + PHYSFS_getLastError());
  }

  // The quest subdirectory may be new, create it if needed.
  if (!quest_write_dir.empty()) {
    set_quest_write_dir(quest_write_dir);
  }
}

/**
 * \brief Returns the subdirectory where files specific to the quest are
 * saved, like savegames and configuration files.
 * \return The quest write directory, relative to the Solarus write directory,
 * or an empty string if it has not been set yet.
 */
const std::string& QuestFiles::get_quest_write_dir() {
  return quest_write_dir;
}

/**
 * \brief Sets the subdirectory where files specific to the quest are
 * saved, like savegames and configuration files.
 *
 * You have to call this function before loading or saving savegames and
 * configuration files.
 * This directory should typically be named like your quest, to be sure other
 * quests will not interfere.
 *
 * \param quest_write_dir The quest write directory, relative to the Solarus
 * write directory.
 */
void QuestFiles::set_quest_write_dir(const std::string& quest_write_dir) {

  if (!QuestFiles::quest_write_dir.empty()) {
    // There was already a previous quest subdirectory: remove it from the
    // search path.
    PHYSFS_removeFromSearchPath(PHYSFS_getWriteDir());
  }

  QuestFiles::quest_write_dir = quest_write_dir;

  // Reset the write directory to the Solarus directory
  // so that we can create the new quest subdirectory.
  std::string full_write_dir = get_base_write_dir() + "/" + solarus_write_dir;
  if (!PHYSFS_setWriteDir(full_write_dir.c_str())) {
    Debug::die(std::string("Cannot set Solarus write directory to '")
        + full_write_dir + "': " + PHYSFS_getLastError());
  }

  if (!quest_write_dir.empty()) {
    // Create the quest subdirectory (if not existing)
    // in the Solarus write directory.
    PHYSFS_mkdir(quest_write_dir.c_str());

    // Set the write directory to this new place.
    full_write_dir = get_base_write_dir() + "/" + solarus_write_dir + "/" + quest_write_dir;
    PHYSFS_setWriteDir(full_write_dir.c_str());

    // Also allow the quest to read savegames, settings and data files there.
    PHYSFS_addToSearchPath(PHYSFS_getWriteDir(), 0);
  }
}

/**
 * \brief Returns the absolute path of the quest write directory.
 */
std::string QuestFiles::get_full_quest_write_dir() {
  return get_base_write_dir() + "/" + get_solarus_write_dir() + "/" + get_quest_write_dir();
}

/**
 * \brief Returns the privileged base write directory, depending on the OS.
 * \return The base write directory.
 */
std::string QuestFiles::get_base_write_dir() {

#if defined(SOLARUS_OSX) || defined(SOLARUS_IOS)
  return std::string(get_user_application_support_directory());
#else
  return std::string(PHYSFS_getUserDir());
#endif
}

/**
 * \brief Creates a temporary file with the specified content and closes it.
 * \param content Content of the file to create.
 * \return Full name of the file created, or an empty string in case of failure.
 */
std::string QuestFiles::create_temporary_file(const std::string& content) {

  // Determine the name of our temporary file.
  std::string file_name;

#ifdef HAVE_MKSTEMP
  // mkstemp+close is safer than tmpname, but POSIX only.
  char name_template[] = "/tmp/solarus.XXXXXX";
  int file_descriptor = mkstemp(name_template);
  if (file_descriptor == -1) {
    // Failure.
    return "";
  }
  close(file_descriptor);
  file_name = name_template;
#else
  file_name = std::tmpnam(nullptr);
#endif

  std::ofstream out(file_name.c_str());

  if (!out) {
    return "";
  }

  // File successfully created.
  temporary_files.push_back(file_name);

  if (!content.empty()) {
    out.write(content.data(), content.size());
    if (!out) {
      file_name = "";
    }
  }

  out.close();

  return file_name;
}

/**
 * \brief Deletes all files previously created with create_temporary_file().
 * \return \c true in case of success, \c false if at least one file could not
 * be deleted.
 */
bool QuestFiles::remove_temporary_files() {

  bool success = true;
  for (const std::string& file_name: temporary_files) {
    if (std::remove(file_name.c_str()) != 0) {
      success = false;
    }
  }

  return success;
}

}

