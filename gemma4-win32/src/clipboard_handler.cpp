#include "clipboard_handler.hpp"
#include <chrono>

namespace FileMove {

std::vector<PendingMoveEntry> ClipboardHandler::CreateEntriesFromPaths(
    const std::vector<std::filesystem::path>& paths,
    const Group& targetGroup,
    DebugMode debugMode,
    const std::string& timestamp,
    const std::string& idPrefix
) {
    std::vector<PendingMoveEntry> batch;
    for (size_t i = 0; i < paths.size(); ++i) {
        PendingMoveEntry entry;
        entry.id = idPrefix + "-" + std::to_string(i);
        entry.groupId = targetGroup.id;
        entry.sourceFilePath = paths[i];
        entry.relativePath = paths[i].filename();
        entry.destinationDirectories = targetGroup.destinationPaths;
        entry.debugTransferMode = debugMode;
        entry.queuedAt = timestamp;
        entry.status = MoveStatus::Queued;

        batch.push_back(entry);
    }
    return batch;
}

} // namespace FileMove
