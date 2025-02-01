/**
 * @file
 * @brief A collection of functions that extract the annotations from source
 * code and annotation files, and convert them into usable data structures
 */

#pragma once

#include "annotations.hpp"
#include <filesystem>
#include <fstream>
#include <future>
#include <iostream>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <vector>
namespace lect {

/**
 * @brief A private function: shouldn't be used by itself. Extracts an
 * annotation from a path if it is a .an file, or looks inside the directory at
 * path
 *
 * @tparam F Function type
 * @param path Path to the file
 * @param add Function that adds an annotation to a container
 */
template <typename F>
void extract_text_annotations_inner(const std::filesystem::path &path, F &add) {
    using namespace std::filesystem;
    if (is_directory(path)) {
        for (auto const &child : directory_iterator{path}) {
            std::future fut = std::async(std::launch::async, [&child, &add] {
                std::cout << std::this_thread::get_id() << "\n";
                extract_text_annotations_inner(child, add);
            });
        }
    }
    if (path.extension() != ".an") {
        return;
    }

    std::ifstream file(path);
    bool first = true;

    std::string id(path.stem());
    std::string title;
    std::string content;
    std::string current_line;
    std::vector<std::string> references;

    while (std::getline(file, current_line)) {
        if (first) {
            first = false;
            if (current_line.at(0) == '#' && current_line.at(1) == ' ') {
                title = current_line.substr(2);
            } else {
                // todo: figure out how to deal with mistakes
                return;
            }
            continue;
        }
        content += current_line;
        content += '\n';
    }
    while (content.at(0) == '\n') {
        content = content.substr(1);
    }

    std::size_t next_pos = content.find('$');
    while (next_pos != std::string::npos && next_pos < content.size()) {
        std::size_t end_pos = content.find_first_not_of(
            "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-",
            next_pos + 1);
        references.push_back(content.substr(next_pos, end_pos - next_pos));
        next_pos = content.find('$', end_pos);
    }
    add(id, title, content, references);
}

/**
 * @brief Extracts all the text annotations from a directory. Returns
 * std::nullopt if it's a file
 *
 * @param root Root directory
 * @return Vector of all text annotations if root is a directory, null otherwise
 */
std::optional<std::vector<lect::TextAnnotation>>
extract_text_annotations(const std::filesystem::path &root) {
    using namespace std::filesystem;
    std::vector<TextAnnotation> annotations;

    if (!is_directory(root)) {
        return std::nullopt;
    }

    std::mutex mutex;
    auto add = [&annotations, &mutex](std::string id, std::string title,
                                      std::string content,
                                      std::vector<std::string> references) {
        const std::lock_guard<std::mutex> lock_guard(mutex);
        annotations.emplace_back(id, title, content, references);
    };

    extract_text_annotations_inner(root, add);

    return {annotations};
}
} // namespace lect
