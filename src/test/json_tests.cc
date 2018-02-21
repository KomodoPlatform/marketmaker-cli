#include <gmock/gmock.h>

#include <json.h>

#include <string>

using namespace std;

TEST(JsonTests, buildJsonFromConfig)
{
    Property expectedProps[] = {
            {"url",      "http://127.0.0.1:7783"},
            {"userpass", "1d8b27b21efabcd96571cd56f91a40fb9aa4cc623d273c63bf9223dc6f8cd81f"},
    };
    PropertyGroup group = {DIMOF(expectedProps), expectedProps};

    err_t err;
    char *json = build_json_request(&group, &err);
    ASSERT_EQ(0, err);
    ASSERT_NE(nullptr, json);
    string expectedJson = R"({"url":"http://127.0.0.1:7783","userpass":"1d8b27b21efabcd96571cd56f91a40fb9aa4cc623d273c63bf9223dc6f8cd81f"})";
    ASSERT_EQ(expectedJson, string(json));
}

