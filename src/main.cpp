#include <fstream>
#include <iostream>
#include <vector>

#include "export.hpp"
#include "extract.hpp"
#include "structures.hpp"

int main(int argc, char **argv) {
    lect::Settings settings(argc, argv);
    std::vector<lect::TextAnnotation> text_annotations;
    try {
        text_annotations =
            lect::extract_text_annotations(settings.text_annotation_path);
    } catch (lect::Exception e) {
        if (true) {
            return 1;
        }
    }

    std::vector<lect::CodeAnnotation> code_annotations;
    try {
        code_annotations = lect::extract_code_annotations(
            settings.code_annotation_path, settings.language);
    } catch (lect::Exception e) {
        if (true) {
            return 1;
        }
    }

    auto dict = lect::annotations_to_json(text_annotations, code_annotations);
    std::ofstream file(settings.output_path);
    file << dict.dump();
    file.close();

    return 0;
}
