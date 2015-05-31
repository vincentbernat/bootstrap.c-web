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
#include <getopt.h>

extern const char *__progname;

static char *
full_path(const char *path)
{
	char *newpath = realpath(path, NULL);
        return newpath;
}

static struct addrinfo *
address(const char *address_port)
{
        struct addrinfo hints = {
                .ai_family = AF_UNSPEC, /* IPv4 or IPv6 */
                .ai_socktype = SOCK_STREAM,
                .ai_flags = AI_PASSIVE
        };
	char *str = strdup(address_port);
	char *sep = strrchr(str, ':');
	if (sep == NULL) {
		fprintf(stderr, "incorrect address:port: %s\n", address_port);
		free(str);
		return NULL;
	}
	*sep = '\0';

	struct addrinfo *info;
        getaddrinfo(str, sep + 1,
            &hints, &info);
	free(str);
        return info;
}

static void
usage()
{
	fprintf(stderr, "Usage: %s", __progname);
	fprintf(stderr, "\n");
	fprintf(stderr, " -d, --debug                be more verbose\n");
	fprintf(stderr, " -h, --help                 display help and exit\n");
	fprintf(stderr, " -v, --version              print version and exit\n");
	fprintf(stderr, " -D TOKENS                  set allowed debug tokens\n");
	fprintf(stderr, " -l ADDRESS:PORT,\n");
	fprintf(stderr, " --listen ADDRESS:PORT      address and port to bind to\n");
	fprintf(stderr, " -w PATH, --web PATH        directory containing static assets\n");
}

int
main(int argc, char *argv[])
{
	int exitcode = EXIT_FAILURE, c;
	int debug = 0;
	const char *web_address = {{cookiecutter.small_prefix|upper}}_WEB_ADDRESS ":" {{cookiecutter.small_prefix|upper}}_WEB_PORT;
	const char *web_path =  {{cookiecutter.small_prefix|upper}}_WEB_DIR;
	struct {{cookiecutter.small_prefix}}_cfg cfg = {};

	/* TODO:3001 If you want to add more options, add them here. */
	/* TODO:3001 Don't forget to update usage above and manual page. */
	static struct option long_options[] = {
		{ "debug", no_argument, 0, 'd' },
		{ "help",  no_argument, 0, 'h' },
		{ "version", no_argument, 0, 'v' },
		{ "listen", required_argument, 0, 'l' },
		{ "web", required_argument, 0, 'w' },
		{ 0 }
	};

	while (1) {
		int option_index = 0;
		c = getopt_long(argc, argv, "dhvl:w:t:T:S:I",
		    long_options, &option_index);
		if (c == -1) break;

		switch (c) {
		case 'd':
			debug++;
			break;
		case 'h':
			usage();
			return 0;
		case 'v':
			fprintf(stdout, "%s\n", PACKAGE_VERSION);
			return 0;
		case 'D':
			log_accept(optarg);
			break;
		case 'l':
			web_address = optarg;
			break;
		case 'w':
			web_path = optarg;
			break;
		case '?':
			usage();
			return 1;
		default:
			fprintf(stderr, "unknown option `%c'\n", c);
			usage();
			return 1;
		}
	}
	if (optind < argc) {
		fprintf(stderr, "positional arguments are not accepted\n");
		usage();
		return 1;
	}

	log_init(debug, __progname);

	cfg.listen = address(web_address);
	if (!cfg.listen) {
		log_crit("main", "unable to resolve %s", web_address);
		goto exit;
	}
	cfg.web = full_path(web_path);
	if (!cfg.web) {
		log_crit("main", "unable to resolve %s", web_path);
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
	if (cfg.listen) freeaddrinfo(cfg.listen);
	free(cfg.web);
	return exitcode;
}
