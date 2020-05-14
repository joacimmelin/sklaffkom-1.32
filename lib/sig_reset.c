/* sig_reset.c */

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
#include <signal.h>

/* 
 * sig_reset - reset signals setup by sig_setup
 */

void sig_reset()
{
    signal(SIGHUP, SIG_DFL);
    signal(SIGPIPE, SIG_DFL);
    signal(SIGTERM, SIG_DFL);
    signal(SIGINT, SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
    signal(SIGTRAP, SIG_DFL);
    signal(SIGALRM, SIG_DFL);
    signal(SIGCHLD, SIG_DFL);
    signal(SIGTTIN, SIG_DFL);
    signal(SIGTTOU, SIG_DFL);
    signal(SIGWINCH, SIG_DFL);
    signal(SIGNAL_NEW_TEXT, SIG_DFL);
    signal(SIGNAL_NEW_MSG, SIG_DFL);
    signal(SIGCONT, SIG_DFL);
#ifdef SIGXCPU
    signal(SIGXCPU, SIG_DFL);
#endif
#ifdef SIGXFSZ
    signal(SIGXFSZ, SIG_DFL);
#endif
}
