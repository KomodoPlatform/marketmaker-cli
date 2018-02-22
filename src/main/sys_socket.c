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

#include "sys_socket.h"

#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/fcntl.h>
#include <unistd.h>

#ifndef min
#define min(a, b)   ((a) < (b) ? (a) : (b))
#endif

#define INVALID_SOCKET  ((SOCKET) - 1)

typedef enum {
    SEL_READ,
    SEL_WRITE,
    SEL_ERROR
} SelectSet;

static bool socket_get_error(SOCKET sockfd, err_t *errp);

static bool socket_select(int sockfd, SelectSet setId, int tmout_ms, err_t *errp);

static size_t socket_read(SysSocket *sock, void *buf, size_t nbyte, int tmout_ms, err_t *errp);

static bool socket_connect(AbstractSocket *absSocket, const URL *url, int tmout_ms, err_t *errp)
{
    SysSocket *sock = (SysSocket *) absSocket;
    struct sockaddr_in sin;
    int flags;

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    memcpy(&sin.sin_addr, &url->addr, sizeof(url->addr));
    sin.sin_port = htons(url->port);
    *errp = 0;
    if ((sock->sockfd = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        *errp = errno;
    } else if (((flags = fcntl(sock->sockfd, F_GETFL, 0)) < 0) ||
               (fcntl(sock->sockfd, F_SETFL, flags | O_NONBLOCK, 0) < 0)) {
        *errp = errno;
    } else if (connect(sock->sockfd, (struct sockaddr *) &sin, sizeof(sin)) < 0) {
        if ((errno == EINPROGRESS) || (errno == EAGAIN)) {
            socket_select(sock->sockfd, SEL_WRITE, tmout_ms, errp);
        } else {
            *errp = errno;
        }
    } else {
        *errp = 0;
    }
    if ((*errp != 0) && (sock->sockfd != INVALID_SOCKET)) {
        close(sock->sockfd);
        sock->sockfd = INVALID_SOCKET;
    }
    return *errp == 0;
}

static void socket_disconnect(AbstractSocket *absSocket)
{
    SysSocket *sock = (SysSocket *) absSocket;

    if (sock->sockfd != INVALID_SOCKET) {
        close(sock->sockfd);
    }
}

static bool socket_write(AbstractSocket *absSocket, const void *data, size_t len, err_t *errp)
{
    SysSocket *sock = (SysSocket *) absSocket;
    *errp = 0;
    const char *p = data;
    for (size_t ofs = 0; ofs < len;) {
        ssize_t n = write(sock->sockfd, &p[ofs], len - ofs);
        if (n <= 0) {
            *errp = errno;
            return false;
        }
        ofs += n;
    }
    return true;
}

static char *socket_read_text(AbstractSocket *absSocket, const char *terminator, int tmout_ms,
                              err_t *errp)
{
    SysSocket *sock = (SysSocket *) absSocket;
    SockReadBuffer *rb = &sock->readBuffer;
    char *data = NULL;
    size_t dataSize = 0;
    for (;;) {
        if (rb->position == 0) {
            rb->position = socket_read(sock, rb->data, sizeof(rb->data), tmout_ms, errp);
            if (*errp != 0) {
                break;
            }
        }
        // rb->filler makes this always safe
        rb->data[rb->position] = '\0';
        char *p = strstr(rb->data, terminator);
        size_t n = (p == NULL) ? rb->position : (p - rb->data) + strlen(terminator);
        if (n > 0) {
            // +1: always leave room for an extra '\0'
            data = realloc(data, dataSize + n + 1);
            memcpy(&data[dataSize], rb->data, n);
            dataSize += n;
            // shift left left-out data
            if ((rb->position -= n) > 0) {
                memmove(rb->data, &rb->data[n], rb->position);
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

static void *socket_read_binary(AbstractSocket *absSocket, size_t expectedSize, size_t *actualSize, int tmout_ms,
                                err_t *errp)
{
    SysSocket *sock = (SysSocket *) absSocket;
    SockReadBuffer *rb = &sock->readBuffer;

    *actualSize = 0;
    *errp = 0;
    char *data = NULL;
    size_t dataSize = 0;
    while (dataSize < expectedSize) {
        if (rb->position == 0) {
            size_t nbytesToRead = min(sizeof(rb->data), expectedSize - dataSize);
            rb->position = socket_read(sock, rb->data, nbytesToRead, tmout_ms, errp);
            if (*errp != 0) {
                break;
            }
            // assert pbBuffer->position > 0
        }
        // +1: always leave for an extra '\0'
        data = realloc(data, dataSize + rb->position + 1);
        memcpy(&data[dataSize], rb->data, rb->position);
        dataSize += rb->position;
        rb->position = 0;
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

void sys_socket_init(SysSocket *sock)
{
    sock->as.connect = socket_connect;
    sock->as.write = socket_write;
    sock->as.read_text = socket_read_text;
    sock->as.read_binary = socket_read_binary;
    sock->as.disconnect = socket_disconnect;

    sock->sockfd = INVALID_SOCKET;
    sock->readBuffer.position = 0;
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

size_t socket_read(SysSocket *sock, void *buf, size_t nbyte, int tmout_ms, err_t *errp)
{
    if (socket_select(sock->sockfd, SEL_READ, tmout_ms, errp)) {
        ssize_t rc = read(sock->sockfd, buf, nbyte);
        if (rc > 0) {
            return (size_t) rc;
        }
        *errp = (rc == 0) ? ENODATA : errno;
    }
    return 0;
}
