/* sort_util.c */

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
 * order_name - put names back in proper order
 * args: string to order (instr), ordered string (outstr)
 * ret: pointer to outstr
 */

char *order_name(instr, outstr)
char *instr, *outstr;
{
    char *ptr, *tmp;
    
    ptr = strchr(instr, ' ');
    if (ptr) {
	if (*(ptr + 1) == '(') {
	    tmp = ptr;
	    *ptr = '\0';
	    ptr = strchr((tmp + 1), ' ');
	    *tmp = ' ';
	}
	if (ptr) {
	    *ptr = '\0';
	    sprintf(outstr, "%s %s", (ptr + 1), instr);
	}
	else {
	    strcpy(outstr, instr);
	}
    }
    else {
	strcpy(outstr, instr);
    }
    return outstr;
}

/*
 * reorder_name - put names in order for searching
 * args: string to reorder (instr), reordered string (outstr)
 * ret: pointer to outstr
 */

char *reorder_name(instr, outstr)
char *instr, *outstr;
{
    char *ptr, *tmp;
    
    ptr = strrchr(instr, ' ');
    if (ptr) {
	if (*(ptr + 1) == '(') {
	    tmp = ptr;
	    *ptr = '\0';
	    ptr = strrchr(instr, ' ');
	    *tmp = ' ';
	}
	if (ptr) {
	    *ptr = '\0';
	    sprintf(outstr, "%s %s", (ptr + 1), instr);
	}
	else {
	    strcpy(outstr, instr);
	}
    }
    else {
	strcpy(outstr, instr);
    }
    return outstr;
}

/*
 * fake_string - fake national characters for sorting
 * args: string to fake (string)
 * ret: pointer to string
 */

char *fake_string(string)
char *string;
{
    char *ptr;
    
    ptr = string;
    while (ptr && *ptr) {
	if (*ptr == '}') *ptr = '{';
	else if (*ptr == '{') *ptr = '|';
	else if (*ptr == '|') *ptr = '}';
	else if (*ptr == ']') *ptr = '[';
	else if (*ptr == '[') *ptr = 0x5c;
	else if (*ptr == 0x5c) *ptr = ']';
	ptr++;
    }
    return string;
}

/*
 * real_string - put string back to normal after sorting
 * args: string to make real (string)
 * ret: pointer to string
 */

char *real_string(string)
char *string;
{
    char *ptr;
    
    ptr = string;
    while (ptr && *ptr) {
	if (*ptr == '{') *ptr = '}';
	else if (*ptr == '|') *ptr = '{';
	else if (*ptr == '}') *ptr = '|';
	else if (*ptr == '[') *ptr = ']';
	else if (*ptr == 0x5c) *ptr = '[';
	else if (*ptr == ']') *ptr = 0x5c;
	ptr++;
    }
    return string;
}
