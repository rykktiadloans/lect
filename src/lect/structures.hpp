/**
 * @file
 * @brief A collection of data structures used in other parts of the application
 */

#pragma once

#include "tree-sitter-cpp.h"
#include "tree_sitter/api.h"
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace lect {

/**
 * @brief Reset the color of the standard output
 */
std::string color_reset = "\x1B[0m";

/**
 * @brief Color the standard output red
 */
std::string color_red = "\x1B[31m";

/**
 * @brief Color the standard output green
 */
std::string color_green = "\x1B[32m";

/**
 * @brief Color the standard output yellow
 */
std::string color_yellow = "\x1B[33m";

/**
 * @brief Color the standard output blue
 */
std::string color_blue = "\x1B[34m";

/**
 * @brief Color the standard output magenta
 */
std::string color_magenta = "\x1B[35m";

/**
 * @brief Color the standard output cyan
 */
std::string color_cyan = "\x1B[36m";

/**
 * @brief Color the standard output white
 */
std::string color_white = "\x1B[37m";

/**
 * @class TextAnnotation
 * @brief A representation of a text annotation
 *
 */
struct TextAnnotation {
    std::string id;
    std::string title;
    std::string content;
    std::vector<std::string> references;

    TextAnnotation(std::string id, std::string title, std::string content,
                   std::vector<std::string> references)
        : id(id), title(title), content(content), references(references) {}

    TextAnnotation() {}
};

/**
 * @class CodeAnnotation
 * @brief A representation of a code annotation and its position in the source
 *
 */
struct CodeAnnotation {
    std::string id;
    std::string title;
    std::string content;
    std::string file;
    int line;

    CodeAnnotation(std::string id, std::string title, std::string content,
                   std::string file, int line)
        : id(id), title(title), content(content), file(file), line(line) {}
};

/**
 * @class Annotations
 * @brief A class that encapsulates the annotations
 *
 */
//$annotations-src Annotations class
struct Annotations {
    std::vector<TextAnnotation> text_annotations;
    std::vector<CodeAnnotation> code_annotations;
};

/**
 * @class Exception
 * @brief Custom exception class for this application
 *
 */
class Exception : public std::exception {
  public:
    /**
     * @brief C string constructor
     *
     * @param message Message string
     */
    explicit Exception(const char *message) : m_message(message) {}

    /**
     * @brief STL string constructor
     *
     * @param message Message string
     */
    explicit Exception(const std::string &message) : m_message(message) {}

    /**
     * @brief Destructor
     */
    virtual ~Exception() noexcept {}

    /**
     * @brief Returns the error message as a C string. Should not be freed
     *
     * @return Message
     */
    virtual const char *what() const noexcept { return m_message.c_str(); }

  private:
    std::string m_message;
};

/**
 * @class CaptureValidator
 * @brief An abstract class that defines an interface for other validator
 * objects, which can be used to validate comment and object captures
 *
 */
//$capture-validator-src Capture validator interface
struct CaptureValidator {
    /**
     * @brief Destructor
     */
    virtual ~CaptureValidator(){};
    /**
     * @brief Validate that a particular comment is a potential code annotation
     *
     * @param string Comment to validate
     * @return true if it is correct, false otherwise
     */
    virtual bool validate_comment(std::string string) = 0;
    /**
     * @brief Validate that a particular object isn't a comment
     *
     * @param string object to validate
     * @return true if it is correct, false otherwise
     */
    virtual bool validate_object(std::string string) = 0;
};

/**
 * @class CSyntaxValidator
 * @brief A class that can be used for validating in languages with C-style
 * syntax
 *
 */
struct CSyntaxValidator : public CaptureValidator {
    /**
     * @brief Make sure the comment is a one-line comment with a dollar after
     * the token
     *
     * @param string Comment to validate
     * @return true if it is correct, false otherwise
     */
    virtual bool validate_comment(std::string string) override {
        uint64_t begin = string.find_first_not_of("\n ");
        if (begin == std::string::npos) {
            return false;
        }

        std::string comment_fragment = string.substr(begin, 2);
        if (comment_fragment != "//") {
            return false;
        }

        uint64_t ptr = begin + 2;
        while (string.at(ptr) == ' ' || string.at(ptr) == '\n') {
            ptr++;
        }

        if (string.at(ptr) != '$') {
            return false;
        }

        return true;
    }

    /**
     * @brief Make sure that an object isn't a comment
     *
     * @param string String to validate
     * @return true if it is correct, false otherwise
     */
    virtual bool validate_object(std::string string) override {
        uint64_t begin = string.find_first_not_of("\n ");
        if (begin == std::string::npos) {
            return false;
        }

        std::string comment_fragment = string.substr(begin, 2);
        if (comment_fragment == "//" || comment_fragment == "/*") {
            return false;
        }

        return true;
    }
};

/**
 * @class Language
 * @brief A class that represents all the language-dependent data for extracting
 * source code annotations
 *
 */
//$language-src Language class
struct Language {
    std::string name;
    std::vector<std::string> extensions;
    std::string query;
    const TSLanguage *language{nullptr};
    std::unique_ptr<CaptureValidator> validator{nullptr};

    /**
     * @brief Generates an object suited for C++ parsing
     *
     * @return Language pack that can be user for C++
     */
    //$language-cpp-src C++ language object builder
    static Language cpp() {
        std::vector<std::string> extensions{".c", ".cpp", ".h", ".hpp"};
        return Language("c++", extensions,
                        "((comment) @comment . (comment)* . (_) @object)",
                        tree_sitter_cpp(),
                        std::make_unique<CSyntaxValidator>());
    }

    /**
     * @brief Get a placeholder language object
     *
     * @return Placeholder
     */
    //$language-placeholder-src Language placeholder
    static Language placeholder() {
        return Language("", std::vector<std::string>(), "", nullptr, nullptr);
    }

  private:
    /**
     * @brief The constructor for the language.
     *
     * @param name Name of the language
     * @param extensions File extensions for the languages
     * @param query Query for the data
     * @param language Language object for parsing
     * appropriate comment
     * @param validator Validator object that can be used to validate captures
     * a comment
     */
    Language(const std::string name, const std::vector<std::string> &extensions,
             const std::string query, const TSLanguage *language,
             std::unique_ptr<CaptureValidator> validator)
        : name(name), extensions(extensions), query(query), language(language),
          validator(std::move(validator)) {}
};

} // namespace lect
