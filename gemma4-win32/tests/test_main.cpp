#include <gtest/gtest.h>
#include "filemove.hpp"
#include "clipboard_handler.hpp"
#include <vector>
#include <string>
#include <filesystem>

class CommandLineTest : public ::testing::Test {
protected:
    std::vector<std::string> args_vec;
    std::vector<char*> argv;

    void setArgs(std::initializer_list<std::string> list) {
        args_vec = list;
        argv.clear();
        for (auto& s : args_vec) {
            argv.push_back(const_cast<char*>(s.c_str()));
        }
    }
};

TEST_F(CommandLineTest, ValidOptions) {
    setArgs({"FileMove", "-D", "MV", "-I", "config.json", "/S", "AZ"});
    int argc = static_cast<int>(argv.size());

    auto options = FileMove::CommandLineParser::parse(argc, argv.data());

    EXPECT_EQ(options.debugMode, FileMove::DebugMode::MV);
    EXPECT_EQ(options.inputJsonPath.string(), "config.json");
    EXPECT_EQ(options.sortMode, FileMove::SortMode::AtoZ);
}

TEST_F(CommandLineTest, CaseInsensitivity) {
    setArgs({"FileMove", "/s", "mru"});
    int argc = static_cast<int>(argv.size());

    auto options = FileMove::CommandLineParser::parse(argc, argv.data());

    EXPECT_EQ(options.sortMode, FileMove::SortMode::MostRecentlyUsed);
}

TEST_F(CommandLineTest, InvalidValueThrows) {
    setArgs({"FileMove", "-D", "INVALID"});
    int argc = static_cast<int>(argv.size());

    EXPECT_THROW(FileMove::CommandLineParser::parse(argc, argv.data()), FileMove::ParseException);
}

TEST_F(CommandLineTest, MissingValueThrows) {
    setArgs({"FileMove", "-I"});
    int argc = static_cast<int>(argv.size());

    EXPECT_THROW(FileMove::CommandLineParser::parse(argc, argv.data()), FileMove::ParseException);
}

TEST_F(CommandLineTest, UnknownOptionThrows) {
    setArgs({"FileMove", "-Z"});
    int argc = static_cast<int>(argv.size());

    EXPECT_THROW(FileMove::CommandLineParser::parse(argc, argv.data()), FileMove::ParseException);
}

class ClipboardHandlerTest : public ::testing::Test {
protected:
    FileMove::Group testGroup;

    void SetUp() override {
        testGroup.id = "group-1";
        testGroup.name = "TestGroup";
        testGroup.destinationPaths = {"/tmp/dest"};
    }
};

TEST_F(ClipboardHandlerTest, CreateEntries_ValidPaths) {
    std::vector<std::filesystem::path> paths = {"/tmp/file1.txt", "/tmp/file2.txt"};
    std::string timestamp = "2024-01-01 12:00:00";
    
    auto entries = FileMove::ClipboardHandler::CreateEntriesFromPaths(paths, testGroup, FileMove::DebugMode::None, timestamp, "clip");

    ASSERT_EQ(entries.size(), 2);
    EXPECT_EQ(entries[0].id, "clip-0");
    EXPECT_EQ(entries[0].groupId, "group-1");
    EXPECT_EQ(entries[0].sourceFilePath.string(), "/tmp/file1.txt");
    EXPECT_EQ(entries[0].relativePath.string(), "file1.txt");
    EXPECT_EQ(entries[0].destinationDirectories[0].string(), "/tmp/dest");
    EXPECT_EQ(entries[0].queuedAt, timestamp);

    EXPECT_EQ(entries[1].id, "clip-1");
    EXPECT_EQ(entries[1].sourceFilePath.string(), "/tmp/file2.txt");
}

TEST_F(ClipboardHandlerTest, CreateEntries_EmptyPaths) {
    std::vector<std::filesystem::path> paths = {};
    auto entries = FileMove::ClipboardHandler::CreateEntriesFromPaths(paths, testGroup, FileMove::DebugMode::None, "timestamp");
    EXPECT_TRUE(entries.empty());
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
