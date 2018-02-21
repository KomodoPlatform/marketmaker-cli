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

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "json.h"

char *build_json_request(const PropertyGroup *group, err_t *errp)
{
    *errp = 0;
    size_t capacity = 64;
    size_t ofs = 0;
    char *buffer = malloc(capacity);
    buffer[ofs++] = '{';
    for (size_t i = 0; i < group->size; i++) {
        if (i > 0) {
            buffer[ofs++] = ',';
        }
        for (;;) {
            Property *prop = &group->properties[i];
            int rc = snprintf(&buffer[ofs], capacity - ofs, "\"%s\":\"%s\"", prop->key, prop->value);
            if (rc < 0) {
                *errp = errno;
                break;
            }
            if (ofs + rc < capacity) {
                ofs += rc;
                break;
            }
            capacity *= 2;
            buffer = realloc(buffer, capacity);
        }
    }
    buffer[ofs++] = '}';
    buffer[ofs] = '\0';
    return buffer;
}
