/* -*- mode: c; c-file-style: "openbsd" -*- */
/* TODO:5002 You may want to change the copyright of all files. This is the
 * TODO:5002 ISC license. Choose another one if you want.
 */
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

#include "{{cookiecutter.project_name|lower}}.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>

extern const char *__progname;

static char *
full_path(const char *path)
{
char *newpath = realpath(path, NULL);
        return newpath;
}

static struct addrinfo *
listen_or_default(struct addrinfo **info)
{
        if (*info) return *info;

        /* Default value for listen */
        struct addrinfo hints = {
                .ai_family = AF_UNSPEC, /* IPv4 or IPv6 */
                .ai_socktype = SOCK_STREAM,
                .ai_flags = AI_PASSIVE
        };
        getaddrinfo({{cookiecutter.small_prefix|upper}}_WEB_ADDRESS,
	    {{cookiecutter.small_prefix|upper}}_WEB_PORT,
            &hints, info);
        return *info;
}

int
main(int argc, char *argv[])
{
	int exitcode = EXIT_FAILURE;

	/* TODO:3001 If you want to add more options, add them here. */
	/* TODO:3001 If you don't want to use libargtable2, you can replace */
	/* TODO:3001 this code by `getopt()`. See the following URL: */
	/* TODO:3001   https://github.com/vincentbernat/bootstrap.c/blob/master/%7B%7Bcookiecutter.project_name%7Clower%7D%7D/src/%7B%7Bcookiecutter.project_name%7Clower%7D%7D.c */
	struct arg_end *arg_fini    = arg_end(5);
	struct arg_lit *arg_debug   = arg_litn("d", "debug", 0, 3, "be more verbose");
	struct arg_lit *arg_help    = arg_lit0("h", "help", "display help and exit");
	struct arg_lit *arg_version = arg_lit0("v", "version", "print version and exit");
	struct arg_str *arg_filter  = arg_strn("D", NULL, NULL, 0, 10, "set allowed debug tokens");
	struct arg_addr *arg_listen = arg_addr0("l", "listen", "address:port", "address and port to bind to", ':');
        struct arg_str  *arg_web    = arg_str0("w", "web", "directory", "directory containing static assets");

	void *argtable[] = { arg_debug,
			     arg_help,
			     arg_version,
			     arg_filter,
			     arg_listen,
			     arg_web,
			     /* Other arguments should be here */
			     arg_fini
	};

	struct {{cookiecutter.small_prefix}}_cfg cfg = {};

	if (arg_nullcheck(argtable) != 0) {
		fprintf(stderr, "%s: insufficient memory\n", __progname);
		goto exit;
	}

	arg_web->sval[0] = {{cookiecutter.small_prefix|upper}}_WEB_DIR;

	int n = arg_parse(argc, argv, argtable);
	if (n != 0 || arg_help->count) {
		if (n != 0)  arg_print_errors(stderr, arg_fini, __progname);
		else exitcode = EXIT_SUCCESS;
		fprintf(stderr, "Usage: %s", __progname);
		arg_print_syntax(stderr, argtable, "\n");
		arg_print_glossary(stderr, argtable, "  %-25s %s\n");
		fprintf(stderr, "\n");
		fprintf(stderr, "see manual page " PACKAGE "(8) for more information\n");
		goto exit;
	}

	if (arg_version->count) {
		fprintf(stdout, "%s\n", PACKAGE_VERSION);
		exitcode = EXIT_SUCCESS;
		goto exit;
	}

	for (int i = 0; i < arg_filter->count; i++)
		log_accept(arg_filter->sval[i]);

	log_init(arg_debug->count, __progname);

	cfg.listen = listen_or_default(&arg_listen->info);
	cfg.web = full_path(arg_web->sval[0]);
	if (!cfg.web) {
		log_crit("main", "unable to resolve %s", arg_web->sval[0]);
		goto exit;
	}

	/* TODO:3000 To do some additional stuff, you need to initialize submodules here.
	   TODO:3000 Try to keep the libevent and http module only targeted for this.
	   TODO:3000 Use another module if you need to do additional stuff.
	*/
	TAILQ_INIT(&cfg.sse_clients);
	TAILQ_INIT(&cfg.ws_clients);
	const char *what;
	if ((what = "libevent", {{cookiecutter.small_prefix}}_event_configure(&cfg)) == -1 ||
	    (what = "http", {{cookiecutter.small_prefix}}_http_configure(&cfg) == -1)) {
		log_crit("main", "%s configuration has failed", what);
		goto exit;
	}

	{{cookiecutter.small_prefix}}_event_loop(&cfg);

	exitcode = EXIT_SUCCESS;
exit:
	{{cookiecutter.small_prefix}}_ws_shutdown(&cfg);
	{{cookiecutter.small_prefix}}_http_shutdown(&cfg);
	{{cookiecutter.small_prefix}}_event_shutdown(&cfg);
	if (arg_listen && arg_listen->info) freeaddrinfo(arg_listen->info);
	arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
	free(cfg.web);
	return exitcode;
}
