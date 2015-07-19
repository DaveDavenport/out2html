# Specify automake version.
AUTOMAKE_OPTIONS  = 1.11.3

##
# Rofi the program
##
bin_PROGRAMS=out2html

LIBS=\
    @glib_LIBS@

AM_CFLAGS=\
	@EXTRA_CFLAGS@\
    @glib_CFLAGS@\
    -I$(top_srcdir)/include/\
	-Werror=missing-prototypes

out2html_SOURCES=\
	src/colors.c\
	src/convert.c\
	include/colors.h

##
# Indent
##

indent: ${out2html_SOURCES}
	uncrustify -c ${top_srcdir}/data/uncrustify.cfg --replace $^

