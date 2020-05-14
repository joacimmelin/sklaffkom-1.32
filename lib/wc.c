/* wc.c */

/*
 *   SklaffKOM, a simple conference system for UNIX.
 *
 *   Copyright (C) 1993-1994  Torbj|rn B}}th, Peter Forsberg, Peter Lindberg,
 *                            Odd Petersson, Carl Sundbom
 *
 *   Program dedicated to the memory of Staffan Bergstr|m.
 *
 *   For questions about this program, mail sklaff@skom.se
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2, or (at your option)
 *   any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "sklaff.h"

/*
 * skip_beyond_rpar - skips beyond right parenthesis
 * args: input string (s), position in string (index)
 */

static void skip_beyond_rpar (s, index)
LINE s;
int *index;
{
    int i;
    i = *index;
    while ((s[i] != ')') && (s[i] != '\0')) {
	i++;
	if (s[i] == '(')
	    skip_beyond_rpar (s, &i);
    }
    *index = i + 1;
}

/*
 * skip_beyond_space - skips beyond next whitespace
 * args: input string (s), position in string (index)
 */

static void skip_beyond_space (s, index)
LINE s;
int *index;
{
    int i;
    i = *index;
    while (isspace (s[i]) || (s[i] == '.'))
	i++;
    *index = i;
}

/*
 * wc - count words in string
 * args: string (s)
 * ret: number of words
 */

int
wc (char *s)
{
    int words = 0, p = 0;
    while (s[p] != '\0') {
	if (isspace (s[p]) || (s[p] == '.')) {
	    skip_beyond_space (s, &p);
	    if (s[p] != '\0')
		words++;
	} else if ((s[p] == '('&& ((p && (isspace (s[p - 1]) ||
					  (s[p-1] == '.'))) || !p))) {
	    skip_beyond_rpar (s, &p);
	    words--;
	} else {
	    p++;
	}
    }
    return words + 1;
}
