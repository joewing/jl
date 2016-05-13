AC_INIT(jl, 0.0.1, joewing@ojewing.net)
AC_PREREQ(2.57)
AC_CONFIG_SRCDIR([src])
AC_CONFIG_HEADER([config.h])
AC_LANG(C)

AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL

CFLAGS="$CFLAGS -I. -fpic -fvisibility=hidden"

AC_ARG_ENABLE(debug,
    AC_HELP_STRING([--enable-debug], [create a debug build]) )
if test "$enable_debug" = "yes"; then
    CFLAGS="$CFLAGS -g -Wall -Werror"
    LDFLAGS="$LDFLAGS -g"
else
    enable_debug="no"
fi

PACKAGE=jl

AC_SUBST(CFLAGS)
AC_SUBST(LDFLAGS)
AC_SUBST(VERSION, "$PACKAGE_VERSION")
AC_SUBST(INSTVERSION, `echo $PACKAGE_VERSION | tr -d .`)
AC_SUBST(BINDIR, "$BINDIR")
AC_SUBST(MANDIR, "$MANDIR")
AC_SUBST(DATADIR, "$DATADIR")
AC_SUBST(DATE, `date -u -d "@${SOURCE_DATE_EPOCH:-$(date +%s)}" +%Y-%m-%d`)
AC_SUBST(SYSCONF, "$SYSCONF")
AC_SUBST(PACKAGE, "$PACKAGE")


AC_OUTPUT(
    Makefile
)
