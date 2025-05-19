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
    void check(const Annotations &annotations) noexcept(false);

    /**
     * @brief A virtual destructor for subclassing
     */
    virtual ~Checker(){};

    /**
     * @brief A function that adds a new checker to the end of the chain
     *
     * @tparam T Type of the checker
     */
    void add(std::unique_ptr<Checker> checker);

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
    virtual void
    _check(const Annotations &annotations) noexcept(false) override;

    void
    _iter(std::string current,
          std::unordered_map<std::string, TextAnnotation> &text_annotation_map,
          std::vector<std::string> prev,
          std::set<std::string> &total_prev) noexcept(false); 
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
    _check(const Annotations &annotations) noexcept(false) override;
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
    _check(const Annotations &annotations) noexcept(false) override;
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
    _check(const Annotations &annotations) noexcept(false) override; 
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
    _check(const Annotations &annotations) noexcept(false) override; 
};

} // namespace lect
