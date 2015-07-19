/* OUT2HTML
 * Copyright (C) 2011-2014 Qball Cow <qball@gmpclient.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <config.h>
/* GLIB stuff */
#include <glib.h>
#include <glib-object.h>
#include <glib/gstdio.h>

#include "colors.h"

typedef enum {
    AC_FIVE,
    AC_SEPARATOR,
    AC_COLOR
} advanced_color;


/* Error throwing */
#define EXCEPTION(test,a, ...) if((test)){fflush(NULL);fprintf(stderr,"\nERROR: " a, __VA_ARGS__); exit(1);}
#define WARNING(a, ...) fprintf(stderr,"\nWARNING: " a, __VA_ARGS__);

/** MAX_ATTR: Maximum number of attributes in one tag */
const int MAX_ATTR = 32;
/** ESCAPE_CHAR: the escape id used by bash. */
const int ESCAPE_CHAR = 27;

/** Parse CMD options  */
/* input/output file */
const gchar *input_file = NULL;
const gchar *output_file = NULL;
const gchar *input_charset = NULL;
const gchar *title = PACKAGE;
const GOptionEntry entries[] = {
	{"input",   'i', 0, G_OPTION_ARG_FILENAME, &input_file, "Input file (default stdin)", NULL},
	{"output",  'o', 0, G_OPTION_ARG_FILENAME, &output_file, "Output file (default stdout)", NULL},
	{"charset", 'c', 0, G_OPTION_ARG_STRING, &input_charset, "Set the charset of the input", NULL},
	{"title",   't', 0, G_OPTION_ARG_STRING, &title, "Set the title off the HTML file", NULL},
	{NULL}
};
/* Option Parser */
static void parse_cmd_options(int *argc, char ***argv)
{
	GError *error = NULL;
	GOptionContext *context = g_option_context_new("");
	g_option_context_set_description(context, "Converts program output (and colors) to HTML");
	g_option_context_set_summary(context, "A simple program to convert program output (f.e. from git diff) to a html file.");
	g_option_context_add_main_entries(context, entries,NULL);
	g_option_context_parse(context, argc, argv, &error);
	g_option_context_free(context);
	EXCEPTION(error != NULL, "Failed to parse commandline options: %s\n", error->message);
}

static bool process_attribute(FILE *out, GIOChannel *chan, GError *error, int attr)
{
    /* Count the depth of the divs */
    static int divs = 0;
    /* If we are going to set color, and there is still one open, close it */
    if(attr == 0) {
	for(;divs;divs--) {
            fputs("</span>", out);
        }
        return false ;
    } else if(attr == 1) {
        fputs("<span style='font-weight:bold'>", out);
    } else if(attr == 2) {
        fputs("<span style='font-weight:lighter'>", out);
    } else if (attr == 3) {
        fputs("<span style='font-style:italic;'>", out);
    } else if (attr == 4) {
        fputs("<span style='text-decoration: underline;'>", out);
    } else if (attr == 5 || attr == 6) {
        fputs("<span style='text-decoration: blink;'>", out);
    } else if (attr == 9) {
        fputs("<span style='text-decoration: line-through;'>", out);
    } else if (attr >= 30 && attr < 38) {
        fprintf(out, "<span style='color:%s'>", colors[attr%10]);
    } else if (attr >= 40 && attr < 48) {
        fprintf(out, "<span style='background-color:%s'>", colors[attr%10]);
    } else if (attr >= 90 && attr < 98) {
        fprintf(out, "<span style='color:%s'>", bright_colors[attr%10]);
    } else if (attr >= 100 && attr < 108) {
        fprintf(out, "<span style='background-color:%s'>", bright_colors[attr%10]);
    } else if (attr  == 38  || attr == 48) {
        // TODO: This is a dirty hack to get the 'advanced' color parsing.
        // This seems to break the normal way of parsing attributes.
        // Format is \033[38;5;<color>
        gunichar in;
        advanced_color state = AC_FIVE;
        int color = 0;
        // Special color code parsing.
        while(g_io_channel_read_unichar(chan, &in, &error) == G_IO_STATUS_NORMAL && error == NULL)
        {
            if(state == AC_FIVE && in == '5') state = AC_SEPARATOR;
            else if (state == AC_SEPARATOR && in == ';') state = AC_COLOR;
            else if (state == AC_COLOR) {
                if(in == ';' || in == 'm'){ 
                    EXCEPTION(( color >= NUM_COLORS ), "Invalid color code element: %d\n", color);
                    fprintf(out, "<span style='%scolor:%s'>",
                            (attr == 38)?"":"background-", colors[color]);
                    divs++;
                    return (in == 'm');
                }
                EXCEPTION((in < '0' || in > '9' ), "Invalid color code element: %d\n", in);
                color*=10;
                color+= (in-'0');
            }else{
                EXCEPTION(1, "Invalid 256 color code: %d\n", in);
            }
        }

    }else {
        return false;
    }
    divs++;
    return false;
}

