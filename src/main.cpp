#include <filesystem>
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

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#define DISABLE_NEWLINE_AUTO_RETURN  0x0008
#include <Windows.h> 
#endif 

int main(int argc, const char **argv) {

    // We'll need this one
    assert(std::string(vis_js).size() == 688913);

#ifdef _WIN32
    HANDLE handleOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD consoleMode;
    GetConsoleMode( handleOut , &consoleMode);
    consoleMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    consoleMode |= DISABLE_NEWLINE_AUTO_RETURN;            
    SetConsoleMode( handleOut , consoleMode );
#endif

    std::unique_ptr<lect::Settings> settings;
    try {
        settings = lect::Settings::build_with_args(argc, argv);
    } catch (lect::Exception &e) {
        if (std::string(e.what()) == "help") {
            return 0;
        }
        std::cout << lect::color_red + "ERROR: " + lect::color_reset + e.what()
                  << "\n";
        return 1;
    }

    lect::Annotations annotations;
    try {
        annotations =
            lect::AnnotationsBuilder()
                .extract_text_annotations(settings->text_annotation_path)
                .extract_code_annotations(settings->code_annotation_path,
                                          settings->language)
                .get_annotations();
    } catch (lect::Exception &e) {
        if (true) {
            return 1;
        }
    }

    try {
        settings->checker->check(annotations);
    } catch (lect::Exception &e) {
        std::cout << lect::color_red + "ERROR: " + lect::color_reset + e.what()
                  << "\n";
        return 1;
    }

    nlohmann::json dict =
        settings->preprocessing.preprocess(annotations);

    try {
        lect::export_to_dir(settings->output_path, dict);
    } catch (lect::Exception &e) {
        std::cout << lect::color_red + "ERROR: " + lect::color_reset + e.what()
                  << "\n";
        return 1;
    }
    std::cout << "Lect successfully generated the documentation at "
              << std::filesystem::canonical(settings->output_path/"index.html").string() << "\n";

    return 0;
}
