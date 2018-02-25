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

#ifndef MARKETMAKER_CLI_URL_H
#define MARKETMAKER_CLI_URL_H

#include "basic.h"

#include <stdbool.h>
#include <stdint.h>
#ifdef _WIN32
#include <winsock2.h>
#else
#include <netinet/in.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    struct in_addr  addr;
    uint16_t        port;
} URL;

bool parse_url(const char *strUrl, URL *url, err_t *errp);

#ifdef __cplusplus
};
#endif

#endif //MARKETMAKER_CLI_URL_H
