/* tty_raw.c */

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
#include <termios.h>
#include <string.h>    					/* 2025-08-10 PL needed to detect callers terminal height */
#include <unistd.h>     				/* 2025-08-10 PL needed to detect callers terminal height */
#include <stdlib.h>     				/* 2025-08-10 PL needed to detect callers terminal height */
#include <sys/ioctl.h>  				/* 2025-08-10 PL needed to detect callers terminal height (linux / freebsd) */

#include "sklaff.h"
#include "ext_globals.h"

/*
 * detect_terminal_lines - tries to autodetect callers terminal height, if fails defaults to 24
 */

int detect_terminal_lines(void) {	/* Not static anymore PL 2025-08-11 */
#ifdef TIOCGWINSZ
    struct winsize ws;
    int fd = isatty(STDOUT_FILENO) ? STDOUT_FILENO :
             (isatty(STDIN_FILENO) ? STDIN_FILENO : -1);
    if (fd >= 0 && ioctl(fd, TIOCGWINSZ, &ws) == 0 && ws.ws_row > 0)
        return ws.ws_row;
#endif
    const char *e = getenv("LINES");
    if (e) {
        long v = strtol(e, NULL, 10);
        if (v >= 10 && v <= 200) return (int)v;
    }
    return 24;
}

/*
 * tty_raw - sets terminal in raw mode
 */

void
tty_raw(void)
{

    struct termios temp_mode;

    tcgetattr(0, &temp_mode);
    memcpy(&Tty_mode, &temp_mode, sizeof(temp_mode));
    temp_mode.c_iflag = 0;
    temp_mode.c_oflag &= ~OPOST;
    temp_mode.c_lflag &= ~(ISIG | ICANON | ECHO);
    temp_mode.c_cc[VMIN] = 1;
    temp_mode.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &temp_mode);
    if (Numlines <= 0) /* 0 or unset = auto */
        Numlines = detect_terminal_lines();  
    Lines = 1;
}
