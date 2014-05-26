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

#include "{{cookiecutter.project_name|lower}}.h"

#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <event2/event.h>
#include <event2/http.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <wslay/wslay.h>
#include <jansson.h>

#define WS_GUID "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"
#define WS_MAX_WRITE (16*1024)

struct {{cookiecutter.small_prefix}}_ws_client {
	struct {{cookiecutter.small_prefix}}_cfg *cfg;

	TAILQ_ENTRY({{cookiecutter.small_prefix}}_ws_client) next;
	struct evhttp_connection *conn;
	struct bufferevent *bev;

	char     *address;
	uint16_t  port;

	wslay_event_context_ptr ctx;
};

/**
 * Return address of the client.
 */
const char *
{{cookiecutter.small_prefix}}_ws_client_addr(struct {{cookiecutter.small_prefix}}_ws_client *client)
{
	return client->address;
}

/**
 * Return the port of the client.
 */
unsigned
{{cookiecutter.small_prefix}}_ws_client_serv(struct {{cookiecutter.small_prefix}}_ws_client *client)
{
	return client->port;
}

/**
 * Kill one client.
 */
static void
{{cookiecutter.small_prefix}}_ws_shutdown_client(struct {{cookiecutter.small_prefix}}_ws_client *ws)
{
	TAILQ_REMOVE(&ws->cfg->ws_clients, ws, next);
	if (ws->ctx) wslay_event_context_free(ws->ctx);
	if (ws->conn) evhttp_connection_free(ws->conn);
	free(ws);
}

/**
 * Compute accept key for WS.
 */
static char *
{{cookiecutter.small_prefix}}_ws_accept_key(const char *client_key)
{
	BIO *bmem, *b64 = NULL;
	BUF_MEM *bptr;
	char *src_key = NULL;
	if (asprintf(&src_key, "%s%s", client_key, WS_GUID) == -1) {
		log_warn("websocket", "unable to allocate memory for websocket key");
		goto error;
	}
	unsigned char sha1buf[SHA_DIGEST_LENGTH];
	if (SHA1((unsigned char *)src_key, strlen(src_key), sha1buf) == NULL) {
		log_warnx("websocket", "unable to compute SHA1 for websocket key");
		goto error;
	}

	/* Use the easy to handle OpenSSL API for base 64 encoding... */
	if ((b64 = BIO_new(BIO_f_base64())) == NULL ||
	    (bmem = BIO_new(BIO_s_mem())) == NULL ||
	    (b64 = BIO_push(b64, bmem)) == NULL ||
	    BIO_write(b64, sha1buf, SHA_DIGEST_LENGTH) <= 0 ||
	    BIO_flush(b64) <= 0 ||
	    (BIO_get_mem_ptr(b64, &bptr), 0)) {
		log_warnx("websocket", "unable to do base64 encoding for websocket key");
		goto error;
	}

	char *accept_key = calloc(1, bptr->length);
	if (accept_key != NULL) {
		memcpy(accept_key, bptr->data, bptr->length - 1);
		accept_key[bptr->length - 1] = '\0';
	}

	BIO_free_all(b64);
	free(src_key);
	log_debug("websocket", "client key: %s, accept key: %s",
	    client_key, accept_key);
	return accept_key;

error:
	if (b64 != NULL) BIO_free_all(b64);
	free(src_key);
	return NULL;
}

static void
{{cookiecutter.small_prefix}}_wslay_try_write(struct {{cookiecutter.small_prefix}}_ws_client *client)
{
	if (wslay_event_send(client->ctx) == -1) {
		log_warnx("websocket",
		    "fatal error while receiving data from [%s]:%d",
		    client->address, client->port);
		{{cookiecutter.small_prefix}}_ws_shutdown_client(client);
		return;
	}
}

static ssize_t
{{cookiecutter.small_prefix}}_wslay_send_callback(wslay_event_context_ptr ctx,
    const uint8_t *data, size_t len, int flags,
    void *arg)
{
	struct {{cookiecutter.small_prefix}}_ws_client *client = arg;

	/* Don't write too much per client. */
	size_t max_length = WS_MAX_WRITE - evbuffer_get_length(bufferevent_get_output(client->bev));
	len = (len > max_length)?max_length:len;
	if (len == 0) {
		wslay_event_set_error(ctx, WSLAY_ERR_WOULDBLOCK);
		return -1;
	}

	/* Write to buffer if possible */
	if (bufferevent_write(client->bev, data, len) == -1) {
		log_warnx("websocket", "unable to write data to [%s]:%d",
		    client->address, client->port);
		wslay_event_set_error(ctx, WSLAY_ERR_CALLBACK_FAILURE);
		return -1;
	}
	return len;
}

