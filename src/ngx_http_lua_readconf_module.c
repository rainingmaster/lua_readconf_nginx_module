
/*
 * Copyright (C) Jim
 */


#ifndef DDEBUG
#define DDEBUG 0
#endif
#include "ddebug.h"


#include <ngx_core.h>
#include <ngx_http.h>
#include <lauxlib.h>
#include "ngx_http_lua_api.h"
//#include "ngx_http_lua_common.h"


ngx_module_t ngx_http_lua_readconf_module;


static ngx_int_t ngx_http_lua_readconf_init(ngx_conf_t *cf);
static int ngx_http_lua_readconf_create_module(lua_State * L);
static int ngx_http_lua_readconf_get_lua_conf(lua_State * L);
static int ngx_http_lua_readconf_get_nginx_conf(lua_State * L);
static void * ngx_http_lua_readconf_get_main_conf(lua_State *L, ngx_module_t module);
static void * ngx_http_lua_readconf_get_loc_conf(lua_State *L, ngx_module_t module);


static ngx_http_module_t ngx_http_lua_readconf_ctx = {
    NULL,                           /* preconfiguration */
    ngx_http_lua_readconf_init,    /* postconfiguration */
    NULL,                           /* create main configuration */
    NULL,                           /* init main configuration */
    NULL,                           /* create server configuration */
    NULL,                           /* merge server configuration */
    NULL,                           /* create location configuration */
    NULL                            /* merge location configuration */
};


ngx_module_t ngx_http_lua_readconf_module = {
    NGX_MODULE_V1,
    &ngx_http_lua_readconf_ctx, /* module context */
    NULL,                        /* module directives */
    NGX_HTTP_MODULE,             /* module type */
    NULL,                        /* init master */
    NULL,                        /* init module */
    NULL,                        /* init process */
    NULL,                        /* init thread */
    NULL,                        /* exit thread */
    NULL,                        /* exit process */
    NULL,                        /* exit master */
    NGX_MODULE_V1_PADDING
};


static ngx_int_t
ngx_http_lua_readconf_init(ngx_conf_t *cf)
{
    if (ngx_http_lua_add_package_preload(cf, "ngx.readconf",
                                         ngx_http_lua_readconf_create_module)
        != NGX_OK)
    {
        return NGX_ERROR;
    }

    return NGX_OK;
}


static int
ngx_http_lua_readconf_create_module(lua_State * L)
{
    lua_createtable(L, 0, 2);

    lua_pushcfunction(L, ngx_http_lua_readconf_get_lua_conf);
    lua_setfield(L, -2, "get_lua_conf");

    lua_pushcfunction(L, ngx_http_lua_readconf_get_nginx_conf);
    lua_setfield(L, -2, "get_ngx_conf");

    return 1;
}


static int
ngx_http_lua_readconf_get_nginx_conf(lua_State * L)
{
    if (lua_gettop(L) != 0) {
        return luaL_error(L, "no argument expected");
    }

    lua_createtable(L, 0, 5);

    lua_pushliteral(L, "conf_file");
    lua_pushlstring(L, ngx_cycle->conf_file.data,
                    ngx_cycle->conf_file.len);
    lua_rawset(L, -3);

    lua_pushliteral(L, "conf_param");
    lua_pushlstring(L, ngx_cycle->conf_param.data,
                    ngx_cycle->conf_param.len);
    lua_rawset(L, -3);

    lua_pushliteral(L, "conf_prefix");
    lua_pushlstring(L, ngx_cycle->conf_prefix.data,
                    ngx_cycle->conf_prefix.len);
    lua_rawset(L, -3);

    lua_pushliteral(L, "connection_n");
    lua_pushinteger(L, ngx_cycle->connection_n);
    lua_rawset(L, -3);
    
    return 1;
}
static int
ngx_http_lua_readconf_get_lua_conf(lua_State * L)
{
    ngx_http_lua_main_conf_t        *lmcf;
    ngx_http_lua_loc_conf_t         *llcf;

    if (lua_gettop(L) != 0) {
        return luaL_error(L, "no argument expected");
    }

    lmcf = (ngx_http_lua_main_conf_t*)ngx_http_lua_readconf_get_main_conf(L, ngx_http_lua_module);
    llcf = (ngx_http_lua_loc_conf_t*)ngx_http_lua_readconf_get_loc_conf(L, ngx_http_lua_module);

    lua_createtable(L, 0, 4);

    /*lua_pushliteral(L, "init_src");
    lua_pushlstring(L, (char *) lmcf->init_src.data,
                    lmcf->init_src.len);*/

    lua_pushliteral(L, "max_running_timers");
    lua_pushinteger(L, lmcf->max_running_timers);
    lua_rawset(L, -3);

    lua_pushliteral(L, "running_timers");
    lua_pushinteger(L, lmcf->running_timers);
    lua_rawset(L, -3);
    
    lua_pushliteral(L, "pending_timers");
    lua_pushinteger(L, lmcf->pending_timers);
    lua_rawset(L, -3);

    lua_pushliteral(L, "max_pending_timers");
    lua_pushinteger(L, lmcf->max_pending_timers);
    lua_rawset(L, -3);

    //lua_pushliteral(L, "lua_cpath");
    //lua_pushliteral(L, lmcf->lua_cpath);
    //lua_rawset(L, -3);
    
    return 1;
}

/*
 * 获取相应模块 main_conf
 */
static void *
ngx_http_lua_readconf_get_main_conf(lua_State *L, ngx_module_t module)
{
    ngx_http_request_t                   *r;

    r = ngx_http_lua_get_request(L);

    if (r == NULL) {
        return ngx_http_cycle_get_module_main_conf(ngx_cycle,
                                                   module);
    }

    return ngx_http_get_module_main_conf(r, module);
}

/*
 * 获取相应模块 srv_conf
 */
static void *
ngx_http_lua_readconf_get_srv_conf(lua_State *L, ngx_module_t module)
{
    ngx_http_request_t                   *r;

    r = ngx_http_lua_get_request(L);

    if (r == NULL) {
        return NULL;
    }

    return ngx_http_get_module_srv_conf(r, module);
}

/*
 * 获取相应模块 loc_conf
 */
static void *
ngx_http_lua_readconf_get_loc_conf(lua_State *L, ngx_module_t module)
{
    ngx_http_request_t                   *r;

    r = ngx_http_lua_get_request(L);

    if (r == NULL) {
        return NULL;
    }

    return ngx_http_get_module_loc_conf(r, module);
}
