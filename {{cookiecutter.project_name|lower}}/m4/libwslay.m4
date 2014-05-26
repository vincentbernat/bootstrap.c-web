#
# {{cookiecutter.small_prefix}}_CHECK_LIBWSLAY
#

AC_DEFUN([{{cookiecutter.small_prefix}}_CHECK_LIBWSLAY], [
  # First, try with pkg-config
  PKG_CHECK_MODULES([LIBWSLAY], [libwslay >= 0.1], [
  ], [
    # No appropriate version, let's use the shipped copy
    AC_MSG_NOTICE([using shipped libwslay])
    LIBWSLAY_EMBEDDED=1
  ])

  if test x"$LIBWSLAY_EMBEDDED" != x; then
    unset LIBWSLAY_LDFLAGS
    LIBWSLAY_CFLAGS="-I\$(top_srcdir)/wslay/lib/includes -I\$(top_builddir)/wslay/lib/includes"
    LIBWSLAY_LIBS="\$(top_builddir)/wslay/lib/libwslay.la"
  fi

  # Override configure arguments
  ac_configure_args="$ac_configure_args --disable-shared --enable-static"
  AC_CONFIG_SUBDIRS([wslay])
  AM_CONDITIONAL([LIBWSLAY_EMBEDDED], [test x"$LIBWSLAY_EMBEDDED" != x])
  AC_SUBST([LIBWSLAY_LIBS])
  AC_SUBST([LIBWSLAY_CFLAGS])
  AC_SUBST([LIBWSLAY_LDFLAGS])
])
