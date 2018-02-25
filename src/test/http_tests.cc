/******************************************************************************
 * Copyright © 2014-2017 The SuperNET Developers.                             *
 *                                                                            *
 * See the AUTHORS, DEVELOPER-AGREEMENT and LICENSE files at                  *
 * the top-level directory of this distribution for the individual copyright  *
 * holder information and the developer policies on copyright and licensing.  *
 *                                                                            *
 * Unless otherwise agreed in a custom licensing agreement, no part of the    *
 * SuperNET software, including this file may be copied, modified, propagated *
 * or distributed except according to the terms contained in the LICENSE file *
 *                                                                            *
 * Removal or modification of this copyright notice is prohibited.            *
 *                                                                            *
 ******************************************************************************/

#include <gmock/gmock.h>

#include <string>

#include "http.h"
#include "MockSocket.h"

using ::testing::_;
using ::testing::AtLeast;
using ::testing::Return;
using ::testing::DoAll;
using ::testing::SetArgPointee;

using namespace std;

TEST(HttpTests, postBalanceMethod)
{
    MockSocket sock;
    EXPECT_CALL(sock, doConnect(_, _, _))
            .WillOnce(Return(true));
    EXPECT_CALL(sock, doWrite(_, _, _))
            .Times(AtLeast(1))
            .WillRepeatedly(Return(true));
    EXPECT_CALL(sock, doReadText("\r\n\r\n", _, _))
            .WillOnce(Return(_strdup("HTTP/1.1 200 OK\r\n"
                    "Access-Control-Allow-Origin: *\r\n"
                    "Access-Control-Allow-Credentials: true\r\n"
                    "Access-Control-Allow-Methods: GET, POST\r\n"
                    "Cache-Control :  no-cache, no-store, must-revalidate\r\n"
                    "Content-Length :       93\r\n\r\n")));
    const char *expectedReponseBody = R"({"result":"success","coin":"KMD","address":"RSpP2Nffy379SwF1cAkooNg6vwPHpakCpC","balance":0}
)";
    EXPECT_CALL(sock, doReadBinary(93, _, _, _))
            .WillOnce(Return((void *) expectedReponseBody));
    EXPECT_CALL(sock, doDisconnect())
            .Times(1);
    err_t err;
    URL url;
    parse_url("http://localhost:7783", &url, &err);
    ASSERT_EQ(0, err);

    auto *requestBody = R"({"method":"balance","coin":"KMD","address":"RSpP2Nffy379SwF1cAkooNg6vwPHpakCpC"})";
    size_t responseBodyLen;
    void *responseBody = http_post((AbstractSocket *) &sock, &url, requestBody, strlen(requestBody), &responseBodyLen,
                                   &err);
    ASSERT_EQ(0, err);
    ASSERT_NE(nullptr, responseBody);

    ASSERT_EQ(string(expectedReponseBody), string((char *) responseBody));
}

TEST(HttpTests, postFailed)
{
    MockSocket sock;
    EXPECT_CALL(sock, doConnect(_, _, _))
            .WillOnce(DoAll(SetArgPointee<2>(1), Return(false)));
    err_t err;
    URL url;
    parse_url("http://localhost:7783", &url, &err);
    ASSERT_EQ(0, err);

    auto *requestBody = R"({"method":"balance","coin":"KMD","address":"RSpP2Nffy379SwF1cAkooNg6vwPHpakCpC"})";
    size_t responseBodyLen;
    void *responseBody = http_post((AbstractSocket *) &sock, &url, requestBody, strlen(requestBody), &responseBodyLen,
                                   &err);
    ASSERT_EQ(1, err);
    ASSERT_EQ(nullptr, responseBody);
}

TEST(HttpTests, missingContentLength)
{
    MockSocket sock;
    EXPECT_CALL(sock, doConnect(_, _, _))
            .WillOnce(Return(true));
    EXPECT_CALL(sock, doWrite(_, _, _))
            .Times(AtLeast(1))
            .WillRepeatedly(Return(true));
    EXPECT_CALL(sock, doReadText("\r\n\r\n", _, _))
            .WillOnce(Return(_strdup("HTTP/1.1 200 OK\r\n"
                                            "Access-Control-Allow-Origin: *\r\n"
                                            "Access-Control-Allow-Credentials: true\r\n"
                                            "Access-Control-Allow-Methods: GET, POST\r\n"
                                            "Cache-Control :  no-cache, no-store, must-revalidate\r\n\r\n")));
    EXPECT_CALL(sock, doDisconnect())
            .Times(1);
    err_t err;
    URL url;
    parse_url("http://localhost:7783", &url, &err);
    ASSERT_EQ(0, err);

    auto *requestBody = R"({"method":"balance","coin":"KMD","address":"RSpP2Nffy379SwF1cAkooNg6vwPHpakCpC"})";
    size_t responseBodyLen;
    void *responseBody = http_post((AbstractSocket *) &sock, &url, requestBody, strlen(requestBody), &responseBodyLen,
                                   &err);
    ASSERT_NE(0, err);
    ASSERT_EQ(nullptr, responseBody);
}