static ssize_t
{{cookiecutter.small_prefix}}_wslay_recv_callback(wslay_event_context_ptr ctx,
    uint8_t *data, size_t len,
    int flags,
    void *arg)
{
	struct {{cookiecutter.small_prefix}}_ws_client *client = arg;
	size_t r = bufferevent_read(client->bev, data, len);
	if (r == 0) {
		wslay_event_set_error(ctx, WSLAY_ERR_WOULDBLOCK);
		return -1;
	}
	return r;
}

static void
{{cookiecutter.small_prefix}}_wslay_on_msg_recv_callback(wslay_event_context_ptr ctx,
    const struct wslay_event_on_msg_recv_arg *msg,
    void *arg)
{
	struct {{cookiecutter.small_prefix}}_ws_client *client = arg;
	if (!wslay_is_ctrl_frame(msg->opcode)) {
		/* We expect JSON, let's decode it. */
		switch (msg->opcode) {
		case WSLAY_TEXT_FRAME:
			/* TODO:3508 Do something useful when receiving a message from
			 * TODO:3508 a websocket client. Here, we broadcast the client
			 * TODO:3508 count to everybody (which is quite a odd thing to do).
			 */
			do {
				size_t count = 0;
				json_t *jcount = NULL;
				char *scount = NULL;
				struct {{cookiecutter.small_prefix}}_cfg *cfg = client->cfg;
				struct {{cookiecutter.small_prefix}}_ws_client *client = NULL;
				TAILQ_FOREACH(client, &cfg->ws_clients, next) count++;
				jcount = json_pack("{si}", "clients", count);
				scount = json_dumps(jcount, JSON_COMPACT);
				{{cookiecutter.small_prefix}}_ws_send(cfg, scount);
				free(scount);
				json_decref(jcount);
			} while (0);
			break;
		default:
			log_debug("websocket",
			    "received from [%s]:%d: opcode=%d (unexpected), len=%zu",
			    client->address, client->port,
			    msg->opcode,
			    msg->msg_length);
			break;
		}
	}
}

struct wslay_event_callbacks {{cookiecutter.small_prefix}}_wslay_callbacks = {
	{{cookiecutter.small_prefix}}_wslay_recv_callback,
	{{cookiecutter.small_prefix}}_wslay_send_callback,
	NULL,
	NULL,
	NULL,
	NULL,
	{{cookiecutter.small_prefix}}_wslay_on_msg_recv_callback
};

/**
 * Handle read event from websocket client
 */
static void
{{cookiecutter.small_prefix}}_ws_read_cb(struct bufferevent *bev, void *arg)
{
	struct {{cookiecutter.small_prefix}}_ws_client *client = arg;
	if (wslay_event_recv(client->ctx) == -1) {
		log_warnx("websocket",
		    "fatal error while receiving data from [%s]:%d",
		    client->address, client->port);
		{{cookiecutter.small_prefix}}_ws_shutdown_client(client);
		return;
	}
	{{cookiecutter.small_prefix}}_wslay_try_write(client);
}

/**
 * Handle write event for websocket client
 */
static void
{{cookiecutter.small_prefix}}_ws_write_cb(struct bufferevent *bev, void *arg)
{
	struct {{cookiecutter.small_prefix}}_ws_client *client = arg;
	{{cookiecutter.small_prefix}}_wslay_try_write(client);
}

/**
 * Handle misc events for websocket client
 */
static void
{{cookiecutter.small_prefix}}_ws_event_cb(struct bufferevent *bev, short events, void *arg)
{
	struct {{cookiecutter.small_prefix}}_ws_client *client = arg;
	if (events & BEV_EVENT_EOF) {
		log_debug("websocket", "received EOF from [%s]:%d",
		    client->address, client->port);
		{{cookiecutter.small_prefix}}_ws_shutdown_client(client);
		return;
	}
	if (events & BEV_EVENT_ERROR) {
		log_warnx("websocket", "got an error from [%s]:%d",
		    client->address, client->port);
		{{cookiecutter.small_prefix}}_ws_shutdown_client(client);
		return;
	}

}

/**
 * All headers has been sent, start websocket handling.
 */
