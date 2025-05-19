/**
 * @file checks.hpp
 * @brief A collection of classes that can analyze and check the annotations for
 * errors
 */

#include "checks.hpp"
#include "structures.hpp"
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <memory>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <unordered_map>

void lect::Checker::check(const Annotations &annotations) noexcept(false) {
    _check(annotations);
    if (m_next.has_value()) {
        m_next->get()->check(annotations);
    }
};

void lect::Checker::add(std::unique_ptr<Checker> checker) {
    if (!m_next.has_value()) {
        m_next = std::move(checker);
        return;
    }
    m_next->get()->add(std::move(checker));
}

void lect::CycleChecker::_check(const Annotations &annotations) noexcept(
    false) {
    std::unordered_map<std::string, int> text_referenced;
    std::vector<std::string> all_reference_ids;
    for (const auto &text_annotation : annotations.text_annotations) {
        text_referenced.insert({text_annotation.id, 0});
        all_reference_ids.push_back(text_annotation.id);
    }
    for (const auto &code_annotation : annotations.code_annotations) {
        text_referenced.insert({code_annotation.id, 0});
        all_reference_ids.push_back(code_annotation.id);
    }

    for (const auto &text_annotation : annotations.text_annotations) {
        for (const auto &ref : text_annotation.references) {
            text_referenced.at(ref)++;
        }
    }

    std::vector<std::string> roots;
    for (const auto &[id, count] : text_referenced) {
        if (count == 0) {
            roots.push_back(id);
        }
    }
    if (roots.size() == 0) {
        throw Exception("There are no root annotations!");
    }

    std::unordered_map<std::string, TextAnnotation> text_annotation_map;

    for (const auto &text_annotation : annotations.text_annotations) {
        text_annotation_map.insert({text_annotation.id, text_annotation});
    }

    std::set<std::string> total_prev;
    for (const auto &root : roots) {
        _iter(root, text_annotation_map, {}, total_prev);
    }

    if (total_prev.size() == annotations.text_annotations.size() +
                                 annotations.code_annotations.size()) {
        return;
    }
    std::vector<std::string> potentially_completely_cyclical;
    std::set_difference(all_reference_ids.begin(), all_reference_ids.end(),
                        total_prev.begin(), total_prev.end(),
                        std::inserter(potentially_completely_cyclical,
                                      potentially_completely_cyclical.begin()));

    for (const auto &annotation : potentially_completely_cyclical) {
        _iter(annotation, text_annotation_map, {}, total_prev);
    }
}

void lect::CycleChecker::_iter(
    std::string current,
    std::unordered_map<std::string, TextAnnotation> &text_annotation_map,
    std::vector<std::string> prev,
    std::set<std::string> &total_prev) noexcept(false) {
    auto found = std::find(prev.begin(), prev.end(), current);
    if (found != prev.end()) {
        std::string m = "There is a cycle of referenced text annotations: ";
        for (const auto &a : prev) {
            m += a + " > ";
        }
        m += current;
        throw Exception(m);
    }

    TextAnnotation a;
    try {
        a = text_annotation_map.at(current);
    } catch (std::out_of_range &e) {
        return;
    }
    std::vector<std::string> new_prev(prev);
    new_prev.push_back(current);
    total_prev.insert(current);
    for (const auto &ref : a.references) {
        _iter(ref, text_annotation_map, new_prev, total_prev);
    }
}

void lect::NonexistentChecker::_check(const Annotations &annotations) noexcept(
    false) {
    std::vector<std::string> ids;

    for (const auto &an : annotations.text_annotations) {
        ids.push_back(an.id);
    }

    for (const auto &an : annotations.code_annotations) {
        ids.push_back(an.id);
    }

    for (const auto &an : annotations.text_annotations) {
        for (const auto &ref : an.references) {
            auto it = std::find(ids.begin(), ids.end(), ref);
            if (it == ids.end()) {
                throw Exception("Annotation `" + ref +
                                "` in text annotation `" + an.id +
                                "` doesn't exist");
            }
        }
    }
}

void lect::IdAllowedSymbolsChecker::_check(
    const Annotations &annotations) noexcept(false) {
    for (const auto &an : annotations.text_annotations) {
        uint64_t p = an.id.find_first_not_of(
            "abcdefghijklmnopqrstuvwxyz-ABCDEFGHIJKLMNOPQRSTUVWXYZ");
        if (p != std::string::npos) {
            throw Exception(an.id + " isn't a valid id. Only latin letters "
                                    "and hyphens are allowed");
        }
    }

    for (const auto &an : annotations.code_annotations) {
        uint64_t p = an.id.find_first_not_of(
            "abcdefghijklmnopqrstuvwxyz-ABCDEFGHIJKLMNOPQRSTUVWXYZ");
        if (p != std::string::npos) {
            throw Exception(an.id + " isn't a valid id. Only latin letters "
                                    "and hyphens are allowed");
        }
    }
}

void lect::DuplicateChecker::_check(const Annotations &annotations) noexcept(
    false) {
    std::set<std::string> id_set;

    for (const auto &annotation : annotations.text_annotations) {
        if (id_set.find(annotation.id) != id_set.end()) {
            throw Exception("There are at least 2 annotations with ID " +
                            annotation.id);
        }
        id_set.insert(annotation.id);
    }

    for (const auto &annotation : annotations.code_annotations) {
        if (id_set.find(annotation.id) != id_set.end()) {
            throw Exception("There are at least 2 annotations with ID " +
                            annotation.id);
        }
        id_set.insert(annotation.id);
    }
}

void lect::CodeAnnotationsSuffixChecker::_check(
    const Annotations &annotations) noexcept(false) {
    for (const auto &annotation : annotations.code_annotations) {
        const std::string id = annotation.id;
        if (_suffix.size() > id.size() ||
            _suffix != id.substr(id.size() - _suffix.size())) {
            throw Exception("Code annotation with ID " + color_blue + "'" + id +
                            "'" + color_reset + " doesn't have suffix " +
                            color_yellow + "'" + _suffix + "'" + color_reset +
                            ", which was supplied with " + color_green +
                            "-suf" + color_reset + " argument");
        }
    }
}
