/* hostname.c */

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
#ifdef SOLARIS
#include <utmpx.h>
#else
#include <utmp.h>
#endif
#include <fcntl.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>

/*
 * get_hostname - gets hostname for tty or tty if local
 * ret: pointer to hostname string or NULL
 */

char	*get_hostname()
{
#ifdef SOLARIS
    struct utmpx ut;
#else
    struct utmp ut;
#endif
    int	uf;
    static char my_host[150];
    char uhost[UT_HOSTSIZE+1];
    LINE tmp;
    char *ptr;
    unsigned long l;
    struct hostent *hp;

    ptr = (char *)ttyname(0);
    if (!ptr) {
	strcpy(my_host, "N/A");
	return my_host;
    }
    strcpy(tmp, ptr);
    ptr = strchr(tmp + 1, '/');
    ptr++;
    uf = open(UTMP_REC, O_RDONLY);
    while (read(uf, &ut, sizeof(ut)) == sizeof(ut)) {
      if (!strncmp(ptr, ut.ut_line, 8)) {
	if (strstr(ptr, "pt") || strstr(ptr, "yp")) {
	  strncpy(uhost, ut.ut_host, UT_HOSTSIZE);
	  uhost[UT_HOSTSIZE]=0;
	  l = inet_addr(uhost);
	  hp = gethostbyaddr((char *) &l, sizeof(l), AF_INET);
	  if (hp)
	    strncpy(my_host, hp->h_name, 149);
	  else
	    strncpy(my_host, uhost, 149);
	} else {
	  strncpy(my_host, ut.ut_line, UT_LINESIZE);
	  my_host[UT_LINESIZE]=0;
	}
	close(uf);
	return my_host;
      }
    }
    close(uf);
    strcpy(my_host, ptr);
    return my_host;
}


