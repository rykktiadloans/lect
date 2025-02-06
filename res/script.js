let annotationsJSON = {
    "code_annotations": [
        {
            "content": "struct Language {\n    std::string name;\n    std::vector<std::string> extensions;\n    std::string query;\n    const TSLanguage *language;\n    std::function<bool(std::string)> validate_comment;\n    std::function<bool(std::string)> validate_object;\n\n    /**\n     * @brief Generates an object suited for C++ parsing\n     *\n     * @return Language pack that can be user for C++\n     */\n    static Language cpp() {\n        std::vector<std::string> extensions{\".c\", \".cpp\", \".h\", \".hpp\"};\n\n        auto validate_comment = [](std::string string) {\n            uint64_t begin = string.find_first_not_of(\"\\n \");\n            if (begin == std::string::npos) {\n                return false;\n            }\n\n            std::string comment_fragment = string.substr(begin, 2);\n            if (comment_fragment != \"//\") {\n                return false;\n            }\n\n            uint64_t ptr = begin + 2;\n            while (string.at(ptr) == ' ' || string.at(ptr) == '\\n') {\n                ptr++;\n            }\n\n            if (string.at(ptr) != '$') {\n                return false;\n            }\n\n            return true;\n        };\n\n        auto validate_object = [](std::string string) {\n            uint64_t begin = string.find_first_not_of(\"\\n \");\n            if (begin == std::string::npos) {\n                return false;\n            }\n\n            std::string comment_fragment = string.substr(begin, 2);\n            if (comment_fragment == \"//\" || comment_fragment == \"/*\") {\n                return false;\n            }\n\n            return true;\n        };\n\n        return Language(\"c++\", extensions,\n                        \"((comment) @comment . (comment)* . (_) @object)\",\n                        tree_sitter_cpp(), validate_comment, validate_object);\n    }\n\n    static Language placeholder() {\n        return Language(\n            \"\", std::vector<std::string>(), \"\", nullptr,\n            [](std::string) { return false; },\n            [](std::string) { return false; });\n    }\n\n    /**\n     * @brief The constructor for the language.\n     *\n     * @param name Name of the language\n     * @param extensions File extensions for the languages\n     * @param query Query for the data\n     * @param language Language object for parsing\n     * @param validate_comment Function to validate that a supplied string is an\n     * appropriate comment\n     * @param validate_object Function to validate that a supplied string isn't\n     * a comment\n     */\n    Language(const std::string name, const std::vector<std::string> &extensions,\n             const std::string query, const TSLanguage *language,\n             std::function<bool(std::string)> validate_comment,\n             std::function<bool(std::string)> validate_object)\n        : name(name), extensions(extensions), query(query), language(language),\n          validate_comment(validate_comment), validate_object(validate_object) {\n    }\n}",
            "file": "../src/lect/structures.hpp",
            "id": "language-src",
            "line": 134,
            "title": "Language"
        },
        {
            "content": "int main(int argc, char **argv) {\n    std::unique_ptr<lect::Settings> settings;\n    try {\n        settings = std::make_unique<lect::Settings>(argc, argv);\n    }\n    catch (lect::Exception e) {\n        if(std::string(e.what()) == \"help\") {\n            return 0;\n        }\n        std::cout << lect::color_red + \"ERROR: \" + lect::color_reset + e.what() << \"\\n\";\n        return 1;\n    }\n\n    std::vector<lect::TextAnnotation> text_annotations;\n    try {\n        text_annotations =\n            lect::extract_text_annotations(settings->text_annotation_path);\n    } catch (lect::Exception e) {\n        if (true) {\n            return 1;\n        }\n    }\n\n    std::vector<lect::CodeAnnotation> code_annotations;\n    try {\n        code_annotations = lect::extract_code_annotations(\n            settings->code_annotation_path, settings->language);\n    } catch (lect::Exception e) {\n        if (true) {\n            return 1;\n        }\n    }\n\n    try {\n        settings->checker->check(text_annotations, code_annotations);\n    }\n    catch (lect::Exception e) {\n        std::cout << lect::color_red + \"ERROR: \" + lect::color_reset + e.what() << \"\\n\";\n        return 1;\n    }\n\n    auto dict = lect::annotations_to_json(text_annotations, code_annotations);\n    std::ofstream file(settings->output_path);\n    file << dict.dump();\n    file.close();\n\n    return 0;\n}",
            "file": "../src/main.cpp",
            "id": "main-src",
            "line": 11,
            "title": "Main function"
        },
        {
            "content": "nlohmann::json\nannotations_to_json(const std::vector<TextAnnotation> &text_annotations,\n                    const std::vector<CodeAnnotation> &code_annotations) {\n    using namespace nlohmann;\n\n    json dict = {{\"text_annotations\", json::array()},\n                 {\"code_annotations\", json::array()}};\n\n    for (const auto &a : text_annotations) {\n        json t = {{\"id\", a.id},\n                  {\"title\", a.title},\n                  {\"content\", a.content},\n                  {\"references\", json::array()}};\n        for (const auto &ref : a.references) {\n            t[\"references\"].push_back(ref);\n        }\n        dict[\"text_annotations\"].push_back(t);\n    }\n\n    for (const auto &a : code_annotations) {\n        json t = {{\"id\", a.id},\n                  {\"title\", a.title},\n                  {\"content\", a.content},\n                  {\"file\", a.file},\n                  {\"line\", a.line}};\n        dict[\"code_annotations\"].push_back(t);\n    }\n\n    return dict;\n}",
            "file": "../src/lect/export.hpp",
            "id": "json-src",
            "line": 22,
            "title": "JSON export"
        },
        {
            "content": "struct Settings {\n    std::filesystem::path text_annotation_path;\n    std::filesystem::path code_annotation_path;\n    std::filesystem::path output_path;\n    Language language{Language::placeholder()};\n    std::unique_ptr<Checker> checker{\n        std::make_unique<IdAllowedSymbolsChecker>()};\n\n    /**\n     * @brief Uses main() function's argc and argv arguments to construct itself\n     *\n     * @param argc Number of command line arguments\n     * @param argv An array of command line arguments\n     */\n    Settings(int argc, char **argv) {\n        checker->add<DuplicateChecker>();\n        checker->add<NonexistentChecker>();\n        checker->add<CycleChecker>();\n\n        int ptr = 1;\n        bool text_path_set = false;\n        bool code_path_set = false;\n        bool output_path_set = false;\n        bool language_set = false;\n\n        while (ptr < argc) {\n            std::string arg = argv[ptr];\n            if (arg == \"-t\") {\n                std::string dir = argv[ptr + 1];\n                ptr++;\n                text_annotation_path = dir;\n                text_path_set = true;\n            } else if (arg == \"-s\") {\n                std::string path = argv[ptr + 1];\n                ptr++;\n                code_annotation_path = path;\n                code_path_set = true;\n            } else if (arg == \"-o\") {\n                std::string path = argv[ptr + 1];\n                ptr++;\n                output_path = path;\n                output_path_set = true;\n            } else if (arg == \"-l\") {\n                std::string lang = argv[ptr + 1];\n                ptr++;\n                std::unordered_map<std::string, std::function<Language(void)>>\n                    language_map{{\"c++\", Language::cpp}};\n                auto constructor = language_map.find(lang);\n                if (constructor == language_map.end()) {\n                    throw Exception(\"Unrecognized language: \" + color_blue +\n                                    lang + color_reset);\n                }\n                language = constructor->second();\n                language_set = true;\n            } else if (arg == \"-h\" || arg == \"--help\") {\n                std::cout\n                    << \"Usage:\\n\"\n                       \"  lect -t <text_ann_dir> -s <src_dir> -l \"\n                       \"<language> -o <output>\"\n                       \"  [<optional_args>...]\\n\\n\"\n                       \"Required arguments:\\n\"\n                       \"  -t          Directory with .an annotation files\\n\"\n                       \"  -s          Source code directory with annotations\\n\"\n                       \"  -l          Programming language of the project\\n\"\n                       \"  -o          Output directory\\n\\n\"\n                       \"Supported languages:\\n\"\n                       \"  c++         C++ (.cpp .c .h .hpp)\\n\\n\"\n                       \"Optional arguments:\\n\"\n                       \"  -h, --help  Help screen\\n\";\n                throw Exception(\"help\");\n\n            } else {\n                throw Exception(\"Unrecognized argument: \" + color_blue + arg +\n                                color_reset);\n            }\n            ptr++;\n        }\n\n        if (!text_path_set) {\n            throw Exception(\"Text annotation path isn't set\");\n        }\n        if (!code_path_set) {\n            throw Exception(\"Code annotation path isn't set\");\n        }\n        if (!output_path_set) {\n            throw Exception(\"Output path isn't set\");\n        }\n        if (!language_set) {\n            throw Exception(\"Language isn't set\");\n        }\n    }\n}",
            "file": "../src/lect/settings.hpp",
            "id": "settings-src",
            "line": 20,
            "title": "Settings class"
        },
        {
            "content": "struct Checker {\n\n    /**\n     * @brief Check text and code annotations for any errors\n     *\n     * @param text_annotations Vector of text annotations\n     * @param code_annotations Vector of code annotations\n     * @throw lect::Exception\n     */\n    virtual void\n    check(std::vector<TextAnnotation> &text_annotations,\n          std::vector<CodeAnnotation> &code_annotations) noexcept(false) = 0;\n\n    /**\n     * @brief A virtual destructor for subclassing\n     */\n    virtual ~Checker(){};\n\n    /**\n     * @brief A function that adds a new checker to the end of the chain\n     *\n     * @tparam T Type of the checker\n     */\n    template <typename T> void add() {\n        if (!m_next.has_value()) {\n            m_next = std::make_unique<T>();\n            return;\n        }\n        m_next->get()->add<T>();\n    }\n\n    /**\n     * @brief Call the next checker in the sequence, if there is any\n     *\n     * @param text_annotations Vector of text annotations to use on the next\n     * check call\n     * @param code_annotations Vector of code annotations to use on the next\n     * check call\n     * @throw lect::Exception\n     */\n    void next(std::vector<TextAnnotation> &text_annotations,\n              std::vector<CodeAnnotation> &code_annotations) noexcept(false) {\n        if (!m_next.has_value()) {\n            return;\n        }\n        m_next->get()->check(text_annotations, code_annotations);\n    }\n\n  private:\n    std::optional<std::unique_ptr<Checker>> m_next = std::nullopt;\n}",
            "file": "../src/lect/checks.hpp",
            "id": "checker-src",
            "line": 24,
            "title": "Checker classes"
        },
        {
            "content": "std::vector<TextAnnotation>\nextract_text_annotations(const std::filesystem::path &root) noexcept(false) {\n    using namespace std::filesystem;\n    std::vector<TextAnnotation> annotations;\n\n    if (!is_directory(root)) {\n        throw Exception(root.string() + \" is not a directory.\");\n    }\n\n    std::mutex mutex;\n    auto add = [&annotations, &mutex](std::string id, std::string title,\n                                      std::string content,\n                                      std::vector<std::string> references) {\n        const std::lock_guard<std::mutex> lock_guard(mutex);\n        annotations.emplace_back(id, title, content, references);\n    };\n\n    extract_text_annotations_inner(root, add);\n\n    return annotations;\n}",
            "file": "../src/lect/extract.hpp",
            "id": "extract-text-src",
            "line": 128,
            "title": "Extraction of text annotations"
        },
        {
            "content": "std::vector<CodeAnnotation>\nextract_code_annotations(const std::filesystem::path &root,\n                         const Language &language) noexcept(false) {\n    using namespace std::filesystem;\n    std::vector<CodeAnnotation> annotations;\n\n    std::mutex mutex;\n    auto add = [&annotations, &mutex](std::string id, std::string title,\n                                      std::string content, std::string file,\n                                      int line) {\n        const std::lock_guard<std::mutex> lock_guard(mutex);\n        annotations.emplace_back(id, title, content, file, line);\n    };\n\n    extract_code_annotations_inner(root, language, add);\n\n    return annotations;\n}",
            "file": "../src/lect/extract.hpp",
            "id": "extract-code-src",
            "line": 288,
            "title": "Extraction of code annotations"
        }
    ],
    "text_annotations": [
        {
            "content": "Language class at $language-src is a class that contains data needed for parsing a certain programming language, such as language's used extensions, query string for finding all the annotations, as well as functions for validating the correct form of annotation comments, and making sure that the captured object isn't some other comment\n",
            "id": "language",
            "references": [
                "language-src"
            ],
            "title": "Language class"
        },
        {
            "content": "Settings class at $settings-src is a class that contains all the options that configure the program.\n\nIt analyzes command line arguments to get paths to annotations, used language, output path, and others. If some argument isn't recognised, an exception is thrown.\n\nIt also builds a Checker $checker chain based on certain command line arguments (TO-DO)\n",
            "id": "settings",
            "references": [
                "settings-src",
                "checker"
            ],
            "title": "Settings"
        },
        {
            "content": "Code and text annotations are extracted separately\n\nCode annotations are extracted using extract_code_annotations() function at $extract-code-src. It uses a Language object $language for all language specific information. It looks recursively looks for all files with extensions used by a language, queries all potential code annotations, and filters out those that aren't. If there are any errors with their syntax, it will throw an exception.\n\nText annotations are extracted using extract_text_annotations() function at $extract-text-src. It recursively looks through all the files in the directory and turns them into text annotations. If the originally supplied file is not a directory, it will throw an exception. It also throws one if an annotation is malformed.\n\nBoth of these functions are asynchronous.\n",
            "id": "extract",
            "references": [
                "extract-code-src",
                "language",
                "extract-text-src"
            ],
            "title": "Extraction functions"
        },
        {
            "content": "Function main() at $main-src is the entry-point of the application.\n\nWhen the program is run, it analyzes it creates a Settings object $settings based on the CLI arguments it received, or notifies the user if there are any errors.\n\nAfter that, it extract text and then code annotations using $extract. If there are any errors, the user is notified.\n\nThen, they are checked against the Checker chain $checkers to see if there are any errors in them.\n\nIf everything is alright, they are exported to the output JSON file using $export.\n",
            "id": "main",
            "references": [
                "main-src",
                "settings",
                "extract",
                "checkers",
                "export"
            ],
            "title": "Main function"
        },
        {
            "content": "Checker class at $checker-src is an abstract class that uses a Chain of Responsibility pattern to sequentially check text and code annotations for any errors. If any error is detected, an exception is thrown.\n",
            "id": "checker",
            "references": [
                "checker-src"
            ],
            "title": "Checkers"
        },
        {
            "content": "Function annotations_to_json() at $json-src translates text and code annotations into a single JSON object that can later be injected into a JS script in the documentation.\n",
            "id": "export",
            "references": [
                "json-src"
            ],
            "title": "JSON export"
        }
    ]
};

