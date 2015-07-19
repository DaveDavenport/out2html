AC_INIT([out2html], [0.15.7-devel], [qball@gmpclient.org])

AC_CONFIG_SRCDIR([src/convert.c])
AC_CONFIG_HEADER([config.h])

AC_DEFINE(COPYRIGHT, "2011-2015",          "Copyright")
AC_DEFINE(AUTHOR,    "Qball Cow",          "Author")
AC_DEFINE(MAIL,      "qball@gmpclient.org","Mail")
##
# Setup automake to be silent and in foreign mode.
##
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects dist-xz])
AM_SILENT_RULES([yes])

##
# Check for compiler
##
AC_PROG_CC([clang gcc cc])
AC_PROG_CC_C99
AM_PROG_CC_C_O

AC_USE_SYSTEM_EXTENSIONS

##
# Check dependencies
##
PKG_PROG_PKG_CONFIG

PKG_CHECK_MODULES([glib], [glib-2.0],[], AC_MSG_ERROR("Glib-2.0 is required") )

AC_SUBST([EXTRA_CFLAGS], ["-Wall -Wextra -Wparentheses -Winline -pedantic"])

AC_CONFIG_FILES([Makefile ])
AC_OUTPUT
