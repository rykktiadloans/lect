/**
 * @file settings.hpp
 * @brief Settings is a class that parser the CLI arguments to configure the
 * program
 */

#pragma once

#include "checks.hpp"
#include "preprocessing.hpp"
#include "structures.hpp"
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>
namespace lect {

const std::string help_string = R"del(
Usage:
  lect -t <text_ann_dir> -s <src_dir> -l <language> -o <output> [<optional_args>...]

Required arguments:
  -t <path>   Directory with .an annotation files
  -s <path>   Source code directory with annotations
  -l <lang>   Programming language of the project
  -o <path>   Output directory

Supported languages:
  c++         C++ (.cpp .c .h .hpp)

Optional arguments:
  -d <dir>    Select a direction (UD, DU, RL, LR)
  -r          Removes the middle lines of code
              annotations
  -suf <suf>  Makes the supplied suffix mandatory for
              code annotations
  -lup <d>    Choose which nodes should be lined up
              (leaves, roots)
  -h, --help  Help screen
)del";

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
                if (argc == ptr + 1) {
                    throw Exception("Text annotation path not supplied after " +
                                    color_green + "'-t'" + color_reset);
                }
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
                if (argc == ptr + 1) {
                    throw Exception("Source code path not supplied after " +
                                    color_green + "'-s'" + color_reset);
                }
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
                if (argc == ptr + 1) {
                    throw Exception("Output path not supplied after " +
                                    color_green + "'-o'" + color_reset);
                }
                std::string path = argv[ptr + 1];
                ptr++;
                settings->output_path = path;
                output_path_set = true;

            } else if (arg == "-l") {
                if (argc == ptr + 1) {
                    throw Exception("Language not supplied after " +
                                    color_green + "'-l'" + color_reset);
                }
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
                if (argc == ptr + 1) {
                    throw Exception("Direction not supplied after " +
                                    color_green + "'-d'" + color_reset +
                                    "\nCan be either `RL`, `LR`, `UD`, `DU`");
                }
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
            } else if (arg == "-suf") {
                if (argc == ptr + 1) {
                    throw Exception("Suffix not supplied after " + color_green +
                                    "'-suf'" + color_reset);
                }
                std::string suffix = argv[ptr + 1];
                ptr++;
                settings->checker->add(
                    std::make_unique<CodeAnnotationsSuffixChecker>(suffix));
            } else if (arg == "-lup") {
                if (argc == ptr + 1) {
                    throw Exception("Lineup not supplied after " + color_green +
                                    "'-lup'" + color_reset +
                                    ".\nAvailable options: 'leaves', 'roots'");
                }
                std::string dir = argv[ptr + 1];
                ptr++;
                if (dir != "leaves" && dir != "roots") {
                    throw Exception("Unrecognised lineup " + color_yellow +
                                    "-lup" + color_reset + " option: " +
                                    color_blue + "'" + dir + "'" + color_reset +
                                    ".\nAvailable options: 'leaves', 'roots'");
                }
                settings->preprocessing_builder.set_lineup(dir);

            } else if (arg == "-h" || arg == "--help") {
                std::cout << help_string;
                throw Exception("help");

            } else {
                throw Exception("Unrecognized argument: " + color_blue + arg +
                                color_reset);
            }

            ptr++;
        }

        std::string except = "";
        if (!text_path_set) {
            except += "Text annotation path isn't set, try using option " +
                      color_green + "'-t'" + color_reset + "\n";
        }
        if (!code_path_set) {
            except += "Code annotation path isn't set, try using option " +
                      color_green + "'-s'" + color_reset + "\n";
        }
        if (!output_path_set) {
            except += "Output path isn't set, try using option " + color_green +
                      "'-o'" + color_reset + "\n";
        }
        if (!language_set) {
            except += "Language isn't set, try using option " + color_green +
                      "'-l'" + color_reset + "\n";
        }
        if(except != "") {
            throw Exception(except.substr(0, except.size() - 1));
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
