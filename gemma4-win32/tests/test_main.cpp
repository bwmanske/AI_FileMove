#include <gtest/gtest.h>
#include "filemove.hpp"
#include <vector>
#include <string>

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

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
