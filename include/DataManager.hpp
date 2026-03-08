#pragma once

#include <string>
#include <filesystem>
#include <fstream>
#include <iostream>

class DataManager {
public:
    explicit DataManager(const std::string& filename);

    bool load(std::string& out_data);

    bool save(const std::string& data);

private:
    std::filesystem::path m_file_path;
    void ensure_directory_exists();
};