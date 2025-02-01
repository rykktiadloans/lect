#include <iostream>

#include "extract.hpp"

int main() {
    auto text_annotations = lect::extract_text_annotations("/home/rykktiadloans/Coding/lect/annotations/");
    if(text_annotations.has_value()) {
        for(auto i : text_annotations.value()) {
            std::cout << i.id << " " << i.title << " " << i.references.size() << "\n";
        }
    }
    else {
        std::cout << "No value?\n";
    }
    return 0;
}
