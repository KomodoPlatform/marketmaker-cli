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

#ifndef MARKETMAKER_CLI_STRUTIL_H
#define MARKETMAKER_CLI_STRUTIL_H

#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#ifdef __cpluscplus
extern "C" {
#endif

static inline bool strequal(const char *s1, const char *s2) {
    return strcmp(s1, s2) == 0;
}

static inline bool strequalIgnoreCase(const char *s1, const char *s2) {
    return strcasecmp(s1, s2) == 0;
}

char *strtrim(char *s);

ssize_t strstartswith(const char *s, const char *prefix);

#ifdef __cpluscplus
};
#endif

#endif //MARKETMAKER_CLI_STRUTIL_H
