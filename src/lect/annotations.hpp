/**
 * @file
 * @brief A collection of data structures used to represent an annotation, as
 * well as a custom exception
 */

#pragma once

#include <string>
#include <vector>

namespace lect {

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
    std::string line;
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

} // namespace lect
