#ifndef NP_PLUGIN_H_
#define NP_PLUGIN_H_

#include "np_server.h"

int np_load_ec_plugin();
struct np_ec_plugin* np_get_ec_plugin();
struct np_ec_clb_ctx* np_new_ec_clb_ctx(np_ec_clb_type type, struct ly_ctx *ly_ctx, struct nc_session *ncs,
        sr_session_ctx_t *srs, sr_datastore_t ds);
int np_clean_ec_plugin();

#endif /* NP_PLUGIN_H_ */
