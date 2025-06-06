/* sklaffacct.c */

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
#include <sys/wait.h>
#include <pwd.h>
#include <fcntl.h>

int
main(int argc, char *argv[])
{
    LINE name, login, passwd, inet, tele, cmdline, fname;
    HUGE_LINE outbuf;
    int fd;
    char *buf;
    struct passwd *pw;

    tty_raw();

    if ((fd = open_file(ACCT_FILE, OPEN_QUIET)) == -1) {
        goto cont;
    }
    if ((buf = read_file(fd)) == NULL) {
        goto cont;
    }
    close_file(fd);
    output("\n%s\n", buf);
    free(buf);
cont:
    output(MSG_INNAME);
    input("", name, LINE_LEN, 0, 0, 0);
errlogin:
    output(MSG_INLOGIN);
    input("", login, 11, 0, 0, 0);
    pw = getpwnam(login);
    if (pw != NULL) {
        output("\n%s\n\n", MSG_UIDINUSE);
        goto errlogin;
    }
    output(MSG_INPASSWD);
    input("", passwd, 13, 0, 0, 0);
    output(MSG_INMODEM);
    input(MSG_YES, inet, 7, 0, 0, 0);
    output(MSG_INTELE);
    input("", tele, 70, 0, 0, 0);
    output("\n");
    strcpy(outbuf, "\n");
    strcpy(outbuf, "------------------------------------------------\n");
    strcat(outbuf, MSG_ACCAPP);
    time_string(time(0), cmdline, 1);
    strcat(outbuf, cmdline);
    strcat(outbuf, "\n------------------------------------------------\n");
    strcat(outbuf, MSG_INSNAME);
    strcat(outbuf, name);
    strcat(outbuf, MSG_INSLOGIN);
    strcat(outbuf, login);
    strcat(outbuf, MSG_INSPASSWD);
    strcat(outbuf, passwd);
    strcat(outbuf, MSG_INSMODEM);
    strcat(outbuf, inet);
    strcat(outbuf, MSG_INSTELE);
    strcat(outbuf, tele);
    strcat(outbuf, "\n");
    sprintf(fname, "/tmp/%d", getpid());
    fd = open_file(fname, OPEN_QUIET | OPEN_CREATE);
    write(fd, outbuf, strlen(outbuf));
    close_file(fd);
    if (fork()) {
        (void) wait(&fd);
    } else {
        close(0);
        close(1);
        close(2);
        (void) open(fname, O_RDONLY);
        (void) open("/dev/null", O_WRONLY);
        (void) dup(1);
        execl(MAILPRGM, MAILPRGM, SKLAFF_ACCT, (char *) 0);
    }
    unlink(fname);
    fd = open_file(ACCT_LOG, OPEN_QUIET | OPEN_CREATE);
    lseek(fd, 0L, 2);
    write(fd, outbuf, strlen(outbuf));
    close_file(fd);
    output(MSG_APPLIED);
    tty_reset();
    exit(0);
}
