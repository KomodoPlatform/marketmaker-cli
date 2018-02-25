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

#ifndef MARKETMAKER_CLI_SYS_SOCKET_H
#define MARKETMAKER_CLI_SYS_SOCKET_H

#include "url.h"
#include "socket.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _WIN32
typedef int SOCKET;
#endif

typedef struct {
    char    data[2048];
    // to guarantee that it's always safe to do data[position] = '\0'
    char    filler;
    size_t  position;
} SockReadBuffer;

typedef struct {
    AbstractSocket  as;
    SOCKET          sockfd;
    SockReadBuffer  readBuffer;
} SysSocket;

void sys_socket_init(SysSocket *sock);

#ifdef __cplusplus
};
#endif

#endif //MARKETMAKER_CLI_SYS_SOCKET_H
