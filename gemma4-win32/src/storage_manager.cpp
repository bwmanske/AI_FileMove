#include "storage_manager.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>
#include <iostream>

namespace FileMove {

using json = nlohmann::json;

// Serialization for Group
void to_json(json& j, const Group& g) {
    j = json{
        {"id", g.id},
        {"name", g.name},
        {"destinationPaths", g.destinationPaths},
        {"defaultAction", g.defaultAction},
        {"createdAt", g.createdAt},
        {"updatedAt", g.updatedAt},
        {"lastUsedAt", g.lastUsedAt}
    };
}

void from_json(const json& j, Group& g) {
    j.at("id").get_to(g.id);
    j.at("name").get_to(g.name);
    // Handle potential legacy single DestinationPath string/array
    if (j.contains("destinationPaths")) {
        j.at("destinationPaths").get_to(g.destinationPaths);
    } else if (j.contains("DestinationPath")) {
        std::string path = j.at("DestinationPath").get<std::string>();
        g.destinationPaths = { std::filesystem::path(path) };
    }

    j.at("defaultAction").get_to(g.defaultAction);
    j.at("createdAt").get_to(g.createdAt);
    j.at("updatedAt").get_to(g.updatedAt);
    j.at("lastUsedAt").get_to(g.lastUsedAt);
}

// Serialization for Settings
void to_json(json& j, const Settings& s) {
    j = json{
        {"version", s.version},
        {"lastSelectedGroupId", s.lastSelectedGroupId ? *s.lastSelectedGroupId : ""},
        {"sortMode", static_cast<int>(s.sortMode)},
        {"placementMode", static_cast<int>(s.placementMode)},
        {"windowWidth", s.windowWidth},
        {"windowHeight", s.windowHeight},
        {"windowLeft", s.windowLeft},
        {"windowTop", s.windowTop},
        {"enableDirectoryMoves", s.enableDirectoryMoves},
        {"enableSidecarFiles", s.enableSidecarFiles},
        {"hideQueuedSourceFiles", s.hideQueuedSourceFiles},
        {"groups", s.groups}
    };
}

void from_json(const json& j, Settings& s) {
    j.at("version").get_to(s.version);
    if (j.contains("lastSelectedGroupId") && !j.at("lastSelectedGroupId").get<std::string>().empty()) {
        s.lastSelectedGroupId = j.at("lastSelectedGroupId").get<std::string>();
    }
    s.sortMode = static_cast<SortMode>(j.at("sortMode").get<int>());
    s.placementMode = static_cast<PlacementMode>(j.at("placementMode").get<int>());
    j.at("windowWidth").get_to(s.windowWidth);
    j.at("windowHeight").get_to(s.windowHeight);
    j.at("windowLeft").get_to(s.windowLeft);
    j.at("windowTop").get_to(s.windowTop);

    // v1.1.0 options with defaults for backward compatibility
    if (j.contains("enableDirectoryMoves")) j.at("enableDirectoryMoves").get_to(s.enableDirectoryMoves);
    if (j.contains("enableSidecarFiles")) j.at("enableSidecarFiles").get_to(s.enableSidecarFiles);
    if (j.contains("hideQueuedSourceFiles")) j.at("hideQueuedSourceFiles").get_to(s.hideQueuedSourceFiles);

    j.at("groups").get_to(s.groups);
}

std::filesystem::path StorageManager::getDefaultDataDirectory() {
    // In a real app, use %AppData%\Roaming\FileMove
#ifdef _WIN32
    const char* appData = std::getenv("APPDATA");
    if (appData) return std::filesystem::path(appData) / "FileMove";
#endif
    return std::filesystem::current_path() / "data";
}

Settings StorageManager::loadSettings(const std::filesystem::path& path) {
    if (!std::filesystem::exists(path)) {
        return Settings{};
    }

    try {
        std::ifstream file(path);
        json j;
        file >> j;
        return j.get<Settings>();
    } catch (const std::exception& e) {
        std::cerr << "Error loading settings: " << e.what() << std::endl;
        return Settings{};
    }
}

void StorageManager::saveSettings(const std::filesystem::path& path, const Settings& settings) {
    try {
        std::filesystem::create_directories(path.parent_path());
        std::ofstream file(path);
        json j = settings;
        file << j.dump(4);
    } catch (const std::exception& e) {
        std::cerr << "Error saving settings: " << e.what() << std::endl;
    }
}

} // namespace FileMove
