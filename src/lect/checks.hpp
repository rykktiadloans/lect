#pragma once

#include "structures.hpp"
#include <algorithm>
#include <cstdint>
#include <iterator>
#include <stdexcept>
#include <string>
#include <unordered_map>
namespace lect {

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

    /**
     * @brief Function that checks whether there are any cycles of annotation
     * references
     *
     * @param text_annotations Text annotations to check
     * @param code_annotations Code annotations to check
     */
    virtual void check(std::vector<TextAnnotation> &text_annotations,
                       std::vector<CodeAnnotation>
                           &code_annotations) noexcept(false) override {
        std::unordered_map<std::string, int> text_referenced;
        for (const auto &text_annotation : text_annotations) {
            text_referenced.insert({text_annotation.id, 0});
        }
        for (const auto &code_annotation : code_annotations) {
            text_referenced.insert({code_annotation.id, 0});
        }

        for (const auto &text_annotation : text_annotations) {
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

        for (const auto &text_annotation : text_annotations) {
            text_annotation_map.insert({text_annotation.id, text_annotation});
        }

        for (const auto &root : roots) {
            iter(root, text_annotation_map, {});
        }

        next(text_annotations, code_annotations);
    }

  private:
    void
    iter(std::string current,
         std::unordered_map<std::string, TextAnnotation> &text_annotation_map,
         std::vector<std::string> prev) noexcept(false) {
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
        for (const auto &ref : a.references) {
            iter(ref, text_annotation_map, new_prev);
        }
    }
};

/**
 * @class DuplicateChecker
 * @brief Check for any duplicate annotations
 *
 */
struct DuplicateChecker : public Checker {
    /**
     * @brief A destructor
     */
    virtual ~DuplicateChecker() override{};

    /**
     * @brief A function that checks for any duplicate annotations
     *
     * @param text_annotations Text annotations to check
     * @param code_annotations Code annotations to check
     * @throw lect::Exception
     */
    virtual void check(std::vector<TextAnnotation> &text_annotations,
                       std::vector<CodeAnnotation>
                           &code_annotations) noexcept(false) override {
        std::vector<std::string> ids;

        for (const auto &an : text_annotations) {
            ids.push_back(an.id);
        }

        for (const auto &an : code_annotations) {
            ids.push_back(an.id);
        }

        for (const auto &an : text_annotations) {
            for (const auto &ref : an.references) {
                auto it = std::find(ids.begin(), ids.end(), ref);
                if (it == ids.end()) {
                    throw Exception("Annotation `" + ref + "` doesn't exist");
                }
            }
        }
        next(text_annotations, code_annotations);
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

    /**
     * @brief A function that checks whether all annotation IDs contain only allowed characters
     *
     * @param text_annotations Text annotations to check
     * @param code_annotations Code annotations to check
     */
    virtual void check(std::vector<TextAnnotation> &text_annotations,
                       std::vector<CodeAnnotation>
                           &code_annotations) noexcept(false) override {
        for (const auto &an : text_annotations) {
            uint64_t p = an.id.find_first_not_of(
                "abcdefghijklmnopqrstuvwxyz-ABCDEFGHIJKLMNOPQRSTUVWXYZ");
            if (p != std::string::npos) {
                throw Exception(an.id + " isn't a valid id. Only latin letters "
                                        "and hyphens are allowed");
            }
        }

        for (const auto &an : code_annotations) {
            uint64_t p = an.id.find_first_not_of(
                "abcdefghijklmnopqrstuvwxyz-ABCDEFGHIJKLMNOPQRSTUVWXYZ");
            if (p != std::string::npos) {
                throw Exception(an.id + " isn't a valid id. Only latin letters "
                                        "and hyphens are allowed");
            }
        }
        next(text_annotations, code_annotations);
    }
};
} // namespace lect
