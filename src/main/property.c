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

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "property.h"
#include "strutil.h"
#include "safe_alloc.h"
#include "file.h"

static const int INITIAL_CAPACITY = 10;

static char *load_file_contents(AbstractFile *absFile, const char *path, err_t *errp);

PropertyGroup *parse_properties(const char *str, char separator, int options, err_t *errp)
{
    *errp = 0;
    char *copy = safe_strdup(str);
    PropertyGroup *group = alloc_properties(INITIAL_CAPACITY);
    group->size = 0;
    char *next_line = NULL;
    for (char *s = copy; s != NULL; s = next_line) {
        if ((next_line = strchr(s, '\n')) != NULL) {
            *next_line++ = '\0';
        }
        s = strtrim(s);
        if (*s != '\0') {
            char *sep;
            if ((sep = strchr(s, separator)) == NULL) {
                if ((options & PARSE_OPT_IGNORE_INVALID_LINES) != 0) {
                    continue;
                }
                *errp = EINVAL;
                break;
            }
            *sep++ = '\0';
            group = add_property(group, strtrim(s), strtrim(sep), errp);
            if (*errp != 0) {
                break;
            }
        }
    }
    if (*errp == 0) {
        // TODO: copy is leaking!
        return realloc_properties(group, group->size);
    }
    free(copy);
    return NULL;
}

PropertyGroup *load_properties(AbstractFile *absFile, const char *path, err_t *err)
{
    char *buffer = load_file_contents(absFile, path, err);
    PropertyGroup *group = (*err == 0) ? parse_properties(buffer, '=', 0, err) : NULL;
    if (group != NULL) {
        sort_properties(group);
    }
    free(buffer);
    return group;
}

char *load_file_contents(AbstractFile *absFile, const char *path, err_t *errp)
{
    if (!absFile->open(absFile, path, "rb", errp)) {
        return NULL;
    }
    char *buffer = NULL;
    if (absFile->seek(absFile, 0, SEEK_END, errp)) {
        long length = absFile->tell(absFile, errp);
        absFile->seek(absFile, 0, SEEK_SET, errp);
        buffer = safe_malloc((size_t) length + 1);
        absFile->read(absFile, buffer, (size_t) length, errp);
        buffer[length] = '\0';
    }
    absFile->close(absFile);
    return buffer;
}

bool save_properties(const PropertyGroup *group, AbstractFile *absFile, const char *path, err_t *err)
{
    char line[256];
    *err = 0;
    if (!absFile->open(absFile, path, "w+b", err)) {
        return false;
    }
    for (int i = 0; i < group->size; i++) {
        snprintf(line, sizeof(line), "%s=%s\n", group->properties[i].key, group->properties[i].value);
        absFile->write(absFile, line, strlen(line), err);
    }
    absFile->close(absFile);
    return true;
}

PropertyGroup *alloc_properties(size_t newCapacity)
{
    return realloc_properties(NULL, newCapacity);
}

PropertyGroup *realloc_properties(PropertyGroup *group, size_t newCapacity)
{
    size_t newTotalSize = sizeof(PropertyGroup) + newCapacity * sizeof(Property);
    PropertyGroup *newGroup = safe_realloc(group, newTotalSize);
    if (group == NULL) {
        newGroup->size = 0;
    }
    newGroup->capacity = newCapacity;
    newGroup->properties = (Property *) (newGroup + 1);
    return newGroup;
}

const char *find_property(const PropertyGroup *group, const char *key)
{
    for (int i = 0; i < group->size; i++) {
        const Property *prop = &group->properties[i];
        if (strequal(prop->key, key)) {
            return prop->value;
        }
    }
    return NULL;
}

const char *find_property_ignore_case(const PropertyGroup *group, const char *key)
{
    for (int i = 0; i < group->size; i++) {
        const Property *prop = &group->properties[i];
        if (strequalIgnoreCase(prop->key, key)) {
            return prop->value;
        }
    }
    return NULL;
}

PropertyGroup *add_property(PropertyGroup *group, const char *key, const char *value, err_t *errp)
{
    *errp = 0;
    if (group->size == group->capacity) {
        group = realloc_properties(group, group->capacity * 2);
    }
    Property *prop = &group->properties[group->size];
    prop->key = key;
    prop->value = value;
    group->size++;

    return group;
}

PropertyGroup *put_property(PropertyGroup *group, const char *key, const char *value, err_t *errp)
{
    for (int i = 0; i < group->size; i++) {
        Property *prop = &group->properties[i];
        if (strequal(prop->key, key)) {
            prop->value = value;
            return group;
        }
    }

    return add_property(group, key, value, errp);
}

PropertyGroup *put_all_properties(PropertyGroup *destGroup, const PropertyGroup *sourceGroup, err_t *errp)
{
    for (int i = 0; i < sourceGroup->size; i++) {
        const Property *prop = &sourceGroup->properties[i];
        destGroup = put_property(destGroup, prop->key, prop->value, errp);
        if (destGroup == NULL) {
            break;
        }
    }
    return destGroup;
}

size_t longest_key_len(const PropertyGroup *group)
{
    size_t the_key_len = 0;
    for (int i = 0; i < group->size; i++) {
        Property *prop = &group->properties[i];
        size_t klen = strlen(prop->key);
        if (klen > the_key_len) {
            the_key_len = klen;
        }
    }
    return the_key_len;
}

static int prop_compare(const void *p1, const void *p2)
{
    int cmp = strcmp(((const Property *) p1)->key, ((const Property *) p2)->key);
    if (cmp == 0) {
        cmp = strcmp(((const Property *) p1)->value, ((const Property *) p2)->value);
    }
    return cmp;
}

void sort_properties(const PropertyGroup *group)
{
    qsort(group->properties, group->size, sizeof(Property), prop_compare);
}
