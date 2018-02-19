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

#include "socket.h"

#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/fcntl.h>
#include <unistd.h>

#ifndef min
#define min(a, b)   ((a) < (b) ? (a) : (b))
#endif

typedef enum {
    SEL_READ,
    SEL_WRITE,
    SEL_ERROR
} SelectSet;

static bool socket_get_error(SOCKET sockfd, err_t *errp);

static bool socket_select(int sockfd, SelectSet setId, int tmout_ms, err_t *errp);

static size_t socket_read(SOCKET sockfd, void *buf, size_t nbyte, int tmout_ms, err_t *errp);

SOCKET socket_connect(const URL *url, int tmout_ms, err_t *errp)
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
               (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK, 0) < 0)) {
        *errp = errno;
    } else if (connect(sockfd, (struct sockaddr *) &sin, sizeof(sin)) < 0) {
        if ((errno == EINPROGRESS) || (errno == EAGAIN)) {
            socket_select(sockfd, SEL_WRITE, tmout_ms, errp);
        } else {
            *errp = errno;
        }
    } else {
        *errp = 0;
    }
    if ((*errp != 0) && (sockfd != INVALID_SOCKET)) {
        close(sockfd);
        sockfd = INVALID_SOCKET;
    }
    return sockfd;
}

bool socket_write(SOCKET sockfd, const void *data, size_t len, err_t *errp)
{
    *errp = 0;
    const char *p = data;
    for (size_t ofs = 0; ofs < len;) {
        ssize_t n = write(sockfd, &p[ofs], len - ofs);
        if (n <= 0) {
            *errp = errno;
            return false;
        }
        ofs += n;
    }
    return true;
}

char *socket_read_text(SOCKET sockfd, const char *terminator, int tmout_ms, SockReadBuffer *pbBuffer, err_t *errp)
{
    char *data = NULL;
    size_t dataSize = 0;
    for (;;) {
        if (pbBuffer->position == 0) {
            pbBuffer->position = socket_read(sockfd, pbBuffer->data, sizeof(pbBuffer->data), tmout_ms, errp);
            if (*errp != 0) {
                break;
            }
        }
        // pbBuffer->filler makes this always safe
        pbBuffer->data[pbBuffer->position] = '\0';
        char *p = strstr(pbBuffer->data, terminator);
        size_t n = (p == NULL) ? pbBuffer->position : (p - pbBuffer->data) + strlen(terminator);
        if (n > 0) {
            // +1: always leave room for an extra '\0'
            data = realloc(data, dataSize + n + 1);
            memcpy(&data[dataSize], pbBuffer->data, n);
            dataSize += n;
            // shift left left-out data
            if ((pbBuffer->position -= n) > 0) {
                memmove(pbBuffer->data, &pbBuffer->data[n], pbBuffer->position);
            }
        }
        if (p != NULL) {
            break;
        }
    }
    if (*errp != 0) {
        free(data);
        data = NULL;
    }
    if (data != NULL) {
        data[dataSize] = '\0';
    }
    return data;
}

void *socket_read_binary(SOCKET sockfd, size_t expectedSize, size_t *actualSize, int tmout_ms, SockReadBuffer *pbBuffer,
                         err_t *errp)
{
    *actualSize = 0;
    *errp = 0;
    char *data = NULL;
    size_t dataSize = 0;
    while (dataSize < expectedSize) {
        if (pbBuffer->position == 0) {
            size_t nbytesToRead = min(sizeof(pbBuffer->data), expectedSize - dataSize);
            pbBuffer->position = socket_read(sockfd, pbBuffer->data, nbytesToRead, tmout_ms, errp);
            if (*errp != 0) {
                break;
            }
            // assert pbBuffer->position > 0
        }
        // +1: always leave for an extra '\0'
        data = realloc(data, dataSize + pbBuffer->position + 1);
        memcpy(&data[dataSize], pbBuffer->data, pbBuffer->position);
        dataSize += pbBuffer->position;
        pbBuffer->position = 0;
    }
    if (*errp != 0) {
        free(data);
        data = NULL;
    }
    if (data != NULL) {
        data[dataSize] = '\0';
        *actualSize = dataSize;
    }
    return data;
}

void sockReadBuffer_init(SockReadBuffer *pbBuffer)
{
    pbBuffer->position = 0;
}

bool socket_select(int sockfd, SelectSet setId, int tmout_ms, err_t *errp)
{
    fd_set fdset;
    fd_set *read_fdset = NULL;
    fd_set *write_fdset = NULL;
    fd_set *error_fdset = NULL;
    struct timeval tv;

    *errp = 0;
    FD_ZERO(&fdset);
    FD_SET(sockfd, &fdset);
    switch (setId) {
        case SEL_READ:
            read_fdset = &fdset;
            break;
        case SEL_WRITE:
            write_fdset = &fdset;
            break;
        case SEL_ERROR:
            error_fdset = &fdset;
            break;
    }
    tv.tv_sec = tmout_ms / 1000;
    tv.tv_usec = (tmout_ms % 1000) * 1000;
    switch (select(sockfd + 1, read_fdset, write_fdset, error_fdset, &tv)) {
        case 1:
            socket_get_error(sockfd, errp);
            break;

        case 0:
            *errp = ETIMEDOUT;
            break;

        default:
            *errp = errno;
            break;
    }
    return *errp == 0;
}

bool socket_get_error(SOCKET sockfd, err_t *errp)
{
    *errp = 0;
    socklen_t len = sizeof(*errp);
    getsockopt(sockfd, SOL_SOCKET, SO_ERROR, errp, &len);
    return *errp == 0;
}

size_t socket_read(SOCKET sockfd, void *buf, size_t nbyte, int tmout_ms, err_t *errp)
{
    if (socket_select(sockfd, SEL_READ, tmout_ms, errp)) {
        ssize_t rc = read(sockfd, buf, nbyte);
        if (rc > 0) {
            return (size_t) rc;
        }
        *errp = (rc == 0) ? ENODATA : errno;
    }
    return 0;
}
