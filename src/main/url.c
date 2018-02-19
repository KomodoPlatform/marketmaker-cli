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

#include <limits.h>
#include <errno.h>
#include <stdlib.h>
#include <netdb.h>
#include <arpa/inet.h>

#define IS_VALID_PORT(p)    (((p) > 0) && ((p) <= UINT16_MAX))

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