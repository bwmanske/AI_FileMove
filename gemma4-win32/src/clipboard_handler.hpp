#ifndef FILEMOVE_CLIPBOARD_HANDLER_HPP
#define FILEMOVE_CLIPBOARD_HANDLER_HPP

#include "filemove.hpp"
#include <vector>
#include <filesystem>
#include <string>

namespace FileMove {

class ClipboardHandler {
public:
    static std::vector<PendingMoveEntry> CreateEntriesFromPaths(
        const std::vector<std::filesystem::path>& paths,
        const Group& targetGroup,
        DebugMode debugMode,
        const std::string& timestamp,
        const std::string& idPrefix = "clip"
    );
};

} // namespace FileMove

#endif // FILEMOVE_CLIPBOARD_HANDLER_HPP
