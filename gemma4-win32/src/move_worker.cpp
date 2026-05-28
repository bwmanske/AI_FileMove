#include "filemove.hpp"
#include "logger.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <chrono>

#ifdef _WIN32
#include <commctrl.h>
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")
#endif

namespace FileMove {

// Helper to convert wstring to string (needed for groupName extraction)
static std::string ToString(const wchar_t* ws) {
    if (!ws) return "";
    std::wstring w(ws);
    return std::string(w.begin(), w.end());
}

MoveWorker::MoveWorker() {}

MoveWorker::~MoveWorker() {
    Stop();
}

void MoveWorker::Start() {
#ifdef _WIN32
    m_running = true;
    m_workerThread = std::thread(&MoveWorker::WorkerLoop, this);
#endif
}

void MoveWorker::Stop() {
#ifdef _WIN32
    m_running = false;
    m_shutdownMode = ShutdownMode::None;
    m_cv.notify_all();
    if (m_workerThread.joinable()) {
        m_workerThread.join();
    }
#endif
}

void MoveWorker::RequestShutdown(ShutdownMode mode) {
#ifdef _WIN32
    m_shutdownMode = mode;
    if (mode == ShutdownMode::Immediate) {
        m_running = false;
    }
    m_cv.notify_all();
#endif
}

std::optional<std::vector<PendingMoveEntry>> MoveWorker::PrepareBatch(const std::vector<PendingMoveEntry>& batch) {
    std::vector<PendingMoveEntry> preparedBatch;

    for (const auto& entry : batch) {
        if (std::filesystem::is_directory(entry.sourceFilePath)) {
            // Expand directory: for each file, create an entry with a relative path to preserve structure
            try {
                for (const auto& dir_entry : std::filesystem::recursive_directory_iterator(entry.sourceFilePath)) {
                    if (dir_entry.is_regular_file()) {
                        PendingMoveEntry expandedEntry = entry;
                        expandedEntry.sourceFilePath = dir_entry.path();
                        expandedEntry.relativePath = std::filesystem::relative(dir_entry.path(), entry.sourceFilePath);

                        // Validate this specific file and its destinations
                        if (!std::filesystem::exists(expandedEntry.sourceFilePath)) return std::nullopt;
                        for (const auto& dest : expandedEntry.destinationDirectories) {
                            if (!std::filesystem::exists(dest) || !std::filesystem::is_directory(dest)) return std::nullopt;
                        }

                        // Check conflict: source == destination (relative to base)
                        bool conflict = false;
                        for (const auto& dest : expandedEntry.destinationDirectories) {
                            if (expandedEntry.sourceFilePath == (dest / expandedEntry.relativePath)) {
                                conflict = true;
                                break;
                            }
                        }
                        if (conflict) return std::nullopt;

                        // Create sidecar
                        std::filesystem::path sidecar = expandedEntry.sourceFilePath.string() + ".filemove-queued";
                        {
                            std::ofstream sidecarFile(sidecar);
                            sidecarFile << "Queued at: " << expandedEntry.queuedAt << std::endl;
                        }
                        preparedBatch.push_back(expandedEntry);
                    }
                }
            } catch (...) { return std::nullopt; }
        } else {
            // Single file validation
            if (!std::filesystem::exists(entry.sourceFilePath)) return std::nullopt;
            for (const auto& dest : entry.destinationDirectories) {
                if (!std::filesystem::exists(dest) || !std::filesystem::is_directory(dest)) return std::nullopt;
            }

            // Check conflict: source == destination
            for (const auto& dest : entry.destinationDirectories) {
                if (entry.sourceFilePath == (dest / entry.sourceFilePath.filename())) return std::nullopt;
            }

            PendingMoveEntry preparedEntry = entry;
            // Ensure relativePath is set for single files to maintain consistent logic in ProcessEntry
            if (preparedEntry.relativePath.empty()) {
                preparedEntry.relativePath = preparedEntry.sourceFilePath.filename();
            }

            std::filesystem::path sidecar = preparedEntry.sourceFilePath.string() + ".filemove-queued";
            {
                std::ofstream sidecarFile(sidecar);
                sidecarFile << "Queued at: " << preparedEntry.queuedAt << std::endl;
            }
            preparedBatch.push_back(preparedEntry);
        }
    }

    return preparedBatch;
}

void MoveWorker::QueueBatch(std::vector<PendingMoveEntry> batch) {
#ifdef _WIN32
    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        m_batchQueue.push(std::move(batch));
        m_queuedCount += batch.size(); 
    }
    m_cv.notify_one();
#endif
}

void MoveWorker::Pause() {
#ifdef _WIN32
    m_paused = true;
    m_workerState = "Manual Pause";
#endif
}

void MoveWorker::Resume() {
#ifdef _WIN32
    m_paused = false;
    m_workerState = "Idle";
    m_cv.notify_one();
#endif
}

size_t MoveWorker::GetQueuedCount() const {
    return m_queuedCount;
}

