/**
 * @file checks.hpp
 * @brief A collection of classes that can analyze and check the annotations for
 * errors
 */

#pragma once

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
namespace lect {

/**
 * @class Checker
 * @brief An base class that implements a handler that checks the
 * text and code annotations for any problems
 *
 */
//$checker-src Checker class
struct Checker {

    /**
     * @brief Check text and code annotations for any errors. If there aren't
     * any, pass them to the next checker
     *
     * @param annotations Annotations to check
     * @throw lect::Exception
     */
    void check(const Annotations &annotations) noexcept(false) {
        _check(annotations);
        if (m_next.has_value()) {
            m_next->get()->check(annotations);
        }
    };

    /**
     * @brief A virtual destructor for subclassing
     */
    virtual ~Checker(){};

    /**
     * @brief A function that adds a new checker to the end of the chain
     *
     * @tparam T Type of the checker
     */
    void add(std::unique_ptr<Checker> checker) {
        if (!m_next.has_value()) {
            m_next = std::move(checker);
            return;
        }
        m_next->get()->add(std::move(checker));
    }

  private:
    std::optional<std::unique_ptr<Checker>> m_next = std::nullopt;

    /**
     * @brief A virtual function that can be overridden by subclasses to provide
     * specific checks
     *
     * @param annotations Annotations to check
     */
    virtual void _check(const Annotations &annotations) noexcept(false) = 0;
};
/**
 * @class CycleChecker
 * @brief A Checker that checks whether there are any cycles of annotation
 * references. If there is, throws an exception.
 *
 */
struct CycleChecker : public Checker {
    /**
     * @brief A destructor
     */
    virtual ~CycleChecker() override{};

  private:
    /**
     * @brief Function that checks whether there are any cycles of annotation
     * references
     *
     * @param annotations Annotations to check
     */
    virtual void
    _check(const Annotations &annotations) noexcept(false) override {
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
        std::set_difference(
            all_reference_ids.begin(), all_reference_ids.end(),
            total_prev.begin(), total_prev.end(),
            std::inserter(potentially_completely_cyclical,
                          potentially_completely_cyclical.begin()));

        for (const auto &annotation : potentially_completely_cyclical) {
            _iter(annotation, text_annotation_map, {}, total_prev);
        }
    }

    void
    _iter(std::string current,
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
        } catch (std::out_of_range e) {
            return;
        }
        std::vector<std::string> new_prev(prev);
        new_prev.push_back(current);
        total_prev.insert(current);
        for (const auto &ref : a.references) {
            _iter(ref, text_annotation_map, new_prev, total_prev);
        }
    }
};

/**
 * @class NonexistentChecker
 * @brief Check for any annotations that reference nonexistent annotations
 *
 */
struct NonexistentChecker : public Checker {
    /**
     * @brief A destructor
     */
    virtual ~NonexistentChecker() override{};

  private:
    /**
     * @brief A function that checks for any annotations that reference
     * nonexistent annotations
     *
     * @param annotations Annotations to check
     * @throw lect::Exception
     */
    virtual void
    _check(const Annotations &annotations) noexcept(false) override {
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
};

/**
 * @class IdAllowedSymbolsChecker
 * @brief Check whether all annotation ID contain only allowed characters
 *
 */
struct IdAllowedSymbolsChecker : public Checker {
    /**
     * @brief A destructor
     */
    virtual ~IdAllowedSymbolsChecker() override{};

  private:
    /**
     * @brief A function that checks whether all annotation IDs contain only
     * allowed characters
     *
     * @param annotations Annotations to check
     */
    virtual void
    _check(const Annotations &annotations) noexcept(false) override {
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
};

/**
 * @class DuplicateChecker
 * @brief Checks whether all annotations have unique IDs (no duplicates)
 *
 */
struct DuplicateChecker : public Checker {
    /**
     * @brief A destructor
     */
    virtual ~DuplicateChecker() override{};

    /**
     * @brief A function that checks whether all annotations have unique IDs
     *
     * @param annotations Annotations to check
     */
    virtual void
    _check(const Annotations &annotations) noexcept(false) override {
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
};

/**
 * @class CodeAnnotationsSuffixChecker
 * @brief An optional checker that makes sure that all code annotations have a
 * specified suffix
 *
 */
struct CodeAnnotationsSuffixChecker : public Checker {

    /**
     * @brief Virtual destructor
     */
    virtual ~CodeAnnotationsSuffixChecker() {}

    /**
     * @brief A constructor
     *
     * @param suffix The suffix with which to check
     */
    CodeAnnotationsSuffixChecker(const std::string suffix) : _suffix(suffix) {}

  private:
    const std::string _suffix;

    virtual void
    _check(const Annotations &annotations) noexcept(false) override {
        for (const auto &annotation : annotations.code_annotations) {
            const std::string id = annotation.id;
            if (_suffix.size() > id.size() ||
                _suffix != id.substr(id.size() - _suffix.size())) {
                throw Exception(
                    "Code annotation with ID " + color_blue + "'" + id + "'" +
                    color_reset + " doesn't have suffix " + color_yellow + "'" +
                    _suffix + "'" + color_reset + ", which was supplied with " +
                    color_green + "-suf" + color_reset + " argument");
            }
        }
    }
};

} // namespace lect
