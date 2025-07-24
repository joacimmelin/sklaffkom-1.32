/* newstoss.c */

/*
 *   SklaffKOM, a simple conference system for UNIX.
 *
 *   Copyright (C) 1994  Carl Sundbom
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

#include "sklaff.h"
#include "ext_globals.h"
#include <pwd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>

#include "globals.h"

struct REFLIST {
    long num;
    LONG_LINE id;
    struct REFLIST *next;
} *rf, *top;

long first_news(char *, long);
long send_news(int, char *, int, int, long);
long find_ref(char *);
int add_text();

int
main(int argc, char *argv[])
{
    LONG_LINE newslib, article, ng;
    LINE refer;
    struct passwd *pw;
    struct stat st;
    char *ptr, *ptr2, *buf, *oldbuf, saved, *buf2, *oldbuf2, *ptr3;
    int fd, confid, flag, fd2;
    long last, first, run, newid, com, first2;
    time_t mtime;
    struct CONF_ENTRY ce;

    if (argc != 2) {
        printf("\n%s\n\n", MSG_NTINFO);
        exit(1);
    }
    if ((fd = open_file(CONF_FILE, 0)) == -1)
        exit(1);
    if ((buf = read_file(fd)) == NULL)
        exit(1);
    oldbuf = buf;
    if (close_file(fd) == -1)
        exit(1);
    buf = get_conf_entry(buf, &ce);

    while ((strcmp(argv[1], ce.name) != 0) && buf)
        buf = get_conf_entry(buf, &ce);
    free(oldbuf);
    if (strcmp(argv[1], ce.name) == 0)
        confid = ce.num;
    else
        exit(0);

    if ((fd = open(NEWS_GROUPS, O_RDONLY, 0)) == -1) {
        printf("\n%s\n\n", MSG_BADACTIVE);
        exit(1);
    }
    if ((buf = read_file(fd)) == NULL)
        exit(1);
    if (close(fd) == -1)
        exit(1);

    strcpy(ng, argv[1]);
    strcat(ng, " ");
    ptr = strstr(buf, ng);
    if (!ptr) {
        printf("\n%s\n\n", MSG_NOGROUP);
        exit(1);
    }
    ptr2 = strchr(ptr, ' ');
    if (ptr2)
        last = atol(ptr2 + 1);
    else {
        printf("\n%s\n\n", MSG_BADACTIVE);
        exit(1);
    }
    free(buf);

    pw = getpwnam(SKLAFF_ACCT);
    if (pw == NULL)
        exit(1);

    sprintf(newslib, "%s/%s", NEWS_SPOOL, argv[1]);
    for (;;) {
        ptr = strchr(newslib, '.');
        if (ptr)
            *ptr = '/';
        else
            break;
    }

    rf = NULL;
    top = NULL;
    flag = 0;
    mtime = 0;
    first = first_news(newslib, last);
    while ((first <= last) && first) {
        sprintf(article, "%s/%ld", newslib, first);
        if ((fd = open(article, O_RDONLY, 0)) != -1) {
            if (!flag) {
                flag = 1;
                sprintf(article, "%s/%d/%ld", SKLAFF_DB, ce.num, ce.last_text);
                if (stat(article, &st) != -1)
                    mtime = st.st_mtime;
                first2 = first_text(ce.num, 0);
                run = first2;
                while (run <= ce.last_text) {
                    sprintf(article, "%s/%d/%ld", SKLAFF_DB, ce.num, run);
                    if ((fd2 = open(article, O_RDONLY, 0)) != -1) {
                        if ((buf2 = read_file(fd2)) == NULL)
                            exit(1);
                        oldbuf2 = buf2;
                        if (close(fd2) == -1)
                            exit(1);
                        ptr = strstr(buf2, MSG_MSGID);
                        if (ptr) {
                            ptr = strchr(ptr, '<');
                            if (ptr)
                                ptr2 = strchr(ptr, '>');
                            if (ptr && ptr2 && (ptr2 > ptr)) {
                                add_text();
                                *ptr2 = '\0';
                                strncpy(rf->id, (ptr + 1), LONG_LINE_LEN - 2);
                                rf->id[LONG_LINE_LEN - 1] = 0;
                                rf->num = run;
                            }
                        }
                        free(oldbuf2);
                    }
                    run++;
                }
            }
            fstat(fd, &st);
            if (st.st_mtime < mtime) {
                close(fd);
            } else {
                if ((buf = read_file(fd)) == NULL)
                    exit(1);
                oldbuf = buf;
                if (close(fd) == -1)
                    exit(1);
                strcpy(refer, "");
                ptr = strstr(buf, MSG_REFID);
                if (ptr) {
                    ptr2 = strchr(ptr, '\n');
                    while (ptr2 > ptr) {
                        ptr2--;
                        if (*ptr2 == '>')
                            break;
                    }
                    if (ptr2 != ptr) {
                        ptr3 = ptr;
                        ptr = ptr2;
                        while (ptr > ptr3) {
                            ptr--;
                            if (*ptr == '<')
                                break;
                        }
                    }
                    if (ptr && ptr2 && (ptr2 > ptr)) {
                        saved = *ptr2;
                        *ptr2 = '\0';
                        strncpy(refer, (ptr + 1), LINE_LEN - 2);
                        refer[LINE_LEN - 1] = 0;
                        *ptr2 = saved;
                    }
                }
                com = find_ref(refer);
                newid = send_news(confid, buf, pw->pw_uid, pw->pw_gid, com);
                ptr = strstr(buf, MSG_MSGID);
                if (ptr && (newid != -1)) {
                    ptr = strchr(ptr, '<');
                    if (ptr)
                        ptr2 = strchr(ptr, '>');
                    if (ptr && ptr2 && (ptr2 > ptr)) {
                        add_text();
                        saved = *ptr2;
                        *ptr2 = '\0';
                        strncpy(rf->id, (ptr + 1), LONG_LINE_LEN - 2);
                        rf->id[LONG_LINE_LEN - 1] = 0;
                        *ptr2 = saved;
                        rf->num = newid;
                    }
                }
                free(oldbuf);
            }
        }
        first++;
    }
    rf = top;
    while (rf) {
        top = rf;
        rf = rf->next;
        free(top);
    }
/*    if (first) notify_all_processes(SIGNAL_NEW_TEXT); */
    exit(0);
}

