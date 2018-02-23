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

#include "sys_file.h"

#include <errno.h>

static bool sys_open(AbstractFile *absFile, const char *pathname, const char *mode, err_t *errp);

static long sys_size(AbstractFile *absFile, err_t *errp);

static size_t sys_read(AbstractFile *absFile, void *ptr, size_t size, err_t *errp);

static bool sys_write(AbstractFile *absFile, const void *ptr, size_t size, err_t *errp);

static void sys_close(AbstractFile *absFile);

void sys_file_init(SysFile *file)
{
    file->af.open = sys_open;
    file->af.size = sys_size;
    file->af.read = sys_read;
    file->af.write = sys_write;
    file->af.close = sys_close;
    file->file = NULL;
}

bool sys_open(AbstractFile *absFile, const char *pathname, const char *mode, err_t *errp)
{
    SysFile *file = (SysFile *) absFile;
    *errp = 0;
    file->file = fopen(pathname, mode);
    if (file->file == NULL) {
        *errp = errno;
        return false;
    }
    return true;
}

long sys_size(AbstractFile *absFile, err_t *errp)
{
    SysFile *file = (SysFile *) absFile;
    *errp = 0;

    long oldPos = ftell(file->file);
    if (oldPos < 0) {
        *errp = errno;
        return 0;
    }
    if (fseek(file->file, 0, SEEK_END) < 0) {
        *errp = errno;
        return 0;
    }
    long lastPos = ftell(file->file);
    if (lastPos < 0) {
        *errp = errno;
    }
    (void) fseek(file->file, oldPos, SEEK_SET);

    return (*errp == 0) ? lastPos : 0;
}

size_t sys_read(AbstractFile *absFile, void *ptr, size_t size, err_t *errp)
{
    SysFile *file = (SysFile *) absFile;
    *errp = 0;

    size_t rc = fread(ptr, 1, size, file->file);
    if (rc == 0) {
        *errp = ferror(file->file) ? errno : 0;
        return 0;
    }
    return rc;
}

bool sys_write(AbstractFile *absFile, const void *ptr, size_t size, err_t *errp)
{
    SysFile *file = (SysFile *) absFile;
    *errp = 0;

    if (size == 0) {
        return true;
    }
    size_t rc = fwrite(ptr, 1, size, file->file);
    if (rc == 0) {
        *errp = errno;
        return false;
    }
    return true;
}

void sys_close(AbstractFile *absFile)
{
    SysFile *file = (SysFile *) absFile;

    fclose(file->file);
    file->file = NULL;
}

