#ifndef _MODINFO_H
#define _MODINFO_H
/*
 * Copyright (c) 2016 MariaDB Corporation Ab
 *
 * Use of this software is governed by the Business Source License included
 * in the LICENSE.TXT file and at www.mariadb.com/bsl.
 *
 * Change Date: 2019-07-01
 *
 * On the date above, in accordance with the Business Source License, use
 * of this software will be governed by version 2 or later of the General
 * Public License.
 */

/**
 * @file modinfo.h The module information interface
 *
 * @verbatim
 * Revision History
 *
 * Date     Who             Description
 * 02/06/14 Mark Riddoch    Initial implementation
 *
 * @endverbatim
 */

struct filter_object;
struct router_object;
struct monitor_object;
struct gw_protocol;
struct gw_authenticator;


/**
 * The status of the module. This gives some idea of the module
 * maturity.
 */
typedef enum
{
    MODULE_IN_DEVELOPMENT = 0,
    MODULE_ALPHA_RELEASE,
    MODULE_BETA_RELEASE,
    MODULE_GA,
    MODULE_EXPERIMENTAL
} MODULE_STATUS;

/**
 * The API implemented by the module
 */
typedef enum
{
    MODULE_API_PROTOCOL = 1,
    MODULE_API_ROUTER,
    MODULE_API_MONITOR,
    MODULE_API_FILTER,
    MODULE_API_AUTHENTICATOR,
    MODULE_API_QUERY_CLASSIFIER,
} MODULE_API;

/**
 * The module version structure.
 *
 * The rules for changing these values are:
 *
 * Any change that affects an inexisting call in the API in question,
 * making the new API no longer compatible with the old,
 * must increment the major version.
 *
 * Any change that adds to the API, but does not alter the existing API
 * calls, must increment the minor version.
 *
 * Any change that is purely cosmetic and does not affect the calling
 * conventions of the API must increment only the patch version number.
 */
typedef struct
{
    int     major;
    int     minor;
    int     patch;
} MODULE_VERSION;

/**
 * The module information structure
 */
typedef struct
{
    MODULE_STATUS   status;            /**< Module maturity */
    MODULE_API      modapi;            /**< Module API type */
    MODULE_VERSION  api_version;       /**< Module API version */
    const char     *description;       /**< Module description */
    const char     *version;           /**< Module version */
    int           (*moduleinit)(void); /**< Module global initialization,
                                        * optional, returns 0 on success */
    void           *object;            /**< Type specific entry points */
} MODULE_INFO;

/**
 * Modules are declared using this macro. Each module must call this macro with
 * the following parameters:
 *
 *   type     Type of the module, one of ROUTER, FILTER, MONITOR, AUTHENTICATOR,
 *            PROTOCOL or QUERY_CLASSIFIER
 *
 *   maturity Module maturity level, one of MODULE_IN_DEVELOPMENT,
 *            MODULE_ALPHA_RELEASE, MODULE_BETA_RELEASE, MODULE_GA or
 *            MODULE_EXPERIMENTAL
 *
 *   desc     Module description as const char*
 *
 *   ver      Module version as const char*
 *
 *   init     Module initialization function of signature int (*)(void). This
 *            function should return 0, if the loading of the module was successful.
 *            If an error occurred, the function should return a non-zero value.
 *
 *   mod      Module object for this module. This is a pointer to the module
 *            type specific structure.
 *
 *
 * Here is an example module definition for a filter:
 *
 * static FILTER_OBJECT MyObject =
 * {
 *     createInstance,
 *     newSession,
 *     closeSession,
 *     freeSession,
 *     setDownstream,
 *     NULL, // No Upstream requirement
 *     routeQuery,
 *     NULL,
 *     diagnostic
 * };
 *
 * MXS_DECLARE_MODULE(FILTER,
 *                    MODULE_GA,
 *                    "A query rewrite filter that uses regular expressions to rewrite queries",
 *                    "V1.1.0",
 *                    NULL, // No need for module initialization
 *                    &MyObject)
 *
 */
#define MXS_DECLARE_MODULE(type, maturity, desc, ver, init, mod)        \
    static MODULE_INFO mxs_module_info = {.modapi = MODULE_API_ ## type, .api_version = MXS_ ## type ## _VERSION, \
                                .status = maturity, .description = desc, .version = ver, \
                                .moduleinit = init, .object = mod};     \
    MODULE_INFO* mxs_get_module_info(void){return &mxs_module_info;}

#define MXS_MODULE_ENTRY_POINT "mxs_get_module_info"

#endif
