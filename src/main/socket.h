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

typedef struct _AbstractSocket AbstractSocket;

struct _AbstractSocket {
    bool (*connect)(AbstractSocket *socket, const URL *url, int tmout_ms, err_t *errp);

    bool (*write)(AbstractSocket *socket, const void *data, size_t len, err_t *errp);

    char *(*read_text)(AbstractSocket *socket, const char *terminator, int tmout_ms, err_t *errp);

    void *(*read_binary)(AbstractSocket *socket, size_t expectedSize, size_t *actualSize, int tmout_ms,
                         err_t *errp);

    void (*disconnect)(AbstractSocket *socket);
};

#ifdef __cplusplus
};
#endif

#endif //MARKETMAKER_CLI_SOCKET_H
