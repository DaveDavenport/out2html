/* ASCII2HTML 
 * Copyright (C) 2011 Qball Cow <qball@gmpclient.org>
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
/* GLIB stuff */
#include <glib.h>
#include <glib-object.h>
#include <glib/gstdio.h>

/* Error throwing */
#define EXCEPTION(test,a, ...) if((test)){fprintf(stderr,"ERROR: " a, __VA_ARGS__); abort();} 
#define WARNING(a, ...) fprintf(stderr,"WARNING: " a, __VA_ARGS__);
//#define DEBUG(a, ...) 
#define DEBUG(a, ...) fprintf(stderr,"DEBUG: " a, __VA_ARGS__);


/** MAX_ATTR: Maximum number of attributes in one tag */
const int MAX_ATTR = 32;
/** ESCAPE_CHAR: the escape id used by bash. */
const int ESCAPE_CHAR = 27;
/** BASH color to html color table  */
const int NUM_COLORS = 8;
const char *colors[] = 
{ "black",	"red",		"green",	"#FFBB00", 	"blue",		"magenta",	"cyan",		"white" };

/** Parse CMD options  */
/* input/output file */
const gchar *input_file = NULL;
const gchar *output_file = NULL;
const gchar *input_charset = NULL;
const gchar *title = "ASCII2HTML";

/* Option Parser */
static void parse_cmd_options(int *argc, char ***argv)
{
	GError *error = NULL;
	GOptionContext *context = NULL;
	GOptionEntry entries[] = {
		{"input",   'i', 0, G_OPTION_ARG_FILENAME, &input_file, "Input file (default stdin)", NULL},
		{"output",  'o', 0, G_OPTION_ARG_FILENAME, &output_file, "Output file (default stdout)", NULL},
		{"charset", 'c', 0, G_OPTION_ARG_STRING, &input_charset, "Set the charset of the input", NULL},
		{"title",   't', 0, G_OPTION_ARG_STRING, &title, "Set the title off the HTML file", NULL},
		{NULL}
	};
	context = g_option_context_new("");
	g_option_context_set_description(context, "Converts program output (and colors) to HTML");
	g_option_context_set_summary(context, "A simple program to convert program output (f.e. from git diff) to a html file.");
	g_option_context_add_main_entries(context, entries,NULL);
	g_option_context_parse(context, argc, argv, &error);
	g_option_context_free(context);
	EXCEPTION(error != NULL, "Failed to parse commandline options: %s\n", error->message);
}

/* Translate color code V to html color name  */
static void put_color(FILE *out, int v)
{
	EXCEPTION(v<0 || v>= NUM_COLORS, "Unknown color code: %i\n", v);
	fputs(colors[v],out);
}

void process_color(FILE *out, int *attr, int num_attr)
{
	/* Count the depth of the divs */
	static int divs = 0;

	/* On explicit close, close it */
	if(num_attr == 1 && attr[0] == 0 && divs > 0) {
		fputs("</span>", out);
		divs--;
		return;
	}

	/* If we are going to set color, and there is still one open, close it */
	if(divs> 0) 
		fputs("</span>", out);
	else 
		divs++;
	fputs("<span style='", out);
	for(int j=0; j<num_attr; j++) {
		if(attr[j] == 1) {
			fputs("font-weight:bold", out);
		} else if (attr[j] >= 30 && attr[j] < 40) {
			fputs("color:", out);
			put_color(out, attr[j]%10);
		} else if (attr[j] >= 40 && attr[j] < 50) {
			fputs("background-color:", out);
			put_color(out, attr[j]%10);
		}
		fputs(";", out);
	}
	fputs("'>", out);
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
	fprintf(out, " </body>\n</html>");
}

/**
 * Main function
 */
enum ParserState {
	PARSE_NORMAL,
	PARSE_ATTRIBUTE,
	PARSE_COLOR_ATTRIBUTE
};

int main (int argc, char **argv)
{
	enum ParserState state = 0;
	gunichar in;
	/* buffer used to convert unichar to utf8 */
	gchar buffer[6];
	gchar *conv_buf;
	gint length;
	/* used to parse attribute */
	gint attributes[MAX_ATTR];
	gint num_attr = 0;	
	/* Input/output */
	FILE *input = stdin;
	FILE  *output = stdout;
	/* Error */
	GError *error = NULL;
	/* needed to get the right charset from g_get_charset */
	setlocale(LC_ALL, "");

	/* init the glib system  */
	g_type_init();
	/* Get charset */
	g_get_charset(&input_charset);

	/* parse options */
	parse_cmd_options(&argc, &argv);

	/* Handle input file */
	if(input_file) {
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

	/* Output html header */
	print_page_header(output);

	/* Convert and print body */
	fprintf(output, "<pre style='font-family:monospace'>\n");
	/* Read input */
	while(g_io_channel_read_unichar(chan, &in, &error) == G_IO_STATUS_NORMAL && error == NULL)
	{
		/* If we hit the escape character, go into 'attribute parsing' mode */
		if(in == ESCAPE_CHAR) {
			state = PARSE_ATTRIBUTE;
			/* reset */
			num_attr = 0;
			attributes[num_attr] = 0;
			continue;
		}
		/* if we are in attribute parsing mode, parse attribute */
		if(state == PARSE_ATTRIBUTE) {
			if(in == '[') {
				/* Begin of attributes */
				state = PARSE_COLOR_ATTRIBUTE;
				continue;
			}
			WARNING("Unknown Escape sequence found: %i\n", in);
			state = PARSE_NORMAL;
			continue;
		} else if(state == PARSE_COLOR_ATTRIBUTE) {
			if (in == ';') { /* End of element */
				attributes[++num_attr] = 0;
				EXCEPTION(num_attr >= MAX_ATTR, "Max number of supported attributes reached (%i)\n", MAX_ATTR);
			} else if(in == 'm') { /* end of attribute */
				num_attr++;
				EXCEPTION(num_attr >= MAX_ATTR, "Max number of supported attributes reached (%i)\n", MAX_ATTR);
				process_color(output, attributes, num_attr);
				state = PARSE_NORMAL;
			} else if(in >= '0' && in <= '9')
			{
				attributes[num_attr] *= 10;
				attributes[num_attr] += in-'0';
			}else if (in == 'h' || in == 'l' ) {
				WARNING("Unsupported attribute found: %i\n", in);
				state = PARSE_NORMAL;
			}
			continue;
		} else if (state == PARSE_NORMAL) {
			/* output the read character */	
			length = g_unichar_to_utf8(in, buffer);
			conv_buf = g_markup_escape_text(buffer, length);
			fputs(conv_buf, output);
			g_free(conv_buf);
		}
	}
	EXCEPTION(error != NULL, "Failed to read input character: %s\n", error->message);

	fprintf(output,"\n  </pre>\n");
	print_page_footer(output);

	/* free input channel */
	g_io_channel_unref(chan);

	/* close i/o */
	if(input != stdin) fclose(input);	
	if(output != stdout) fclose(output);

	return EXIT_SUCCESS;
}
