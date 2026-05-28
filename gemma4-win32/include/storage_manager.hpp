#ifndef STORAGE_MANAGER_HPP
#define STORAGE_MANAGER_HPP

#include <filesystem>
#include "filemove.hpp"

namespace FileMove {

class StorageManager {
public:
    static std::filesystem::path getDefaultDataDirectory();
    static Settings loadSettings(const std::filesystem::path& path);
    static void saveSettings(const std::filesystem::path& path, const Settings& settings);
};

} // namespace FileMove

#endif // STORAGE_MANAGER_HPP