let nodes = [];

for (let el of annotationsJSON.code_annotations) {
    nodes.push({
        id: el.id,
        label: el.id + "\n" + el.title,
        color: {
            border: '#733e0e',
            background: '#ff9635',
            highlight: {
                border: '#73190e',
                background: '#da3521'
            },
            hover: {
                border: '#73310e',
                background: '#da4d21'
            }
        },
    });
}

for (let el of annotationsJSON.text_annotations) {
    nodes.push({ id: el.id, label: el.id + "\n" + el.title });
}


let edges = [];

for (let el of annotationsJSON.text_annotations) {
    for (let ref of el.references) {
        edges.push({
            from: el.id,
            to: ref,
        });
    }
}

nodes.forEach(node => {
    node.sortkey = "0";
});

nodes.forEach(node => {
    edges.forEach(edge => {
        if (edge.to == node.id) {
            var parent = nodes.find(e => e.id == edge.from);
            node.sortkey = parent.sortkey + "." + node.id;
        }
    })
});

nodes = nodes.sort((a, b) => {
    if (a.sortkey > b.sortkey) return 1;
    if (a.sortkey < b.sortkey) return -1;
    return 0;
});

let container = document.querySelector("#network");

let data = {
    nodes: nodes,
    edges: edges
};

let options = {
    layout: {
        randomSeed: undefined,
        improvedLayout: true,
        clusterThreshold: 150,
        hierarchical: {
            enabled: true,
            levelSeparation: 120,
            nodeSpacing: 200,
            treeSpacing: 300,
            blockShifting: true,
            edgeMinimization: true,
            parentCentralization: false,
            direction: 'UD',        // UD, DU, LR, RL
            sortMethod: 'directed',  // hubsize, directed
            shakeTowards: 'leaves'  // roots, leaves
        }
    },
    edges: {
        arrows: {
            to: true
        },
        shadow: {
            enabled: true
        }
    },
    nodes: {
        shape: "box",
        widthConstraint: true,
        color: {
            border: '#116E00',
            background: '#4CC835',
            highlight: {
                border: '#009c33',
                background: '#06be37'
            },
            hover: {
                border: '#0fb217',
                background: '#3ada42'
            }
        },
        shadow: {
            enabled: true
        }
    },
    physics: false,
    interaction: {
        hover: true,
        //selectable: false
    }

    /*
    physics: {
        hierarchicalRepulsion: {
            avoidOverlap: 1,
            springConstant: 0.001
        },
    },
    */
};

