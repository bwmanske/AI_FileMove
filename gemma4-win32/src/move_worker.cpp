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
    m_running = true;
    m_workerThread = std::thread(&MoveWorker::WorkerLoop, this);
}

void MoveWorker::Stop() {
    m_running = false;
    m_shutdownMode = ShutdownMode::None;
    m_cv.notify_all();
    if (m_workerThread.joinable()) {
        m_workerThread.join();
    }
}

void MoveWorker::RequestShutdown(ShutdownMode mode) {
    m_shutdownMode = mode;
    if (mode == ShutdownMode::Immediate) {
        m_running = false;
    }
    m_cv.notify_all();
}

std::optional<std::vector<PendingMoveEntry>> MoveWorker::PrepareBatch(const std::vector<PendingMoveEntry>& batch) {
    std::vector<PendingMoveEntry> preparedBatch;

    for (const auto& entry : batch) {
        if (std::filesystem::is_directory(entry.sourceFilePath)) {
            try {
                std::vector<PendingMoveEntry> expandedEntries;
                for (const auto& dir_entry : std::filesystem::recursive_directory_iterator(entry.sourceFilePath)) {
                    if (dir_entry.is_regular_file()) {
                        std::filesystem::path p = dir_entry.path();
                        if (p.extension() == ".filemove-queued") continue;

                        PendingMoveEntry expandedEntry = entry;
                        expandedEntry.sourceFilePath = p;
                        expandedEntry.relativePath = std::filesystem::relative(p, entry.sourceFilePath);

                        bool dests_ok = true;
                        for (const auto& dest : expandedEntry.destinationDirectories) {
                            if (!std::filesystem::exists(dest) || !std::filesystem::is_directory(dest)) {
                                dests_ok = false;
                                break;
                            }
                        }
                        if (!dests_ok) continue;

                        bool conflict = false;
                        for (const auto& dest : expandedEntry.destinationDirectories) {
                            if (expandedEntry.sourceFilePath == (dest / expandedEntry.relativePath)) {
                                conflict = true;
                                break;
                            }
                        }
                        if (conflict) continue;

                        expandedEntries.push_back(expandedEntry);
                    }
                }

                for (auto& expandedEntry : expandedEntries) {
                    std::filesystem::path sidecar = expandedEntry.sourceFilePath.string() + ".filemove-queued";
                    {
                        std::ofstream sidecarFile(sidecar);
                        sidecarFile << "Queued at: " << expandedEntry.queuedAt << std::endl;
                    }
                    preparedBatch.push_back(expandedEntry);
                }
            } catch (...) { }
        } else {

            if (!std::filesystem::exists(entry.sourceFilePath)) continue;
            bool dests_ok = true;
            for (const auto& dest : entry.destinationDirectories) {
                if (!std::filesystem::exists(dest) || !std::filesystem::is_directory(dest)) {
                    dests_ok = false;
                    break;
                }
            }
            if (!dests_ok) continue;

            bool conflict = false;
            for (const auto& dest : entry.destinationDirectories) {
                if (entry.sourceFilePath == (dest / entry.relativePath)) {
                    conflict = true;
                    break;
                }
            }
            if (conflict) continue;

            PendingMoveEntry preparedEntry = entry;
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

    if (preparedBatch.empty() && !batch.empty()) return std::nullopt;
    return preparedBatch;
}

void MoveWorker::QueueBatch(std::vector<PendingMoveEntry> batch) {
    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        m_batchQueue.push(std::move(batch));
        m_queuedCount += batch.size(); 
    }
    m_cv.notify_one();
}

void MoveWorker::Pause() { m_paused = true; m_workerState = "Manual Pause"; }
void MoveWorker::Resume() { m_paused = false; m_workerState = "Idle"; m_cv.notify_one(); }

size_t MoveWorker::GetQueuedCount() const { return m_queuedCount; }
size_t MoveWorker::GetProcessedCount() const { return m_processedCount; }
std::string MoveWorker::GetWorkerState() const { return (m_paused ? "Manual Pause" : m_workerState); }

std::vector<PendingMoveEntry> MoveWorker::GetQueuedItems() const {
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(m_queueMutex));
    std::vector<PendingMoveEntry> allItems;
    std::queue<std::vector<PendingMoveEntry>> tempQueue = m_batchQueue;
    while (!tempQueue.empty()) {
        for (const auto& entry : tempQueue.front()) allItems.push_back(entry);
        tempQueue.pop();
    }
    return allItems;
}

void MoveWorker::WorkerLoop() {
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
}

void MoveWorker::ProcessBatch(const std::vector<PendingMoveEntry>& batch) {
    for (const auto& entry : batch) {
        if (!m_running) break;
        ProcessEntry(entry);
        m_processedCount++;
    }
}

void MoveWorker::ProcessEntry(const PendingMoveEntry& entry) {
    try {
        std::filesystem::path sidecar = entry.sourceFilePath.string() + ".filemove-queued";
        std::cout << "DEBUG: Processing Entry. Source: [" << entry.sourceFilePath << "] RelativePath: [" << entry.relativePath << "]" << std::endl;
        bool isCopyMode = (entry.debugTransferMode == DebugMode::CP);
        bool immediateShutdownRequested = (m_shutdownMode == ShutdownMode::Immediate);
        
        std::vector<std::filesystem::path> completedDestinations;

        for (const auto& destDir : entry.destinationDirectories) {
            if (immediateShutdownRequested) break;
            std::filesystem::path targetPath = destDir / entry.relativePath;
            if (!targetPath.parent_path().empty()) std::filesystem::create_directories(targetPath.parent_path());

            if (isCopyMode) {
                std::filesystem::copy_file(entry.sourceFilePath, targetPath, std::filesystem::copy_options::overwrite_existing);
            } else if (entry.destinationDirectories.size() == 1 && entry.relativePath == entry.sourceFilePath.filename()) {
                std::filesystem::rename(entry.sourceFilePath, targetPath);
            } else {
                std::filesystem::copy_file(entry.sourceFilePath, targetPath, std::filesystem::copy_options::overwrite_existing);
            }
            completedDestinations.push_back(targetPath);
        }

        if (immediateShutdownRequested) {
            for (const auto& partial : completedDestinations) if (std::filesystem::exists(partial)) std::filesystem::remove(partial);
            return;
        }

        if (!isCopyMode) {
            if (std::filesystem::exists(entry.sourceFilePath)) std::filesystem::remove(entry.sourceFilePath);
        }

        try {
             if (!entry.activeLogFilePath.empty()) {
                Logger::LogTransfer(entry.activeLogFilePath, 
                                    entry.sourceFilePath.filename().string(),
                                    entry.sourceFilePath.parent_path(),
                                    entry.destinationDirectories[0], 
                                    entry.queuedAt,
                                    "Success");
             }
        } catch (...) {}

        if (std::filesystem::exists(sidecar)) std::filesystem::remove(sidecar);
    } catch (const std::exception& e) {
        std::cerr << "Move failed: " << e.what() << " for " << entry.sourceFilePath << std::endl;
    }
}

} // namespace FileMove
