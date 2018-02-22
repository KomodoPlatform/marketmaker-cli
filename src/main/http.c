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

void *http_post(AbstractSocket *absSocket, const URL *url, const void *requestBody, size_t requestBodyLen,
                size_t *responseBodyLen, err_t *errp)
{
    if (!absSocket->connect(absSocket, url, CONNECT_TIMEOUT, errp)) {
        return NULL;
    }
    char requestHeader[128];
    snprintf(requestHeader, sizeof(requestHeader), "POST / HTTP/1.1\r\n"
            "Accept: */*\r\n"
            "Content-Length: %d\r\n"
            "Content-Type: application/x-www-form-urlencoded\r\n"
            "\r\n", (int) strlen(requestBody));

    void *responseBody = NULL;
    if (absSocket->write(absSocket, requestHeader, strlen(requestHeader), errp)
        && absSocket->write(absSocket, requestBody, requestBodyLen, errp)) {
        char *responseHeader = absSocket->read_text(absSocket, "\r\n\r\n", READ_TIMEOUT, errp);
        if (responseHeader != NULL) {
            PropertyGroup *headers = parse_properties(responseHeader, ':', PARSE_OPT_IGNORE_INVALID_LINES, errp);
            free(responseHeader);
            if (*errp == 0) {
                size_t contentLength = http_header_get_ulong(headers, "Content-Length", errp);
                free(headers);
                if (*errp == 0) {
                    responseBody = absSocket->read_binary(absSocket, contentLength, responseBodyLen, READ_TIMEOUT,
                                                          errp);
                }
            }
        }
    }

    absSocket->disconnect(absSocket);
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
