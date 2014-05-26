/* -*- mode: c; c-file-style: "openbsd" -*- */
/*
 * Copyright (c) 2014 Vincent Bernat <bernat@luffy.cx>
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

#include "{{cookiecutter.project_name}}.h"
#include "event.h"

#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <event2/http.h>
#include <event2/buffer.h>
#include <jansson.h>

struct {{cookiecutter.small_prefix}}_http_private {
	struct evhttp *http;
};

struct {{cookiecutter.small_prefix}}_sse_client {
	struct {{cookiecutter.small_prefix}}_cfg *cfg;

	TAILQ_ENTRY({{cookiecutter.small_prefix}}_sse_client) next;
	struct evhttp_request *req;

	/* TODO:3505 Each SSE client is kept in a structure like this.
	 * TODO:3505 You can store anything related the client here.
	 */
};

/**
 * Log HTTP request.
 *
 * Create a classic log line for an HTTP server with remote address,
 * date and time, request command (GET/POST), requested URI, response
 * code if available and user agent.
 *
 * @param req The request to be logged.
 */
static void
{{cookiecutter.small_prefix}}_http_log(struct evhttp_request *req) {
	const char *cmdtype;

	switch (evhttp_request_get_command(req)) {
	case EVHTTP_REQ_GET:     cmdtype = "GET";     break;
	case EVHTTP_REQ_POST:    cmdtype = "POST";    break;
	case EVHTTP_REQ_HEAD:    cmdtype = "HEAD";    break;
	case EVHTTP_REQ_PUT:     cmdtype = "PUT";     break;
	case EVHTTP_REQ_DELETE:  cmdtype = "DELETE";  break;
	case EVHTTP_REQ_OPTIONS: cmdtype = "OPTIONS"; break;
	case EVHTTP_REQ_TRACE:   cmdtype = "TRACE";   break;
	case EVHTTP_REQ_CONNECT: cmdtype = "CONNECT"; break;
	case EVHTTP_REQ_PATCH:   cmdtype = "PATCH";   break;
	default: cmdtype = "unknown"; break;
	}

	/* Compute time */
	time_t       now = time(NULL);
	char        *nows = ctime(&now);
	if (!nows) return;
	nows[strlen(nows) -1 ] = '\0';

	/* Grab information from client */
	struct evhttp_connection *connection = evhttp_request_get_connection(req);
	char        *remote_address;
	ev_uint16_t  remote_port;
	const char  *user_agent = evhttp_find_header(evhttp_request_get_input_headers(req),
	    "user-agent");
	evhttp_connection_get_peer(connection, &remote_address, &remote_port);

	/* Log */
	log_info("http", "%s [%s] %s %s %d \"%s\"",
            remote_address, nows,
            cmdtype, evhttp_request_get_uri(req),
            evhttp_request_get_response_code(req),
            user_agent?user_agent:"");
}

/**
 * Handle end of request.
 *
 * @param req The request we need to end.
 * @param error The error we want to send.
 *
 * This function will send an answer (if not already done) and log the
 * request.
 */
static void
{{cookiecutter.small_prefix}}_http_end(struct evhttp_request *req, int error) {
	/* No answer sent yet, send one */
	if (!evhttp_request_get_response_code(req))
		evhttp_send_error(req, error, 0);
	{{cookiecutter.small_prefix}}_http_log(req);
}

/* Guess content-type */
static const struct table_entry {
	const char *extension;
	const char *content_type;
} content_type_table[] = {
	{ "txt",  "text/plain"	    },
	{ "html", "text/html"	    },
	{ "xml",  "text/xml"	    },
	{ "css",  "text/css"	    },
	{ "less", "text/css"	    },
	{ "jpg",  "image/jpeg"	    },
	{ "png",  "image/png"	    },
	{ "ico",  "image/x-icon"    },
	{ "js",	  "text/javascript" },
	{ "svg",  "image/svg+xml"   },
	{ NULL,	  NULL		    },
};

/**
 * Try to guess the content-type of a static file.
 *
 * @param path The path to the file we need to content type for.
 * @return the guesses content type can never be \c NULL.
 */
static const char *
{{cookiecutter.small_prefix}}_http_guess_content_type(const char *path) {
	const char *last_period, *extension;
	const struct table_entry *ent;
	last_period = strrchr(path, '.');
	if (!last_period || strchr(last_period, '/'))
		goto not_found; /* no extension */
	extension = last_period + 1;
	for (ent = &content_type_table[0]; ent->extension; ++ent) {
		if (!strcmp(ent->extension, extension))
			return ent->content_type;
	}

not_found:
	return "application/misc";
}