/** Print the header of the html page */
static void print_page_header(FILE *out)
{
	fprintf(out, "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\"\n \"http://www.w3.org/TR/html4/strict.dtd\">\n");
	fprintf(out, "<html>\n <head>\n");
	fprintf(out, "  <title>%s</title>\n", title);
	fprintf(out, "  <meta http-equiv='Content-Type' content='text/html; charset=UTF-8'>\n");
	fprintf(out, " </head>\n <body>\n");
}

/** Print the ending of the html page  */
static void print_page_footer(FILE *out)
{
	fprintf(out, " <p><i>Generated by <b>%s</b> version %s.",PACKAGE, VERSION);
	fprintf(out, " Copyright %s: %s",COPYRIGHT, AUTHOR);
	fprintf(out, " &lt;<a href=\"mailto:%s\">%s</a>&gt;</i></p>\n",MAIL, MAIL);
	fprintf(out, " </body>\n</html>");
}

/* Main function  */
enum ParserState {
	PARSE_NORMAL,
	PARSE_ATTRIBUTE,
	PARSE_COLOR_ATTRIBUTE
};

int main (int argc, char **argv)
{
	enum ParserState state = 0;
	gunichar in;
	gboolean init = FALSE;
	/* used to parse attribute */
	gint attributes = 0;
	/* Input/output */
	FILE *input = stdin;
	FILE  *output = stdout;
	/* Error */
	GError *error = NULL;

	/* init the glib system  */
#if ! GLIB_CHECK_VERSION(2,36,0)
	g_type_init();
#endif
	/* needed to get the right charset from g_get_charset */
	setlocale(LC_ALL, "");
	/* Get charset */
	g_get_charset(&input_charset);

	/* parse options */
	parse_cmd_options(&argc, &argv);

	/* Handle input file */
	if(input_file && strcmp(input_file, "-")) {
		input = g_fopen(input_file, "r");
		EXCEPTION(input == NULL, "Failed to open: %s: %s\n", input_file, strerror(errno));
	}
	/* Handle output file */
	if(output_file) {
		output = g_fopen(output_file, "w");
		EXCEPTION(output == NULL, "Failed to open: %s: %s\n", output_file, strerror(errno));
	}
	/* Create channel for input */
	GIOChannel *chan = g_io_channel_unix_new(fileno(input));
	/* Set channel encoding */
	g_io_channel_set_encoding(chan, input_charset,&error);
	EXCEPTION(error != NULL, "Failed to set input encoding: %s\n", error->message);

	/* Read input */
	while(error == NULL && g_io_channel_read_unichar(chan, &in, &error) == G_IO_STATUS_NORMAL && error == NULL)
	{
		if(!init) {
			/* Output html header */
			print_page_header(output);
			/* Convert and print body */
			fprintf(output, "<pre style='font-family:monospace'>\n");
			init = TRUE;
		}
		/* If we hit the escape character, go into 'attribute parsing' mode */
		if(in == ESCAPE_CHAR) {
			state = PARSE_ATTRIBUTE;
			/* reset */
			attributes = 0;
		}
		/* if we are in attribute parsing mode, parse attribute */
		else if(state == PARSE_ATTRIBUTE) {
			if(in == '[') {
				/* Begin of attributes */
				state = PARSE_COLOR_ATTRIBUTE;
			}else {
				WARNING("Unknown Escape sequence found: %i\n", in);
				state = PARSE_NORMAL;
			}
		} else if(state == PARSE_COLOR_ATTRIBUTE) {
			if (in == ';') { /* End of element */
				if(process_attribute(output,chan,error, attributes))
                    state = PARSE_NORMAL;
                else
                    attributes = 0;
			} else if(in == 'm') { /* end of attribute */
				process_attribute(output,chan, error, attributes);
				state = PARSE_NORMAL;
			} else if(in >= '0' && in <= '9') {
				attributes *= 10;
				attributes += in-'0';
			}else if (in == 'h' || in == 'l' ) {
				WARNING("Unsupported attribute found: %i\n", in);
				state = PARSE_NORMAL;
			}
			continue;
		} else if (state == PARSE_NORMAL) {
			/* special chars (htmlspecialchars php doc) */
			if(in == '"') fputs("&quot;", output);
			else if(in == '\'') fputs("&#039;", output);
			else if(in == '&') fputs("&amp;", output);
			else if(in == '<') fputs("&lt;", output);
			else if(in == '>') fputs("&gt;", output);
			/* ascii values stay ascii*/
			else if(in >= 0 && in <= 177)
				fprintf(output, "%c", (char)in);
			/* Rest we encode in utf8 */
			else fprintf(output, "&#%i;", in);
		}
	}
	EXCEPTION(error != NULL, "Failed to read input character: %s\n", error->message);
	if(init) {
		/* Close open tags */
		process_attribute(output,NULL, NULL, 0);
		fprintf(output,"\n  </pre>\n");
		print_page_footer(output);
	}

	/* free input channel */
	g_io_channel_unref(chan);

	/* close i/o */
	if(input != stdin) fclose(input);
	if(output != stdout) fclose(output);

	return EXIT_SUCCESS;
}
