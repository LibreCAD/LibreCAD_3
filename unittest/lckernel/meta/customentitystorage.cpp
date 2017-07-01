#include <gtest/gtest.h>
#include <memory>
#include <cad/meta/customentitystorage.h>

using namespace lc;

TEST(CustomEntityStorageTest, Creation) {
    auto plugin = "Plugin Name";
    auto entity = "Entity Name";
    geo::Coordinate base(10, 20, 0);

    auto ces = std::make_shared<const CustomEntityStorage>(plugin, entity, base);

    EXPECT_EQ(plugin, ces->pluginName());
    EXPECT_EQ(entity, ces->entityName());
}

TEST(CustomEntityStorageTest, Params) {
    auto paramName = "Existing param";
    auto paramValue = "Param value";

    auto ces = std::make_shared<const CustomEntityStorage>("Plugin", "Entity", geo::Coordinate());

    EXPECT_EQ("", ces->param(paramName));

    ces = ces->setParam(paramName, paramValue);
    EXPECT_EQ(paramValue, ces->param(paramName));
}