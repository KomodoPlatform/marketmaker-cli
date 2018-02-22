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

#include "basic.h"

#include <stdbool.h>
#include <arpa/inet.h>

#ifndef MARKETMAKER_CLI_HTTP_H
#define MARKETMAKER_CLI_HTTP_H

#include "url.h"
#include "socket.h"

#ifdef __cplusplus
extern "C" {
#endif

void *http_post(AbstractSocket *socket, const URL *url, const void *requestBody, size_t requestBodyLen,
                size_t *responseBodyLen, err_t *errp);

#ifdef __cplusplus
};
#endif

#endif //MARKETMAKER_CLI_HTTP_H
