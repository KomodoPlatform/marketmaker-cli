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

#include "api_help.h"

#include "strutil.h"
#include "json.h"
#include "http.h"
#include "property.h"
#include "safe_alloc.h"

#include <errno.h>
#include <stdlib.h>

#define INITIAL_CAPACITY 50

static Property UNDOCUMENTED_API_PROPS[] = {{"help", ""}};
static const PropertyGroup UNDOCUMENTED_API = {
        DIMOF(UNDOCUMENTED_API_PROPS),
        DIMOF(UNDOCUMENTED_API_PROPS),
        UNDOCUMENTED_API_PROPS
};

static char *strip_params(char *src);

PropertyGroup *parse_api_help(const char *str, err_t *errp)
{
    *errp = 0;
    char *copy = safe_strdup(str);
    PropertyGroup *group = alloc_properties(INITIAL_CAPACITY);
    group->size = 0;
    char *s = copy;
    char *p;
    do {
        if ((p = strchr(s, '\n')) != NULL) {
            *p++ = '\0';
        }
        char *sep;
        // strip script-like comments
        if ((sep = strchr(s, '#')) != NULL) {
            *sep = '\0';
        }
        // strip C/C++ one-line comments
        if ((sep = strstr(s, "//")) != NULL) {
            *sep = '\0';
        }
        // strip everything beyond end-of-parameters delimiter
        if ((sep = strchr(s, ')')) != NULL) {
            *sep = '\0';
        }
        // try find start-of-parameters delimiter
        if ((sep = strchr(s, '(')) != NULL) {
            *sep++ = '\0';
            const char *key = strtrim(s);
            if (*key != '\0') {
                group = add_property(group, key, strip_params(strtrim(sep)), errp);
                if (*errp != 0) {
                    break;
                }
            }
        }
        s = p;
    } while (s != NULL);
    if (*errp == 0) {
        return realloc_properties(group, group->size);
    }
    free(copy);
    return NULL;
}

PropertyGroup *fetch_api(AbstractSocket *absSocket, const URL *url, err_t *errp)
{
    *errp = 0;
    Property props[] = {
            {"method", "help"}
    };
    PropertyGroup group = {DIMOF(props), DIMOF(props), props};
    char *jsonRequest = build_json_request(&group, errp);
    if (*errp != 0) {
        return NULL;
    }
    size_t responseLen = 0;
    char *response = http_post(absSocket, url, jsonRequest, strlen(jsonRequest), &responseLen, errp);
    free(jsonRequest);
    if (*errp != 0) {
        return NULL;
    }
    PropertyGroup *api = parse_api_help(response, errp);
    if (api != NULL) {
        api = put_all_properties(api, &UNDOCUMENTED_API, errp);
    }
    free(response);
    return api;
}

char *strip_params(char *src)
{
    char *dest = src;   // in-place result
    char *d = dest;     // result cursor
    char *next_parm = NULL;
    bool has_next = true;
    for (char *s = src; has_next; s = next_parm) {
        if ((next_parm = strchr(s, ',')) != NULL) {
            *next_parm++ = '\0';
        } else {
            has_next = false;
        }
        s = strtrim(s);
        size_t slen = strlen(s);
        // ignore parameters wrapped with '<>'
        if ((slen >= 2) && (*s == '<') && (s[slen - 1] == '>')) {
            continue;
        }
        // strip wrappers '[]' used to indicate optional parameters
        if ((slen >= 2) && (*s == '[') && (s[slen - 1] == ']')) {
            s[slen - 1] = '\0';
            s = strtrim(&s[1]);
        }
        slen = strlen(s);
        // strip suffix '[]' used to indicate a multi-dimensional parameter
        if ((slen >= 2) && (s[slen - 2] == '[') && (s[slen - 1] == ']')) {
            s[slen - 2] = '\0';
            s = strtrim(s);
        }
        char *sep;
        // strip default value
        if ((sep = strchr(s, '='))) {
            *sep = '\0';
        }
        slen = strlen(s);
        if (slen > 0) {
            if (d > dest) {
                *d++ = ',';
            }
            memmove(d, s, slen);
            d += slen;
        }
    }
    *d = '\0';
    return dest;
}
