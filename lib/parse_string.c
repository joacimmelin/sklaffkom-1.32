/* parse_string.c */

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

static void skip_beyond_space ();

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
    i++;
    if (isspace (s[i]) || (s[i] == '.'))
	    skip_beyond_space (s, &i);
    *index = i;
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
    while (!isspace (s[i]) && (s[i] != '\0') && (s[i] != '.'))
	    i++;
    while (isspace (s[i]) || (s[i] == '.'))
	    i++;
    if (s[i] == '(')
	    skip_beyond_rpar (s, &i);
    *index = i;
}

/*
 * parse_strings - parse input strings
 * args: string1 (s1), string2 (s2), word count (wcnt), parameters (parm)
 * ret: number of words marched
 */

int parse_strings (s1, s2, wcnt, parm)
char *s1, *s2;
int wcnt;
char *parm;
{
    static LINE tmp1, tmp2, orig;
    int p1 = 0, p2 = 0, words = 0;

    rtrim (s1);
    rtrim (s2);
    strcpy (tmp1, s1);
    strcpy (tmp2, s2);
    strcpy (orig, tmp2);
    up_string (tmp1);
    up_string (tmp2);
    for (;;) {
	if (tmp1[p1] == '(')
		skip_beyond_rpar (tmp1, &p1);
	if (tmp2[p2] == '(')
		skip_beyond_rpar (tmp2, &p2);
	if (tmp1[p1] == '\0') {
	    if (tmp2[p2] == '\0')
		    return words + 1; /* strings did match */
	    else
		    break;
	}
	if (tmp1[p1] != tmp2[p2])
		return words;
	while ((tmp1[p1] == tmp2[p2]) && (tmp1[p1] != '\0') && (tmp2[p2] != '\0')) {
	    if (isspace (tmp2[p2]) || (tmp2[p2] == '.')) {
		skip_beyond_space (tmp1, &p1);
		skip_beyond_space (tmp2, &p2);
		if (tmp2[p2] != '\0')
			words++;
	    } else {
		p1++;
		p2++;
	    }
	}
	if (tmp1[p1] == '(')
		skip_beyond_rpar (tmp1, &p1);
	if (tmp2[p2] == '(')
		skip_beyond_rpar (tmp2, &p2);
	if (isspace (tmp2[p2]) || (tmp2[p2] == '.')) {
	    skip_beyond_space (tmp1, &p1);
	    skip_beyond_space (tmp2, &p2);
	    if (tmp2[p2] != '\0')
		    words++;
	}
	if (tmp1[p1] == '\0') {
	    if (tmp2[p2] == '\0')
		    return words + 1; /* strings did match */
	    else
		    break;
	}
	if (tmp2[p2] == '\0')
		return words + 1; /* if we got this far, the strings match */
	if (words == wcnt) {
	    break;
	}
    }
    if (isspace (tmp2[p2]) || (tmp2[p2] == '.')) {
	skip_beyond_space (tmp2, &p2);
	if (tmp2[p2] != '\0')
		words++;
    }
    if (parm != NULL) {
	p1 = 0;
	while (orig[p2] != '\0')
		tmp1[p1++] = orig[p2++];
	tmp1[p1++] = '\0';
	strcpy (parm, tmp1);
    }
    return words;
}