long
send_news(int confid, char *mbuf, int ouid, int ogrp, long com)
{
    LINE textfile, cname, newline;
    struct CONF_ENTRY ce;
    struct TEXT_HEADER th;
    int fd, fdo;
    long i;
    char *buf, *oldbuf, *nbuf, *ptr, *tmp, *fbuf;

    if ((fd = open_file(CONF_FILE, 0)) == -1)
        return -1;
    if ((buf = read_file(fd)) == NULL)
        return -1;
    oldbuf = buf;
    while ((buf = get_conf_entry(buf, &ce)))
        if (ce.num == confid)
            break;
    if (ce.num == confid) {
        ce.last_text++;
        nbuf = replace_conf(&ce, oldbuf);
        if (!nbuf) {
            printf("\n%s\n\n", MSG_CONFMISSING);
            return -1;
        }
    } else {
        printf("\n%s\n\n", MSG_CONFMISSING);
        return -1;
    }

    sprintf(textfile, "%s/%d/%ld", SKLAFF_DB, confid, ce.last_text);
    if ((fdo = open_file(textfile, OPEN_QUIET | OPEN_CREATE)) == -1) {
        printf("\n%s\n\n", MSG_ERRCREATET);
        return -1;
    }
    /* swedish character conversion */

    ptr = mbuf;
    while (*ptr) {
        if ((unsigned char) *ptr == 134)
            *ptr = '}';
        else if ((unsigned char) *ptr == 132)
            *ptr = '{';
        else if ((unsigned char) *ptr == 148)
            *ptr = '|';
        else if ((unsigned char) *ptr == 143)
            *ptr = ']';
        else if ((unsigned char) *ptr == 142)
            *ptr = '[';
        else if ((unsigned char) *ptr == 153)
            *ptr = 0x05c;
        if ((unsigned char) *ptr == 229)
            *ptr = '}';
        else if ((unsigned char) *ptr == 228)
            *ptr = '{';
        else if ((unsigned char) *ptr == 246)
            *ptr = '|';
        else if ((unsigned char) *ptr == 197)
            *ptr = ']';
        else if ((unsigned char) *ptr == 196)
            *ptr = '[';
        else if ((unsigned char) *ptr == 214)
            *ptr = 0x05c;
        if ((unsigned char) *ptr == 140)
            *ptr = '}';
        else if ((unsigned char) *ptr == 138)
            *ptr = '{';
        else if ((unsigned char) *ptr == 154)
            *ptr = '|';
        else if ((unsigned char) *ptr == 129)
            *ptr = ']';
        else if ((unsigned char) *ptr == 128)
            *ptr = '[';
        else if ((unsigned char) *ptr == 133)
            *ptr = 0x05c;
        ptr++;
    }

    fbuf = (char *) malloc(strlen(mbuf) + LONG_LINE_LEN);
    if (fbuf == NULL) {
        sys_error("send_news", 1, "malloc");
        return -1;
    }
    ptr = strstr(mbuf, MSG_EMSUB);
    if (ptr) {
        ptr = ptr + strlen(MSG_EMSUB);
        tmp = strchr(ptr, '\n');
        *tmp = '\0';
        strncpy(th.subject, ptr, SUBJECT_LEN - 2);
        th.subject[SUBJECT_LEN - 1] = 0;
        *tmp = '\n';
    } else
        strcpy(th.subject, "");

    th.size = 0;
    ptr = mbuf;
    while (1) {
        ptr = strchr(ptr, '\n');
        if (ptr) {
            th.size++;
            ptr++;
        } else
            break;
    }

    th.time = time(0);

    memset(fbuf, 0, strlen(mbuf) + LONG_LINE_LEN);
    sprintf(fbuf, "%ld:%d:%ld:%ld:%d:%d:%d\n",
    (long)ce.last_text, 0,
    (long)th.time, (long)com, 0,
    0, th.size);

    strcat(fbuf, th.subject);
    strcat(fbuf, "\n");
    strcat(fbuf, mbuf);

    if (write_file(fdo, fbuf) == -1)
        return -1;
    if (close_file(fdo) == -1)
        return -1;

    chown(textfile, ouid, ogrp);

    if (write_file(fd, nbuf) == -1)
        return -1;
    if (close_file(fd) == -1)
        return -1;

    /* comment linking */

    if (com) {
        sprintf(cname, "%s/%d/%ld", SKLAFF_DB, confid, com);
        if ((fd = open_file(cname, OPEN_QUIET)) == -1) {
            output("\n%s\n\n", MSG_NOTEXT);
            return 0;
        }
        if ((buf = read_file(fd)) == NULL) {
            output("\n%s\n\n", MSG_NOREAD);
            return 0;
        }
        i = strlen(buf) + LONG_LINE_LEN;
        nbuf = (char *) malloc(i);
        if (!nbuf) {
            return -1;
        }
        memset(nbuf, 0, i);

        sprintf(newline, "%ld:%d\n", ce.last_text, 0);
        strcpy(nbuf, buf);
        strcat(nbuf, newline);
        free(buf);

        critical();
        if (write_file(fd, nbuf) == -1) {
            output("\n%s\n\n", MSG_NOREPPTR);
            return 0;
        }
        if (close_file(fd) == -1) {
            return 0;
        }
        non_critical();
    }
    return ce.last_text;
}


