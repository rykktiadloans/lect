#pragma once

#include "structures.hpp"
#include <algorithm>
#include <cstdint>
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
struct Checker {

    /**
     * @brief Check text and code annotations for any errors
     *
     * @param text_annotations Vector of text annotations
     * @param code_annotations Vector of code annotations
     * @throw lect::Exception
     */
    virtual void
    check(std::vector<TextAnnotation> &text_annotations,
          std::vector<CodeAnnotation> &code_annotations) noexcept(false) = 0;

    /**
     * @brief A virtual destructor for subclassing
     */
    virtual ~Checker(){};

    /**
     * @brief A function that adds a new checker to the end of the chain
     *
     * @tparam T Type of the checker
     */
    template <typename T> void add() {
        if (!m_next.has_value()) {
            m_next = std::make_unique<T>();
            return;
        }
        m_next->get()->add<T>();
    }

    /**
     * @brief Call the next checker in the sequence, if there is any
     *
     * @param text_annotations Vector of text annotations to use on the next
     * check call
     * @param code_annotations Vector of code annotations to use on the next
     * check call
     * @throw lect::Exception
     */
    void next(std::vector<TextAnnotation> &text_annotations,
              std::vector<CodeAnnotation> &code_annotations) noexcept(false) {
        if (!m_next.has_value()) {
            return;
        }
        m_next->get()->check(text_annotations, code_annotations);
    }

  private:
    std::optional<std::unique_ptr<Checker>> m_next = std::nullopt;
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
 * @class NonexistentChecker
 * @brief Check for any annotations that reference nonexistent annotations
 *
 */
struct NonexistentChecker : public Checker {
    /**
     * @brief A destructor
     */
    virtual ~NonexistentChecker() override{};

    /**
     * @brief A function that checks for any annotations that reference
     * nonexistent annotations
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
     * @brief A function that checks whether all annotation IDs contain only
     * allowed characters
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
     * @param text_annotations Text Annotations to check
     * @param code_annotations Code annotations to check
     */
    virtual void check(std::vector<TextAnnotation> &text_annotations,
                       std::vector<CodeAnnotation>
                           &code_annotations) noexcept(false) override {
        std::set<std::string> id_set;

        for (const auto &annotation : text_annotations) {
            if (id_set.find(annotation.id) != id_set.end()) {
                throw Exception("There are at least 2 annotations with ID " +
                                annotation.id);
            }
            id_set.insert(annotation.id);
        }

        for (const auto &annotation : code_annotations) {
            if (id_set.find(annotation.id) != id_set.end()) {
                throw Exception("There are at least 2 annotations with ID " +
                                annotation.id);
            }
            id_set.insert(annotation.id);
        }
    }
};

} // namespace lect
