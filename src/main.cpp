#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

#include "checks.hpp"
#include "export.hpp"
#include "extract.hpp"
#include "nlohmann/json_fwd.hpp"
#include "settings.hpp"
#include "structures.hpp"
#include "vis_js.hpp"

int main(int argc, char **argv) {

    // We'll need this one
    assert(std::string(vis_js).size() == 688913);

    std::unique_ptr<lect::Settings> settings;
    try {
        settings = lect::Settings::build_with_args(argc, argv);
    } catch (lect::Exception e) {
        if (std::string(e.what()) == "help") {
            return 0;
        }
        std::cout << lect::color_red + "ERROR: " + lect::color_reset + e.what()
                  << "\n";
        return 1;
    }

    lect::Annotations annotations;
    try {
        annotations = lect::AnnotationsBuilder()
            .extract_text_annotations(settings->text_annotation_path)
            .extract_code_annotations(settings->code_annotation_path, settings->language)
            .get_annotations();
    } catch (lect::Exception e) {
        if (true) {
            return 1;
        }
    }

    try {
        settings->checker->check(annotations);
    } catch (lect::Exception e) {
        std::cout << lect::color_red + "ERROR: " + lect::color_reset + e.what()
                  << "\n";
        return 1;
    }

    nlohmann::json dict = settings->preprocessing_builder.build().preprocess(annotations);

    try {
        lect::export_to_dir(settings->output_path, dict);
    } catch (lect::Exception e) {
        std::cout << lect::color_red + "ERROR: " + lect::color_reset + e.what()
                  << "\n";
        return 1;
    }

    return 0;
}
