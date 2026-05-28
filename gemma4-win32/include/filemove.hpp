#ifndef FILEMOVE_HPP
#define FILEMOVE_HPP

#include <string>
#include <vector>
#include <optional>
#include <filesystem>
#include <stdexcept>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <queue>
#include "logger.hpp"

namespace FileMove {

enum class DebugMode {
    None,
    MV,
    CP
};

enum class SortMode {
    MostRecentlyUsed,
    LeastRecentlyUsed,
    AddedFirst,
    AddedLast,
    AtoZ,
    ZtoA
};

enum class PlacementMode {
    UpperLeft,
    UpperRight,
    LowerLeft,
    LowerRight,
    LastLocation
};

enum class ShutdownMode {
    None,
    FinishCurrent,
    Immediate
};

enum class MoveStatus {
    Queued,
    Processing,
    Completed,
    Failed,
    Canceled
};

class ParseException : public std::runtime_error {
public:
    explicit ParseException(const std::string& message) : std::runtime_error(message) {}
};

struct PendingMoveEntry {
    std::string id;
    std::string groupId;
    std::filesystem::path sourceFilePath;
    std::filesystem::path relativePath; // Path relative to the original source root (used for directory expansion)
    std::vector<std::filesystem::path> destinationDirectories;
    std::filesystem::path activeLogFilePath;
    DebugMode debugTransferMode = DebugMode::MV;
    std::string queuedAt;
    std::string releaseBatchId;
    MoveStatus status = MoveStatus::Queued;
};

struct Group {
    std::string id;
    std::string name;
    std::vector<std::filesystem::path> destinationPaths;
    std::string defaultAction;
    std::string createdAt;
    std::string updatedAt;
    std::string lastUsedAt;
};

struct Settings {
    int version = 1;
    std::optional<std::string> lastSelectedGroupId;
    SortMode sortMode = SortMode::MostRecentlyUsed;
    PlacementMode placementMode = PlacementMode::UpperLeft;
    int windowWidth = 400;
    int windowHeight = 500;
    int windowLeft = 100;
    int windowTop = 100;

    // v1.1.0 Options
    bool enableDirectoryMoves = false;
    bool enableSidecarFiles = false;
    bool hideQueuedSourceFiles = false;

    std::vector<Group> groups;
};

struct CommandLineOptions {
    DebugMode debugMode = DebugMode::None;
    std::filesystem::path inputJsonPath;
    std::filesystem::path outputLogPath;
    SortMode sortMode = SortMode::MostRecentlyUsed;
    PlacementMode placementMode = PlacementMode::UpperLeft;
};

class CommandLineParser {
public:
    static CommandLineOptions parse(int argc, char* argv[]);
};

class MoveWorker {
public:
    MoveWorker();
    ~MoveWorker();

    void Start();
    void Stop();
    void RequestShutdown(ShutdownMode mode);
    std::optional<std::vector<PendingMoveEntry>> PrepareBatch(const std::vector<PendingMoveEntry>& batch);
    void QueueBatch(std::vector<PendingMoveEntry> batch);
    void Pause();
    void Resume();

    size_t GetQueuedCount() const;
    std::vector<PendingMoveEntry> GetQueuedItems() const;
    size_t GetProcessedCount() const;
    std::string GetWorkerState() const;

private:
    void WorkerLoop();
    void ProcessBatch(const std::vector<PendingMoveEntry>& batch);
    void ProcessEntry(const PendingMoveEntry& entry);

    std::atomic<bool> m_running{false};
    std::atomic<bool> m_paused{false};
    std::atomic<ShutdownMode> m_shutdownMode{ShutdownMode::None};
    std::atomic<size_t> m_queuedCount{0};
    std::atomic<size_t> m_processedCount{0};
    std::string m_workerState = "Idle";

    std::queue<std::vector<PendingMoveEntry>> m_batchQueue;
    mutable std::mutex m_queueMutex;
    std::condition_variable m_cv;
    std::thread m_workerThread;
};

} // namespace FileMove

#endif // FILEMOVE_HPP

