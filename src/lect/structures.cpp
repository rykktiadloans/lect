/**
 * @file
 * @brief A collection of data structures used in other parts of the application
 */

#include "structures.hpp"
#include "tree-sitter-cpp.h"
#include "tree-sitter-java.h"
#include "tree_sitter/api.h"
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

std::string lect::color_reset = "\x1B[0m";

std::string lect::color_red = "\x1B[31m";

std::string lect::color_green = "\x1B[32m";

std::string lect::color_yellow = "\x1B[33m";

std::string lect::color_blue = "\x1B[34m";

std::string lect::color_magenta = "\x1B[35m";

std::string lect::color_cyan = "\x1B[36m";

std::string lect::color_white = "\x1B[37m";

bool lect::CSyntaxValidator::validate_comment(std::string string) {
    uint64_t begin = string.find_first_not_of("\n ");
    if (begin == std::string::npos) {
        return false;
    }

    std::string comment_fragment = string.substr(begin, 2);
    if (comment_fragment != "//") {
        return false;
    }

    uint64_t ptr = begin + 2;
    while (string.at(ptr) == ' ' || string.at(ptr) == '\n') {
        ptr++;
    }

    if (string.at(ptr) != '$') {
        return false;
    }

    return true;
}

bool lect::CSyntaxValidator::validate_object(std::string string) {
    uint64_t begin = string.find_first_not_of("\n ");
    if (begin == std::string::npos) {
        return false;
    }

    std::string comment_fragment = string.substr(begin, 2);
    if (comment_fragment == "//" || comment_fragment == "/*") {
        return false;
    }

    return true;
}

lect::Language lect::Language::cpp() {
    std::vector<std::string> extensions{".c", ".cpp", ".h", ".hpp"};
    return Language("c++", extensions,
                    "((comment) @comment . (comment)* . (_) @object)",
                    tree_sitter_cpp(), std::make_unique<CSyntaxValidator>());
}

lect::Language lect::Language::java() {
    std::vector<std::string> extensions{".java"};
    return Language("java", extensions,
                    "((line_comment) @comment . [(line_comment) "
                    "(block_comment)]* . (_) @object)",
                    tree_sitter_java(), std::make_unique<CSyntaxValidator>());
}

lect::Language lect::Language::placeholder() {
    return Language("", std::vector<std::string>(), "", nullptr, nullptr);
}
