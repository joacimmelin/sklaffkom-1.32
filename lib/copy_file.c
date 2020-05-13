/* copy_file.c */

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
 * copy_file - copy a file
 * args: file to copy (fil1), copy (fil2)
 * ret: ok (0), failure (-1)
 */

int copy_file(fil1, fil2)
char *fil1, *fil2;
{
    char	*buf;
    int	fd1, fd2;
    
    if ((fd1 = open_file(fil1, 0)) == -1) {
	sys_error("copy_file", 1, "open_file");
	return -1;
    }
    
    if ((buf = read_file(fd1)) == NULL) {
	sys_error("copy_file", 2, "read_file");
	return -1;
    }
    
    if ((fd2 = open_file(fil2, OPEN_CREATE | OPEN_QUIET)) == -1) {
	sys_error("copy_file", 3, "open_file");
	return -1;
    }
    
    if (write_file(fd2, buf) == -1) {
	sys_error("copy_file", 4, "write_file");
	return -1;
    }
    
    if (close_file(fd2) == -1) {
	sys_error("copy_file", 5, "close_file");
	return -1;
    }
    
    if (close_file(fd1) == -1) {
	sys_error("copy_file", 6, "close_file");
	return -1;
    }
    
    return 0;
}

