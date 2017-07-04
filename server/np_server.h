/**
 * @file np_server.h
 * @author Fanchanghu <fan.changhu@zte.com.cn>
 * @brief The netopeer2 server public header.
 *
 * Copyright (c) 2015-2017 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef NP_SERVER_H_
#define NP_SERVER_H_

#include <libyang/libyang.h>
#include <nc_server.h>

#ifdef __cplusplus
extern "C"
{
#endif
//sysrepo.h does not define extern C
#include <sysrepo.h>

/**
 * @mainpage About
 *
 */

/**
 *
 */
typedef enum
{
    EDIT_CONFIG, COPY_CONFIG,
} np_ec_clb_type;

struct np_ec_clb_ctx
{
    int id;
    np_ec_clb_type type;
    struct ly_ctx *ly_ctx;
    struct nc_session *ncs;
    sr_session_ctx_t *srs;
    sr_datastore_t ds;
    void *user_data;
};

typedef int (*np_ec_before_clb)(struct np_ec_clb_ctx *ec_ctx, struct lyd_node *config);
typedef int (*np_ec_node_clb)(struct np_ec_clb_ctx *ec_ctx, struct lyd_node *node);
typedef void (*np_ec_after_clb)(struct np_ec_clb_ctx *ec_ctx);

struct np_ec_plugin
{
    np_ec_before_clb before_clb;
    np_ec_node_clb node_clb;
    np_ec_after_clb after_clb;
};

#ifdef __cplusplus
}
#endif

#endif /* NP_SERVER_H_ */
