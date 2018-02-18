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

#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <netdb.h>
#include <stdint.h>

#include "http.h"
#include "strutil.h"

typedef int SOCKET;

#define INVALID_SOCKET  ((SOCKET) - 1)

#define IS_VALID_PORT(p)    (((p) > 0) && ((p) <= UINT16_MAX))

typedef enum {
    IN_HEADER,
    IN_BODY
} ReadState;

static int tcp_connect(const URL *url, err_t *errp);

static bool resolve_hostname(const char *name, struct in_addr *addr, err_t *errp);

bool parse_url(const char *strUrl, URL *url, err_t *errp)
{
    char copy[64];
    char *hostname;

    strcopy(copy, strUrl, sizeof(copy));
    memset(url, 0, sizeof(*url));

    ssize_t index;
    if ((index = strstartswith(copy, "http://")) < 0) {
        *errp = EINVAL;
        return false;
    }
    hostname = &copy[index];
    char *p;
    if ((p = strchr(hostname, ':')) != NULL) {
        *p++ = '\0';
        long port = strtol(p, NULL, 0);
        if (IS_VALID_PORT(port)) {
            url->port = (uint16_t) port;
            resolve_hostname(hostname, &url->addr, errp);
        } else {
            *errp = EINVAL;
        }
    } else {
        *errp = EINVAL;
    }
    return (*errp == 0);
}

char *http_post(const URL *url, const char *body, err_t *errp)
{
    *errp = 0;
    SOCKET socket = tcp_connect(url, errp);
    if (*errp != 0) {
        return NULL;
    }
    if (*errp == 0) {
        char headers[128];
        snprintf(headers, sizeof(headers), "POST / HTTP/1.1\r\n"
                "Accept: */*\r\n"
                "Content-Length: %d\r\n"
                "Content-Type: application/x-www-form-urlencoded\r\n"
                "\r\n", (int) strlen(body));
        ssize_t rc = write(socket, headers, strlen(headers));
        if (rc < 0) {
            *errp = errno;
        }
    }
    if (*errp == 0) {
        ssize_t rc = write(socket, body, strlen(body));
        if (rc < 0) {
            *errp = errno;
        }
    }

    if (*errp != 0) {
        close(socket);
        return NULL;
    }
    size_t capacity = 256;
    char *buffer = malloc(capacity);
    ReadState readState = IN_HEADER;
    for (size_t ofs = 0; ofs + 1 < capacity; ) {
        // always leave room for '\0'
        ssize_t rc = read(socket, &buffer[ofs], capacity - ofs - 1);
        if (rc < 0) {
            *errp = errno;
            break;
        }
        if (rc == 0) {
            *errp = EIO;
            break;
        }
        ofs += rc;
        buffer[ofs] = '\0';
        if (readState == IN_HEADER) {
            // headers fully read?
            char *endMark;
            if ((endMark = strstr(buffer, "\r\n\r\n")) != 0) {
                readState = IN_BODY;
                size_t dataOfs = (endMark - buffer) + 4;
                char *p;
                *errp = EINVAL;
                if ((p = strstr(buffer, "\r\nContent-Length")) != NULL) {
                    char *value;
                    if ((value = strchr(p, ':')) != NULL) {
                        *value++ = '\0';
                        if ((p = strstr(value, "\r\n")) != NULL) {
                            *p = '\0';
                            capacity = (size_t) atol(strtrim(value)) + 1;
                            buffer = realloc(buffer, capacity);
                            ofs -= dataOfs;
                            if (ofs > 0) {
                                memmove(buffer, &buffer[dataOfs], ofs);
                            } else {
                                ofs = 0;
                            }
                            *errp = 0;
                        }
                    }
                }
                if (*errp != 0) {
                    break;
                }
            }
        }
    }
    if (*errp != 0) {
        free(buffer);
        buffer = NULL;
    }
    close(socket);
    return buffer;
}

bool resolve_hostname(const char *name, struct in_addr *addr, err_t *errp)
{
    *errp = 0;
    if ((addr->s_addr = inet_addr(name)) == (in_addr_t) -1) {
        struct hostent *hep;

        if ((hep = gethostbyname(name)) != NULL) {
            memcpy(&addr->s_addr, hep->h_addr, sizeof(addr->s_addr));
        } else {
            *errp = 500 + h_errno;
        }
    }

    return (*errp == 0);
}

int tcp_connect(const URL *url, err_t *errp)
{
    struct sockaddr_in sin;
    SOCKET sockfd;
    int flags;

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    memcpy(&sin.sin_addr, &url->addr, sizeof(url->addr));
    sin.sin_port = htons(url->port);
    *errp = 0;
    if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        *errp = errno;
    } else if (((flags = fcntl(sockfd, F_GETFL, 0)) < 0) ||
               (fcntl(sockfd, F_SETFL, flags /*| O_NONBLOCK*/, 0) < 0)) {
        *errp = errno;
    } else if ((connect(sockfd, (struct sockaddr *) &sin, sizeof(sin)) == 0) ||
               (errno == EINPROGRESS)) {
        *errp = 0;
    } else {
        *errp = errno;
    }
    if ((*errp != 0) && (sockfd != INVALID_SOCKET)) {
        close(sockfd);
        sockfd = INVALID_SOCKET;
    }
    return sockfd;
}
