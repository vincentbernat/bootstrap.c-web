AM_CFLAGS = $(MORE_CFLAGS)
AM_CPPFLAGS = $(MORE_CPPFLAGS)
AM_LDFLAGS = $(MORE_LDFLAGS)

bin_PROGRAMS = {{cookiecutter.project_name|lower}}
dist_man_MANS = {{cookiecutter.project_name|lower}}.8

## TODO:5000 When you want to add more files, add them below.
## TODO:5000 Automake will find dependencies by itself. Run
## TODO:5000 ./autogen.sh after modifying this file.

{{cookiecutter.project_name|lower|replace("-","_")}}_SOURCES  = log.c log.h {{cookiecutter.project_name|lower}}.h {{cookiecutter.project_name|lower|replace("-","_")}}.c
{{cookiecutter.project_name|lower|replace("-","_")}}_SOURCES += event.h event.c http.c websocket.c
{{cookiecutter.project_name|lower|replace("-","_")}}_CFLAGS   = $(AM_CFLAGS)  @LIBEVENT_CFLAGS@ @JANSSON_CFLAGS@ @LIBWSLAY_CFLAGS@ @CRYPTO_CFLAGS@
{{cookiecutter.project_name|lower|replace("-","_")}}_LDFLAGS  = $(AM_LDFLAGS) @LIBEVENT_LIBS@   @JANSSON_LIBS@   @LIBWSLAY_LIBS@   @CRYPTO_LIBS@

## TODO:2004 Each time you have used `PKG_CHECK_MODULES` macro
## TODO:2004 in `configure.ac`, you get two variables that
## TODO:2004 you can substitute like above.

if LIBWSLAY_EMBEDDED
log.c: @LIBWSLAY_LIBS@
@LIBWSLAY_LIBS@: $(top_srcdir)/wslay/lib/*.c $(top_srcdir)/wslay/lib/*.h
	(cd $(top_builddir)/wslay && $(MAKE))
endif
