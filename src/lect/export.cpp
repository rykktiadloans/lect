/**
 * @file
 * @brief A collection of functions used to output the documentation
 */

#include "export.hpp"
#include "index_html.hpp"
#include "nlohmann/json.hpp"
#include "script_js.hpp"
#include "structures.hpp"
#include "vis_js.hpp"
#include <filesystem>
#include <fstream>
#include <vector>


void lect::export_to_dir(const std::filesystem::path &path,
                   const nlohmann::json &json) noexcept(false) {

    if (!std::filesystem::exists(path)) {
        try {
            std::filesystem::create_directory(path);
        } catch (std::filesystem::filesystem_error &e) {
            throw Exception("File `" + path.string() +
                            "` already exists, but it needs to be a directory");
        }
    }

    std::ofstream write_file(path / "annotations.js");
    write_file << "const annotationsJSON = " + json.dump();
    write_file.close();

    write_file.open(path / "index.html");
    write_file << index_html;
    write_file.close();

    write_file.open(path / "vis-network.min.js");
    write_file << vis_js;
    write_file.close();

    write_file.open(path / "script.js");
    write_file << script_js;
    write_file.close();
}
