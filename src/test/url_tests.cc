#include <gtest/gtest.h>

#include <url.h>

TEST(UrlTests, parseValidUrl)
{
    err_t err;
    URL url;
    bool result = parse_url("http://localhost:7783", &url, &err);
    ASSERT_TRUE(result);
    ASSERT_EQ(0, err);
    ASSERT_EQ(0x0100007f, url.addr.s_addr);
    ASSERT_EQ(7783, url.port);
}

class InvalidUrlTest : public ::testing::TestWithParam<const char *> {};

TEST_P(InvalidUrlTest, parseInvalidUrl)
{
    err_t err;
    URL url;
    bool result = parse_url(GetParam(), &url, &err);
    ASSERT_FALSE(result);
    ASSERT_NE(0, err);
}

INSTANTIATE_TEST_CASE_P(InvalidUrlTestCases, InvalidUrlTest, ::testing::Values(
        "https://localhost:7783",
        "localhost:7783",
        "http://localhost",
        "http://localhost:70000"
));