/**
 * Handle static files
 *
 * This function is a fallback function and will try to serve the
 * requested URI as a static file. The file will be requested to be
 * cached for 10 minutes.
 */
static void
{{cookiecutter.small_prefix}}_http_static_cb(struct evhttp_request *req, void *arg)
{
	const char *docroot = ((struct {{cookiecutter.small_prefix}}_cfg *)arg)->web;
	const char *uri      = evhttp_request_get_uri(req);
	const char *path     = NULL;  /* Relative path */
	char *decopath = NULL;        /* Relative decoded path */
	char *canpath  = NULL;        /* Canonical path */
	char *fullpath = NULL;        /* Full path (with docroot) */
	struct stat st;
	struct evhttp_uri *decoded = NULL;
	struct evbuffer   *evb     = NULL;
	int fd = -1;

	/* We only handle GET requests */
	if (evhttp_request_get_command(req) != EVHTTP_REQ_GET)
		goto static_done;

	/* Decode URI */
	if (!(decoded = evhttp_uri_parse(uri)))
		goto static_done;

	/* Build and check path */
	if (!(path = evhttp_uri_get_path(decoded)) ||
	    !strcmp(path, "/"))
		path = "/index.html";
	if ((decopath = evhttp_uridecode(path, 0, NULL)) == NULL)
		goto static_done;
	if (asprintf(&fullpath, "%s/%s", docroot, decopath) == -1)
		goto static_done;
	if ((canpath = realpath(fullpath, NULL)) == NULL) {
		log_debug("http", "full path %s cannot be resolved", fullpath);
		evhttp_send_error(req, HTTP_NOTFOUND, 0);
		goto static_done;
	}
	if (strncmp(docroot, canpath, strlen(docroot)) ||
	    ((docroot[strlen(docroot) - 1] != '/') &&
		(canpath[strlen(docroot)] != '/')) ||
	    stat(canpath, &st) < 0) {
		log_debug("http", "requested path %s is outside of root %s",
		    canpath, docroot);
		evhttp_send_error(req, HTTP_NOTFOUND, 0);
		goto static_done;
	}

	/* We only serve files */
	if (!S_ISREG(st.st_mode)) {
		log_debug("http", "requested path %s is not a regular file",
		    canpath);
		evhttp_send_error(req, HTTP_NOTFOUND, 0);
		goto static_done;
	}

	/* Send file */
	const char *type = {{cookiecutter.small_prefix}}_http_guess_content_type(canpath);
	if ((fd = open(canpath, O_RDONLY)) < 0) {
		log_warn("http", "unable to open %s", canpath);
		evhttp_send_error(req, 403, "Not allowed");
		goto static_done;
	}
	if (fstat(fd, &st)<0) {
		log_warn("http", "unable to fstat %s", canpath);
		evhttp_send_error(req, HTTP_INTERNAL, 0);
		goto static_done;
	}
	evb = evbuffer_new();
#ifdef EVBUFFER_FLAG_DRAINS_TO_FD
	if (st.st_size > (1<<18))
		evbuffer_set_flags(evb, EVBUFFER_FLAG_DRAINS_TO_FD); /* Use sendfile */
#endif
	if (evbuffer_add_file(evb, fd, 0, st.st_size) == -1) {
		log_debug("http", "cannot send %s to client", canpath);
		evhttp_send_error(req, HTTP_INTERNAL, 0);
		goto static_done;
	}
	evhttp_add_header(evhttp_request_get_output_headers(req),
	    "Content-Type", type);
	evhttp_add_header(evhttp_request_get_output_headers(req),
	    "Cache-Control", "public, max-age=3600");
	evhttp_send_reply(req, 200, "OK", evb);
	fd = -1;                      /* Dont close it! */

static_done:
	{{cookiecutter.small_prefix}}_http_end(req, HTTP_BADREQUEST);

	/* Cleanup */
	if (decoded) evhttp_uri_free(decoded);
	if (evb)     evbuffer_free(evb);
	if (fd >= 0) close(fd);
	free(decopath);
	free(fullpath);
	free(canpath);

	return;
}

/**
 * Bind to the appropriate socket
 */
