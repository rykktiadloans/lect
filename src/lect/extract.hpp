/**
 * @file
 * @brief A collection of functions that extract the annotations from source
 * code and annotation files, and convert them into usable data structures
 */

#pragma once

#include "structures.hpp"
#include "tree_sitter/api.h"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <tree-sitter-cpp.h>
#include <vector>
#include <string>
#include <sstream>

namespace lect {

//$annotations-builder-src Annotations builder
/**
 * @class AnnotationsBuilder
 * @brief A builder class for assembling an annotation object
 *
 */
struct AnnotationsBuilder {

    /**
     * @brief A function that extracts all code annotations from the
     * file/directory
     *
     * @param root Path in which to look for annotations
     * @param language Language object
     * @return This builder (for chaining purposes)
     * @throw lect::Exception
     */
    AnnotationsBuilder &
    extract_code_annotations(const std::filesystem::path &root,
                             const Language &language) noexcept(false) {
        using namespace std::filesystem;

        std::mutex mutex;
        std::vector<CodeAnnotation> &code_annotations =
            _annotations.code_annotations;
        auto add = [&code_annotations,
                    &mutex](std::string id, std::string title,
                            std::string content, std::string file, int line) {
            const std::lock_guard<std::mutex> lock_guard(mutex);
            code_annotations.emplace_back(id, title, content, file, line);
        };

        _extract_code_annotations_inner(root, language, add);
        return *this;
    }

    /**
     * @brief Finds all the text annotations in a directory and returns them.
     * Throws an exception if the path isn't a directory
     *
     * @param root Root directory of the annotations
     * @return This builder (for chaining purposes)
     * @throw lect:Exception
     */
    AnnotationsBuilder &extract_text_annotations(
        const std::filesystem::path &root) noexcept(false) {
        using namespace std::filesystem;

        if (!is_directory(root)) {
            throw Exception(root.string() + " is not a directory.");
        }

        std::mutex mutex;
        std::vector<TextAnnotation> &annotations =
            _annotations.text_annotations;
        auto add = [&annotations, &mutex](std::string id, std::string title,
                                          std::string content,
                                          std::vector<std::string> references) {
            const std::lock_guard<std::mutex> lock_guard(mutex);
            annotations.emplace_back(id, title, content, references);
        };

        _extract_text_annotations_inner(root, add);
        return *this;
    }

    /**
     * @brief Returns the assembled annotations
     *
     * @return Annotations
     */
    Annotations get_annotations() { return _annotations; }

  private:
    Annotations _annotations;

    /**
     * @brief An inner function that extracts code annotations from a file, or
     * looks for other files in the directory
     *
     * @tparam F function type for adding a code annotation to an array
     * @param path Path of the current file
     * @param language Language object
     * @param add Function that adds a code annotation to an array
     * @throw lect::Exception
     */
    template <typename F>
    void _extract_code_annotations_inner(const std::filesystem::path &path,
                                         const Language &language,
                                         F &add) noexcept(false) {
        using namespace std::filesystem;
        if (is_directory(path)) {
            std::vector<std::future<void>> futures;
            for (auto const &child : directory_iterator{path}) {
                futures.push_back(std::async(
                    std::launch::async, [child, &add, &language, this] {
                        std::cout << std::this_thread::get_id() << "\n";
                        _extract_code_annotations_inner(child, language, add);
                    }));
            }
            for (auto &fut : futures) {
                try {
                    fut.get();
                } catch (Exception e) {
                    if (true) {
                        throw e;
                    }
                }
            }
            return;
        }
        if (std::find(language.extensions.begin(), language.extensions.end(),
                      path.extension()) == language.extensions.end()) {
            return;
        }

        std::ifstream file(path);
        std::stringstream file_stream;
        file_stream << file.rdbuf();
        std::string file_contents(file_stream.str());

        TSParser *parser = ts_parser_new();
        ts_parser_set_language(parser, language.language);
        TSTree *tree = ts_parser_parse_string(
            parser, nullptr, file_contents.c_str(), file_contents.size());

        uint32_t error_offset;
        TSQueryError query_error;
        TSQuery *query =
            ts_query_new(language.language, language.query.c_str(),
                         language.query.size(), &error_offset, &query_error);

        if (query_error != 0) {
            std::cout << "Issue with query at " << error_offset << " of kind "
                      << query_error << "\n";
            return;
        }

        TSQueryCursor *cursor = ts_query_cursor_new();
        ts_query_cursor_exec(cursor, query, ts_tree_root_node(tree));

        TSQueryMatch match;
        while (ts_query_cursor_next_match(cursor, &match)) {
            int start_comment = ts_node_start_byte(match.captures[0].node);
            int end_comment = ts_node_end_byte(match.captures[0].node);
            std::string capture_comment = file_contents.substr(
                start_comment, end_comment - start_comment);
            if (!language.validator->validate_comment(capture_comment)) {
                continue;
            }

            int start_object = ts_node_start_byte(match.captures[1].node);
            int end_object = ts_node_end_byte(match.captures[1].node);
            std::string capture_object =
                file_contents.substr(start_object, end_object - start_object);
            if (!language.validator->validate_object(capture_object)) {
                continue;
            }

            uint64_t dollar = capture_comment.find_first_of("$");
            uint64_t end_of_id = capture_comment.find_first_not_of(
                "abcdefghijklmnopqrstuvwxyz-ABCDEFGHIJKLMNOPQRSTUVWXYZ",
                dollar + 1);

            if (end_of_id == std::string::npos) {
                std::cout
                    << color_red + "ERROR: " + color_reset + color_yellow +
                           canonical(path).string() + color_reset + ":" +
                           color_blue +
                           std::to_string(
                               ts_node_start_point(match.captures[0].node)
                                   .row) +
                           color_reset +
                           "\n  The source code annotation directive doesn't "
                           "have "
                           "an "
                           "identity\n"
                           "  Example `//$identity Elaborate title`\n";
                throw Exception(
                    path.string() + " line " +
                    std::to_string(
                        ts_node_start_point(match.captures[0].node).row));
            }
            std::string id =
                capture_comment.substr(dollar + 1, end_of_id - dollar - 1);
            std::string title = capture_comment.substr(end_of_id + 1);
            if (title.find_first_not_of("\n ") == std::string::npos) {
                std::cout << color_red + "ERROR: " + color_reset +
                                 color_yellow + canonical(path).string() +
                                 color_reset + ":" + color_blue +
                                 std::to_string(
                                     ts_node_start_point(match.captures[0].node)
                                         .row) +
                                 color_reset +
                                 "\n  The source code annotation directive "
                                 "doesn't have "
                                 "a "
                                 "title\n"
                                 "  Example `//$identity Elaborate title`\n";
                throw Exception(
                    path.string() + " line " +
                    std::to_string(
                        ts_node_start_point(match.captures[0].node).row));
            }

            add(id, title, capture_object, relative(path).string(),
                ts_node_start_point(match.captures[0].node).row);
        }
    }

