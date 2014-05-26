/* -*- mode: c; c-file-style: "openbsd" -*- */
/*
 * Copyright (c) 2014 Vincent Bernat <vbe@deezer.com>
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

#include <string.h>

enum {
	EMINCOUNT=1,
	EMAXCOUNT,
	EOTHER
};

static void
arg_addr_resetfn(struct arg_addr *parent)
{
	if (parent->info) {
		freeaddrinfo(parent->info);
	}
	parent->info = NULL;
	parent->count = 0;
}

static int
arg_addr_scanfn(struct arg_addr *parent, const char *argval)
{
	int errorcode = 0;
	if (parent->count != 0) {
		errorcode = EMAXCOUNT;
		goto end;
        }
	if (!argval) {
		parent->count++;
		goto end;
	}

	/* Get address and service. The service is the part after the
	 * last separator */
	char sep = parent->sep;
	char *node = NULL;
	const char *service = strrchr(argval, sep);
	if (!service) {
		service = argval;
	} else {
		node = strndup(argval, strlen(argval) - strlen(service));
		if (!node) {
			errorcode = EOTHER;
			goto end;
		}
		service++;
	}

	struct addrinfo hints = {
		.ai_family = AF_UNSPEC, /* IPv4 or IPv6 */
		.ai_socktype = SOCK_STREAM,
		.ai_flags = AI_PASSIVE /* Will be ignored if we have a node */
	};
	errorcode = getaddrinfo(node, service, &hints, &parent->info);
	free(node);
	if (!errorcode) parent->count++;

end:
    return errorcode;
}

static int
arg_addr_checkfn(struct arg_addr *parent)
{
	int errorcode = (parent->count < parent->hdr.mincount) ? EMINCOUNT : 0;
	return errorcode;
}

static void
arg_addr_errorfn(struct arg_addr *parent, FILE *fp, int errorcode,
    const char *argval, const char *progname)
{
	const char *shortopts = parent->hdr.shortopts;
	const char *longopts  = parent->hdr.longopts;
	const char *datatype  = parent->hdr.datatype;

	argval = argval ? argval : "";

	fprintf(fp,"%s: ",progname);
	switch(errorcode) {
	case 0:
		break;

        case EMINCOUNT:
		fputs("missing option \"",fp);
		arg_print_option(fp, shortopts, longopts, datatype, "\"\n");
		break;

        case EMAXCOUNT:
		fputs("excess option \"", fp);
		arg_print_option(fp, shortopts, longopts, argval, "\"\n");
		break;

	case EOTHER:
		fputs("internal error when handling option \"",  fp);
		arg_print_option(fp, shortopts, longopts, datatype, "\"\n");
		break;

	default:
		fprintf(fp, "value \"%s\" is not a valid address: %s\n", argval, gai_strerror(errorcode));
		break;
	}
}

static struct arg_addr *arg_addrn(const char *shortopts, const char *longopts,
    const char *datatype, int mincount, int maxcount,
    const char *glossary, char sep)
{
	/* We don't handle maxcount > 1 */
	size_t nbytes;
	struct arg_addr *result;
	nbytes = sizeof(struct arg_addr);
	result = malloc(nbytes);
	if (!result) return NULL;

	result->hdr.flag      = ARG_HASVALUE;
	result->hdr.shortopts = shortopts;
	result->hdr.longopts  = longopts;
	result->hdr.datatype  = datatype ? datatype : "addr:port";
	result->hdr.glossary  = glossary;
	result->hdr.mincount  = mincount;
	result->hdr.maxcount  = maxcount;
	result->hdr.parent    = result;
	result->hdr.resetfn   = (arg_resetfn*)arg_addr_resetfn;
	result->hdr.scanfn    = (arg_scanfn*)arg_addr_scanfn;
	result->hdr.checkfn   = (arg_checkfn*)arg_addr_checkfn;
	result->hdr.errorfn   = (arg_errorfn*)arg_addr_errorfn;
	result->hdr.priv      = NULL;

	result->sep = sep;
	result->info = NULL;
	result->count = 0;

	return result;
}


struct arg_addr *arg_addr1(const char *shortopts, const char *longopts,
    const char *datatype, const char *glossary, char sep)
{
	return arg_addrn(shortopts, longopts, datatype, 1, 1,
	    glossary, sep);
}

struct arg_addr *arg_addr0(const char *shortopts, const char *longopts,
    const char *datatype, const char *glossary, char sep)
{
	return arg_addrn(shortopts, longopts, datatype, 0, 1,
	    glossary, sep);
}
