/* sig_setup.c */

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

#include <signal.h>

#include "sklaff.h"

/*
 * sig_setup - setup signal for SklaffKOM
 */

void
sig_setup(void)
{
    signal(SIGHUP, exec_logout);
    signal(SIGPIPE, exec_logout);
    signal(SIGTERM, exec_logout);
    signal(SIGALRM, timeout);
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGTRAP, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGWINCH, SIG_IGN);
    signal(SIGNAL_NEW_TEXT, baffo);
    signal(SIGNAL_NEW_MSG, newmsg);
    signal(SIGCONT, SIG_IGN);
#ifdef SIGXCPU
    signal(SIGXCPU, exec_logout);
#endif
#ifdef SIGXFSZ
    signal(SIGXFSZ, exec_logout);
#endif
}
