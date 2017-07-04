#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <dlfcn.h>
#include <pthread.h>

#include "config.h"
#include "np_plugin.h"
#include "log.h"

#define EC_PLUGIN_NAME "_np_ec_plugin"

static struct np_ec_plugin *ec_plugin = NULL;
static int ec_id = 0;
static pthread_mutex_t ec_id_mutex = PTHREAD_MUTEX_INITIALIZER;

struct np_ec_plugin*
np_get_ec_plugin()
{
    return ec_plugin;
}

struct np_ec_clb_ctx*
np_new_ec_clb_ctx(np_ec_clb_type type, struct ly_ctx *ly_ctx, struct nc_session *ncs, sr_session_ctx_t *srs,
        sr_datastore_t ds)
{
    struct np_ec_clb_ctx *ctx;

    ctx = calloc(1, sizeof(*ctx));
    if (!ctx) {
        EMEM;
        return NULL;
    }

    pthread_mutex_lock(&ec_id_mutex);
    ec_id++;
    ctx->id = ec_id;
    pthread_mutex_unlock(&ec_id_mutex);

    ctx->type = type;
    ctx->ly_ctx = ly_ctx;
    ctx->ncs = ncs;
    ctx->srs = srs;
    ctx->ds = ds;

    return ctx;
}

int
np_load_ec_plugin()
{
    const char *plugin_pos;
    void *dl_handler;
    char *dl_err;

    plugin_pos = NP2SRV_PLUGIN_FILE;

    if (access(plugin_pos, R_OK)) {
        VRB("Accessing netopeer2 server plugin \"%s\" failed (%s).", plugin_pos, strerror(errno));
        /* return success if accessing plugin file failed */
        return 0;
    }

    /* load the plugin - first, try if it is already loaded... */
    dl_handler = dlopen(plugin_pos, RTLD_NOW | RTLD_NOLOAD);
    dlerror();
    if (dl_handler) {
        /* the plugin is already loaded */
        VRB("Netopeer2 server plugin \"%s\" already loaded.", plugin_pos);
        dlclose(dl_handler);
        return 0;
    }

    /* ... and if not, load it */
    dl_handler = dlopen(plugin_pos, RTLD_NOW);
    dl_err = dlerror();
    if (!dl_handler) {
        ERR("Loading \"%s\" as netopeer2 server plugin failed (%s).", plugin_pos, dl_err);
        return -1;
    }
    VRB("Netopeer2 server plugin \"%s\" successfully loaded.", plugin_pos);

    /* get the plugin data */
    ec_plugin = dlsym(dl_handler, EC_PLUGIN_NAME);
    dl_err = dlerror();
    if (dl_err) {
        ERR("Processing \"%s\" as netopeer2 server plugin failed, missing plugin object (%s).", plugin_pos, dl_err);
        dlclose(dl_handler);
        return -1;
    }

    return 0;
}

