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

#include "safe_alloc.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static void assert_not_null(void *ptr);

void *safe_malloc(size_t size)
{
    void *data = malloc(size);
    assert_not_null(data);
    return data;
}

char *safe_strdup(const char *str)
{
    char *copy = _strdup(str);
    assert_not_null(copy);
    return copy;
}

void *safe_realloc(void *data, size_t size)
{
    void *newData = realloc(data, size);
    assert_not_null(newData);
    return newData;
}

void assert_not_null(void *ptr)
{
    if (ptr == NULL) {
        fputs("Out of memory!", stderr);
        exit(-1);
    }
}
