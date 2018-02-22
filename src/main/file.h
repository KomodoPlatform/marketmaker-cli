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

#ifndef MARKETMAKER_CLI_FILE_H
#define MARKETMAKER_CLI_FILE_H

#include <stdbool.h>
#include <stdio.h>
#include "basic.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _AbstractFile AbstractFile;

struct _AbstractFile {
    bool (*open)(AbstractFile *absFile, const char *pathname, const char *mode, err_t *errp);

    bool (*seek)(AbstractFile *absFile, long offset, int whence, err_t *errp);

    long (*tell)(AbstractFile *absFile, err_t *errp);

    size_t (*read)(AbstractFile *absFile, void *ptr, size_t size, err_t *errp);

    bool (*write)(AbstractFile *absFile, const void *ptr, size_t size, err_t *errp);

    void (*close)(AbstractFile *absFile);
};

#ifdef __cplusplus
};
#endif

#endif //MARKETMAKER_CLI_FILE_H
