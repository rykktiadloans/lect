/**
 * @file settings.hpp
 * @brief Settings is a class that parser the CLI arguments to configure the
 * program
 */

#pragma once

#include "checks.hpp"
#include "structures.hpp"
#include <filesystem>
#include <iostream>
#include <memory>
namespace lect {

/**
 * @class Settings
 * @brief A class that parser the CLI arguments and makes an object out of it.
 *
 */
//$settings-src Settings class
struct Settings {
    std::filesystem::path text_annotation_path;
    std::filesystem::path code_annotation_path;
    std::filesystem::path output_path;
    Language language{Language::placeholder()};
    std::unique_ptr<Checker> checker{
        std::make_unique<IdAllowedSymbolsChecker>()};

    /**
     * @brief Uses main() function's argc and argv arguments to construct itself
     *
     * @param argc Number of command line arguments
     * @param argv An array of command line arguments
     */
    Settings(int argc, char **argv) {
        checker->add<DuplicateChecker>();
        checker->add<NonexistentChecker>();
        checker->add<CycleChecker>();

        int ptr = 1;
        bool text_path_set = false;
        bool code_path_set = false;
        bool output_path_set = false;
        bool language_set = false;

        while (ptr < argc) {
            std::string arg = argv[ptr];
            if (arg == "-t") {
                std::string dir = argv[ptr + 1];
                ptr++;
                text_annotation_path = dir;
                if (!std::filesystem::exists(text_annotation_path)) {
                    throw Exception("Text annotation path `" +
                                    text_annotation_path.string() +
                                    "` doesn't exist");
                }
                if (!std::filesystem::is_directory(text_annotation_path)) {
                    throw Exception(
                        "Text annotation path `" +
                        std::filesystem::canonical(text_annotation_path)
                            .string() +
                        "` must be a directory");
                }
                text_path_set = true;
            } else if (arg == "-s") {
                std::string path = argv[ptr + 1];
                ptr++;
                code_annotation_path = path;
                if (!std::filesystem::exists(code_annotation_path)) {
                    throw Exception("Code annotation path `" +
                                    code_annotation_path.string() +
                                    "` doesn't exist");
                }
                code_path_set = true;
            } else if (arg == "-o") {
                std::string path = argv[ptr + 1];
                ptr++;
                output_path = path;
                output_path_set = true;
            } else if (arg == "-l") {
                std::string lang = argv[ptr + 1];
                ptr++;
                std::unordered_map<std::string, std::function<Language(void)>>
                    language_map{{"c++", Language::cpp}};
                auto constructor = language_map.find(lang);
                if (constructor == language_map.end()) {
                    throw Exception("Unrecognized language: " + color_blue +
                                    lang + color_reset);
                }
                language = constructor->second();
                language_set = true;
            } else if (arg == "-h" || arg == "--help") {
                std::cout
                    << "Usage:\n"
                       "  lect -t <text_ann_dir> -s <src_dir> -l "
                       "<language> -o <output>"
                       "  [<optional_args>...]\n\n"
                       "Required arguments:\n"
                       "  -t          Directory with .an annotation files\n"
                       "  -s          Source code directory with annotations\n"
                       "  -l          Programming language of the project\n"
                       "  -o          Output directory\n\n"
                       "Supported languages:\n"
                       "  c++         C++ (.cpp .c .h .hpp)\n\n"
                       "Optional arguments:\n"
                       "  -h, --help  Help screen\n";
                throw Exception("help");

            } else {
                throw Exception("Unrecognized argument: " + color_blue + arg +
                                color_reset);
            }
            ptr++;
        }

        if (!text_path_set) {
            throw Exception("Text annotation path isn't set");
        }
        if (!code_path_set) {
            throw Exception("Code annotation path isn't set");
        }
        if (!output_path_set) {
            throw Exception("Output path isn't set");
        }
        if (!language_set) {
            throw Exception("Language isn't set");
        }
    }
};
} // namespace lect
