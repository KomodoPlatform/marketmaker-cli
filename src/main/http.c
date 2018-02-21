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

#include "http.h"
#include "sys_socket.h"
#include "strutil.h"
#include "property.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>

static const int CONNECT_TIMEOUT = 3000;
static const int READ_TIMEOUT = 5000;

static unsigned long http_header_get_ulong(PropertyGroup *headers, const char *key, err_t *errp);

void *http_post(const URL *url, const void *requestBody, size_t requestBodyLen, size_t *responseBodyLen, err_t *errp)
{
    *errp = 0;
    SOCKET sockfd = socket_connect(url, CONNECT_TIMEOUT, errp);
    if (*errp != 0) {
        return NULL;
    }
    char requestHeader[128];
    snprintf(requestHeader, sizeof(requestHeader), "POST / HTTP/1.1\r\n"
            "Accept: */*\r\n"
            "Content-Length: %d\r\n"
            "Content-Type: application/x-www-form-urlencoded\r\n"
            "\r\n", (int) strlen(requestBody));
    socket_write(sockfd, requestHeader, strlen(requestHeader), errp);
    if (*errp == 0) {
        socket_write(sockfd, requestBody, requestBodyLen, errp);
    }

    void *responseBody = NULL;

    if (*errp == 0) {
        SockReadBuffer buffer;
        sockReadBuffer_init(&buffer);
        char *responseHeader;
        if ((responseHeader = socket_read_text(sockfd, "\r\n\r\n", READ_TIMEOUT, &buffer, errp)) != NULL) {
            PropertyGroup *headers = parse_properties(responseHeader, ':', PARSE_OPT_IGNORE_INVALID_LINES, errp);
            free(responseHeader);
            if (*errp == 0) {
                size_t contentLength = http_header_get_ulong(headers, "Content-Length", errp);
                free(headers);
                if (*errp == 0) {
                    responseBody = socket_read_binary(sockfd, contentLength, responseBodyLen, READ_TIMEOUT, &buffer,
                                                      errp);
                }
            }
        }
    }

    close(sockfd);
    return responseBody;
}

unsigned long http_header_get_ulong(PropertyGroup *headers, const char *key, err_t *errp)
{
    *errp = ENOENT;
    if (headers != NULL) {
        const char *valueStr = find_property_ignore_case(headers, key);
        if (valueStr != NULL) {
            unsigned long value = strtoul(valueStr, NULL, 10);
            if (value != ULONG_MAX) {
                *errp = 0;
                return value;
            }
        }
    }
    return 0;
}
