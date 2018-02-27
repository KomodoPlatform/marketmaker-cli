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
#include <string.h>

#include "version.h"
#include "property.h"
#include "strutil.h"
#include "api_help.h"
#include "http.h"
#include "json.h"
#include "path.h"
#include "sys_socket.h"
#include "safe_alloc.h"
#include "sys_file.h"

static const char *const CONFIG_PATH  = ".mmcli.config";
static const char *const API_PATH     = ".mmcli.api";
static const char *const KEY_URL      = "url";
static const char *const KEY_USERPASS = "userpass";

static const char *configPath;
static const char *apiPath;

static void print_help(const char *programPath, const PropertyGroup *api);

static void print_syserr(const char *context, err_t err);

static PropertyGroup *handle_config(const char *method, const char *programPath, int argc, char *argv[], err_t *errp);

static const PropertyGroup *handle_api(const char *method, const char *programPath, const URL *url, int argc,
                                       err_t *errp);

static PropertyGroup *build_param_list(const char *method, const char *userpass, const char *paramNames,
                                       int argc, char *argv[], err_t *errp);

static void init_globals(void);

static void print_help_api(FILE *out, const PropertyGroup *api);

int main(int argc, char *argv[])
{
    init_globals();

    const char *programPath = argv[0];
    err_t err;
    if ((argc <= 1) || strequal(argv[1], "-h") || strequal(argv[1], "--help")) {
        SysFile file;
        sys_file_init(&file);
        PropertyGroup *api = load_properties(&file.af, apiPath, &err);
        print_help(programPath, api);
        return EXIT_SUCCESS;
    }
    const char *method = argv[1];
    argc -= 2;
    argv = &argv[2];

    if (strequal(method, "--version")) {
        printf("Version %s\n", APP_VERSION);
        return EXIT_SUCCESS;
    }
    PropertyGroup *config = handle_config(method, programPath, argc, argv, &err);
    if (config == NULL) {
        return (err == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
    }

    URL url;
    const char *strUrl = find_property(config, KEY_URL);
    if (!parse_url(strUrl, &url, &err)) {
        fprintf(stderr, "*** Invalid URL: %s\n", strUrl);
        return EXIT_FAILURE;
    }

    const PropertyGroup *api = handle_api(method, programPath, &url, argc, &err);
    if (api == NULL) {
        return (err == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
    }

    const char *paramNames = find_property(api, method);
    if (paramNames == NULL) {
        fprintf(stderr, "*** Unknown method: %s\n", method);
        return EXIT_FAILURE;
    }

    if ((argc == 1) && (strequal(argv[0], "-h") || strequal(argv[0], "--help"))) {
        printf("Parameters for method '%s': %s\n", method, paramNames);
        return EXIT_SUCCESS;
    }

    const char *userpass = find_property(config, KEY_USERPASS);
    PropertyGroup *paramList = build_param_list(method, userpass, paramNames, argc, argv, &err);
    if (err != 0) {
        fprintf(stderr, "*** Wrong number of arguments given: %d\nParameters: %s\n", argc, paramNames);
        return EXIT_FAILURE;
    }
    char *jsonRequest = build_json_request(paramList, &err);
    if (err != 0) {
        print_syserr("building JSON request", err);
        return EXIT_FAILURE;
    }
    size_t responseLen = 0;
    SysSocket sock;
    sys_socket_init(&sock);
    char *response = http_post((AbstractSocket *) &sock, &url, jsonRequest, strlen(jsonRequest), &responseLen, &err);
    if (err != 0) {
        print_syserr("sending/receiving HTTP message", err);
        return EXIT_FAILURE;
    }
    puts(response);
    return EXIT_SUCCESS;
}

void init_globals()
{
#ifdef _WIN32
    WSADATA wsaData;
    int rc = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (rc != 0) {
        printf("WSAStartup failed: %d\n", rc);
        exit(EXIT_FAILURE);
    }
#endif
    apiPath = home_path(API_PATH);
    configPath = home_path(CONFIG_PATH);
}

PropertyGroup *build_param_list(const char *method, const char *userpass, const char *paramNames,
                                int argc, char *argv[], err_t *errp)
{
    *errp = 0;
    char *names = safe_strdup(paramNames);
    PropertyGroup *paramList = realloc_properties(NULL, 10);
    paramList->size = 0;
    add_property(paramList, "method", method);
    add_property(paramList, KEY_USERPASS, userpass);
    char *next_param = NULL;
    for (char *s = names; s != NULL; s = next_param) {
        if ((next_param = strchr(s, ',')) != NULL) {
            *next_param++ = '\0';
        }
        if (*s == '\0') {
            continue;
        }
        if (argc == 0) {
            // hopefully the missing arguments are optional
            break;
        }
        --argc;
        paramList = add_property(paramList, s, *argv++);
    }
    // check if user provided extra parameters
    if (argc > 0) {
        *errp = EINVAL;
        free(paramList);
        paramList = NULL;
    }
    return paramList;
}

void print_syserr(const char *context, err_t err)
{
    fprintf(stderr, "***Error while %s: errno=%d, msg=%s\n", context, err, strerror(err));
}

void print_help(const char *programPath, const PropertyGroup *api)
{
    fprintf(stderr, "Syntax: %s [-h | --help | --version | _config URL USERPASS | _refresh | method [-h | --help | params*]]\n", programPath);
    if (api != NULL) {
        print_help_api(stderr, api);
    }
}

void print_help_api(FILE *out, const PropertyGroup *api)
{
    static const char dashes[] = "=============================================";
    size_t max_key_len = longest_key_len(api) + 2;
    fprintf(out, "\n%-*s  %s\n%.*s  %s\n", (int) max_key_len, "Method", "Parameters",
            (int) max_key_len, dashes, dashes);
    for (size_t i = 0; i < api->size; i++) {
        Property *prop = &api->properties[i];
        size_t klen = strlen(prop->key);
        fputs(prop->key, out);
        fputc(' ', out);
        for (; klen < max_key_len; klen++) {
            fputc('.', out);
        }
        fputc(' ', out);
        fputs(prop->value, out);
        fputc('\n', out);
    }
}

PropertyGroup *handle_config(const char *method, const char *programPath, int argc, char **argv, err_t *errp)
{
    *errp = 0;
    if (strequal(method, "_config")) {
        if (argc != 2) {
            print_help(programPath, NULL);
            return NULL;
        }
        const char *url = argv[0];
        const char *userpass = argv[1];
        Property props[] = {
                {KEY_URL,      url},
                {KEY_USERPASS, userpass}
        };
        PropertyGroup config = {DIMOF(props), DIMOF(props), props};
        SysFile file;
        sys_file_init(&file);
        if (!save_properties(&config, &file.af, configPath, errp)) {
            print_syserr("saving config file", *errp);
        }
        return NULL;
    }
    SysFile file;
    sys_file_init(&file);
    PropertyGroup *config = load_properties(&file.af, configPath, errp);
    if (*errp != 0) {
        if (*errp != ENOENT) {
            print_syserr("loading config file", *errp);
            return NULL;
        } else {
            fprintf(stderr, "*** You must configure URL and USERPASS first! Use the _config to provide these values.\n");
            return NULL;
        }
    }
    return config;
}

const PropertyGroup *handle_api(const char *method, const char *programPath, const URL *url, int argc, err_t *errp)
{
    *errp = 0;
    const bool doRefresh = strequal(method, "_refresh");
    if (doRefresh && (argc != 0)) {
        print_help(programPath, NULL);
        return NULL;
    }

    PropertyGroup *api = NULL;
    if (!doRefresh) {
        SysFile file;
        sys_file_init(&file);
        api = load_properties(&file.af, apiPath, errp);
        if ((*errp != 0) && (*errp != ENOENT)) {
            print_syserr("loading api", *errp);
            return NULL;
        }
    }

    if (api == NULL) {
        SysSocket sock;
        sys_socket_init(&sock);
        api = fetch_api(&sock.as, url, errp);
        if (*errp != 0) {
            print_syserr("fetching api", *errp);
            return NULL;
        }
        SysFile file;
        sys_file_init(&file);
        if (!save_properties(api, &file.af, apiPath, errp)) {
            print_syserr("saving api", *errp);
            return NULL;
        }
    }

    return doRefresh ? NULL : api;
}