    /**
     * @brief Extracts an annotation from a path if it is a .an file, or looks
     * inside the directory at path
     *
     * @tparam F Function type
     * @param path Path to the file
     * @param add Function that adds an annotation to a container
     * @throw lect::Exception
     */
    template <typename F>
    void _extract_text_annotations_inner(const std::filesystem::path &path,
                                         F &add) noexcept(false) {
        using namespace std::filesystem;
        if (is_directory(path)) {
            std::vector<std::future<void>> futures;
            for (const std::filesystem::path &child :
                 directory_iterator{path}) {
                futures.push_back(
                    std::async(std::launch::async, [child, &add, this] {
                        std::cout << std::this_thread::get_id() << "\n";
                        _extract_text_annotations_inner(child, add);
                    }));
            }
            for (auto &fut : futures) {
                try {
                    fut.get();
                } catch (Exception e) {
                    if (true) {
                        throw e;
                    }
                }
            }
        }
        if (path.extension() != ".an") {
            return;
        }

        std::ifstream file(path);
        bool first = true;

        std::string id(path.stem().string());
        std::string title;
        std::string content;
        std::string current_line;
        std::vector<std::string> references;
        int line_counter = 1;

        while (std::getline(file, current_line)) {
            if (first &&
                current_line.find_first_not_of("\n ") == std::string::npos) {
                line_counter++;
                continue;
            }
            if (first) {
                first = false;
                if (current_line.at(0) == '#' && current_line.at(1) == ' ') {
                    title = current_line.substr(2);
                } else {
                    std::cout
                        << color_red + "ERROR: " + color_reset + color_yellow
                        << canonical(path).string() + color_reset + ":" +
                               color_blue + std::to_string(line_counter) +
                               color_reset +
                               "\n  The file doesn't follow the text "
                               "annotation format.\n"
                               "  First line of the file should be `#` "
                               "followed by the "
                               "annotations title.\n"
                               "  Example: `# Elaborate annotation "
                               "title`\n";
                    throw Exception(path.string() +
                                    " doesn't have a proper title");
                }
                line_counter++;
                continue;
            }
            line_counter++;
            content += current_line;
            content += '\n';
        }
        if (content.size() < 1) {
            std::cout << color_red + "ERROR: " + color_reset + color_yellow
                      << canonical(path).string() + color_reset + ":" +
                             color_blue + std::to_string(line_counter) +
                             color_reset +
                             "\n  The file doesn't follow the text "
                             "annotation format.\n"
                             "  Annotation contains no body after the title\n"
                             "  Example: `# Elaborate annotation "
                             "title`\n";
            throw Exception(path.string() + " doesn't have a proper title");
        }
        while (content.at(0) == '\n') {
            content = content.substr(1);
        }

        std::size_t next_pos = content.find('$');
        while (next_pos != std::string::npos && next_pos < content.size()) {
            std::size_t end_pos = content.find_first_not_of(
                "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-",
                next_pos + 1);
            references.push_back(
                content.substr(next_pos + 1, end_pos - next_pos - 1));
            next_pos = content.find('$', end_pos);
        }
        add(id, title, content, references);
        return;
    }
};

} // namespace lect
