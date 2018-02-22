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

#ifndef MARKETMAKER_CLI_SAFE_ALLOC_H
#define MARKETMAKER_CLI_SAFE_ALLOC_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void *safe_malloc(size_t size);

void *safe_realloc(void *data, size_t size);

char *safe_strdup(const char *str);

#ifdef __cplusplus
};
#endif

#endif //MARKETMAKER_CLI_SAFE_ALLOC_H
