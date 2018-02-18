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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "property.h"
#include "strutil.h"
#include "api_help.h"
#include "http.h"
#include "json.h"
#include "path.h"

#define CONFIG_PATH     ".mmcli.config"
#define API_PATH        ".mmcli.api"

static void print_help(const char *programPath);

static void print_syserr(const char *context, err_t err);

static void print_err(const char *fmt, ...);

static PropertyGroup *handle_config(const char *method, const char *programPath, int argc, char *argv[], err_t *errp);

static PropertyGroup *handle_api(const char *method, const char *programPath, const URL *url, int argc,
                                 err_t *errp);

static PropertyGroup *build_param_list(const char *method, const char *userpass, const char *paramNames,
                                       int argc, char *argv[], err_t *errp);

int main(int argc, char *argv[])
{
    const char *programPath = argv[0];
    err_t err;
    if (argc <= 1) {
        print_help(programPath);
        return EXIT_SUCCESS;
    }
    const char *method = argv[1];
    argc -= 2;
    argv = &argv[2];

    PropertyGroup *config = handle_config(method, programPath, argc, argv, &err);
    if (config == NULL) {
        return (err == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
    }

    URL url;
    const char *strUrl = find_property(config, "url");
    if (!parse_url(strUrl, &url, &err)) {
        print_err("Invalid URL: %s", strUrl);
    }

    PropertyGroup *api = handle_api(method, programPath, &url, argc, &err);
    if (api == NULL) {
        return (err == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
    }

    const char *paramNames = find_property(api, method);
    if (paramNames == NULL) {
        print_err("Unknown method: %s", method);
        return EXIT_FAILURE;
    }

    const char *userpass = find_property(config, "userpass");
    PropertyGroup *paramList = build_param_list(method, userpass, paramNames, argc, argv, &err);
    if (err != 0) {
        print_err("Wrong number of parameters: %d", argc);
        return EXIT_FAILURE;
    }
    char *jsonRequest = build_json_request(paramList, &err);
    if (err != 0) {
        print_syserr("building JSON request", err);
        return EXIT_FAILURE;
    }
    char *response = http_post(&url, jsonRequest, &err);
    if (err != 0) {
        print_syserr("sending/receiving HTTP message", err);
        return EXIT_FAILURE;
    }
    puts(response);
    return EXIT_SUCCESS;
}

PropertyGroup *build_param_list(const char *method, const char *userpass, const char *paramNames,
                                int argc, char *argv[], err_t *errp)
{
    *errp = 0;
    char *names = strdup(paramNames);
    if (names == NULL) {
        *errp = ENOMEM;
        return NULL;
    }
    size_t capacity = 10;
    PropertyGroup *paramList = realloc_properties(NULL, capacity);
    if (paramList == NULL) {
        *errp = ENOMEM;
        return NULL;
    }
    Property *prop = &paramList->properties[0];
    prop->key = "method";
    prop->value = method;
    ++prop;
    prop->key = "userpass";
    prop->value = userpass;
    paramList->size = 2;
    char *next_param = NULL;
    for (char *s = names; s != NULL; s = next_param) {
        if ((next_param = strchr(s, ',')) != NULL) {
            *next_param++ = '\0';
        }
        if (*s == '\0') {
            continue;
        }
        if (argc == 0) {
            *errp = EINVAL;
            break;
        }
        if (paramList->size == capacity) {
            capacity *= 2;
            paramList = realloc_properties(paramList, capacity);
            if (paramList == NULL) {
                *errp = ENOMEM;
                break;
            }
        }
        Property *prop = &paramList->properties[paramList->size];
        prop->key = s;
        prop->value = *argv++;
        --argc;
        paramList->size++;
    }
    // check if user provided extra parameters
    if ((*errp == 0) && (argc > 0)) {
        *errp = EINVAL;
    }

    if (*errp != 0) {
        free(paramList);
        paramList = NULL;
    }
    return paramList;
}

void print_err(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    fputs("***", stderr);
    vfprintf(stderr, fmt, ap);
    fputc('\n', stderr);
    va_end(ap);
}

void print_syserr(const char *context, err_t err)
{
    fprintf(stderr, "***Error while %s: errno=%d, msg=%s\n", context, err, strerror(err));
}

void print_help(const char *programPath)
{
    fprintf(stderr, "Syntax: %s [_config URL USERPASS | _refresh | method params*]\n", programPath);
}

PropertyGroup *handle_config(const char *method, const char *programPath, int argc, char **argv, err_t *errp)
{
    *errp = 0;
    char configPath[128];
    home_path(CONFIG_PATH, configPath, sizeof(configPath));
    if (strequal(method, "_config")) {
        if (argc != 2) {
            print_help(programPath);
            return NULL;
        }
        const char *url = argv[0];
        const char *userpass = argv[1];
        Property props[] = {
                {"url",      url},
                {"userpass", userpass}
        };
        PropertyGroup config = {2, props};
        if (!save_properties(&config, configPath, errp)) {
            print_syserr("saving config file", *errp);
        }
        return NULL;
    }
    PropertyGroup *config = load_properties(configPath, errp);
    if (*errp != 0) {
        if (*errp != ENOENT) {
            print_syserr("loading config file", *errp);
            return NULL;
        } else {
            print_err("You must configure URL and USERPASS first! Use the _config to provide these values.");
            return NULL;
        }
    }
    return config;
}

PropertyGroup *
handle_api(const char *method, const char *programPath, const URL *url, int argc, err_t *errp)
{
    *errp = 0;
    char apiPath[128];
    home_path(API_PATH, apiPath, sizeof(apiPath));
    if (strequal(method, "_refresh")) {
        if (argc != 0) {
            print_help(programPath);
            return NULL;
        }
        PropertyGroup *api = fetch_api(url, errp);
        if (*errp != 0) {
            print_syserr("fetching api", *errp);
            return NULL;
        }
        if (!save_properties(api, apiPath, errp)) {
            print_syserr("saving api", *errp);
            return NULL;
        }
        return NULL;
    }

    PropertyGroup *api = load_properties(apiPath, errp);
    if (*errp != 0) {
        if (*errp != ENOENT) {
            print_syserr("loading api", *errp);
            return NULL;
        }
        api = fetch_api(url, errp);
        if (*errp != 0) {
            print_syserr("fetching api", *errp);
            return NULL;
        }
        if (!save_properties(api, apiPath, errp)) {
            print_syserr("saving api", *errp);
            return NULL;
        }
    }

    return api;
}
