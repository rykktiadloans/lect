#include "vis_js.hpp"
#include <gtest/gtest.h>
#include <string>

TEST(ResourceTest, VisJSProperlyAdded) {
    std::string vis_str = vis_js;
    EXPECT_EQ(vis_str.size(), 475182) << "It seems that the Vis.js library wasn't properly read. The expected size is " << 475182 << ", got " << vis_str.size() << "\n";
}
