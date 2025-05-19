#include "checks.hpp"
#include "structures.hpp"
#include <gtest/gtest.h>
#include <memory>

class CheckingTest : public testing::Test {
  protected:
    std::unique_ptr<lect::Checker> checker;

    CheckingTest()
        : checker(std::make_unique<lect::IdAllowedSymbolsChecker>()) {
        checker->add(std::make_unique<lect::DuplicateChecker>());
        checker->add(std::make_unique<lect::NonexistentChecker>());
        checker->add(std::make_unique<lect::CycleChecker>());
        checker->add(
            std::make_unique<lect::CodeAnnotationsSuffixChecker>("-src"));
    }
};

TEST_F(CheckingTest, NoFalsePositives) {
    lect::Annotations annotations;
    annotations.text_annotations.emplace_back(
        "id-a", "title", "blah blah $id-b $id-c",
        std::vector<std::string>{"id-b", "id-c"});
    annotations.text_annotations.emplace_back(
        "id-b", "title", "blah blah $id-d-src",
        std::vector<std::string>{"id-d-src"});
    annotations.text_annotations.emplace_back("id-c", "title", "blah blah",
                                              std::vector<std::string>{});
    annotations.code_annotations.emplace_back("id-d-src", "title", "blah blah",
                                              "file.cpp", 5);
    ASSERT_NO_THROW(checker->check(annotations))
        << "There seems to be some sort of false positive case where the "
           "checker catches something that it shouldn't've";
}

TEST_F(CheckingTest, CheckSpecialSymbols) {
    lect::Annotations annotations;

    annotations.text_annotations.emplace_back("$id", "title", "bagsd",
                                              std::vector<std::string>());

    ASSERT_THROW(checker->check(annotations), lect::Exception)
        << "The checker should throw if there are any special characters "
           "besides hyphens";
}

TEST_F(CheckingTest, CheckLatinCharacters) {
    lect::Annotations annotations;

    // The `i` is cyrillic
    annotations.text_annotations.emplace_back("іd", "title", "bagsd",
                                              std::vector<std::string>());

    ASSERT_THROW(checker->check(annotations), lect::Exception)
        << "There shouldn't be any non-latin characters in the ID";
}

TEST_F(CheckingTest, CheckForNumbers) {
    lect::Annotations annotations;

    annotations.code_annotations.emplace_back("id-3-src", "title", "fdas",
                                              "file.txt", 4);

    ASSERT_THROW(checker->check(annotations), lect::Exception)
        << "The ID shouldn't contain any numbers";
}

TEST_F(CheckingTest, CheckForDuplicates) {
    lect::Annotations annotations;

    annotations.text_annotations.emplace_back("id", "title", "fda",
                                              std::vector<std::string>());

    annotations.text_annotations.emplace_back("id", "fda", "fdsa", std::vector<std::string>());

    ASSERT_THROW(checker->check(annotations), lect::Exception)
        << "There shouldn't be any duplicate IDs";
}

TEST_F(CheckingTest, CheckForDanglingReferences) {
    lect::Annotations annotations;

    annotations.text_annotations.emplace_back(
        "id", "title", "fasd $id-src", std::vector<std::string>{"id-src"});

    ASSERT_THROW(checker->check(annotations), lect::Exception)
        << "It should be able to find a reference to an annotation that "
           "doesn't exist";
}

TEST_F(CheckingTest, FindCycles) {
    lect::Annotations annotations;

    annotations.text_annotations.emplace_back("id-a", "title", "fda $id-b",
                                              std::vector<std::string>{"id-b"});

    annotations.text_annotations.emplace_back("id-b", "title", "fda $id-a",
                                              std::vector<std::string>{"id-a"});

    ASSERT_THROW(checker->check(annotations), lect::Exception)
        << "It should be able to find the id-a -> id-b -> id-a cycle";
}

TEST_F(CheckingTest, CheckForSuffixes) {
    lect::Annotations annotations;

    annotations.code_annotations.emplace_back("id", "title", "fdas", "file.txt",
                                              4);

    ASSERT_THROW(checker->check(annotations), lect::Exception)
        << "It should check for the lack of `-src` suffix";
}