let network = new vis.Network(container, data, options);

let viewer = document.querySelector("#viewer");

function onNodeClick(event) {
    network.unselectAll();
    let nodes = event.nodes;
    if (nodes.length === 0) {
        return;
    }
    if (nodes.length > 1) {
        return;
    }
    let isCode = false;
    let nodeId = nodes[0];

    let annotation = annotationsJSON.text_annotations
        .find((annotation) => nodeId === annotation.id);
    if (annotation === undefined) {
        annotation = annotationsJSON.code_annotations
            .find((annotation) => nodeId === annotation.id);
        isCode = true;
    }
    viewer.style.display = "flex";
    viewer.querySelector("h4").textContent = annotation.id;
    viewer.querySelector("h1").textContent = annotation.title;
    let content = viewer.querySelector(".content");
    content.innerHTML = "";
    for(let line of annotation.content.split("\n")) {
        let pos = 0;
        for(let char of line) {
            if(char != " ") {
                break;
            }
            pos++;
        }
        console.log(pos, line);
        for(let i = 0; i < pos; i++) {
            content.innerHTML += "&#x00A0";
        }
        content.appendChild(document.createTextNode(line));
        content.append(document.createElement("br"));

    }
    if(isCode) {
        content.style.color = "white";
        content.style.backgroundColor = "rgb(38, 38, 38)";
        content.style.overflowX = "scroll";
        content.style.whiteSpace = "nowrap";
        let file = document.createElement("p");
        file.textContent = annotation.file;
        content.before(file);
    }
    else {
        content.style.color = "black";
        content.style.backgroundColor = "white";
        content.style.overflowX = "hidden";
        content.style.whiteSpace = "normal";
    }
}

network.on("click", onNodeClick);

document.querySelector(".close").onclick = () => {
    viewer.style.display = "none";
}
