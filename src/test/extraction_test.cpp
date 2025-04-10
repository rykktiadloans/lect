
#include "extract.hpp"
#include "settings.hpp"
#include "structures.hpp"
#include <gtest/gtest.h>

class ExtractionTest : public testing::Test {
  protected:
    ExtractionTest() {
        const char *argv[] = {"lect", "-t", "../test/test1/annotations/",
                              "-s", "../test/test1/code/",
                              "-l", "c++",
                              "-o", "../test/test1/out"};

        std::unique_ptr<lect::Settings> settings =
            lect::Settings::build_with_args(9, argv);

        _annotations =
            lect::AnnotationsBuilder()
                .extract_code_annotations(settings->code_annotation_path,
                    settings->language)
                .extract_text_annotations(settings->text_annotation_path)
                .get_annotations();
    }

    lect::Annotations _annotations;
};

TEST_F(ExtractionTest, BasicCodeExtractionCorrectness) {
    ASSERT_EQ(_annotations.code_annotations.size(), 1)
        << "Incorrect amount of code annotations extracted. Expected 1, got "
        << _annotations.code_annotations.size();
    lect::CodeAnnotation code_annotation = _annotations.code_annotations.at(0);
    EXPECT_STREQ(code_annotation.id.c_str(), "test-src");
    EXPECT_STREQ(code_annotation.title.c_str(), "Test class");
    EXPECT_EQ(code_annotation.line, 2);
    EXPECT_STREQ(code_annotation.content.c_str(), "class Whatever {}");
}

TEST_F(ExtractionTest, BasicTextExtractionCorrectness) {
    ASSERT_EQ(_annotations.text_annotations.size(), 1)
        << "Incorrect amount of text annotations extracted. Expected 1, got "
        << _annotations.code_annotations.size();
    lect::TextAnnotation text_annotation = _annotations.text_annotations.at(0);
    EXPECT_STREQ(text_annotation.id.c_str(), "test");
    EXPECT_STREQ(text_annotation.title.c_str(), "Test class description");
    EXPECT_STREQ(text_annotation.content.c_str(), "$test-src thingie.\n");
    EXPECT_EQ(text_annotation.references.size(), 1);
}
