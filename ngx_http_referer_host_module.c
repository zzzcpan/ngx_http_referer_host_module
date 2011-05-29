
/*
 * Copyright 2011 Alexandr Gomoliako
 */

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

static ngx_int_t ngx_http_referer_host_add_vars(ngx_conf_t *cf);
static ngx_int_t ngx_http_referer_host_variable(ngx_http_request_t *r, 
    ngx_http_variable_value_t *v, uintptr_t data);

static ngx_http_module_t  ngx_http_referer_host_module_ctx = {
    ngx_http_referer_host_add_vars,        /* preconfiguration */
    NULL,                                  /* postconfiguration */

    NULL,                                  /* create main configuration */
    NULL,                                  /* init main configuration */

    NULL,                                  /* create server configuration */
    NULL,                                  /* merge server configuration */

    NULL,                                  /* create location configuration */
    NULL                                   /* merge location configuration */
};

ngx_module_t  ngx_http_referer_host_module = {
    NGX_MODULE_V1,
    &ngx_http_referer_host_module_ctx,     /* module context */
    NULL,                                  /* module directives */
    NGX_HTTP_MODULE,                       /* module type */
    NULL,                                  /* init master */
    NULL,                                  /* init module */
    NULL,                                  /* init process */
    NULL,                                  /* init thread */
    NULL,                                  /* exit thread */
    NULL,                                  /* exit process */
    NULL,                                  /* exit master */
    NGX_MODULE_V1_PADDING
};

static ngx_http_variable_t  ngx_http_referer_host_vars[] = {

    { ngx_string("referer_host"), NULL,
      ngx_http_referer_host_variable,
      0, 0, 0 },

    { ngx_null_string, NULL, NULL, 0, 0, 0 }
};


static ngx_int_t
ngx_http_referer_host_add_vars(ngx_conf_t *cf)
{
    ngx_http_variable_t  *var, *v;

    for (v = ngx_http_referer_host_vars; v->name.len; v++) {
        var = ngx_http_add_variable(cf, &v->name, v->flags);
        if (var == NULL) {
            return NGX_ERROR;
        }

        var->get_handler = v->get_handler;
        var->data = v->data;
    }

    return NGX_OK;
}


static ngx_int_t
ngx_http_referer_host_variable(ngx_http_request_t *r, 
    ngx_http_variable_value_t *v, uintptr_t data)
{
    u_char                    *p, *ref, *last;
    size_t                     len;
    ngx_uint_t                 i;

    if (r->headers_in.referer == NULL) {
        goto invalid;
    }

    len = r->headers_in.referer->value.len;
    ref = r->headers_in.referer->value.data;

    if (len < sizeof("http://i.ru") - 1) {
	goto invalid;
    }

    last = ref + len;

    if (ngx_strncasecmp(ref, (u_char *) "http://", 7) == 0) {
	ref += 7;
    } else if (ngx_strncasecmp(ref, (u_char *) "https://", 8) == 0) {
	ref += 8;
    } else {
	goto invalid;
    }

    i = 0;

    for (p = ref; p < last; p++) {
        if (*p == '/' || *p == ':') {
            break;
        } 

	*p = ngx_tolower(*p); 

        if (++i == 256) {
            goto invalid;
        }
    }

    if (i <= 0) {
	goto invalid;
    }

    v->len = i;
    v->data = ref;
    v->no_cacheable = 0;
    v->not_found = 0;

    return NGX_OK;

invalid:

    v->not_found = 1;

    return NGX_OK;
}

