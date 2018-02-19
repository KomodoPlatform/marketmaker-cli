#include <gmock/gmock.h>

#include <property.h>

#include "property_utils.h"

TEST(PropertyGroupTests, parseEmptyConfig)
{
    err_t err;
    PropertyGroup *group = parse_properties("", '=', 0, &err);
    ASSERT_EQ(0, err);
    PropertyGroup expectedGroup = {0, nullptr};
    ASSERT_EQ(expectedGroup, *group);
}

TEST(PropertyGroupTests, parseMalformedConfig)
{
    err_t err;
    PropertyGroup *group = parse_properties("line1\nline2", '=', 0, &err);
    ASSERT_EQ(EINVAL, err);
    PropertyGroup expectedGroup = {0, nullptr};
    ASSERT_EQ(nullptr, group);
}

TEST(PropertyGroupTests, parseConfig)
{
    err_t err;
    PropertyGroup *group = parse_properties(" url = http://127.0.0.1:7783 \n\n"
                                                    "userpass=1d8b27b21efabcd96571cd56f91a40fb9aa4cc623d273c63bf9223dc6f8cd81f\r\n",
                                            '=', 0, &err);
    ASSERT_EQ(0, err);
    Property expectedProps[] = {
            {"url",      "http://127.0.0.1:7783"},
            {"userpass", "1d8b27b21efabcd96571cd56f91a40fb9aa4cc623d273c63bf9223dc6f8cd81f"},
    };
    PropertyGroup expectedGroup = {DIMOF(expectedProps), expectedProps};
    ASSERT_EQ(expectedGroup, *group);
}

TEST(PropertyGroupTests, parseHttpHeaders)
{
    err_t err;
    PropertyGroup *group = parse_properties("HTTP/1.1 200 OK\r\n"
                                                    "Access-Control-Allow-Origin: *\r\n"
                                                    "Access-Control-Allow-Credentials: true\r\n"
                                                    "Access-Control-Allow-Methods: GET, POST\r\n"
                                                    "Cache-Control :  no-cache, no-store, must-revalidate\r\n"
                                                    "Content-Length :       71\r\n\r\n", ':',
                                            PARSE_OPT_IGNORE_INVALID_LINES, &err);
    ASSERT_EQ(0, err);
    Property expectedProps[] = {
            {"Access-Control-Allow-Origin",      "*"},
            {"Access-Control-Allow-Credentials", "true"},
            {"Access-Control-Allow-Methods",     "GET, POST"},
            {"Cache-Control",                    "no-cache, no-store, must-revalidate"},
            {"Content-Length",                   "71"},
    };
    PropertyGroup expectedGroup = {DIMOF(expectedProps), expectedProps};
    ASSERT_EQ(expectedGroup, *group);
}
