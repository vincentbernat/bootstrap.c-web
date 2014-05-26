#
# {{cookiecutter.small_prefix}}_ARG_WITH
#

dnl {{cookiecutter.small_prefix}}_ARG_WITH(name, help1, default)

AC_DEFUN([{{cookiecutter.small_prefix}}_ARG_WITH],[
  eval _default=`(test "x$prefix" = xNONE && prefix="$ac_default_prefix"
                  test "x$exec_prefix" = xNONE && exec_prefix="${prefix}"
                  eval _lcl="$3"
                  eval _lcl="\"[$]_lcl\"" ; eval _lcl="\"[$]_lcl\""
                  echo "[$]_lcl")`
  AC_ARG_WITH([$1],
        AS_HELP_STRING([--with-$1],
                [$2 @<:@default=$3@:>@]),
        AC_DEFINE_UNQUOTED(AS_TR_CPP([$1]), ["$withval"], [$2])
        AC_SUBST(AS_TR_CPP([$1]), [\"$withval\"]),
        AC_DEFINE_UNQUOTED(AS_TR_CPP([$1]), ["${_default}"], [$2])
        AC_SUBST(AS_TR_CPP([$1]), [\"${_default}\"]))
])
