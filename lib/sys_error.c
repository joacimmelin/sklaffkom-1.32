/* sys_error.c */

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
#include "ext_globals.h"
#include <errno.h>

#ifdef MT_XINU
extern int errno;
#endif

/*
 * sys_error - display error message for failed system call
 * args: function called (calling_function), index (error_index),
 *       function caused (causing function)
 */

#ifndef FREEBSD
extern char *sys_errlist[];
#endif

void	sys_error(calling_function, error_index, causing_function)
char	*calling_function, *causing_function;
int	error_index;
{
  LONG_LINE rok;

    sprintf(rok, "%s[%s #%d] %s(): %s\n", Program_name,
	   calling_function, error_index, causing_function,
	   sys_errlist[errno]);
    debuglog(rok, 5);
    output("%s[%s #%d] %s(): %s\n", Program_name,
	   calling_function, error_index, causing_function,
	   sys_errlist[errno]);
    return;
}
