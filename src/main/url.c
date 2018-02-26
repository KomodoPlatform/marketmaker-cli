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

#include "url.h"
#include "strutil.h"
#include "safe_alloc.h"

#include <limits.h>
#include <errno.h>
#include <stdlib.h>
#ifndef _WIN32
#include <netdb.h>
#include <arpa/inet.h>
#endif

#define IS_VALID_PORT(p)    (((p) > 0) && ((p) <= UINT16_MAX))

static bool resolve_hostname(const char *name, struct in_addr *addr, err_t *errp);

bool parse_url(const char *strUrl, URL *url, err_t *errp)
{
    *errp = 0;
    memset(url, 0, sizeof(*url));

    ssize_t index = strstartswith(strUrl, "http://");
    if (index < 0) {
        *errp = EINVAL;
        return false;
    }
    char *copy = safe_strdup(strUrl);
    char *hostname = &copy[index];
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
    free(copy);
    return (*errp == 0);
}

bool resolve_hostname(const char *name, struct in_addr *addr, err_t *errp)
{
    struct addrinfo hints;
    struct addrinfo *result = NULL;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    *errp = getaddrinfo(name, "http", &hints, &result);
    if (*errp == 0) {
        *addr = ((struct sockaddr_in *) result->ai_addr)->sin_addr;
        freeaddrinfo(result);
    }

    return (*errp == 0);
}
