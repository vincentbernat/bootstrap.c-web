/* -*- mode: c; c-file-style: "openbsd" -*- */
/*
 * Copyright (c) 2014 {{cookiecutter.full_name}} <{{cookiecutter.email}}>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _BOOTSTRAP_H
#define _BOOTSTRAP_H

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#include "log.h"

#define _GNU_SOURCE
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/queue.h>
#include <netdb.h>
#include <argtable2.h>
#include <event2/http.h>

#define SERVSTRLEN 6

struct {{cookiecutter.small_prefix}}_cfg;

/* TODO:5001 Declare here functions that you will use in several files. Those
 * TODO:5001 functions should not be prefixed with `static` keyword. All other
 * TODO:5001 functions should.
 */

/* arg.c */
struct arg_addr {
	struct arg_hdr hdr;
	int count;
	char sep;
	struct addrinfo *info;
};
struct arg_addr *arg_addr1(const char *, const char *,
    const char *, const char *, char);
struct arg_addr *arg_addr0(const char *, const char *,
    const char *, const char *, char);

/* http.c */
struct {{cookiecutter.small_prefix}}_sse_client;
struct {{cookiecutter.small_prefix}}_http_private;
int  {{cookiecutter.small_prefix}}_http_configure(struct {{cookiecutter.small_prefix}}_cfg *);
void {{cookiecutter.small_prefix}}_http_shutdown(struct {{cookiecutter.small_prefix}}_cfg *);
void {{cookiecutter.small_prefix}}_http_sse_send(struct {{cookiecutter.small_prefix}}_cfg *, const char *);
void {{cookiecutter.small_prefix}}_http_sse_send_to(struct {{cookiecutter.small_prefix}}_cfg *, const char *, struct {{cookiecutter.small_prefix}}_sse_client *);

/* websocket.c */
struct {{cookiecutter.small_prefix}}_ws_client;
const char *{{cookiecutter.small_prefix}}_ws_client_addr(struct {{cookiecutter.small_prefix}}_ws_client *);
unsigned    {{cookiecutter.small_prefix}}_ws_client_serv(struct {{cookiecutter.small_prefix}}_ws_client *);
int   {{cookiecutter.small_prefix}}_ws_send(struct {{cookiecutter.small_prefix}}_cfg *, const char *);
int   {{cookiecutter.small_prefix}}_ws_send_to(struct {{cookiecutter.small_prefix}}_cfg *,
    const char *, struct {{cookiecutter.small_prefix}}_ws_client *);
int   {{cookiecutter.small_prefix}}_ws_handle_req(struct {{cookiecutter.small_prefix}}_cfg *, struct evhttp_request *, const char *);
void  {{cookiecutter.small_prefix}}_ws_shutdown(struct {{cookiecutter.small_prefix}}_cfg *);

/* event.c */
struct {{cookiecutter.small_prefix}}_event_private;
int  {{cookiecutter.small_prefix}}_event_configure(struct {{cookiecutter.small_prefix}}_cfg *);
void {{cookiecutter.small_prefix}}_event_loop(struct {{cookiecutter.small_prefix}}_cfg *);
void {{cookiecutter.small_prefix}}_event_shutdown(struct {{cookiecutter.small_prefix}}_cfg *);

/* TODO:5003 When you want to use a "global" variable, put it in this structure.
 * TODO:5003 You can use substructures to be used in different part of the
 * TODO:5003 application. It is better to keep stuff seperate.
 */

struct {{cookiecutter.small_prefix}}_cfg {
	struct addrinfo *listen; /* Address to listen to */
	char *web;               /* Path to static files for HTTP */

	/* List of clients */
	TAILQ_HEAD(, {{cookiecutter.small_prefix}}_sse_client) sse_clients;
	TAILQ_HEAD(, {{cookiecutter.small_prefix}}_ws_client) ws_clients;

	struct {{cookiecutter.small_prefix}}_event_private *event;
	struct {{cookiecutter.small_prefix}}_http_private *http;
};

#endif
