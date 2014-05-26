#                                               -*- Autoconf -*-
AC_PREREQ([2.64])

AC_INIT([{{cookiecutter.project_name}}],
        [m4_esyscmd_s([./get-version])],
        [{{cookiecutter.email}}])
AC_CONFIG_SRCDIR([src/log.c])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_FILES([Makefile src/Makefile web/Makefile])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign -Wall -Werror tar-ustar])
AM_MAINTAINER_MODE
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES(yes)])
m4_pattern_allow([AM_PROG_AR])
AM_PROG_AR

# Configure libtool
LT_INIT

#######################
### Checks

# Checks for programs.
AC_PROG_CC
AC_PROG_CC_C99
AC_PROG_CXX
AM_PROG_CC_C_O
AC_PROG_LIBTOOL
AC_PROG_LN_S
AC_PROG_EGREP
AC_PATH_PROG([GRUNT],[grunt],[no])

# Check some compiler flags
AX_CFLAGS_GCC_OPTION([-fdiagnostics-show-option])
AX_CFLAGS_GCC_OPTION([-pipe])
AX_CFLAGS_GCC_OPTION([-Wall])
AX_CFLAGS_GCC_OPTION([-W])
AX_CFLAGS_GCC_OPTION([-Wextra])
AX_CFLAGS_GCC_OPTION([-Wformat])
AX_CFLAGS_GCC_OPTION([-Wformat-security])
AX_CFLAGS_GCC_OPTION([-Wfatal-errors])
AX_CFLAGS_GCC_OPTION([-Wcast-align])
AX_CFLAGS_GCC_OPTION([-Winline])
AX_CFLAGS_GCC_OPTION([-fstack-protector])
AX_CFLAGS_GCC_OPTION([-D_FORTIFY_SOURCE=2])
AX_CFLAGS_GCC_OPTION([-Wno-unused-parameter])
AX_CFLAGS_GCC_OPTION([-Wno-missing-field-initializers])
AX_LDFLAGS_OPTION([-Wl,-z,relro])
AX_LDFLAGS_OPTION([-Wl,-z,now])

AC_CACHE_SAVE

## TODO:2003 Add the dependencies of your project here.

PKG_CHECK_MODULES([ARGTABLE], [argtable2 >= 9])
PKG_CHECK_MODULES([LIBEVENT], [libevent >= 2.0.4])
PKG_CHECK_MODULES([JANSSON],  [jansson >= 2.1])
PKG_CHECK_MODULES([CRYPTO],   [libcrypto >= 0.9.8])
{{cookiecutter.small_prefix}}_CHECK_LIBWSLAY

AC_CACHE_SAVE

## TODO:2005 Add compile-time options here. They are
## TODO:2005 usually overridable using the command-line.

{{cookiecutter.small_prefix}}_ARG_WITH([{{cookiecutter.small_prefix}}-web-dir],
             [Which directory contains static web files],
             [${datadir}/${PACKAGE_NAME}/web])
{{cookiecutter.small_prefix}}_ARG_WITH([{{cookiecutter.small_prefix}}-web-address],
             [Address to bind to by default],
             [localhost])
{{cookiecutter.small_prefix}}_ARG_WITH([{{cookiecutter.small_prefix}}-web-port],
             [Port to listen to by default],
             [8087])

AM_CONDITIONAL([HAVE_GRUNT], [test x"$GRUNT" != x"no"])

AC_OUTPUT

if test x"$LIBWSLAY_EMBEDDED" = x; then
   libwslay=system
else
   libwslay=embedded
fi

cat <<EOF

------------------ Summary ------------------
 $PACKAGE_NAME version $PACKAGE_VERSION
  Prefix.........: $prefix
  C Compiler.....: $CC $CFLAGS $CPPFLAGS
  Linker.........: $LD $LDFLAGS $LIBS
  libwslay.......: $libwslay
---------------------------------------------

Check the above options and compile with:
 ${MAKE-make}

EOF
