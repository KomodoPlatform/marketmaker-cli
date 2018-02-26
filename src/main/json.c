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
#include <string.h>
#include <errno.h>

#include "json.h"
#include "property.h"
#include "safe_alloc.h"

char *build_json_request(const PropertyGroup *group, err_t *errp)
{
    *errp = 0;
    size_t capacity = 64;
    size_t ofs = 0;
    char *buffer = malloc(capacity);
    buffer[ofs++] = '{';
    for (size_t i = 0; i < group->size; i++) {
        Property *prop = &group->properties[i];
        size_t klen = strlen(prop->key);
        size_t vlen = strlen(prop->value);
        size_t xlen = (i > 0) + klen + vlen + (2 + 1 + 2 + 1 + 1);
        if (ofs + xlen > capacity) {
            do {
                capacity *= 2;
            } while (ofs + xlen > capacity);
            buffer = safe_realloc(buffer, capacity);
        }
        if (i > 0) {
            buffer[ofs++] = ',';
        }
        buffer[ofs++] = '"';
        memcpy(&buffer[ofs], prop->key, klen);
        ofs += klen;
        buffer[ofs++] = '"';
        buffer[ofs++] = ':';
        buffer[ofs++] = '"';
        memcpy(&buffer[ofs], prop->value, vlen);
        ofs += vlen;
        buffer[ofs++] = '"';
    }
    buffer[ofs++] = '}';
    buffer[ofs] = '\0';
    return buffer;
}
