#include <iostream>
#include <vector>

#include "annotations.hpp"
#include "extract.hpp"

int main() {
    std::vector<lect::TextAnnotation> text_annotations;
    try {
        text_annotations = lect::extract_text_annotations(
            "/home/rykktiadloans/Coding/lect/annotations");

    } catch (lect::Exception) {

    }
    for (auto i : text_annotations) {
        std::cout << i.id << " " << i.title << " " << i.references.size()
                  << "\n";
    }
    return 0;
}
