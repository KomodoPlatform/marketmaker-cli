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

TEST(UrlTests, parseInvalidHttpsUrl)
{
    err_t err;
    URL url;
    bool result = parse_url("https://localhost:7783", &url, &err);
    ASSERT_FALSE(result);
    ASSERT_NE(0, err);
}

TEST(UrlTests, parseInvalidUrl)
{
    err_t err;
    URL url;
    bool result = parse_url("localhost:7783", &url, &err);
    ASSERT_FALSE(result);
    ASSERT_NE(0, err);
}
