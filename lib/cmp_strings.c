/* cmp_strings.c */

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

static void skip_beyond_space (char *, int *);

/*
 * skip_beyond_rpar - skips beyond right parenthesis
 * args: input string (s), position in string (index)
 */

static void skip_beyond_rpar(char *s, int *index)
{
    int i;
    i = *index;
    while ((s[i] != ')') && (s[i] != '\0')) {
	i++;
	if (s[i] == '(')
		skip_beyond_rpar (s, &i);
    }
    i++;			/* increment to get beyond the right parenthesis */
    if (isspace (s[i]) || (s[i] == '.'))
	    skip_beyond_space (s, &i);
    *index = i;
}

/*
 * skip_beyond_space - skips beyond next whitespace
 * args: input string (s), position in string (index)
 */

static void skip_beyond_space(char *s, int *index)
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
 * cmp_strings - compare strings
 * args: strings to compare (s1, s2)
 * ret: match (1), no match (0)
 */

int cmp_strings(char *s1, char *s2)
{
    static LINE tmp1, tmp2;
    int p1 = 0, p2 = 0;
    
    rtrim (s1);
    rtrim (s2);
    strcpy (tmp1, s1);
    strcpy (tmp2, s2);
    up_string (tmp1);
    up_string (tmp2);
    for (;;) {
	if (tmp1[p1] == '(') 
		skip_beyond_rpar (tmp1, &p1);
	if (tmp2[p2] == '(')
		skip_beyond_rpar (tmp2, &p2);
	if (tmp1[p1] != tmp2[p2])
		return 0; 
	while ((tmp1[p1] == tmp2[p2]) && (tmp1[p1] != '\0') && (tmp2[p2] != '\0')) {
	    if (isspace (tmp2[p2]) || (tmp2[p2] == '.')) {
		skip_beyond_space (tmp1, &p1);
		skip_beyond_space (tmp2, &p2);
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
	}
	if (tmp1[p1] == '\0') {
	    if (tmp2[p2] == '\0')
		    return 1; /* strings did match */
	    else
		    return 0; /* s2 longer than s1, no match */
	} 
	if (tmp2[p2] == '\0')
		return 1; /* if we got this far, the strings match */
    }
}
