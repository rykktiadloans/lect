/**
 * @file
 * @brief A collection of functions used to output the documentation
 */

#pragma once

#include "nlohmann/json.hpp"
#include "nlohmann/json_fwd.hpp"
#include "structures.hpp"
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

} // namespace lect