long
first_news(char *newslib, long last)
{
    LINE fname;
    long first, ptr;

    first = 1L;
    ptr = 1L;
    while (last > first) {
        ptr = (first + last) / 2L;
        sprintf(fname, "%s/%ld", newslib, ptr);
        if (file_exists(fname) == -1) {
            sprintf(fname, "%s/%ld", newslib, (ptr - 1L));
            if (file_exists(fname) == -1) {
                sprintf(fname, "%s/%ld", newslib, (ptr - 2L));
                if (file_exists(fname) == -1) {
                    sprintf(fname, "%s/%ld", newslib, (ptr - 3L));
                    if (file_exists(fname) == -1) {
                        first = (ptr + 1L);
                        ptr = first;
                    } else {
                        last = (ptr - 3L);
                        ptr = last;
                    }
                } else {
                    last = (ptr - 2L);
                    ptr = last;
                }
            } else {
                last = (ptr - 1L);
                ptr = last;
            }
        } else {
            last = ptr;
        }
    }
    return ptr;
}


int
add_text(void)
{
    if (top) {
        rf->next = (struct REFLIST *) malloc
        (sizeof(struct REFLIST) + 1);
        if (rf->next == NULL) {
            return -1;
        }
        rf = rf->next;
        rf->next = NULL;
    } else {
        rf = (struct REFLIST *) malloc
            (sizeof(struct REFLIST) + 1);
        if (rf == NULL) {
            return -1;
        }
        top = rf;
        rf->next = NULL;
    }
    return 0;
}


long
find_ref(char *refer)
{
    struct REFLIST *look;

    if (!strlen(refer))
        return 0;
    look = top;
    while (look) {
        if (!strcmp(look->id, refer))
            return look->num;
        look = look->next;
    }
    return 0;
}
