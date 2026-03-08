#include "DataManager.hpp"

DataManager::DataManager(const std::string& filename) : m_file_path(filename) {
    // Optionally ensure the directory exists immediately upon instantiation
    ensure_directory_exists();
}

void DataManager::ensure_directory_exists() {
    if (m_file_path.has_parent_path()) {
        std::error_code ec;
        if (!std::filesystem::exists(m_file_path.parent_path())) {
            std::filesystem::create_directories(m_file_path.parent_path(), ec);
        }
    }
}

bool DataManager::save(const std::string& data) {
    try {
        ensure_directory_exists();
        
        // ios::trunc ensures the file is overwritten
        std::ofstream ofs(m_file_path, std::ios::out | std::ios::trunc);
        
        if (!ofs.is_open()) return false;

        ofs << data;
        ofs.close();
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

bool DataManager::load(std::string& out_data) {
    if (!std::filesystem::exists(m_file_path)) {
        return false;
    }

    std::ifstream ifs(m_file_path, std::ios::in);
    if (!ifs.is_open()) return false;

    // Efficiently read the entire file into the string
    std::stringstream buffer;
    buffer << ifs.rdbuf();
    out_data = buffer.str();
    
    ifs.close();
    return true;
}