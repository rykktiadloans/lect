#include <iostream>
#include <vector>

#include "extract.hpp"
#include "structures.hpp"

int main() {
    std::vector<lect::TextAnnotation> text_annotations;
    try {
        text_annotations = lect::extract_text_annotations(
            "/home/rykktiadloans/Coding/lect/annotations");

    } catch (lect::Exception const &e) {
        std::cout << "Exception caught\n";
    }
    for (auto i : text_annotations) {
        std::cout << i.id << " " << i.title << " " << i.references.size()
                  << "\n";
    }

    std::vector<lect::CodeAnnotation> code_annotations =
        lect::extract_code_annotations("/home/rykktiadloans/Coding/lect/src",
                                       lect::Language::cpp());
    for (const auto &annotation : code_annotations) {
        std::cout << annotation.id << " " << annotation.title << " "
                  << annotation.content << " " << annotation.file << " " << annotation.line << "\n";
    }
    return 0;
}
