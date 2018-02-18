#include <gmock/gmock.h>

#include <property.h>

#include "property_utils.h"

TEST(PropertyGroupTests, parseEmptyConfig) {
    err_t err;
    PropertyGroup *group = parse_properties("", &err);
    ASSERT_EQ(0, err);
    PropertyGroup expectedGroup = { 0, NULL };
    ASSERT_EQ(expectedGroup, *group);
}

TEST(PropertyGroupTests, parseMalformedConfig) {
    err_t err;
    PropertyGroup *group = parse_properties("line1\nline2", &err);
    ASSERT_EQ(EINVAL, err);
    PropertyGroup expectedGroup = { 0, NULL };
    ASSERT_EQ(NULL, group);
}

TEST(PropertyGroupTests, parseConfig) {
    err_t err;
    PropertyGroup *group = parse_properties(" url = http://127.0.0.1:7783 \n\n"
                                                 "userpass=1d8b27b21efabcd96571cd56f91a40fb9aa4cc623d273c63bf9223dc6f8cd81f\r\n", &err);
    ASSERT_EQ(0, err);
    Property expectedProps[] = {
            { "url", "http://127.0.0.1:7783" },
            { "userpass", "1d8b27b21efabcd96571cd56f91a40fb9aa4cc623d273c63bf9223dc6f8cd81f"},
    };
    PropertyGroup expectedGroup = { DIMOF(expectedProps), expectedProps };
    ASSERT_EQ(expectedGroup, *group);
}