static void
{{cookiecutter.small_prefix}}_ws_handshake_done_cb(struct bufferevent *bev, void *arg)
{
	struct {{cookiecutter.small_prefix}}_ws_client *client = arg;

	log_debug("websocket", "disable Nagle algorithm");
	int val = 1;
	if (setsockopt(bufferevent_getfd(bev), IPPROTO_TCP, TCP_NODELAY,
		&val, (socklen_t)sizeof(val)) == -1) {
		log_warn("websocket", "unable to disable Nagle algorithm");
	}

	if (wslay_event_context_server_init(&client->ctx,
		&{{cookiecutter.small_prefix}}_wslay_callbacks, client) == -1) {
		log_warnx("websocket",
		    "unable to initialize websocket context for [%s]:%d",
		    client->address, client->port);
		{{cookiecutter.small_prefix}}_ws_shutdown_client(client);
		return;
	}

	log_debug("websocket", "websocket with [%s]:%d ready",
	    client->address, client->port);
	bufferevent_enable(client->bev, EV_READ|EV_WRITE);
	bufferevent_setcb(client->bev,
	    {{cookiecutter.small_prefix}}_ws_read_cb,
	    {{cookiecutter.small_prefix}}_ws_write_cb,
	    {{cookiecutter.small_prefix}}_ws_event_cb,
	    client);
}

/**
 * Send a message to the websocket client.
 *
 * @param client  Client we want to send message to.
 * @param message NULL-terminated message we want to send.
 * @return 0 on success, -1 on failure
 *
 * This function is succesful as soon as the message is queued succesfully.
 */
int
{{cookiecutter.small_prefix}}_ws_send_to(struct {{cookiecutter.small_prefix}}_cfg *cfg,
    const char *message, struct {{cookiecutter.small_prefix}}_ws_client *client)
{
	struct wslay_event_msg msg = {
		WSLAY_TEXT_FRAME,
		(const uint8_t*)message,
		strlen(message)
	};
	int ret = wslay_event_queue_msg(client->ctx, &msg);
	if (ret < 0) {
		log_warnx("websocket", "unable to queue message for [%s]:%d (%d)",
		    client->address, client->port, ret);
		return -1;
	}
	{{cookiecutter.small_prefix}}_wslay_try_write(client);
	return 0;
}

/**
 * Send a message to all websocket clients.
 *
 * @param message NULL-terminated message we want to send.
 * @return 0 on success, -1 on failure
 *
 * This function is succesful as soon as the message is queued succesfully.
 */
int
{{cookiecutter.small_prefix}}_ws_send(struct {{cookiecutter.small_prefix}}_cfg *cfg,
    const char *message)
{
	struct {{cookiecutter.small_prefix}}_ws_client *client;
	int status, ret = 0;
	TAILQ_FOREACH(client, &cfg->ws_clients, next) {
		status = {{cookiecutter.small_prefix}}_ws_send_to(cfg, message, client);
		if (status == -1) ret = -1;
	}
	return ret;
}

/**
 * Handle a new websocket client.
 *
 * @return -1 in case of error (if this makes sense to recover)
 */
int
{{cookiecutter.small_prefix}}_ws_handle_req(struct {{cookiecutter.small_prefix}}_cfg *cfg, struct evhttp_request *req, const char *client_key)
{
	char *accept_key = {{cookiecutter.small_prefix}}_ws_accept_key(client_key);
	if (accept_key == NULL) return -1;

	struct {{cookiecutter.small_prefix}}_ws_client *client = calloc(1, sizeof(struct {{cookiecutter.small_prefix}}_ws_client));
	if (client == NULL) {
		log_warn("websocket", "no memory for new client");
		free(accept_key);
		return -1;
	}

	/* Take ownership of the request */
	evhttp_request_own(req);
	struct evhttp_connection *conn = evhttp_request_get_connection(req);
	struct bufferevent *bev = evhttp_connection_get_bufferevent(conn);
	client->cfg = cfg;
	client->bev = bev;
	client->conn = conn;
	evhttp_connection_get_peer(conn, &client->address, &client->port);

	/* Put our own callbacks */
	bufferevent_setcb(bev,
	    NULL,
	    {{cookiecutter.small_prefix}}_ws_handshake_done_cb,
	    {{cookiecutter.small_prefix}}_ws_event_cb,
	    client);
	bufferevent_enable(bev, EV_WRITE);
	bufferevent_disable(bev, EV_READ);

	evbuffer_add_printf(bufferevent_get_output(bev),
	    "HTTP/1.1 101 Switching Protocols\r\n"
	    "Upgrade: websocket\r\n"
	    "Connection: Upgrade\r\n"
	    "Sec-WebSocket-Accept: %s\r\n"
	    "\r\n", accept_key);
	free(accept_key);

	TAILQ_INSERT_TAIL(&cfg->ws_clients, client, next);
	return 0;
}

void
{{cookiecutter.small_prefix}}_ws_shutdown(struct {{cookiecutter.small_prefix}}_cfg *cfg)
{
	struct {{cookiecutter.small_prefix}}_ws_client *ws, *ws_next;
	for (ws = TAILQ_FIRST(&cfg->ws_clients);
	     ws;
	     ws = ws_next) {
		ws_next = TAILQ_NEXT(ws, next);
		{{cookiecutter.small_prefix}}_ws_shutdown_client(ws);
	}
}
