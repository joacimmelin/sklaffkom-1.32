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
#include <stdlib.h>
#if defined(SOLARIS) || defined(FREEBSD)
#include <utmpx.h>
#else
#include <utmp.h>
#endif
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>

/*
 * get_hostname - gets hostname for tty or tty if local
 * ret: pointer to hostname string or NULL
 */

#ifdef FREEBSD

#define KOM_MAX_HOST 256

char *
get_hostname(void)
{
    static char myhost[KOM_MAX_HOST + 1];
    struct utmpx ul, *u;
    char *tty;
    struct hostent *h;

    strncpy(myhost, "N/A", sizeof(KOM_MAX_HOST));
    tty = ttyname(STDIN_FILENO);
    if (tty == NULL)
        return myhost;
    memset(&ul, 0, sizeof(ul));
    strlcpy(ul.ut_line, tty + strlen("/dev/"), sizeof(ul.ut_line));
    u = getutxline(&ul);
    if (u == NULL)
        return myhost;
    strncpy(myhost, u->ut_host, KOM_MAX_HOST);

    h = gethostbyname(myhost);
    if (h == NULL) {
        int family = AF_INET;
        socklen_t len = sizeof(struct sockaddr_in);
        unsigned char addr[sizeof(struct in6_addr)];

        if (inet_pton(family, myhost, addr) < 0) {
            family = AF_INET6;
            len = sizeof(struct sockaddr_in6);
            if (inet_pton(family, myhost, addr) < 0)
                return myhost;
        }
        h = gethostbyaddr(addr, len, family);
    }
    if (h == NULL)
        return myhost;
    strlcpy(myhost, h->h_name, sizeof(myhost));

    return myhost;
}

#else
char *
get_hostname(void)
{
#if defined(SOLARIS)
    struct utmpx ut;
#else
    struct utmp ut;
#endif
    int uf;
    static char my_host[256 + 1];
    char uhost[UT_HOSTSIZE + 1];
    LINE tmp;
    char *ptr;
    unsigned long l;
    struct hostent *hp;

    ptr = ttyname(0);
    if (!ptr) {
        strcpy(my_host, "N/A");
        return my_host;
    }
    strlcpy(tmp, ptr, sizeof(my_host));
    ptr = strchr(tmp + 1, '/');
    ptr++;
    uf = open(UTMP_REC, O_RDONLY);
    while (read(uf, &ut, sizeof(ut)) == sizeof(ut)) {
        if (!strncmp(ptr, ut.ut_line, UT_LINESIZE)) {
            if (strstr(ptr, "pt") || strstr(ptr, "yp")) {
                strncpy(uhost, ut.ut_host, UT_HOSTSIZE);
                uhost[UT_HOSTSIZE] = 0;
                l = inet_addr(uhost);
                hp = gethostbyaddr((char *) &l, sizeof(l), AF_INET);
                if (hp)
                    strncpy(my_host, hp->h_name, sizeof(my_host));
                else
                    strncpy(my_host, uhost, sizeof(my_host));
                my_host[256] = 0;
            } else {
                strncpy(my_host, ut.ut_line, UT_LINESIZE);
                my_host[UT_LINESIZE] = 0;
            }
            close(uf);
            return my_host;
        }
    }
    close(uf);
    strcpy(my_host, ptr);
    return my_host;
}

#endif
