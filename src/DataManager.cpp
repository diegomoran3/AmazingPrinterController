#include "DataManager.hpp"
#include <iostream>

namespace fs = std::filesystem;

DataManager::DataManager(const std::string& filename) : m_file_path(filename) {
    // Optionally auto-load on creation
    load();
}

void DataManager::ensure_directory_exists() {
    fs::path p(m_file_path);
    if (p.has_parent_path()) {
        fs::create_directories(p.parent_path());
    }
}

bool DataManager::load() {
    return false;
}

bool DataManager::save() {
    return false;

}