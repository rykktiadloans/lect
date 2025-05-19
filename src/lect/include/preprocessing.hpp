#pragma once

#include "nlohmann/json.hpp"
#include "structures.hpp"
#include <functional>
#include <set>
#include <string>
#include <unordered_map>

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

    Preprocessing() {}

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
     * @brief Adds a step that adds a direction of the tree to the final JSON
     * file
     *
     * @param direction The direction string, should be either "UD", "DU", "RL"
     * or "LR"
     * @return The reference to the builder
     */
    PrepocessingBuilder &add_direction(std::string direction); 

    PrepocessingBuilder &set_lineup(std::string lineup); 

    PrepocessingBuilder &add_jetbrains_flag(); 

    PrepocessingBuilder &remove_code_annotations_middle();

    /**
     * @brief Resolves and builds the final preprocessing object
     *
     * @return The final preprocessing object
     */
    Preprocessing build(); 

  private:
    std::function<nlohmann::json(nlohmann::json &)> _json_preprocessing =
        [](nlohmann::json &dict) { return dict; };

    std::function<void(Annotations &)> _annotations_preprocessing =
        [](Annotations &) {};

    static nlohmann::json _annotations_to_json(const Annotations &annotations);

    /**
     * @brief Gets a mapping between a node and a set of nodes connected to it
     *
     * @param annotations Annotations
     * @return Map of nodes and sets of nodes connected to them
     */
    static std::unordered_map<std::string, std::set<std::string>>
    _get_connected(const Annotations &annotations);

    static void _get_connected_iter(
        std::string node,
        std::unordered_map<std::string, std::set<std::string>> &connections,
        const std::unordered_map<std::string, std::set<std::string>>
            &references,
        std::set<std::string> prev);

    static nlohmann::json _add_direction(nlohmann::json &dict,
                                         const std::string &dir); 

    static nlohmann::json _set_lineup(nlohmann::json &dict,
                                      const std::string &lineup);

    static nlohmann::json _add_jetbrains_flag(nlohmann::json &dict); 

    static void _remove_code_annotations_middle(Annotations &annotations); 

};
} // namespace lect
