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

#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

static void
{{cookiecutter.small_prefix}}_event_log(int severity, const char *msg)
{
	switch (severity) {
	case _EVENT_LOG_DEBUG: log_debug("libevent", "%s", msg); break;
	case _EVENT_LOG_MSG: log_info("libevent", "%s", msg); break;
	case _EVENT_LOG_ERR: log_crit("libevent", "%s", msg); break;
	case _EVENT_LOG_WARN:
	default:
		log_warnx("libevent", "%s", msg); break;
	}
}

static void
{{cookiecutter.small_prefix}}_event_stop(evutil_socket_t fd, short what, void *arg)
{
	struct event_base *base = arg;
	event_base_loopbreak(base);
}

/**
 * Configure libevent
 *
 * @return -1 on failure
 */
int
{{cookiecutter.small_prefix}}_event_configure(struct {{cookiecutter.small_prefix}}_cfg *cfg)
{
	if ((cfg->event = calloc(1, sizeof(struct {{cookiecutter.small_prefix}}_event_private))) == NULL) {
		log_warn("event", "unable to allocate memory for events");
		return -1;
	}

	event_set_log_callback({{cookiecutter.small_prefix}}_event_log);
	if ((cfg->event->base = event_base_new()) == NULL) {
		log_warnx("event", "unable to get an event base");
		return -1;
	}
	log_info("event", "libevent %s initialized with %s method",
	    event_get_version(),
	    event_base_get_method(cfg->event->base));

	/* Signals */
	log_debug("event", "register signals");
	signal(SIGPIPE, SIG_IGN);
	signal(SIGHUP, SIG_IGN);
	evsignal_add(cfg->event->signals.sigint = evsignal_new(cfg->event->base,
		SIGINT, {{cookiecutter.small_prefix}}_event_stop, cfg->event->base),
	    NULL);
	evsignal_add(cfg->event->signals.sigterm = evsignal_new(cfg->event->base,
		SIGTERM, {{cookiecutter.small_prefix}}_event_stop, cfg->event->base),
	    NULL);

	return 0;
}

static void
nothing(evutil_socket_t fd, short what, void *arg)
{
}

/**
 * Run main loop
 */
void
{{cookiecutter.small_prefix}}_event_loop(struct {{cookiecutter.small_prefix}}_cfg *cfg)
{
	/* We need a fake timeout event to avoid exiting the loop if we only
	 * provide a web API. */
	struct event *ev = event_new(cfg->event->base, -1, EV_PERSIST, nothing, NULL);
	struct timeval one_minute = { 60, 0 };
	event_add(ev, &one_minute);
	/* Start the loop */
	log_debug("event", "starting main loop");
	if (event_base_loop(cfg->event->base, 0) != 0)
		log_warnx("event", "problem while running event loop");
}

/**
 * Release libevent related resources
 */
void
{{cookiecutter.small_prefix}}_event_shutdown(struct {{cookiecutter.small_prefix}}_cfg *cfg)
{
	if (cfg->event) {
		if (cfg->event->signals.sigint)
			event_free(cfg->event->signals.sigint);
		if (cfg->event->signals.sigterm)
			event_free(cfg->event->signals.sigterm);

		if (cfg->event->base)
			event_base_free(cfg->event->base);
		free(cfg->event);
	}
}
