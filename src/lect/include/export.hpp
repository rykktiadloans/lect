/**
 * @file
 * @brief A collection of functions used to output the documentation
 */

#pragma once

#include "index_html.hpp"
#include "nlohmann/json.hpp"
#include "script_js.hpp"
#include "structures.hpp"
#include "vis_js.hpp"
#include <filesystem>
#include <fstream>
#include <vector>

namespace lect {

/**
 * @brief Generate the documentation at the directory at path using the
 * annotations in the JSON document
 *
 * @param path Path at which to create the documentation
 * @param json JSON that contains the annotations
 */
void export_to_dir(const std::filesystem::path &path,
                   const nlohmann::json &json) noexcept(false);

} // namespace lect
