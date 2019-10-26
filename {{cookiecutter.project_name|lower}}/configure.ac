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
AX_CFLAGS_GCC_OPTION([-fdiagnostics-show-option],[MORE_CFLAGS])
AX_CFLAGS_GCC_OPTION([-pipe],[MORE_CFLAGS])
AX_CFLAGS_GCC_OPTION([-Wall],[MORE_CFLAGS])
AX_CFLAGS_GCC_OPTION([-W],[MORE_CFLAGS])
AX_CFLAGS_GCC_OPTION([-Wextra],[MORE_CFLAGS])
AX_CFLAGS_GCC_OPTION([-Wformat],[MORE_CFLAGS])
AX_CFLAGS_GCC_OPTION([-Wformat-security],[MORE_CFLAGS])
AX_CFLAGS_GCC_OPTION([-Wfatal-errors],[MORE_CFLAGS])
AX_CFLAGS_GCC_OPTION([-Wcast-align],[MORE_CFLAGS])
AX_CFLAGS_GCC_OPTION([-Winline],[MORE_CFLAGS])
AX_CFLAGS_GCC_OPTION([-fstack-protector],[MORE_CFLAGS])
AX_CFLAGS_GCC_OPTION([-D_FORTIFY_SOURCE=2],[MORE_CPPFLAGS])
AX_CFLAGS_GCC_OPTION([-Wno-unused-parameter],[MORE_CFLAGS])
AX_CFLAGS_GCC_OPTION([-Wno-missing-field-initializers],[MORE_CFLAGS])
AX_LDFLAGS_OPTION([-Wl,-z,relro],[MORE_LDFLAGS])
AX_LDFLAGS_OPTION([-Wl,-z,now],[MORE_LDFLAGS])
AX_LDFLAGS_OPTION([-Wl,--push-state,--no-as-needed -lSegFault -Wl,--pop-state], [MORE_LDFLAGS])

dnl Enable _GNU_SOURCE and the like
AX_CFLAGS_GCC_OPTION([-D_GNU_SOURCE],[MORE_CPPFLAGS])    dnl GNU systems (asprintf, ...)
case $host_os in
   solaris*)
     AX_CFLAGS_GCC_OPTION([-D__EXTENSIONS__],[MORE_CPPFLAGS]) dnl (CMSG_*)
     AX_CFLAGS_GCC_OPTION([-D_XPG4_2],[MORE_CPPFLAGS])        dnl (CMSG_*)
     ;;
   hpux*)
     AX_CFLAGS_GCC_OPTION([-D_XOPEN_SOURCE=500],[MORE_CPPFLAGS])      dnl HP-UX
     AX_CFLAGS_GCC_OPTION([-D_XOPEN_SOURCE_EXTENDED],[MORE_CPPFLAGS]) dnl HP-UX
     ;;
esac

AX_CFLAGS_GCC_OPTION([-D_GNU_SOURCE])    dnl GNU systems (asprintf, ...)
case $host_os in
   solaris*)
     AX_CFLAGS_GCC_OPTION([-D__EXTENSIONS__]) dnl (CMSG_*)
     AX_CFLAGS_GCC_OPTION([-D_XPG4_2])        dnl (CMSG_*)
     ;;
   hpux*)
     AX_CFLAGS_GCC_OPTION([-D_XOPEN_SOURCE=500])      dnl HP-UX
     AX_CFLAGS_GCC_OPTION([-D_XOPEN_SOURCE_EXTENDED]) dnl HP-UX
     ;;
esac

AC_CACHE_SAVE

## TODO:2003 Add the dependencies of your project here.

PKG_CHECK_MODULES([LIBEVENT], [libevent >= 2.0.17])
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

AC_SUBST([MORE_CFLAGS])
AC_SUBST([MORE_CPPFLAGS])
AC_SUBST([MORE_LDFLAGS])
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
  C Compiler.....: $CC $MORE_CFLAGS $MORE_CPPFLAGS $CFLAGS $CPPFLAGS
  Linker.........: $LD $MORE_LDFLAGS $LDFLAGS $LIBS
  libwslay.......: $libwslay
---------------------------------------------

Check the above options and compile with:
 ${MAKE-make}

EOF
