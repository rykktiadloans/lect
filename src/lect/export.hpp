/**
 * @file
 * @brief A collection of functions used to output the documentation
 */

#pragma once

#include "index_html.hpp"
#include "nlohmann/json.hpp"
#include "nlohmann/json_fwd.hpp"
#include "script_js.hpp"
#include "structures.hpp"
#include "vis_js.hpp"
#include <filesystem>
#include <fstream>
#include <vector>

namespace lect {

/**
 * @brief A function that takes in the text and code annotations and makes a
 * JSON document out of them
 *
 * @param text_annotations A vector of text annotations
 * @param code_annotations A vector of code annotations
 * @return
 */
//$json-src JSON export
nlohmann::json
annotations_to_json(const std::vector<TextAnnotation> &text_annotations,
                    const std::vector<CodeAnnotation> &code_annotations) {
    using namespace nlohmann;

    json dict = {{"text_annotations", json::array()},
                 {"code_annotations", json::array()}};

    for (const auto &a : text_annotations) {
        json t = {{"id", a.id},
                  {"title", a.title},
                  {"content", a.content},
                  {"references", json::array()}};
        for (const auto &ref : a.references) {
            t["references"].push_back(ref);
        }
        dict["text_annotations"].push_back(t);
    }

    for (const auto &a : code_annotations) {
        json t = {{"id", a.id},
                  {"title", a.title},
                  {"content", a.content},
                  {"file", a.file},
                  {"line", a.line}};
        dict["code_annotations"].push_back(t);
    }

    return dict;
}

/**
 * @brief Generate the documentation at the directory at path using the
 * annotations in the JSON document
 *
 * @param path Path at which to create the documentation
 * @param json JSON that contains the annotations
 */
//$export-src Export the documentation
void export_to_dir(const std::filesystem::path &path,
                   const nlohmann::json &json) noexcept(false) {

    if (!std::filesystem::exists(path)) {
        try {
            std::filesystem::create_directory(path);
        } catch (std::filesystem::filesystem_error e) {
            throw Exception("File `" + path.string() +
                            "` already exists, but it needs to be a directory");
        }
    }

    std::ofstream write_file(path / "annotations.js");
    write_file << "const annotationsJSON = " + json.dump();
    write_file.close();

    write_file.open(path / "index.html");
    write_file << index_html;
    write_file.close();

    write_file.open(path / "vis-network.min.js");
    write_file << vis_js;
    write_file.close();

    write_file.open(path / "script.js");
    write_file << script_js;
    write_file.close();
}

} // namespace lect
