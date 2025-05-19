#include "preprocessing.hpp"
#include "nlohmann/json.hpp"
#include "structures.hpp"
#include <cstdint>
#include <filesystem>
#include <functional>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

lect::PrepocessingBuilder &
lect::PrepocessingBuilder::add_direction(std::string direction) {
    auto prev = _json_preprocessing;
    _json_preprocessing = [prev, direction](nlohmann::json &dict) {
        auto new_dict = prev(dict);
        return _add_direction(new_dict, direction);
    };
    return *this;
}

lect::PrepocessingBuilder &
lect::PrepocessingBuilder::set_lineup(std::string lineup) {
    auto prev = _json_preprocessing;
    _json_preprocessing = [prev, lineup](nlohmann::json &dict) {
        auto new_dict = prev(dict);
        return _set_lineup(new_dict, lineup);
    };
    return *this;
}

lect::PrepocessingBuilder &lect::PrepocessingBuilder::add_jetbrains_flag() {
    auto prev = _json_preprocessing;
    _json_preprocessing = [prev](nlohmann::json &dict) {
        auto new_dict = prev(dict);
        return _add_jetbrains_flag(new_dict);
    };
    return *this;
}

lect::PrepocessingBuilder &
lect::PrepocessingBuilder::remove_code_annotations_middle() {
    auto prev = _annotations_preprocessing;
    _annotations_preprocessing = [prev](Annotations &annotations) {
        _remove_code_annotations_middle(annotations);
        prev(annotations);
    };
    return *this;
}

lect::Preprocessing lect::PrepocessingBuilder::build() {
    auto json_preprocessing = _json_preprocessing;
    auto annotations_preprocessing = _annotations_preprocessing;
    std::function<nlohmann::json(Annotations &)> function =
        [json_preprocessing,
         annotations_preprocessing](Annotations &annotations) {
            annotations_preprocessing(annotations);
            auto dict = _annotations_to_json(annotations);
            return json_preprocessing(dict);
        };
    return Preprocessing(function);
}

nlohmann::json lect::PrepocessingBuilder::_annotations_to_json(
    const Annotations &annotations) {
    using namespace nlohmann;

    auto connections = _get_connected(annotations);

    json dict = {{"text_annotations", json::array()},
                 {"code_annotations", json::array()}};

    for (const auto &a : annotations.text_annotations) {
        json t = {
            {"id", a.id},
            {"title", a.title},
            {"content", a.content},
            {"connected_to", connections.at(a.id)},
            {"references", std::set(a.references.begin(), a.references.end())}};
        dict["text_annotations"].push_back(t);
    }

    for (const auto &a : annotations.code_annotations) {
        json t = {{"id", a.id},
                  {"title", a.title},
                  {"content", a.content},
                  {"file", a.file},
                  {"full_file", std::filesystem::canonical(a.file).string()},
                  {"line", a.line},
                  {"connected_to", connections.at(a.id)}};
        dict["code_annotations"].push_back(t);
    }

    return dict;
}

std::unordered_map<std::string, std::set<std::string>>
lect::PrepocessingBuilder::_get_connected(const Annotations &annotations) {
    std::unordered_map<std::string, std::set<std::string>> connections;
    std::unordered_map<std::string, std::set<std::string>> references;
    std::unordered_map<std::string, int> reference_count;
    for (const auto &a : annotations.text_annotations) {
        connections.insert({a.id, {}});
        references.insert(
            {a.id, std::set(a.references.begin(), a.references.end())});
        reference_count.insert({a.id, 0});
    }

    for (const auto &a : annotations.code_annotations) {
        connections.insert({a.id, {}});
        references.insert({a.id, {}});
        reference_count.insert({a.id, 0});
    }

    for (const auto &annotations_references : references) {
        for (const auto &ref : annotations_references.second) {
            reference_count.at(ref)++;
        }
    }
    std::vector<std::string> roots;
    for (const auto &[id, count] : reference_count) {
        if (count == 0) {
            roots.push_back(id);
        }
    }

    for (const auto &root : roots) {
        _get_connected_iter(root, connections, references, {});
    }

    return connections;
}

void lect::PrepocessingBuilder::_get_connected_iter(
    std::string node,
    std::unordered_map<std::string, std::set<std::string>> &connections,
    const std::unordered_map<std::string, std::set<std::string>> &references,
    std::set<std::string> prev) {

    prev.insert(node);
    for (auto &p : prev) {
        std::set<std::string> &con = connections.at(p);
        con.insert(prev.begin(), prev.end());
    }
    for (const auto &ref : references.at(node)) {
        _get_connected_iter(ref, connections, references, prev);
    }
}

nlohmann::json
lect::PrepocessingBuilder::_add_direction(nlohmann::json &dict,
                                          const std::string &dir) {
    dict["dir"] = dir;
    return dict;
}

nlohmann::json
lect::PrepocessingBuilder::_set_lineup(nlohmann::json &dict,
                                       const std::string &lineup) {
    dict["shake"] = lineup;
    return dict;
}

nlohmann::json
lect::PrepocessingBuilder::_add_jetbrains_flag(nlohmann::json &dict) {
    dict["jetbrains"] = true;
    return dict;
}

void lect::PrepocessingBuilder::_remove_code_annotations_middle(
    Annotations &annotations) {
    for (auto &annotation : annotations.code_annotations) {
        uint64_t first_newline = annotation.content.find_first_of("\n");
        uint64_t last_newline = annotation.content.find_last_of("\n");
        if (first_newline == std::string::npos ||
            last_newline == std::string::npos) {
            continue;
        }
        annotation.content =
            annotation.content.substr(0, first_newline + 1) + "  ..." +
            annotation.content.substr(last_newline,
                                      annotation.content.size() - last_newline);
    }
}