static int
{{cookiecutter.small_prefix}}_http_bind(struct {{cookiecutter.small_prefix}}_cfg *cfg)
{
	struct addrinfo *re, *rem = cfg->listen;
	char addr[INET6_ADDRSTRLEN];
	char serv[SERVSTRLEN];
	for (re = rem; re != NULL; re = re->ai_next) {
		getnameinfo(re->ai_addr, re->ai_addrlen,
		    addr, sizeof(addr),
		    serv, sizeof(serv),
		    NI_NUMERICHOST | NI_NUMERICSERV);
		log_debug("http", "try to listen to [%s]:%s", addr, serv);
		if (evhttp_bind_socket(cfg->http->http, addr, atoi(serv)) == 0) {
			log_info("http", "listening on [%s]:%s", addr, serv);
			break;
		}
	}
	if (re == NULL) {
		log_warn("http", "unable to bind to [%s]:%s", addr, serv);
		return -1;
	}
	return 0;
}

/**
 * Send a JSON object as answer.
 *
 * @param req The request object we will use to send the answer.
 * @param export The JSON object to send back.
 * @return 0 on error, 1 on success.
 *
 * This function will dump the JSON object to a string, add the
 * appropriate headers to the request and send the answer.
 */
static int
{{cookiecutter.small_prefix}}_http_json_send(struct evhttp_request *req, json_t *export) {
	struct evbuffer    *evb    = NULL;
	char               *output = NULL;

	if (export) {
		if ((output = json_dumps(export,
			    JSON_INDENT(2) | JSON_PRESERVE_ORDER)) == NULL) {
			log_warnx("http", "unable to encode JSON object");
			return 0;
		}
	}

	evb = evbuffer_new();
	if (evbuffer_add(evb,
		export?output:"{}",
		export?strlen(output):2) == -1) {
		log_warnx("http", "cannot send JSON answer to client");
		evhttp_send_error(req, HTTP_INTERNAL, 0);
		evbuffer_free(evb);
		free(output);
		return 0;
	}
	free(output);

	evhttp_add_header(evhttp_request_get_output_headers(req),
	    "Content-Type", "application/json");
	/* See:
	   http://stackoverflow.com/questions/1046966/whats-the-difference-between-cache-control-max-age-0-and-no-cache */
	evhttp_add_header(evhttp_request_get_output_headers(req),
	    "Cache-Control", " private, max-age=0");
	evhttp_send_reply(req, 200, "OK", evb);
	evbuffer_free(evb);
	return 1;
}

/**
 * <tt>GET /hello</tt>: say hello in JSON
 *
 * @ingroup httpapi
 *
 * \code
 * {
 *  "hello": "world"
 * }
 * \endcode
 */
static void
{{cookiecutter.small_prefix}}_http_hello_cb(struct evhttp_request *req, void *arg) {
	/* Check we have a GET request */
	switch (evhttp_request_get_command(req)) {
	case EVHTTP_REQ_GET: break;
	default:
		{{cookiecutter.small_prefix}}_http_end(req, HTTP_BADREQUEST);
		return;
	}

	/* Build and send the answer */
	json_t *export = json_pack("{ss}", "hello", "world");
	if (!export) {
		log_warnx("http", "unable to build a new JSON object");
		goto done;
	}
	{{cookiecutter.small_prefix}}_http_json_send(req, export);
done:
	{{cookiecutter.small_prefix}}_http_end(req, HTTP_INTERNAL);
	json_decref(export);
}

/**
 * Callback when an SSE client disconnects.
 */
static void
{{cookiecutter.small_prefix}}_http_sse_closecb(struct evhttp_connection *evconn,
    void *arg)
{
	struct {{cookiecutter.small_prefix}}_sse_client *client = arg;
	struct {{cookiecutter.small_prefix}}_cfg *cfg = client->cfg;
	/* TODO:3507 An SSE client has been disconnected, free anything that should
	 * TODO:3507 be freed here.
	 */
	TAILQ_REMOVE(&cfg->clients, client, next);
	free(client);
}

/**
 * Callback when an SSE client connects.
 */
