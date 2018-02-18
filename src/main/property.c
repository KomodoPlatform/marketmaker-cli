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
#include <malloc.h>

#include "property.h"
#include "strutil.h"

#define INITIAL_CAPACITY 10

static char *load_file_contents(const char *path, err_t *err);

PropertyGroup *parse_properties(const char *str, err_t *err)
{
    *err = 0;
    char *copy = strdup(str);
    if (copy == NULL) {
        *err = ENOMEM;
        return NULL;
    }
    size_t capacity = INITIAL_CAPACITY;
    PropertyGroup *group = realloc_properties(NULL, capacity);
    if (group == NULL) {
        free(copy);
        *err = ENOMEM;
        return NULL;
    }
    group->size = 0;
    char *next_line = NULL;
    for (char *s = copy; s != NULL; s = next_line) {
        if ((next_line = strchr(s, '\n')) != NULL) {
            *next_line++ = '\0';
        }
        s = strtrim(s);
        if (*s != '\0') {
            char *sep;
            if ((sep = strchr(s, '=')) == NULL) {
                *err = EINVAL;
                break;
            }
            *sep++   = '\0';
            if (group->size == capacity) {
                capacity *= 2;
                group = realloc_properties(group, capacity);
                if (group == NULL) {
                    *err = ENOMEM;
                    break;
                }
            }
            group->properties[group->size].key     = strtrim(s);
            group->properties[group->size++].value = strtrim(sep);
        }
    }
    if (*err == 0) {
        return realloc_properties(group, group->size);
    }
    free(copy);
    return NULL;
}

PropertyGroup *load_properties(const char *path, err_t *err) {
    char * buffer = load_file_contents(path, err);
    PropertyGroup *group = (*err == 0) ? parse_properties(buffer, err) : NULL;
    free(buffer);
    return group;
}

char *load_file_contents(const char *path, err_t *err) {
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        *err = errno;
        return NULL;
    }
    char *buffer = NULL;
    if (fseek(file, 0, SEEK_END) >= 0) {
        int length = ftell(file);
        fseek(file, 0, SEEK_SET);
        buffer = malloc((size_t) length + 1);
        if (buffer != NULL) {
            fread(buffer, 1, (size_t) length, file);
            buffer[length] = '\0';
        } else {
            *err = ENOMEM;
        }
    }
    fclose(file);
    return buffer;
}

bool save_properties(const PropertyGroup *group, const char *path, err_t *err) {
    *err = 0;
    FILE *file = fopen(path, "w+b");
    if (file == NULL) {
        *err = errno;
        return false;
    }
    for (int i = 0; i < group->size; i++) {
        fprintf(file, "%s=%s\n", group->properties[i].key, group->properties[i].value);
    }
    fclose(file);
    return true;
}

PropertyGroup *realloc_properties(PropertyGroup *group, size_t newCapacity) {
    size_t newTotalSize = sizeof(PropertyGroup) + newCapacity * sizeof(Property);
    group = realloc(group, newTotalSize);
    group->properties = (Property *) (group + 1);
    return group;
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
