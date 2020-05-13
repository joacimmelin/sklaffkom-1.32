/* file.c */

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
#include <fcntl.h>
#include <sys/wait.h>

/*
 * rebuild_index_file - rebuild file index in current conference
 * ret: ok (0) or error (-1)
 */

int rebuild_index_file()
{
    LINE cwd, currfile, filed;
    LONG_LINE outrec, fn, fn2, fn3;
    int fd, fd2, fd3, subba, remo;
    char *buf, *oldbuf, *buf2, *oldbuf2, *ptr;
    struct FILE_ENTRY fe;

    remo = 1;
    sprintf(fn, "%s/%d%s", FILE_DB, Current_conf, INDEX_FILE);
    sprintf(fn3, "%s/%d%s", FILE_DB, Current_conf, NEWINDEX_FILE);
    
    critical();
    
    if ((fd = open_file(fn, OPEN_CREATE)) == -1) {
	return -1;
    }

    if ((fd3 = open_file(fn3, OPEN_CREATE)) == -1) {
	return -1;
    }

#ifndef GETCWD
    getwd(cwd);
#else
    getcwd(cwd, LINE_LEN);
#endif    

    sprintf(filed, "%s/%d", FILE_DB, Current_conf);
    chdir(filed);
    sprintf(fn2, "/tmp/%d", getpid());
    if (fork()) {
	(void)wait(&subba);
    }
    else {
	close(0);
	close(1);
	close(2);
	(void)open("/dev/null", O_RDONLY);
	(void)open(fn2, O_WRONLY | O_CREAT, 0777);
	(void)open("/dev/null", O_WRONLY);
	execl(LSPRGM, LSPRGM, LSOPT, (char *)0);
    }

    chdir(cwd);
    
    if ((oldbuf = read_file(fd)) == NULL) {
	return -1;
    }

    if ((fd2 = open_file(fn2, 0)) == -1) {
	return -1;
    }

    if ((buf2 = read_file(fd2)) == NULL) {
	return -1;
    }

    oldbuf2 = buf2;

    while (*buf2) {
	ptr = strchr(buf2, '\n');
	if (ptr) {
	    remo = 0;
	    *ptr = '\0';
	    strcpy(currfile, buf2);
	    *ptr = '\n';
	    buf2 = ptr + 1;
	    buf = oldbuf;
	    while (buf) {
		buf = get_file_entry(buf, &fe);
		if (buf) {
		    if (!strcmp(currfile, fe.name)) {
			sprintf(outrec, "%s:%s\n", currfile, fe.desc);
			write(fd3, outrec, strlen(outrec));
			break;
		    }
		}
	    }
	    if (!buf) {
		sprintf(outrec, "%s:%s\n", currfile, "");
		write(fd3, outrec, strlen(outrec));
	    }
	}
	else {
	    *buf2 = '\0';
	}
    }
    
    close_file(fd3);
    close_file(fd2);
    close_file(fd);

    free(oldbuf);
    free(oldbuf2);

    copy_file(fn3, fn);
    unlink(fn3);
    
    unlink(fn2);
    if (remo) unlink(fn);
    non_critical();

    return 0;
}