static void
{{cookiecutter.small_prefix}}_http_sse_cb(struct evhttp_request *req, void *arg)
{
	struct {{cookiecutter.small_prefix}}_cfg *cfg = arg;

	/* Only accepts GET requests */
	switch (evhttp_request_get_command(req)) {
	case EVHTTP_REQ_GET: break;
	default:
		log_warnx("http", "reject non-GET request on sse URI");
		{{cookiecutter.small_prefix}}_http_end(req, HTTP_BADREQUEST);
		return;
	}

	{{cookiecutter.small_prefix}}_http_log(req);

	/* Register the client */
	struct {{cookiecutter.small_prefix}}_sse_client *client = calloc(1, sizeof(struct {{cookiecutter.small_prefix}}_sse_client));
	if (client == NULL) {
		log_warn("http", "no memory for a new client");
		{{cookiecutter.small_prefix}}_http_end(req, HTTP_SERVUNAVAIL);
	}
	client->cfg = cfg;
	client->req = req;
	TAILQ_INSERT_TAIL(&cfg->clients, client, next);
	evhttp_connection_set_closecb(evhttp_request_get_connection(req),
	    {{cookiecutter.small_prefix}}_http_sse_closecb, client);

	/* Start chunked reply */
	evhttp_add_header(evhttp_request_get_output_headers(req),
	    "Content-Type", "text/event-stream");
	evhttp_add_header(evhttp_request_get_output_headers(req),
	    "Cache-Control", "no-cache");
	evhttp_send_reply_start(req, 200, "OK");

	/* TODO:3506 If you need to do some special actions when an SSE client connects,
	 * TODO:3506 do it here. Currently, we broadcast the number of connected clients
	 * TODO:3506 to everybody.
	 */
	size_t count = 0;
	json_t *jcount = NULL;
	char *scount = NULL;
	TAILQ_FOREACH(client, &cfg->clients, next) count++;
	jcount = json_pack("{si}", "clients", count);
	scount = json_dumps(jcount, JSON_COMPACT);
	{{cookiecutter.small_prefix}}_http_sse_send(cfg, scount);
	free(scount);
	json_decref(jcount);
}

/**
 * Broadcast to all SSE clients.
 *
 * @param result String to be broadcasted
 */
void
{{cookiecutter.small_prefix}}_http_sse_send(struct {{cookiecutter.small_prefix}}_cfg *cfg,
    const char *result)
{
	struct {{cookiecutter.small_prefix}}_sse_client *client;
	TAILQ_FOREACH(client, &cfg->clients, next) {
		{{cookiecutter.small_prefix}}_http_sse_send_to(cfg, result, client);
	}
}

/*
 * Send to a given SSE client.
 *
 * @param result String to be sent
 * @param client Target client
 */
void
{{cookiecutter.small_prefix}}_http_sse_send_to(struct {{cookiecutter.small_prefix}}_cfg *cfg,
    const char *result,
    struct {{cookiecutter.small_prefix}}_sse_client *client)
{
	struct evbuffer *buf = NULL;
	if ((buf = evbuffer_new()) == NULL ||
	    evbuffer_add(buf, "data: ", 6) == -1 ||
	    evbuffer_add(buf, result, strlen(result)) == -1 ||
	    evbuffer_add(buf, "\n\n", 2) == -1) {
		log_warnx("http", "unable to build outgoing buffer");
		if (buf) evbuffer_free(buf);
		return;
	}
	evhttp_send_reply_chunk(client->req, buf);
	evbuffer_free(buf);
}

/**
 * Register some HTTP callbacks
 */
static int
{{cookiecutter.small_prefix}}_http_register(struct {{cookiecutter.small_prefix}}_cfg *cfg)
{
	log_debug("http", "register callbacks");
	/* TODO:3002 To add more HTTP endpoint, add them here. */
	evhttp_set_cb(cfg->http->http, "/api/1.0/hello", {{cookiecutter.small_prefix}}_http_hello_cb, cfg);
	evhttp_set_cb(cfg->http->http, "/api/1.0/sse", {{cookiecutter.small_prefix}}_http_sse_cb, cfg);
	evhttp_set_gencb(cfg->http->http, {{cookiecutter.small_prefix}}_http_static_cb, cfg);
	return 0;
}

/**
 * Setup HTTP part
 *
 * @param cfg The global configuration containing libevent base.
 * @return -1 on error
 */
int
{{cookiecutter.small_prefix}}_http_configure(struct {{cookiecutter.small_prefix}}_cfg *cfg)
{
	log_debug("http", "setup HTTP server");
	if ((cfg->http = calloc(1, sizeof(struct {{cookiecutter.small_prefix}}_http_private))) == NULL) {
		log_warn("http", "unable to allocate memory for HTTP");
		return -1;
	}
	if ((cfg->http->http = evhttp_new(cfg->event->base)) == NULL) {
		log_warnx("http", "unable to create new HTTP base");
		return -1;
	}
	return ({{cookiecutter.small_prefix}}_http_bind(cfg) == -1 ||
	    {{cookiecutter.small_prefix}}_http_register(cfg) == -1)?-1:0;
}

/**
 * Destroy HTTP part
 */
void
{{cookiecutter.small_prefix}}_http_shutdown(struct {{cookiecutter.small_prefix}}_cfg *cfg)
{
	log_debug("http", "tear down HTTP server");
	if (cfg->http && cfg->http->http) {
		evhttp_free(cfg->http->http);
	}
	free(cfg->http);
}
