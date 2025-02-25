/**
 * @file settings.hpp
 * @brief Settings is a class that parser the CLI arguments to configure the
 * program
 */

#pragma once

#include "checks.hpp"
#include "export.hpp"
#include "nlohmann/json_fwd.hpp"
#include "preprocessing.hpp"
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
    std::unique_ptr<Checker> checker;
    PrepocessingBuilder preprocessing_builder;

    /**
     * @brief Uses main() function's argc and argv arguments to construct a
     * settings object
     *
     * @param argc Number of command line arguments
     * @param argv An array of command line arguments
     */
    //$settings-builder-src Settings builder method
    static std::unique_ptr<Settings> build_with_args(int argc, char **argv) {
        std::unique_ptr<Settings> settings{new Settings()};
        settings->checker = std::make_unique<IdAllowedSymbolsChecker>();
        settings->checker->add(std::make_unique<DuplicateChecker>());
        settings->checker->add(std::make_unique<NonexistentChecker>());
        settings->checker->add(std::make_unique<CycleChecker>());

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
                settings->text_annotation_path = dir;
                if (!std::filesystem::exists(settings->text_annotation_path)) {
                    throw Exception("Text annotation path `" +
                                    settings->text_annotation_path.string() +
                                    "` doesn't exist");
                }
                if (!std::filesystem::is_directory(
                        settings->text_annotation_path)) {
                    throw Exception("Text annotation path `" +
                                    std::filesystem::canonical(
                                        settings->text_annotation_path)
                                        .string() +
                                    "` must be a directory");
                }
                text_path_set = true;

            } else if (arg == "-s") {
                std::string path = argv[ptr + 1];
                ptr++;
                settings->code_annotation_path = path;
                if (!std::filesystem::exists(settings->code_annotation_path)) {
                    throw Exception("Code annotation path `" +
                                    settings->code_annotation_path.string() +
                                    "` doesn't exist");
                }
                code_path_set = true;

            } else if (arg == "-o") {
                std::string path = argv[ptr + 1];
                ptr++;
                settings->output_path = path;
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
                settings->language = constructor->second();
                language_set = true;

            } else if (arg == "-d") {
                std::string dir = argv[ptr + 1];
                ptr++;
                if (dir != "UD" && dir != "DU" && dir != "LR" && dir != "RL") {
                    throw Exception("Unrecognized direction: " + color_blue +
                                    dir + color_reset + 
                                    "\nCan be either `RL`, `LR`, `UD`, `DU`");
                }
                settings->preprocessing_builder.add_direction(dir);

            } else if (arg == "-r") {
                settings->preprocessing_builder
                    .remove_code_annotations_middle();

            } else if (arg == "-h" || arg == "--help") {
                std::cout
                    << "Usage:\n"
                       "  lect -t <text_ann_dir> -s <src_dir> -l "
                       "<language> -o <output>"
                       "  [<optional_args>...]\n\n"
                       "Required arguments:\n"
                       "  -t <path>   Directory with .an annotation files\n"
                       "  -s <path>   Source code directory with annotations\n"
                       "  -l <lang>   Programming language of the project\n"
                       "  -o <path>   Output directory\n\n"
                       "Supported languages:\n"
                       "  c++         C++ (.cpp .c .h .hpp)\n\n"
                       "Optional arguments:\n"
                       "  -d <dir>  Select a direction (UD, DU, RL, LR)\n"
                       "  -r        Removes the middle lines of code "
                       "annotations\n"
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
        return settings;
    }

  private:
    /**
     * @brief Private constructor. Makes it so that the use of builder in
     * necessary
     */
    Settings() {}
};
} // namespace lect
