#pragma once

#include "nlohmann/json.hpp"
#include "nlohmann/json_fwd.hpp"
#include "structures.hpp"
#include <functional>
#include <iostream>
#include <string>

namespace lect {

//$preprocessing-src Preprocessing class
/**
 * @class Preprocessing
 * @brief Preprocessing class is a wrapper over the preprocessing function that
 * converts and modifies annotations into a JSON document
 *
 */
struct Preprocessing {
    /**
     * @brief A constructor that initializes the function
     *
     * @param preprocess The preprocessing function
     */
    Preprocessing(std::function<nlohmann::json(Annotations &)> preprocess)
        : _preprocess(preprocess) {}

    /**
     * @brief Call the stored preprocessing function
     *
     * @param annotations Annotations to preprocess
     * @return Final JSON document
     */
    nlohmann::json preprocess(Annotations &annotations) {
        return _preprocess(annotations);
    }

  private:
    std::function<nlohmann::json(Annotations &)> _preprocess;
};

//$preprocessing-builder-src PreprocessingBuilder class
/**
 * @class PrepocessingBuilder
 * @brief A builder for the Preprocessing class
 *
 */
struct PrepocessingBuilder {
    /**
     * @brief Adds a step that adds a direction of the tree to the final JSON file
     *
     * @param direction The direction string, should be either "UD", "DU", "RL" or "LR"
     * @return The reference to the builder
     */
    PrepocessingBuilder &add_direction(std::string direction) {
        auto prev = _json_preprocessing;
        _json_preprocessing = [prev, direction](nlohmann::json &dict) {
            auto new_dict = prev(dict);
            return _add_direction(new_dict, direction);
        };
        return *this;
    }

    /**
     * @brief Resolves and builds the final preprocessing object
     *
     * @return The final preprocessing object
     */
    Preprocessing build() {
        auto json_preprocessing = _json_preprocessing;
        std::function<nlohmann::json(Annotations &)> function =
            [json_preprocessing](Annotations &annotations) {
                auto dict = _annotations_to_json(annotations);
                return json_preprocessing(dict);
            };
        return Preprocessing(function);
    }

  private:
    std::function<nlohmann::json(nlohmann::json &)> _json_preprocessing =
        [](nlohmann::json &dict) { return dict; };

    static nlohmann::json _annotations_to_json(const Annotations &annotations) {
        using namespace nlohmann;

        json dict = {{"text_annotations", json::array()},
                     {"code_annotations", json::array()}};

        for (const auto &a : annotations.text_annotations) {
            json t = {{"id", a.id},
                      {"title", a.title},
                      {"content", a.content},
                      {"references", json::array()}};
            for (const auto &ref : a.references) {
                t["references"].push_back(ref);
            }
            dict["text_annotations"].push_back(t);
        }

        for (const auto &a : annotations.code_annotations) {
            json t = {{"id", a.id},
                      {"title", a.title},
                      {"content", a.content},
                      {"file", a.file},
                      {"line", a.line}};
            dict["code_annotations"].push_back(t);
        }

        return dict;
    }

    static nlohmann::json _add_direction(nlohmann::json &dict,
                                         const std::string &dir) {
        dict["dir"] = dir;
        return dict;
    }
};
} // namespace lect
