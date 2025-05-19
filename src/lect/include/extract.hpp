/**
 * @file
 * @brief A collection of functions that extract the annotations from source
 * code and annotation files, and convert them into usable data structures
 */

#pragma once

#include "structures.hpp"
#include <filesystem>
#include <tree-sitter-cpp.h>

namespace lect {

//$annotations-builder-src Annotations builder
/**
 * @class AnnotationsBuilder
 * @brief A builder class for assembling an annotation object
 *
 */
struct AnnotationsBuilder {

    /**
     * @brief A function that extracts all code annotations from the
     * file/directory
     *
     * @param root Path in which to look for annotations
     * @param language Language object
     * @return This builder (for chaining purposes)
     * @throw lect::Exception
     */
    AnnotationsBuilder &
    extract_code_annotations(const std::filesystem::path &root,
                             const Language &language) noexcept(false); 

    /**
     * @brief Finds all the text annotations in a directory and returns them.
     * Throws an exception if the path isn't a directory
     *
     * @param root Root directory of the annotations
     * @return This builder (for chaining purposes)
     * @throw lect:Exception
     */
    AnnotationsBuilder &extract_text_annotations(
        const std::filesystem::path &root) noexcept(false); 

    /**
     * @brief Returns the assembled annotations
     *
     * @return Annotations
     */
    Annotations get_annotations() { return _annotations; }

  private:
    Annotations _annotations;

    /**
     * @brief An inner function that extracts code annotations from a file, or
     * looks for other files in the directory
     *
     * @tparam F function type for adding a code annotation to an array
     * @param path Path of the current file
     * @param language Language object
     * @param add Function that adds a code annotation to an array
     * @throw lect::Exception
     */
    template <typename F>
    void _extract_code_annotations_inner(const std::filesystem::path &path,
                                         const Language &language,
                                         F &add) noexcept(false); 

    /**
     * @brief Extracts an annotation from a path if it is a .an file, or looks
     * inside the directory at path
     *
     * @tparam F Function type
     * @param path Path to the file
     * @param add Function that adds an annotation to a container
     * @throw lect::Exception
     */
    template <typename F>
    void _extract_text_annotations_inner(const std::filesystem::path &path,
                                         F &add) noexcept(false); 
};

} // namespace lect