std::vector<PendingMoveEntry> MoveWorker::GetQueuedItems() const {
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(m_queueMutex));
    std::vector<PendingMoveEntry> allItems;
    // We can't easily iterate a std::queue without copying or using a different container.
    // But we only need this for UI updates occasionally.
    // Since m_batchQueue is private, and we are in the class, we have access.
    // However, std::queue doesn't provide iterators. 
    // Let's change m_batchQueue to std::deque if we want easier iteration, or just copy it.
    // Actually, let's just use a temporary copy of the queue to iterate through it.
    std::queue<std::vector<PendingMoveEntry>> tempQueue = m_batchQueue;
    while (!tempQueue.empty()) {
        const auto& batch = tempQueue.front();
        for (const auto& entry : batch) {
            allItems.push_back(entry);
        }
        tempQueue.pop();
    }
    return allItems;
}

size_t MoveWorker::GetProcessedCount() const {
    return m_processedCount;
}

std::string MoveWorker::GetWorkerState() const {
#ifdef _WIN32
    if (m_paused) return "Manual Pause";
    return m_workerState;
#else
    return "Idle";
#endif
}

void MoveWorker::WorkerLoop() {
#ifdef _WIN32
    while (m_running) {
        std::vector<PendingMoveEntry> currentBatch;
        {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            m_cv.wait(lock, [this] { return !m_batchQueue.empty() || !m_running; });

            if (!m_running) break;

            currentBatch = std::move(m_batchQueue.front());
            m_batchQueue.pop();
        }

        if (!currentBatch.empty()) {
            m_workerState = "Processing";
            ProcessBatch(currentBatch);
            m_workerState = "Idle";
        }
    }
#endif
}

void MoveWorker::ProcessBatch(const std::vector<PendingMoveEntry>& batch) {
#ifdef _WIN32
    for (const auto& entry : batch) {
        while (m_paused && m_running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        if (!m_running) break;

        ProcessEntry(entry);
        m_processedCount++;
    }
#endif
}

void MoveWorker::ProcessEntry(const PendingMoveEntry& entry) {
#ifdef _WIN32
    try {
        std::filesystem::path sidecar = entry.sourceFilePath.string() + ".filemove-queued";
        bool isCopyMode = (entry.debugTransferMode == DebugMode::CP);
        bool immediateShutdownRequested = (m_shutdownMode == ShutdownMode::Immediate);
        
        std::vector<std::filesystem::path> completedDestinations;

        for (const auto& destDir : entry.destinationDirectories) {
            if (immediateShutdownRequested) break;

            // Use relativePath to preserve directory structure during expansion
            std::filesystem::path targetPath = destDir / entry.relativePath;
            
            // Ensure subdirectories exist in the destination
            if (!targetPath.parent_path().empty()) {
                std::filesystem::create_directories(targetPath.parent_path());
            }

            if (isCopyMode) {
                std::filesystem::copy_file(entry.sourceFilePath, targetPath, std::filesystem::copy_options::overwrite_existing);
            } else if (entry.destinationDirectories.size() == 1 && entry.relativePath == entry.sourceFilePath.filename()) {
                // For single file moves to a single destination, rename is safe and efficient
                std::filesystem::rename(entry.sourceFilePath, targetPath);
            } else {
                // For multi-destination or expanded directories, use copy first to ensure safety
                std::filesystem::copy_file(entry.sourceFilePath, targetPath, std::filesystem::copy_options::overwrite_existing);
            }
            completedDestinations.push_back(targetPath);
        }

        if (immediateShutdownRequested) {
            for (const auto& partial : completedDestinations) {
                if (std::filesystem::exists(partial)) {
                    std::filesystem::remove(partial);
                }
            }
            if (!entry.activeLogFilePath.empty()) {
                 Logger::LogTransfer(entry.activeLogFilePath, 
                                    entry.sourceFilePath.filename().string(),
                                    entry.sourceFilePath.parent_path(),
                                    "", 
                                    entry.queuedAt,
                                    "Canceled during shutdown");
            }
            return;
        }

        if (!isCopyMode && entry.destinationDirectories.size() > 1) {
            std::filesystem::remove(entry.sourceFilePath);
        }

        // Log transfer result to the active log file recorded in the queue entry
        if (!entry.activeLogFilePath.empty()) {
             Logger::LogTransfer(entry.activeLogFilePath, 
                                 entry.sourceFilePath.filename().string(),
                                 entry.sourceFilePath.parent_path(),
                                 entry.destinationDirectories[0], 
                                 entry.queuedAt,
                                 "Success");
        }

        FileMove::Logger::LogDebug("Successfully moved: " + entry.sourceFilePath.filename().string());

        std::filesystem::remove(sidecar);
    } catch (const std::exception& e) {
        std::cerr << "Move failed for " << entry.sourceFilePath << ": " << e.what() << std::endl;
        FileMove::Logger::LogDebug("Failed to move: " + entry.sourceFilePath.filename().string() + " Error: " + e.what());
    }
#endif
}

} // namespace FileMove
