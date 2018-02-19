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

#include "property.h"

#ifndef MARKETMAKER_CLI_API_HELP_H
#define MARKETMAKER_CLI_API_HELP_H

#include "property.h"
#include "url.h"

#ifdef __cplusplus
extern "C" {
#endif

PropertyGroup *parse_api_help(const char *str, err_t *errp);

PropertyGroup *fetch_api(const URL *url, err_t *errp);

#ifdef __cplusplus
};
#endif

#endif //MARKETMAKER_CLI_API_HELP_H
