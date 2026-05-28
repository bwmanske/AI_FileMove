#include "filemove.hpp"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <string>

namespace FileMove {

static std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return std::tolower(c); });
    return s;
}

CommandLineOptions CommandLineParser::parse(int argc, char* argv[]) {
    CommandLineOptions options;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg.length() >= 2 && (arg[0] == '/' || arg[0] == '-') && std::isalpha(static_cast<unsigned char>(arg[1]))) {
            char opt = std::toupper(static_cast<unsigned char>(arg[1]));
            
            if (opt == 'D') {
                if (i + 1 < argc) {
                    std::string val = toLower(argv[++i]);
                    if (val == "mv") options.debugMode = DebugMode::MV;
                    else if (val == "cp") options.debugMode = DebugMode::CP;
                    else throw ParseException("Invalid value for /D: must be MV or CP");
                } else {
                    throw ParseException("Missing value for /D");
                }
            } else if (opt == 'I') {
                if (i + 1 < argc) {
                    options.inputJsonPath = argv[++i];
                } else {
                    throw ParseException("Missing value for /I");
                }
            } else if (opt == 'O') {
                if (i + 1 < argc) {
                    options.outputLogPath = argv[++i];
                } else {
                    throw ParseException("Missing value for /O");
                }
            } else if (opt == 'S') {
                if (i + 1 < argc) {
                    std::string val = toLower(argv[++i]);
                    if (val == "mru" || val == "mostrecentlyused") options.sortMode = SortMode::MostRecentlyUsed;
                    else if (val == "lru" || val == "leastrecentlyused") options.sortMode = SortMode::LeastRecentlyUsed;
                    else if (val == "af" || val == "addedfirst") options.sortMode = SortMode::AddedFirst;
                    else if (val == "al" || val == "addedlast") options.sortMode = SortMode::AddedLast;
                    else if (val == "az" || val == "atoz") options.sortMode = SortMode::AtoZ;
                    else if (val == "za" || val == "ztoa") options.sortMode = SortMode::ZtoA;
                    else throw ParseException("Invalid value for /S");
                } else {
                    throw ParseException("Missing value for /S");
                }
            } else if (opt == 'P') {
                if (i + 1 < argc) {
                    std::string val = toLower(argv[++i]);
                    if (val == "ul" || val == "upperleft") options.placementMode = PlacementMode::UpperLeft;
                    else if (val == "ur" || val == "upperright") options.placementMode = PlacementMode::UpperRight;
                    else if (val == "ll" || val == "lowerleft") options.placementMode = PlacementMode::LowerLeft;
                    else if (val == "lr" || val == "lowerright") options.placementMode = PlacementMode::LowerRight;
                    else if (val == "last" || val == "lastlocation") options.placementMode = PlacementMode::LastLocation;
                    else throw ParseException("Invalid value for /P");
                } else {
                    throw ParseException("Missing value for /P");
                }
            } else {
                throw ParseException("Unknown option: " + arg);
            }
        } else {
            // The spec says recognize an option only when preceded by / or -
            // If it's not, we ignore it as a potential filename/text unless we are in error mode.
            // For now, let's just skip unknown non-option args.
        }
    }

    return options;
}

} // namespace FileMove
