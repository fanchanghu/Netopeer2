#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <dlfcn.h>
#include <pthread.h>

#include "config.h"
#include "np_plugin.h"
#include "log.h"

#define EC_PLUGIN_NAME "_np_ec_plugin"

static void *ec_dl_handler = NULL;
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
np_clean_ec_plugin()
{
    int ret = 0;

    ec_plugin = NULL;
    if (ec_dl_handler) {
        ret = dlclose(ec_dl_handler);
        ec_dl_handler = NULL;
    }
    return ret;
}

int
np_load_ec_plugin()
{
    const char *plugin_file;
    void *dl_handler;
    char *dl_err;

    /* try to get the plugin file from environment variable */
    plugin_file = getenv("NP2SRV_PLUGINS_FILE");
    if (!plugin_file) {
        plugin_file = NP2SRV_PLUGIN_FILE;
    }

    if (access(plugin_file, R_OK)) {
        VRB("Accessing netopeer2 server plugin \"%s\" failed (%s).", plugin_file, strerror(errno));
        /* return success if accessing plugin file failed */
        return 0;
    }

    /* load it */
    dl_handler = dlopen(plugin_file, RTLD_NOW);
    dl_err = dlerror();
    if (!dl_handler) {
        ERR("Loading \"%s\" as netopeer2 server plugin failed (%s).", plugin_file, dl_err);
        return -1;
    }
    VRB("Netopeer2 server plugin \"%s\" successfully loaded.", plugin_file);

    /* get the plugin data */
    ec_plugin = dlsym(dl_handler, EC_PLUGIN_NAME);
    dl_err = dlerror();
    if (dl_err) {
        ERR("Processing \"%s\" as netopeer2 server plugin failed, missing plugin object (%s).", plugin_file, dl_err);
        dlclose(dl_handler);
        return -1;
    }

    ec_dl_handler = dl_handler;
    return 0;
}

