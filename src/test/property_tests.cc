#include <gmock/gmock.h>

#include <property.h>
#include <string>
#include <sstream>

using namespace std;

#include "property_utils.h"
#include "MockFile.h"

using ::testing::_;
using ::testing::NotNull;
using ::testing::DoAll;
using ::testing::Return;
using ::testing::Invoke;
using ::testing::SetArrayArgument;

static Property PROPS1[] = {
        {"autoprice", "base,rel,fixed,minprice,maxprice,margin,refbase,refrel,factor,offset"},
        {"disable",   "coin"},
        {"enable",    "coin"},
        {"goal",      "coin,val"},
        {"myprice",   "base,rel"},
};
static const PropertyGroup GROUP1 = {
        DIMOF(PROPS1),
        DIMOF(PROPS1),
        PROPS1
};

TEST(PropertyGroupTests, dontFindProperty)
{
    const char *value = find_property(&GROUP1, "MyPrice");
    ASSERT_EQ(nullptr, value);
}

TEST(PropertyGroupTests, findProperty)
{
    const char *value = find_property(&GROUP1, "myprice");
    ASSERT_NE(nullptr, value);
    ASSERT_EQ(string("base,rel"), string(value));
}

TEST(PropertyGroupTests, findPropertyIgnoreCase)
{
    const char *value = find_property_ignore_case(&GROUP1, "MyPrice");
    ASSERT_NE(nullptr, value);
    ASSERT_EQ(string("base,rel"), string(value));
}

TEST(PropertyGroupTests, longestKeyLength)
{
    size_t len = longest_key_len(&GROUP1);
    ASSERT_EQ(9, len);
}

TEST(PropertyGroupTests, putAllAndSort)
{
    Property props1[] = {
            {"autoprice", "base,rel,fixed,minprice,maxprice,margin,refbase,refrel,factor,offset"},
            {"goal",      "coin,val"},
            {"myprice",   "base,rel"},
            {"enable",    "coin"},
            {"disable",   "coin"},
    };
    PropertyGroup group1 = {
            DIMOF(props1),
            DIMOF(props1),
            props1
    };
    err_t err;
    PropertyGroup *group2 = realloc_properties(nullptr, 10);
    group2 = add_property(group2, "help", "", &err);
    ASSERT_EQ(0, err);
    group2 = add_property(group2, "goal", "coin", &err);
    ASSERT_EQ(0, err);
    group2 = add_property(group2, "myprice", "base,coin,rel", &err);
    ASSERT_EQ(0, err);
    ASSERT_EQ(3, group2->size);

    group2 = put_all_properties(group2, &group1, &err);
    ASSERT_EQ(0, err);
    ASSERT_EQ(6, group2->size);
    ASSERT_NE(nullptr, group2);

    Property expectedProps[] = {
            {"autoprice", "base,rel,fixed,minprice,maxprice,margin,refbase,refrel,factor,offset"},
            {"disable",   "coin"},
            {"enable",    "coin"},
            {"goal",      "coin,val"},
            {"help",      ""},
            {"myprice",   "base,rel"},
    };
    PropertyGroup expectedGroup = {
            DIMOF(expectedProps),
            DIMOF(expectedProps),
            expectedProps
    };

    sort_properties(group2);
    ASSERT_EQ(expectedGroup, *group2);
}

TEST(PropertyGroupTests, parseEmptyConfig)
{
    err_t err;
    PropertyGroup *group = parse_properties("", '=', 0, &err);
    ASSERT_EQ(0, err);
    PropertyGroup expectedGroup = {0, 0, nullptr};
    ASSERT_EQ(expectedGroup, *group);
}

TEST(PropertyGroupTests, parseMalformedConfig)
{
    err_t err;
    PropertyGroup *group = parse_properties("line1\nline2", '=', 0, &err);
    ASSERT_EQ(EINVAL, err);
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
    PropertyGroup expectedGroup = {DIMOF(expectedProps), DIMOF(expectedProps), expectedProps};
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
    PropertyGroup expectedGroup = {DIMOF(expectedProps), DIMOF(expectedProps), expectedProps};
    ASSERT_EQ(expectedGroup, *group);
}

TEST(PropertyGroupTests, loadProperties)
{
    const char *expectedContents = "autoprice=base,rel,fixed,minprice,maxprice,margin,refbase,refrel,factor,offset\n"
            "goal=coin,val\n"
            "myprice=base,rel\n"
            "enable=coin\n"
            "disable=coin\n";
    size_t expectedContentsLen = strlen(expectedContents);

    MockFile file;
    EXPECT_CALL(file, doOpen("/path/to/file", "rb", _))
            .WillOnce(Return(true));
    EXPECT_CALL(file, doSeek(_, _, _))
            .WillRepeatedly(Return(true));
    EXPECT_CALL(file, doTell(_))
            .WillOnce(Return(expectedContentsLen));

    EXPECT_CALL(file, doRead(NotNull(), _, _))
            .WillOnce(DoAll(SetArrayArgument<0>(expectedContents, expectedContents + expectedContentsLen),
                            Return(expectedContentsLen)));
    EXPECT_CALL(file, doClose())
            .Times(1);
    err_t err = 0;
    PropertyGroup *group = load_properties(&file, "/path/to/file", &err);
    ASSERT_EQ(0, err);
    ASSERT_NE(nullptr, group);
    ASSERT_EQ(GROUP1, *group);
}

TEST(PropertyGroupTests, saveProperties)
{
    struct WrittenLines {
        bool doWrite(const void *ptr, size_t size, err_t *errp)
        {
            this->str.append((char *) ptr, size);
        }

        std::string str;
    };
    WrittenLines written_lines;

    const char *expectedContents = "autoprice=base,rel,fixed,minprice,maxprice,margin,refbase,refrel,factor,offset\n"
            "disable=coin\n"
            "enable=coin\n"
            "goal=coin,val\n"
            "myprice=base,rel\n";
    size_t expectedContentsLen = strlen(expectedContents);

    MockFile file;
    EXPECT_CALL(file, doOpen("/path/to/file", "w+b", _))
            .WillOnce(Return(true));
    EXPECT_CALL(file, doWrite(NotNull(), _, _))
            .WillRepeatedly(Invoke(&written_lines, &WrittenLines::doWrite));
    EXPECT_CALL(file, doClose())
            .Times(1);
    err_t err = 0;
    bool status = save_properties(&GROUP1, &file, "/path/to/file", &err);
    ASSERT_TRUE(status);
    ASSERT_EQ(std::string(expectedContents), written_lines.str);
}