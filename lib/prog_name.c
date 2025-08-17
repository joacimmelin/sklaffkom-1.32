/* prog_name.c */

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

#include <pwd.h>

#include "sklaff.h"
#include "ext_globals.h"

/*
 * prog_name - finds out absolute path to program running
 * args: program name typed (pname)
 * ret: abolute path to program
 */

char *
prog_name(char *pname)
{
    static LINE absname;
    char *path, *p1, *tmp;
    HUGE_LINE p, cwd;
    struct passwd *pw;

    if (*pname == '-') {
        pw = getpwuid(getuid());
        strcpy(absname, pw->pw_shell);
        return absname;
    }
    if (getcwd(cwd, HUGE_LINE_LEN) == NULL) {
    perror("getcwd"); /* keeps linux compiler happy 2025-07-25, PL */
    return NULL;
    }

    if (*pname == '/') {
        strcpy(absname, pname);
        return absname;
    }
    if (strchr(pname, '/')) {
        strcpy(absname, cwd);
        strcat(absname, "/");
        strcat(absname, pname);
        return absname;
    }
    path = getenv("PATH");
    strcpy(p, path);
    tmp = p;
    while (*tmp) {
        p1 = strchr(tmp, ':');
        if (!p1) {
            while (*p1 != 0)
                p1++;
        } else {
            *p1 = '\0';
            p1++;
        }
        strcpy(absname, tmp);
        strcat(absname, "/");
        strcat(absname, pname);
        if (file_exists(absname) != -1) {
            break;
        }
        tmp = p1;
    }

    if (*absname != '/') {
        strcpy(p, absname);
        strcpy(absname, cwd);
        strcat(absname, "/");
        strcat(absname, p);
    }
    return absname;
}
