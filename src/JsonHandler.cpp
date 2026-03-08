#include "JsonHandler.hpp"

JsonHandler::JsonHandler(const std::string& path) 
    : m_path(path)
    {
    const char* home_env = std::getenv("HOME");
    if (home_env) {
        std::filesystem::path config_dir = std::filesystem::path(home_env) / ".config" / "AmazingPrinterController";
        m_path = (config_dir / "settings.json").string();
    } else {
        m_path = "settings.json"; 
    }
}

