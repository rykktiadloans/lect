# Lect
Lect is a program that allows the user to document design patterns by using multilevel annotations. Currently at it's MVP.

## Usage
For Lect to work, you need text and code annotations.

Each text annotation is placed inside a file with the `.an` extension. All the annotation files should share the same parent directory.
The stem of the annotation's filename will be used as its ID, which can contain only latin characters and hyphens.
The first line of the file should a a `#` followed by a whitespace and the annotation's title. The rest of the file is it's content.
Inside said content, you can reference other annotations by using a dollar sign, followed the ID of the annotation you want to reference.

Example:
```
# Here's the title

Now this is the content. And $these are  $references-to-other annotations
```

A code annotation can be inserted directly into the source code by writing a comment (only single-line comments are allowed), followed by a dollar sign with the annotation's ID and its title.
The program will automatically capture the next code block after the annotation.

Example (in C++):
```c++
//$id-thing Elaborate title
struct Thing {
    int a;
    std::string b;
} // Here, the capture ends
```

To run the program, you need to give it the paths to the source code, text annotation directory, documentation output path, and the used programming language.

Here's an example of a how to run the program when you want to use `annotations/` as a text annotation directory, 
`src/` as a source code directory, `docs/` as an output directory, and C++ as a language
```bash
lect -t annotations/ -s src/ -o docs/ -l c++
```

You can view the program's options using `-h` and `--help` CLI flags. 

## Supported languages
- C++: using `c++`

## Dependencies
- [Tree-sitter](https://github.com/tree-sitter/tree-sitter): used for extracting source code annotations. Can support a wide range of languages, as long as there are parsers for them.
- [Tree-sitter-cpp](https://github.com/tree-sitter/tree-sitter-cpp): a Tree-sitter parser for C++.
- [Nlohmann's JSON library](https://github.com/nlohmann/json): used for serializing annotations into an easily-readable format.
- [Vis.js Network](https://github.com/visjs/vis-network): JS library for presenting network graphs. Used to draw the annotation tree in the documentation

## To-do List
- [ ] Add optional checks
- [ ] Add Java support
- [x] Add annotation tree displayer
- [x] Add annotation inspector
- [x] Add basic output checking
- [x] Add -h
- [x] Add colored output
- [x] Figure out error checking
- [x] Add basic command line arguments
- [x] Add JSON export
- [x] Add source code annotations
- [x] Add proper format checking to text annotations 

## Remember
- [ ] Make sure Doxygen is everywhere
