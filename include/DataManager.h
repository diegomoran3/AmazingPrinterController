#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

#include "glaze/glaze.hpp"
#include <string>
#include <filesystem>

// The data you want to persist
struct UserSettings {
    int last_score = 0;
    std::string username = "LinuxUser";
    bool fullscreen = false;
};

class DataManager {
public:
    // Constructor defines where to save
    DataManager(const std::string& filename);

    // Load data from disk into the settings object
    bool load();

    // Save current settings object to disk
    bool save();

    // Accessor for the data
    UserSettings settings;

private:
    std::string m_file_path;
    void ensure_directory_exists();
};

#endif