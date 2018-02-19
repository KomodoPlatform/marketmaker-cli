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

#ifndef MARKETMAKER_CLI_SOCKET_H
#define MARKETMAKER_CLI_SOCKET_H

#include "url.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int SOCKET;

#define INVALID_SOCKET  ((SOCKET) - 1)

typedef struct {
    char    data[2048];
    // to guarantee that it's always safe to do data[position] = '\0'
    char    filler;
    size_t  position;
} SockReadBuffer;

void sockReadBuffer_init(SockReadBuffer *pbBuffer);

SOCKET socket_connect(const URL *url, int tmout_ms, err_t *errp);

bool socket_write(SOCKET sockfd, const void *data, size_t len, err_t *errp);

char *socket_read_text(SOCKET sockfd, const char *terminator, int tmout_ms, SockReadBuffer *pbBuffer, err_t *errp);

void *socket_read_binary(SOCKET sockfd, size_t expectedSize, size_t *actualSize, int tmout_ms, SockReadBuffer *pbBuffer,
                         err_t *errp);


#ifdef __cplusplus
};
#endif

#endif //MARKETMAKER_CLI_SOCKET_H
