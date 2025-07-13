/* unlock.c */

/*
 *   SklaffKOM, a simple conference system for UNIX.
 *
 *   Copyright (C) 1993-1994  Torbj|rn B}}th, Peter Forsberg, Peter Lindberg,
 *                            Odd Petersson, Carl Sundbom
 *
 *   Program dedicated to the memory of Staffan Bergstr|m.
 *
 *   For questions about this program, mail sklaff@sklaffkom.se
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

#include <sys/file.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include "sklaff.h"
#include <sys/stat.h>
/*
 * unlock - unlocks locked file
 * args: file descriptor (fd)
 * ret: return value from system
 */

void
unlock(int fd)
{
    char path[64];
    char filename[1024];
    ssize_t len;
    
    /* Modified by PL 2025-07-13 for more verbose output */

    /* Get the file path from /proc/self/fd/<fd> */
    snprintf(path, sizeof(path), "/proc/self/fd/%d", fd);
    len = readlink(path, filename, sizeof(filename) - 1);

    if (len != -1) {
        filename[len] = '\0';  // null-terminate
    } else {
        strcpy(filename, "(unknown)");
    }

    lseek(fd, 0L, 0);
    if (flock(fd, LOCK_UN)) {
        printf("\nError %d at flock() on file: %s. Please note this error and inform us.\n",
               errno, filename);
    } else {
	/* Optional debug logging. Can be commented out if it produces too much output, but I'll leave it here for now */
        debuglog("Unlocked: %s\n", 20);
    }
}
