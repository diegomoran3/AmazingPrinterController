#pragma once

#include <glaze/glaze.hpp>
#include <string>
#include <fstream>
#include <filesystem>

class JsonHandler {
public:
    JsonHandler(const std::string& path = ".config/AmazingPrinterController/.settings.json");

    template <typename T>
    inline bool save(const T &settings)
    {
    try {
        std::filesystem::path p(m_path);

        // 1. Check for the directory, not the file.
        // If the path is "home/diego/.config/App/settings.json", 
        // this creates "home/diego/.config/App/"
        if (p.has_parent_path()) {
            std::error_code ec;
            std::filesystem::create_directories(p.parent_path(), ec);
            if (ec) return false; // Failed to create directory (Permissions?)
        }

        // 2. Let Glaze handle the file creation. 
        // write_file_json creates the file if it doesn't exist 
        // and overwrites it if it does.
        auto ecc = glz::write_file_json(settings, m_path, std::string{});
        
        return !static_cast<bool>(ecc);
    } catch (...) {
        return false;
    }
    }

    template <typename T>
    inline bool load(T &settings)
    {
        if (!std::filesystem::exists(m_path)) {
            return false;
        }

        auto ec = glz::read_file_json(settings, m_path, std::string{});
        
        if (ec) {
            return false;
        }
        return true;
    }

private:
    std::string m_path;
};
