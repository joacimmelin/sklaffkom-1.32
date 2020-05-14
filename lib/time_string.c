/* time_string.c */

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

/*
 * time_string - converts time to string
 * args: time (in_time), timestring (out_time), absolute date (show_date)
 * ret: pointer to timestring
 */

static char weekdays[7][10] = {MSG_SUNDAY, MSG_MONDAY, MSG_TUESDAY,
    MSG_WEDNESDAY, MSG_THURSDAY, MSG_FRIDAY,
MSG_SATURDAY};

static char months[12][4] = {MSG_JAN, MSG_FEB, MSG_MAR, MSG_APR, MSG_MAY,
    MSG_JUN, MSG_JUL, MSG_AUG, MSG_SEP, MSG_OCT,
MSG_NOV, MSG_DEC};

static char closedays[2][10] = {MSG_TODAY, MSG_YESTERDAY};

char *
time_string(in_time, out_time, show_date)
    time_t in_time;
    char *out_time;
    int show_date;
{
    struct tm ts, ts_now;
    time_t now;
    int out_day, comp_year;
    char day_string[9];

    memcpy(&ts, localtime(&in_time), sizeof(struct tm));
    if (show_date) {
        sprintf(out_time, "%d %s %d %.2d:%.2d", ts.tm_mday,
            months[ts.tm_mon], 1900 + ts.tm_year, ts.tm_hour, ts.tm_min);
    } else {
        time(&now);
        memcpy(&ts_now, localtime(&now), sizeof(struct tm));
        out_day = ts_now.tm_yday - ts.tm_yday;
        comp_year = 1900 + ts_now.tm_year;
        if (out_day < 0) {
            if (ts.tm_year % 4)
                out_day += 365;
            else
                out_day += 366;
            comp_year--;
        }
        if ((out_day < 7) && (comp_year == 1900 + ts.tm_year)) {
            if (out_day < 2)
                strcpy(day_string, closedays[out_day]);
            else
                strcpy(day_string, weekdays[ts.tm_wday]);
            sprintf(out_time, "%s %.2d:%.2d", day_string, ts.tm_hour, ts.tm_min);
        } else
            sprintf(out_time, "%d %s %d %.2d:%.2d", ts.tm_mday,
                months[ts.tm_mon], 1900 + ts.tm_year, ts.tm_hour,
                ts.tm_min);
    }
    return out_time;
}
