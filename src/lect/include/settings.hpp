/**
 * @file settings.hpp
 * @brief Settings is a class that parser the CLI arguments to configure the
 * program
 */

#pragma once

#include "checks.hpp"
#include "nlohmann/json.hpp"
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
  java        Java (.java)

Optional arguments:
  -d <dir>    Select a direction (UD, DU, RL, LR)
  -r          Removes the middle lines of code
              annotations
  -suf <suf>  Makes the supplied suffix mandatory for
              code annotations
  -lup <d>    Choose which nodes should be lined up
              (leaves, roots)
  -jb         Open code links with Jetbrains IDEs
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
    Preprocessing preprocessing;

    /**
     * @brief Uses main() function's argc and argv arguments to construct a
     * settings object
     *
     * @param argc Number of command line arguments
     * @param argv An array of command line arguments
     */
    //$settings-builder-src Settings builder method
    static std::unique_ptr<Settings> build_with_args(int argc, const char **argv); 

  private:
    /**
     * @brief Private constructor. Makes it so that the use of builder in
     * necessary
     */
    Settings() {}
};
} // namespace lect
