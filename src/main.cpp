#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

#include "checks.hpp"
#include "export.hpp"
#include "extract.hpp"
#include "settings.hpp"
#include "structures.hpp"
#include "vis_js.hpp"

//$main-src Main function
int main(int argc, char **argv) {

    // We'll need this one
    assert(std::string(vis_js).size() == 688913);

    std::unique_ptr<lect::Settings> settings;
    try {
        settings = std::make_unique<lect::Settings>(argc, argv);
    } catch (lect::Exception e) {
        if (std::string(e.what()) == "help") {
            return 0;
        }
        std::cout << lect::color_red + "ERROR: " + lect::color_reset + e.what()
                  << "\n";
        return 1;
    }

    std::vector<lect::TextAnnotation> text_annotations;
    try {
        text_annotations =
            lect::extract_text_annotations(settings->text_annotation_path);
    } catch (lect::Exception e) {
        if (true) {
            return 1;
        }
    }

    std::vector<lect::CodeAnnotation> code_annotations;
    try {
        code_annotations = lect::extract_code_annotations(
            settings->code_annotation_path, settings->language);
    } catch (lect::Exception e) {
        if (true) {
            return 1;
        }
    }

    try {
        settings->checker->check(text_annotations, code_annotations);
    } catch (lect::Exception e) {
        std::cout << lect::color_red + "ERROR: " + lect::color_reset + e.what()
                  << "\n";
        return 1;
    }

    auto dict = lect::annotations_to_json(text_annotations, code_annotations);

    try {
        lect::export_to_dir(settings->output_path, dict);
    } catch (lect::Exception e) {
        std::cout << lect::color_red + "ERROR: " + lect::color_reset + e.what()
                  << "\n";
        return 1;
    }

    return 0;
}
