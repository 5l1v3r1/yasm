/* $Id: errwarn.c,v 1.2 2001/05/21 18:31:43 peter Exp $
 * Error and warning reporting and related functions.
 *
 *  Copyright (C) 2001  Peter Johnson
 *
 *  This file is part of YASM.
 *
 *  YASM is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  YASM is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include "errwarn.h"
#include "globals.h"

unsigned int error_count = 0;
unsigned int warning_count = 0;

static char *fatal_msgs[] = {
    "unknown",
    "out of memory"
};

static char *err_msgs[] = {
    "unknown error",
    "parser error: %s",
    "missing '%1'",
    "missing argument to %s",
    "invalid argument to %s"
};

static char *warn_msgs[] = {
    "unknown",
    "ignoring unrecognized character '%s'"
};

/* conv_unprint: convert a possibly unprintable character into a printable
 * string, using standard cat(1) convention for unprintable characters. */
static char unprint[5];
char *conv_unprint(char ch)
{
    int pos=0;

    if(!isascii(ch) && !isprint(ch)) {
	unprint[pos++] = 'M';
	unprint[pos++] = '-';
	ch = toascii(ch);
    }
    if(iscntrl(ch)) {
	unprint[pos++] = '^';
	unprint[pos++] = (ch == '\177') ? '?' : ch | 0100;
    } else
	unprint[pos++] = ch;
    unprint[pos] = '\0';

    return unprint;
}

/* yyerror: parser error handler */
void yyerror(char *s)
{
    Error(ERR_PARSER, (char *)NULL, s);
}

void Fatal(fatal_num num)
{
    fprintf(stderr, "FATAL: %s\n", fatal_msgs[num]);
    exit(EXIT_FAILURE);
}

/* replace %1, %2, etc in src with %c, %s, etc. in argtypes. */
/* currently limits maximum number of args to 9 (%1-%9). */
static char *process_argtypes(char *src, char *argtypes)
{
    char *dest;
    char *argtype[9];
    int at_num;
    char *destp, *srcp, *argtypep;

    if(argtypes) {
	dest = malloc(strlen(src) + strlen(argtypes));
	if(!dest)
	    Fatal(FATAL_NOMEM);
	/* split argtypes by % */
	at_num = 0;
	while((argtypes = strchr(argtypes, '%')) && at_num < 9)
	    argtype[at_num++] = ++argtypes;
	/* search through src for %, copying as we go */
	destp = dest;
	srcp = src;
	while(*srcp != '\0') {
	    *(destp++) = *srcp;
	    if(*(srcp++) == '%') {
		if(isdigit(*srcp)) {
		    /* %1, %2, etc */
		    argtypep = argtype[*srcp-'1'];
		    while((*argtypep != '%') && (*argtypep != '\0'))
			*(destp++) = *(argtypep++);
		} else
		    *(destp++) = *srcp;
		srcp++;
	    }
	}
    } else {
	dest = strdup(src);
	if(!dest)
	    Fatal(FATAL_NOMEM);
    }
    return dest;
}

void Error(err_num num, char *argtypes, ...)
{
    va_list ap;
    char *printf_str;

    printf_str = process_argtypes(err_msgs[num], argtypes);

    fprintf(stderr, "filename:%u: ", line_number);
    va_start(ap, argtypes);
    vfprintf(stderr, printf_str, ap);
    va_end(ap);
    fprintf(stderr, "\n");

    free(printf_str);

    error_count++;
}

void Warning(warn_num num, char *argtypes, ...)
{
    va_list ap;
    char *printf_str;

    printf_str = process_argtypes(warn_msgs[num], argtypes);

    fprintf(stderr, "filename:%u: warning: ", line_number);
    va_start(ap, argtypes);
    vfprintf(stderr, printf_str, ap);
    va_end(ap);
    fprintf(stderr, "\n");

    free(printf_str);

    warning_count++;
}

