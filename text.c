/* text.c */

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

#include "sklaff.h"
#include "ext_globals.h"
#include <signal.h>
#include <ctype.h>
#include <strings.h>

/*
 * extract_display_name - helper for humanizing header (real name in usenet posts)
 * PL 2025-08-09
 *       
 */
static void
extract_display_name(const char *from, char *out, size_t outlen)
{
        
    if (!from || !*from) { out[0] = '\0'; return; }


    char buf[256];
    snprintf(buf, sizeof(buf), "%s", from);

    /* Trim leading/trailing spaces */
    char *s = buf;
    while (*s && (*s==' ' || *s=='\t')) s++;
    char *e = s + strlen(s);
    while (e > s && (e[-1]==' ' || e[-1]=='\t' || e[-1]=='\r' || e[-1]=='\n')) --e;
    *e = '\0';

    /* Case 1: "Name" <email>  => prefer Name (strip quotes) */
    char *lt = strchr(s, '<');
    if (lt) {
        /* take the part before '<' */
        while (lt > s && (lt[-1]==' ' || lt[-1]=='\t')) --lt;
        *lt = '\0';
        /* strip optional surrounding quotes */
        if (*s=='"' && e> s+1 && e[-1]=='"') { s++; e--; *e='\0'; }
        /* if non-empty, use it */
        if (*s) { snprintf(out, outlen, "%s", s); return; }
        /* else fall back to content inside <...> */
        char *gt = strchr(lt+1, '>');
        if (gt && gt > lt+1) {
            *gt = '\0';
            snprintf(out, outlen, "%s", lt+1);
            return;
        }
    }

    /* Case 2: email (Name) => prefer (Name) */
    char *lp = strchr(s, '(');
    if (lp) {
        char *rp = strchr(lp+1, ')');
        if (rp && rp > lp+1) {
            *rp = '\0';
            /* trim inner spaces/quotes */
            char *ns = lp+1;
            while (*ns==' '||*ns=='\t') ns++;
            char *ne = ns + strlen(ns);
            while (ne>ns && (ne[-1]==' '||ne[-1]=='\t'||ne[-1]=='"')) --ne;
            if (*ns=='"') ns++;
            *ne = '\0';
            if (*ns) { snprintf(out, outlen, "%s", ns); return; }
        }
    }

    /* Default: just use input as-is */
    snprintf(out, outlen, "%s", *s ? s : "");
}

/*
 * better blank line detection for usenet headers PL 2025.
 * sometimes sklaffkom stripped not only the usenet-header but the
 * body also, this helper prevents that
*/
int is_blank_line(const char *line) {
    if (!line) return 1;
    while (*line) {
        if (!isspace((unsigned char)*line))
            return 0;
        line++;
    }
    return 1;
}

/*
 * RFC 2047 + underline helpers. This code is partly AI-generated and I must
 * admit I don't understand it fully myself, but it has been tested thoroughly
 * and works. Purpose : display text humans can read in usenet subjects and
 * name of the author. PL 2025-08-09
*/
static size_t 
utf8_disp_len(const char *s) /* Count display chars (UTF-8 codepoints ≈ 1 col each; good enough for headers) */
{
    size_t n = 0;
    while (*s) {
        unsigned char c = (unsigned char)*s++;
        if ((c & 0xC0) != 0x80) /* count non-continuation bytes */
            n++;
    }
    return n;
}

static void utf8_trunc_cols(const char *in, size_t max_cols, char *out, size_t outlen) /* 2025-08-10, PL: truncate by display columns (UTF-8 safe, 1 col/codepoint) */
{
    size_t cols = 0, o = 0;
    if (!in || !out || outlen == 0) return;
    while (*in && o + 4 < outlen) {
        unsigned char c = (unsigned char)in[0];
        size_t clen;
        if ((c & 0x80) == 0x00) clen = 1;
        else if ((c & 0xE0) == 0xC0) clen = 2;
        else if ((c & 0xF0) == 0xE0) clen = 3;
        else if ((c & 0xF8) == 0xF0) clen = 4;
        else clen = 1; 

        if (cols + 1 > max_cols) break;
        if (o + clen >= outlen) break;

        
        for (size_t i = 0; i < clen && in[i]; i++) out[o++] = in[i];
        in += clen;
        cols++;
    }
    out[o] = '\0';
}

static void
print_underlined_line(const char *line) /* Build underline matching a printed line */
{
    LINE under;
    size_t i, w = utf8_disp_len(line);
    if (w >= sizeof(under)) w = sizeof(under) - 1;
    for (i = 0; i < w; i++) under[i] = '-';
    under[i] = '\0';
    output("%s\n", line);
    output("%s\n", under);
}

static int b64v(int c)  /* Base64 table */
{
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    return -1;
}

static size_t
qp_decode_bytes(const char *in, size_t inlen, unsigned char *out, size_t outlen)    /* Decode a single encoded-word's bytes with quoted-printable (Q) */
{
    size_t i = 0, o = 0;
    while (i < inlen && o < outlen) {
        char c = in[i++];
        if (c == '_') { out[o++] = ' '; continue; }
        if (c == '=' && i + 1 < inlen && isxdigit((unsigned char)in[i]) && isxdigit((unsigned char)in[i+1])) {
            int hi = isdigit((unsigned char)in[i]) ? in[i]-'0' : (tolower((unsigned char)in[i])-'a'+10);
            int lo = isdigit((unsigned char)in[i+1]) ? in[i+1]-'0' : (tolower((unsigned char)in[i+1])-'a'+10);
            unsigned char b = (unsigned char)((hi<<4) | lo);
            if (o < outlen) out[o++] = b;
            i += 2;
        } else {
            out[o++] = (unsigned char)c;
        }
    }
    return o;
}

static size_t
b64_decode_bytes(const char *in, size_t inlen, unsigned char *out, size_t outlen)   /* Decode a single encoded-word's bytes with base64 (B) */
{
    size_t i = 0, o = 0;
    while (i + 3 < inlen) {
        int a = b64v(in[i++]);
        int b = b64v(in[i++]);
        int c = (in[i] == '=') ? -1 : b64v(in[i]);
        i++;
        int d = (in[i] == '=') ? -1 : b64v(in[i]);
        i++;
        if (a < 0 || b < 0 || (c < -1) || (d < -1)) break;
        if (o < outlen) out[o++] = (unsigned char)((a<<2) | (b>>4));
        if (c >= 0 && o < outlen) out[o++] = (unsigned char)(((b&0x0F)<<4) | (c>>2));
        if (d >= 0 && o < outlen) out[o++] = (unsigned char)(((c&0x03)<<6) | d);
    }
    return o;
}

static size_t
latin1_to_utf8(const unsigned char *in, size_t inlen, char *out, size_t outlen) /* Minimal charset -> UTF-8: utf-8 (pass), us-ascii (pass), iso-8859-1 (map) */

{
    size_t o = 0;
    for (size_t i = 0; i < inlen; i++) {
        unsigned char c = in[i];
        if (c < 0x80) {
            if (o + 1 >= outlen) break;
            out[o++] = (char)c;
        } else {
            if (o + 2 >= outlen) break;
            out[o++] = (char)(0xC0 | (c >> 6));
            out[o++] = (char)(0x80 | (c & 0x3F));
        }
    }
    if (o < outlen) out[o] = '\0';
    return o;
}

static size_t
bytes_to_utf8(const char *charset, const unsigned char *in, size_t inlen, char *out, size_t outlen)
{
    if (!charset) charset = "us-ascii";
    /* lowercase compare */
    char cs[32]; size_t n = 0;
    while (charset[n] && n+1 < sizeof(cs)) { cs[n] = (char)tolower((unsigned char)charset[n]); n++; }
    cs[n] = '\0';

    if (!strcmp(cs, "utf-8") || !strcmp(cs, "us-ascii")) {
        size_t copy = (inlen >= outlen-1) ? (outlen-1) : inlen;
        memcpy(out, in, copy);
        out[copy] = '\0';
        return copy;
    }
    if (!strcmp(cs, "iso-8859-1") || !strcmp(cs, "latin1") || !strcmp(cs, "iso8859-1")) {
        return latin1_to_utf8(in, inlen, out, outlen);
    }

    /* Fallback: best-effort raw copy (won't crash; shows something) */
    size_t copy = (inlen >= outlen-1) ? (outlen-1) : inlen;
    memcpy(out, in, copy);
    out[copy] = '\0';
    return copy;
}


static void
rfc2047_decode(const char *in, char *out, size_t outlen)    /* RFC 2047 decoder: decodes any number of encoded-words in a header field */
{
    const char *p = in;
    size_t o = 0;
    if (!in || !*in) { if (outlen) out[0] = '\0'; return; }

    while (*p && o + 1 < outlen) {
        const char *start = strstr(p, "=?");
        if (!start) {
            /* copy the rest */
            size_t rem = strlen(p);
            if (rem >= outlen - 1 - o) rem = outlen - 1 - o;
            memcpy(out + o, p, rem);
            o += rem;
            break;
        }
        /* copy literal up to start */
        size_t lit = (size_t)(start - p);
        if (lit) {
            size_t rem = (lit >= outlen - 1 - o) ? (outlen - 1 - o) : lit;
            memcpy(out + o, p, rem);
            o += rem;
        }

        /* parse =?charset?enc?text?= */
        const char *q1 = strchr(start + 2, '?'); if (!q1) { p = start + 2; continue; }
        const char *q2 = strchr(q1 + 1, '?');    if (!q2) { p = q1 + 1; continue; }
        const char *q3 = strstr(q2 + 1, "?=");   if (!q3) { p = q2 + 1; continue; }

        char charset[32];
        size_t cslen = (size_t)(q1 - (start + 2));
        if (cslen >= sizeof(charset)) cslen = sizeof(charset) - 1;
        memcpy(charset, start + 2, cslen);
        charset[cslen] = '\0';

        char enc = (char)toupper((unsigned char)q1[1]);

        /* raw decoded bytes */
        unsigned char bytes[512];
        size_t blen = 0;

        const char *payload = q2 + 1;
        size_t plen = (size_t)(q3 - payload);

        if (enc == 'B') {
            blen = b64_decode_bytes(payload, plen, bytes, sizeof(bytes));
        } else if (enc == 'Q') {
            blen = qp_decode_bytes(payload, plen, bytes, sizeof(bytes));
        } else {
            /* unknown encoding, copy raw */
            blen = (plen > sizeof(bytes)) ? sizeof(bytes) : plen;
            memcpy(bytes, payload, blen);
        }

        /* convert to UTF-8 (minimal supported charsets) */
        o += bytes_to_utf8(charset, bytes, blen, out + o, (outlen - o));

        /* advance past ?= and any single space between adjacent encoded-words */
        p = q3 + 2;
        while (*p == ' ' || *p == '\t') {
            const char *peek = p;
            while (*peek == ' ' || *peek == '\t') peek++;
            if (peek[0] == '=' && peek[1] == '?') p = peek; /* glue encoded-words */
            break;
        }
    }
    if (o < outlen) out[o] = '\0';
}


static void
normalize_label(const char *raw, char *norm, size_t nlen)   /* Normalize a label to ensure exactly one trailing ": " */
{
    size_t L = raw ? strlen(raw) : 0;
    int ends_with_colon = (L > 0 && raw[L-1] == ':');
    snprintf(norm, nlen, "%s%s", raw ? raw : "", ends_with_colon ? " " : ": ");
}



/*
 * display_header - displays textheader
 * args: pointer to TEXT_HEADER (th), allow editing of subject (edit_subject),
 *       conf/uid (type), absolute date? (dtype)
 */

void
display_header(struct TEXT_HEADER * th, int edit_subject, int type, int dtype, char *mailrec)
{
    LINE time_val, username, confname; /* + confname for humanized header */
    char fname[128];  /* increased from LINE to avoid overflow, modified on 2025-07-12, PL */
    int uid, right, nc, fd;
    char *tmp, *buf, *oldbuf;
    char *ptr = NULL;   /* modified on 2025-07-12, PL */
    struct CONF_ENTRY *ce = NULL;

    if (mailrec && type && (th->author == 0)) {
        strcpy(username, mailrec);
    } else {
        user_name(th->author, username);
        Current_author = th->author;
    }
  /* Humanized headers PL 2025-08-09 */
{
    LINE from_dec, disp;
    rfc2047_decode(username, from_dec, sizeof(from_dec));
    extract_display_name(from_dec, disp, sizeof(disp));
    snprintf(username, sizeof(username), "%.*s", (int)sizeof(username)-1, disp);
}
/* 2025-08-10, PL: strip surrounding quotes from display name */
{
    size_t L__ = strlen(username);
    if (L__ >= 2 && username[0] == '"' && username[L__-1] == '"') {
        username[L__-1] = '\0';
        memmove(username, username + 1, L__ - 1);
    }
}
    if (th->num == 0) {
        output("%s %s\n", MSG_WRITTENBY, username);
    } else {
/* 2025-08-09, PL: Human-first layout */
time_string(th->time, time_val, (dtype | Date));

if (Current_conf != 0) {
    /* In a conference: "Text N i <conf> <date>" */
    conf_name(Current_conf, confname);
    output_ansi_fmt("%s " GREEN "%d" DOT " %s " YELLOW "%s " DOT "%s\n", "%s %d %s %s %s\n",
        (th->type == TYPE_TEXT) ? MSG_TEXTNAME : MSG_SURVEYNAME,
        th->num, MSG_IN, confname, time_val);

    /* Then: "Skriven av <name>" on its own line */
    output_ansi_fmt("%s " YELLOW "%s\n" DOT, "%s %s\n", MSG_WRITTENBY, username);
} else {
    /* In mailbox: keep old style */
    output("%s %d %s %s %s\n",
        (th->type == TYPE_TEXT) ? MSG_TEXTNAME : MSG_SURVEYNAME,
        th->num, MSG_WRITTENBY, username, time_val);
}
    }
    switch (th->size) {
    case 0:
        if (th->num)
            output(" %s\n",
                (th->type == TYPE_TEXT) ? MSG_EMPTYTEXT : MSG_EMPTYSURVEY);
        break;
    case 1:
        //output(" %s\n", MSG_ONELINE);
        break;
    default:
	//output(" %d %s\n", th->size, MSG_LINES);
        break;
    }
    if (th->type == TYPE_SURVEY && (th->num != 0)) {
	time_string(th->sh.time, time_val, (dtype | Date));
        output("%s: %d; %s: %s\n", MSG_NQUESTIONS, th->sh.n_questions,
            MSG_REPORTRESULT, time_val);
    }
    if (th->comment_num) {
        if (th->comment_conf) {
            ce = get_conf_struct(th->comment_conf);
            right = can_see_conf(Uid, th->comment_conf, ce->type, ce->creator);
        } else {
            right = 1;
        }
        if (right) {
            output_ansi_fmt("%s " GREEN "%d " DOT, "%s %d ", MSG_REPLYTO, th->comment_num);
	    nc = th->comment_conf;
            if (!nc)
                nc = Current_conf;
            /* I put this chunk last instead to allow for display of author
             * also for text commented from other conferences. /OR 98-07-29 if
             * (th->comment_conf) { if (!nc) nc = Current_conf; conf_name(nc,
             * username); output ("%s %s\n", MSG_IN, username); } else */
            {
                if (!th->comment_author) {
                    strcpy(username, MSG_UNKNOWNU);
                    if (nc) {
                        sprintf(fname, "%s/%d/%ld", SKLAFF_DB,
                            nc, th->comment_num);
                    } else {
                        snprintf(fname, sizeof(fname), "%s/%ld", Mbox, th->comment_num);  /* modified on 2025-07-12, PL */
                    }
                    if ((fd = open_file(fname, OPEN_QUIET)) != -1) {
                        if ((buf = read_file(fd)) == NULL) {
                            output("\n%s\n\n", MSG_NOREAD);
                            return;
                        }
                        oldbuf = buf;
                        if (close_file(fd) == -1) {
                            return;
                        }
                        ptr = strstr(buf, MSG_EMFROM);
                        if (ptr) {
                            tmp = strchr(ptr, '\n');
                            *tmp = '\0';
                            strcpy(username, (ptr + strlen(MSG_EMFROM)));
                            *tmp = '\n';
                        }
                        free(oldbuf);
                    }
                } else {
                    user_name(th->comment_author, username);
                }
		/* 2025-08-09, PL: prefer human name (no email) on follow-up line */
		{
		    char disp[256];
		    extract_display_name(username, disp, sizeof(disp));
		    snprintf(username, sizeof(username), "%.*s", (int)sizeof(username)-1, disp);
		}
		
		/* 2025-08-10, PL: strip surrounding quotes from display name */
		{
		    size_t L__ = strlen(username);
		    if (L__ >= 2 && username[0] == '"' && username[L__-1] == '"') {
		        username[L__-1] = '\0';
		        memmove(username, username + 1, L__ - 1);
		    }
		}
output_ansi_fmt("%s " YELLOW "%s" DOT, "%s %s", MSG_BY, username); /* 2025-08-09, PL: print "av <name>" only once */
		if (th->comment_conf) {
                    conf_name(nc, username);
        	    sprintf(fname, "  showing MSG_IN");
                    debuglog(fname, 20);
                    output(" %s %s\n", MSG_IN, username);
                } else
                    output("\n");

            }
        }
    }
    if (!Current_conf && (th->author == Uid) &&
        (th->time > 0) && th->comment_author &&
        (th->comment_author != Uid) && (!Last_conf)) {
        user_name(th->comment_author, username);
        output("%s %s\n", MSG_COPYTO, username);
    }
    /* 2025-08-09, PL: Conference name is baked into line 1 now */

    if (Current_conf == 0) {
    /* Mailbox: keep "Mottagare:" logic unchanged */
    if (mailrec && !type) {
        output("%s %s\n", MSG_RECIPIENT, mailrec);
    } else {
        if (type < 0) {
            uid = -type;
            user_name(uid, username);
        } else {
            conf_name(type, username);
        }
        output("%s %s\n", MSG_RECIPIENT, username);
        }
    }
    //* decoded subject + trying to match underline everywhere WORK IN PROGRESS PL 2025-08-10*/
    if (edit_subject) {
    output(MSG_SUBJECT);
    input(th->subject, th->subject, SUBJECT_LEN, 0, 0, 0);
    } else {
    const char *raw_label = MSG_SUBJECT; /* 2025-08-10, PL: safe default to avoid NULL */
    LINE subj_dec, label_norm, subj_line;

    rfc2047_decode(th->subject, subj_dec, sizeof(subj_dec));
    normalize_label(raw_label, label_norm, sizeof(label_norm));

    snprintf(subj_line, sizeof(subj_line), "%s%s", label_norm, subj_dec);
    print_underlined_line(subj_line);  /* prints line + perfectly matching dashes (soon ;)) */
    }
}

/*
 * push_unread - push text on unread stack
 * args: conference (conf), text (num)
 * ret: ok (0) or failure (-1)
 */

int
push_unread(int conf, long num)
{
    struct UR_STACK *tmp, *saved;

    tmp = ustack;
    saved = NULL;

    while (tmp) {
        saved = tmp;
        tmp = tmp->next;
    }

    if ((tmp = (struct UR_STACK *)
            malloc(sizeof(struct UR_STACK))) == NULL) {
        sys_error("push_unread", 1, "malloc");
        return -1;
    }
    tmp->num = num;
    tmp->conf = conf;
    tmp->next = NULL;

    if (!saved)
        ustack = tmp;
    else
        saved->next = tmp;

    return 0;
}

/*
 * pop_unread - pops text from unread stack
 * args: pointer to conference variable (conf)
 * ret: updated conf and text number or -1 if no more texts
 */

long
pop_unread(int *conf)
{
    struct UR_STACK *tmp, *saved, *tmp2;
    long num;

    tmp = ustack;
    saved = NULL;

    while (tmp) {
        saved = tmp;
        tmp = tmp->next;
    }

    if (!saved)
        return -1;
    else {
        num = saved->num;
        *conf = saved->conf;
        if (ustack == saved)
            ustack = NULL;
        else {
            tmp = ustack;
            tmp2 = NULL;
            while (tmp != saved) {
                tmp2 = tmp;
                tmp = tmp->next;
            }
            tmp2->next = NULL;
        }
        free(saved);
        return num;
    }
}

/*
 * push_unread2 - push text on temporary unread stack
 * args: conference (conf), text (num)
 * ret: ok (0) or failure (-1)
 */

int
push_unread2(int conf, long num)
{
    struct UR_STACK *tmp, *saved;

    tmp = ustack2;
    saved = NULL;

    while (tmp) {
        saved = tmp;
        tmp = tmp->next;
    }

    if ((tmp = (struct UR_STACK *)
            malloc(sizeof(struct UR_STACK))) == NULL) {
        sys_error("push_unread", 1, "malloc");
        return -1;
    }
    tmp->num = num;
    tmp->conf = conf;
    tmp->next = NULL;

    if (!saved)
        ustack2 = tmp;
    else
        saved->next = tmp;

    return 0;
}

/*
 * pop_unread2 - pops text from temporary unread stack
 * args: pointer to conference variable (conf)
 * ret: updated conf and text number or -1 if no more texts
 */

long
pop_unread2(int *conf)
{
    struct UR_STACK *tmp, *saved, *tmp2;
    long num;

    tmp = ustack2;
    saved = NULL;

    while (tmp) {
        saved = tmp;
        tmp = tmp->next;
    }

    if (!saved)
        return -1;
    else {
        num = saved->num;
        *conf = saved->conf;
        if (ustack2 == saved)
            ustack2 = NULL;
        else {
            tmp = ustack2;
            tmp2 = NULL;
            while (tmp != saved) {
                tmp2 = tmp;
                tmp = tmp->next;
            }
            tmp2->next = NULL;
        }
        free(saved);
        return num;
    }
}

/*
 * push_comment - push comment on comment stack
 * args: text (num)
 * ret: ok (0) or failure (-1)
 */

int
push_comment(long num)
{
    struct COM_STACK *tmp, *saved;

    tmp = cstack;
    saved = NULL;

    while (tmp) {
        saved = tmp;
        tmp = tmp->next;
    }

    if ((tmp = (struct COM_STACK *)
            malloc(sizeof(struct COM_STACK))) == NULL) {
        sys_error("push_comment", 1, "malloc");
        return -1;
    }
    tmp->num = num;
    tmp->next = NULL;

    if (!saved)
        cstack = tmp;
    else
        saved->next = tmp;

    return 0;
}

/*
 * pop_comment - pops comment from comment stack
 * ret: textnumber or -1 if no more texts
 */

long
pop_comment(void)
{
    struct COM_STACK *tmp, *saved, *tmp2;
    long num;

    tmp = cstack;
    saved = NULL;

    while (tmp) {
        saved = tmp;
        tmp = tmp->next;
    }

    if (!saved)
        return -1;
    else {
        num = saved->num;
        if (cstack == saved)
            cstack = NULL;
        else {
            tmp = cstack;
            tmp2 = NULL;
            while (tmp != saved) {
                tmp2 = tmp;
                tmp = tmp->next;
            }
            tmp2->next = NULL;
        }
        free(saved);
        return num;
    }
}

/*
 * clear_comment - empty comment stack
 */

void
clear_comment(void)
{
    long textnum;

    textnum = pop_comment();
    while (textnum != -1) {
        textnum = pop_comment();
    }
}

/*
 * push_read - push text on read stack
 * args: conference (conf), text (num)
 * ret: ok (0) or failure (-1)
 */

int
push_read(int conf, long num)
{
    struct UR_STACK *tmp, *saved;

    tmp = rstack;
    saved = NULL;

    while (tmp) {
        saved = tmp;
        tmp = tmp->next;
    }

    if ((tmp = (struct UR_STACK *)
            malloc(sizeof(struct UR_STACK))) == NULL) {
        sys_error("push_unread", 1, "malloc");
        return -1;
    }
    tmp->num = num;
    tmp->conf = conf;
    tmp->next = NULL;

    if (!saved)
        rstack = tmp;
    else
        saved->next = tmp;

    return 0;
}

/*
 * pop_read - pops text from read stack
 * args: pointer to conference variable (conf)
 * ret: updated conf and text number or -1 if no more texts
 */

long
pop_read(int *conf)
{
    struct UR_STACK *tmp, *saved, *tmp2;
    long num;

    tmp = rstack;
    saved = NULL;

    while (tmp) {
        saved = tmp;
        tmp = tmp->next;
    }

    if (!saved)
        return -1;
    else {
        num = saved->num;
        *conf = saved->conf;
        if (rstack == saved)
            rstack = NULL;
        else {
            tmp = rstack;
            tmp2 = NULL;
            while (tmp != saved) {
                tmp2 = tmp;
                tmp = tmp->next;
            }
            tmp2->next = NULL;
        }
        free(saved);
        return num;
    }
}

/*
 * mark_as_read - mark text as read
 * args: text (text), conference (conf)
 * ret: ok (1) or already marked (0) or failure (-1)
 */

int
mark_as_read(long text, int conf)
{
    int bound, found, fd;
    struct CONFS_ENTRY cse;
    struct INT_LIST *int_list_next, *int_list_sav, *saved, *tmpsav;
    char *buf, *oldbuf, *nbuf;
    char fname[128];  /* increased from LINE to avoid overflow, modified on 2025-07-12, PL */
    
    strcpy(fname, Home);
    strcat(fname, CONFS_FILE);

    if ((fd = open_file(fname, 0)) == -1) {
        sys_error("mark_as_read", 1, "open_file");
        return -1;
    }
    if ((buf = read_file(fd)) == NULL) {
        sys_error("mark_as_read", 2, "read_file");
        return -1;
    }
    oldbuf = buf;

    /* find confs-entry */

    while (buf) {
        buf = get_confs_entry(buf, &cse);
        if (cse.num == conf)
            break;
        free_confs_entry(&cse);
    }

    buf = oldbuf;
    if (cse.num == conf) {
        int_list_sav = cse.il;
        found = 0;
        while (cse.il) {
            if ((cse.il->from <= text) && (cse.il->to >= text)) {
                found = 1;
                break;
            }
            cse.il = cse.il->next;
        }
        cse.il = int_list_sav;

        if (found) {
            free_confs_entry(&cse);
            if (close_file(fd) == -1) {
                sys_error("mark_as_read", 5, "close_file");
                return -1;
            }
            free(oldbuf);
            return 0;
        }
        bound = 0;

        /* kolla hur m}nga intervall som h{nger ihop med den aktuella texten
         * (text) */

        while (cse.il) {
            if ((cse.il->from == (text + 1L)) || (cse.il->to == (text - 1L))) {
                bound++;
            }
            cse.il = cse.il->next;
        }
        cse.il = int_list_sav;

        /* om inget intervall h{nger ihop s} allokerar vi ett nytt intervall */

        if (bound == 0) {
            if (cse.il) {
                if (cse.il->from > text) {
                    tmpsav = (struct INT_LIST *)
                        malloc(sizeof(struct INT_LIST));
                    if (tmpsav == NULL) {
                        sys_error("mark_as_read", 5, "malloc");
                        return -1;
                    }
                    tmpsav->next = cse.il;
                    tmpsav->from = text;
                    tmpsav->to = text;
                    cse.il = tmpsav;
                    int_list_sav = tmpsav;
                } else {
                    while (cse.il) {
                        saved = cse.il;
                        cse.il = cse.il->next;
                        if (cse.il && (saved->to < text) &&
                            (cse.il->from > text))
                            break;
                    }
                    if (cse.il) {
                        tmpsav = (struct INT_LIST *)
                            malloc(sizeof(struct INT_LIST));
                        if (tmpsav == NULL) {
                            sys_error("mark_as_read", 5, "malloc");
                            return -1;
                        }
                        saved->next = tmpsav;
                        tmpsav->next = cse.il;
                        tmpsav->from = text;
                        tmpsav->to = text;
                    } else {
                        cse.il = saved;
                        cse.il->next = (struct INT_LIST *)
                            malloc(sizeof(struct INT_LIST));
                        if (cse.il->next == NULL) {
                            sys_error("mark_as_read", 3, "malloc");
                            return -1;
                        }
                        cse.il = cse.il->next;
                        cse.il->from = text;
                        cse.il->to = text;
                        cse.il->next = NULL;
                    }
                }
            } else {
                cse.il = (struct INT_LIST *)
                    malloc(sizeof(struct INT_LIST));
                if (cse.il == NULL) {
                    sys_error("mark_as_read", 3, "malloc");
                    return -1;
                }
                cse.il->from = text;
                cse.il->to = text;
                cse.il->next = NULL;
                int_list_sav = cse.il;
            }
        }
        /* om ett intervall h{nger ihop s} {ndrar vi p} det intervallet */

        else if (bound == 1) {

            /* find and modify interval */

            while (cse.il) {
                if (cse.il->from == text + 1L) {
                    cse.il->from = text;
                    break;
                }
                if (cse.il->to == text - 1L) {
                    cse.il->to = text;
                    break;
                }
                cse.il = cse.il->next;
            }
            cse.il = int_list_sav;
        }
        /* om tv} intervall h{nger ihop, sl} ihop dem till ett */

        else if (bound == 2) {

            /* find and remove higher interval */

            while (cse.il) {
                if (cse.il->to == (text - 1L)) {
                    int_list_next = cse.il->next;
                    cse.il->to = int_list_next->to;
                    cse.il->next = int_list_next->next;
                    free(int_list_next);
                    break;
                }
                cse.il = cse.il->next;
            }
        }
        cse.il = int_list_sav;

        nbuf = replace_confs(&cse, buf);

        critical();
        if (write_file(fd, nbuf) == -1) {
            sys_error("mark_as_read", 4, "write_file");
            return -1;
        }
        if (close_file(fd) == -1) {
            sys_error("mark_as_read", 5, "close_file");
            return -1;
        }
        non_critical();
        free_confs_entry(&cse);
        return 1;
    }
    if (close_file(fd) == -1) {
        sys_error("mark_as_read", 6, "close_file");
        return -1;
    }
    return 0;
}

/*
 * check_if_read - checks if text is read
 * args: text (text), conference (conf)
 * ret: yes (1) or no (0) or error (-1)
 */

int
check_if_read(long text, int conf)
{
    int found, fd;
    struct CONFS_ENTRY cse;
    char *buf, *oldbuf;
    char fname[128];  /* increased from LINE to avoid overflow, modified on 2025-07-12, PL */

    strcpy(fname, Home);
    strcat(fname, CONFS_FILE);

    if ((fd = open_file(fname, 0)) == -1) {
        sys_error("check_if_read", 1, "open_file");
        return -1;
    }
    if ((buf = read_file(fd)) == NULL) {
        sys_error("check_if_read", 2, "read_file");
        return -1;
    }
    oldbuf = buf;

    if (close_file(fd) == -1) {
        sys_error("check_if_read", 3, "close_file");
        return -1;
    }
    /* find confs-entry */

    while (buf) {
        buf = get_confs_entry(buf, &cse);
        if (cse.num == conf)
            break;
        free_confs_entry(&cse);
    }
    free(oldbuf);

    if (cse.num == conf) {
        found = 0;
        while (cse.il) {
            if ((cse.il->from <= text) && (cse.il->to >= text)) {
                found = 1;
                break;
            }
            cse.il = cse.il->next;
        }
        free_confs_entry(&cse);
        return found;
    }
    free_confs_entry(&cse);
    return 0;
}

/*
 * next_text - get next unread text
 * args: conference to look in (conf)
 * ret: textnumber or no texts (0) or error (-1)
 */

long
next_text(int conf)
{
    int fd, deleted, flag, i;
    long text, last, first, high;
    struct CONFS_ENTRY cse;
    char *buf, *oldbuf, *nbuf, *tmpbuf, saved;
    LINE textname, confsname;
    char fname[128];  /* increased from LINE to avoid overflow, modified on 2025-07-12, PL */
    strcpy(fname, Home);
    strcat(fname, CONFS_FILE);

    last = last_text(conf, Uid);
    deleted = 1;
    flag = 0;
    first = 0L;
    high = 0L;

    while (deleted) {
        if ((fd = open_file(fname, 0)) == -1) {
            sys_error("next_text", 1, "open_file");
            return -1L;
        }
        if ((buf = read_file(fd)) == NULL) {
            sys_error("next_text", 2, "read_file");
            return -1L;
        }
        oldbuf = buf;

        if (close_file(fd) == -1) {
            sys_error("next_text", 3, "close_file");
            return -1L;
        }
        while (buf) {
            buf = get_confs_entry(buf, &cse);
            if (cse.num == conf)
                break;
            free_confs_entry(&cse);
        }

        if (cse.num == conf) {
            if (cse.il == NULL) {
                text = 1L;
                high = text;
            } else if (cse.il->from > 1L) {
                text = 1L;
                high = 0L;
            } else {
                text = (cse.il->to + 1L);
                if (cse.il->next)
                    high = 0L;
                else
                    high = text;
            }
            if (text > last)
                text = 0L;
        } else {
            return -1L;
        }

        free_confs_entry(&cse);

        if ((text == 0L) || (conf == 0)) {
            deleted = 0;
        } else {
            sprintf(textname, "%s/%d/%ld", SKLAFF_DB, conf, text);
            if (file_exists(textname) == -1) {
                if (!flag && high) {
                    flag = 1;
                    first = first_text(conf, Uid);
                }
                if (high && (first > text)) {
                    i = strlen(oldbuf) + 10;
                    nbuf = (char *) malloc(i);
                    if (!nbuf) {
                        sys_error("next_text", 1, "malloc");
                        return -1;
                    }
                    memset(nbuf, 0, i);
                    tmpbuf = buf;
                    tmpbuf--;
                    while ((tmpbuf > oldbuf) && (*tmpbuf == '\n'))
                        tmpbuf--;
                    while ((tmpbuf > oldbuf) && (*tmpbuf != '\n'))
                        tmpbuf--;
                    if (tmpbuf > oldbuf)
                        tmpbuf++;
                    saved = *tmpbuf;
                    *tmpbuf = 0;
                    strcpy(nbuf, oldbuf);
                    *tmpbuf = saved;
                    sprintf(confsname, "%d:1-%ld\n", cse.num, (first - 1));
                    strcat(nbuf, confsname);
                    strcat(nbuf, buf);
                    critical();
                    if ((fd = open_file(fname, 0)) == -1)
                        return -1L;
                    if (write_file(fd, nbuf) == -1)
                        return -1L;
                    if (close_file(fd) == -1)
                        return -1L;
                    non_critical();
                    high = 0L;
                } else
                    mark_as_read(text, conf);
            } else {
                deleted = 0;
            }
        }
        free(oldbuf);
    }
    Nexttext = text;
    return text;
}

/*
 * mark_as_unread - mark a text as unread
 * args: text (text), conference (conf)
 * ret: yes (1) or already marked (0) or error (-1)
 */

int
mark_as_unread(long text, int conf)
{
    int found, fd;
    struct CONFS_ENTRY cse;
    char *buf, *oldbuf, *nbuf;
    char fname[128];  /* increased from LINE to avoid overflow, modified on 2025-07-12, PL */

    struct INT_LIST
    *int_list_sav, *saved, *tmpsav;

    strcpy(fname, Home);
    strcat(fname, CONFS_FILE);

    if ((fd = open_file(fname, 0)) == -1) {
        sys_error("mark_as_unread", 1, "open_file");
        return -1;
    }
    if ((buf = read_file(fd)) == NULL) {
        sys_error("mark_as_unread", 2, "read_file");
        return -1;
    }
    oldbuf = buf;

    /* find confs-entry */

    while (buf) {
        buf = get_confs_entry(buf, &cse);
        if (cse.num == conf)
            break;
        free_confs_entry(&cse);
    }

    buf = oldbuf;

    if (cse.num == conf) {

        /* look if already marked as unread */

        int_list_sav = cse.il;
        found = 0;
        while (cse.il) {
            if ((cse.il->from <= text) && (cse.il->to >= text)) {
                found = 1;
                break;
            }
            cse.il = cse.il->next;
        }

        cse.il = int_list_sav;

        if (!found) {
            free_confs_entry(&cse);
            if (close_file(fd) == -1) {
                sys_error("mark_as_unread", 3, "close_file");
                return -1;
            }
            free(oldbuf);
            return 0;
        }
        saved = cse.il;
        while (cse.il) {
            if ((cse.il->from <= text) && (cse.il->to >= text)) {
                if (cse.il->from == cse.il->to) {
                    if (cse.il == int_list_sav) {
                        tmpsav = cse.il;
                        cse.il = cse.il->next;
                        int_list_sav = cse.il;
                        free(tmpsav);
                    } else {
                        saved->next = cse.il->next;
                        free(cse.il);
                    }
                    break;
                } else if (cse.il->from == text) {
                    cse.il->from += 1;
                    break;
                } else if (cse.il->to == text) {
                    cse.il->to -= 1;
                    break;
                } else {
                    tmpsav = (struct INT_LIST *) malloc
                        (sizeof(struct INT_LIST));
                    if (tmpsav == NULL) {
                        sys_error("mark_as_unread", 4, "malloc");
                        return -1;
                    }
                    tmpsav->from = (text + 1);
                    tmpsav->to = cse.il->to;
                    tmpsav->next = cse.il->next;
                    cse.il->to = (text - 1);
                    cse.il->next = tmpsav;
                    break;
                }
            }
            saved = cse.il;
            cse.il = cse.il->next;
        }

        cse.il = int_list_sav;
        nbuf = replace_confs(&cse, buf);

        critical();
        if (write_file(fd, nbuf) == -1) {
            sys_error("mark_as_unread", 5, "write_file");
            return -1;
        }
        free_confs_entry(&cse);

        if (close_file(fd) == -1) {
            sys_error("mark_as_unread", 6, "close_file");
            return -1;
        }
        non_critical();
        return 1;
    }
    if (close_file(fd) == -1) {
        sys_error("mark_as_unread", 7, "close_file");
        return -1;
    }
    return 0;
}

/*
 * display_text - display text
 * args: conference (conf), text (num), add to comment stack (stack)
 *       absolute date (dtype)
 * ret: ok (0) or failure (-1)
 */

int
display_text(int conf, long num, int stack, int dtype)
{
    LINE username, home, emau, aname;
    char fname[128];  /* increased from LINE to avoid overflow, modified on 2025-07-12, PL */
    int fd, uid, type, endwritten, bypass, rot;
    int survey_flag = 0;  /* initialized to avoid maybe-uninitialized warning, modified on 2025-07-12, PL */
    int survey_valid, quest;
    char *buf, *oldbuf;
    char *ptr = NULL;   /* modified on 2025-07-12, PL */
    struct TEXT_ENTRY te, te2;
    struct TEXT_HEADER *th;
    struct TEXT_BODY *tb;
    struct COMMENT_LIST *cl, *tmpcl, *savedcl;
    char *survey_reply = NULL;  /* modified on 2025-07-12, PL */

    rot = Rot13;
    Rot13 = 0;
    if (num == 0) {
        output("\n%s\n\n", MSG_NOTEXTNUM);
        return -1;
    }
    if (conf > 0) {
        sprintf(fname, "%s/%d/%ld", SKLAFF_DB, conf, num);
    } else {
        if (conf) {
            uid = conf - (conf * 2);
            mbox_dir(uid, home);
        } else {
            uid = Uid;
            strcpy(home, Mbox);
        }
        sprintf(fname, "%s/%ld", home, num);
    }


    if ((fd = open_file(fname, OPEN_QUIET)) == -1) {
        output("\n%s\n\n", MSG_NOTEXT);
        return -1;
    }
    if ((buf = read_file(fd)) == NULL) {
        output("\n%s\n\n", MSG_NOREAD);
        return -1;
    }
    oldbuf = buf;

    if (close_file(fd) == -1) {
        return -1;
    }
    buf = get_text_entry(buf, &te);
    free(oldbuf);
    output("\n");

    th = &te.th;

    strcpy(Sub, th->subject);
    th->num = num;
    if (Last_conf) {
        type = Last_conf;
    } else {
        type = Uid - (Uid * 2);
    }
    if (Clear)
        cmd_cls(home);

    bypass = 0;
    if (th->author) {
        display_header(th, 0, type, dtype, NULL);
    } else {
        /* Rewritten 2025-07-09 to avoid segfault if author = 0 and header-lookup fails */
	tb = te.body;
ptr = NULL;
char *tmp = NULL;  // <- this was missing before

// Look for From-line variants
while (tb) {
    if ((ptr = strstr(tb->line, MSG_EMFROM)) != NULL) {
        ptr += strlen(MSG_EMFROM);
        break;
    } else if ((ptr = strstr(tb->line, MSG_EMFROM2)) != NULL) {
        ptr += strlen(MSG_EMFROM2);
        break;
    } else if ((ptr = strstr(tb->line, MSG_EMFROM3)) != NULL) {
        ptr += strlen(MSG_EMFROM3);
        break;
//    } else if ((ptr = strstr(tb->line, MSG_EMFROM4)) != NULL) {
//        ptr += strlen(MSG_EMFROM4);
//        break;
//    } else if ((ptr = strstr(tb->line, MSG_EMFROM5)) != NULL) {
//        ptr += strlen(MSG_EMFROM5);
//        break;

    }
    tb = tb->next;
}

if (!ptr || strlen(ptr) == 0) {
    snprintf(emau, sizeof(emau), "Unknown sender");
    debuglog("WARNING: Could not extract 'From' address. Using fallback 'Unknown sender'", 2);
    
} else {
    strncpy(emau, ptr, LINE_LEN - 1);
    emau[LINE_LEN - 1] = '\0';  // Ensure null termination

    // Strip trailing newline or whitespace
    tmp = strchr(emau, '\n');
    if (tmp) *tmp = '\0';

    
}

    





    if (!Header && te.body) {
    tb = te.body;




    while (tb) {
        if (is_blank_line(tb->line)) {
            bypass++;  // count the blank line too
            tb = tb->next;
            break;
        }
        bypass++;
        tb = tb->next;
    }
}

   th->size -= bypass;
        display_header(th, 0, type, dtype, emau);
        strcpy(aname, emau);
    }
    tb = te.body;
    if (rot)
        Rot13 = 1;

    /* Set up variables if this is a survey text */

    if (th->type == TYPE_SURVEY) {
        survey_flag = 0;
        if (!check_if_survey_taken(num, conf) && Numlines > 0 && time(0) < th->sh.time) {
            survey_flag = TAKE_SURVEY;
            survey_valid = 1;
            survey_reply = (char *) malloc(LINE_LEN * th->sh.n_questions);
            if (survey_reply == NULL) {
                sys_error("display_text", 1, "malloc");
                return -1;
            }
            memset(survey_reply, 0, LINE_LEN * th->sh.n_questions);
            quest = 0;
        }
    }
    while (tb) {
        if (!bypass) {
            if (tb->line[0] == '\f') {
                tb->line[0] = '\0';
                Lines = Numlines;
            }
            if (th->type == TYPE_SURVEY) {
                if (make_survey(survey_reply + quest * LINE_LEN, &quest,
                        tb->line, survey_flag) == -1) {
                    survey_valid = 0;
                    break;
                }
            } else {
                if (output("%s\n", tb->line) == -1) {
                    survey_valid = 0;
                    break;
                }
            }
        } else
            bypass--;
        tb = tb->next;
    }

    Rot13 = 0;
    endwritten = 0;
    cl = te.cl;
    while (cl) {
        if (conf > 0) {
            sprintf(fname, "%s/%d/%ld", SKLAFF_DB, conf,
                cl->comment_num);
            if (file_exists(fname) == -1) {
                mark_as_read(cl->comment_num, conf);
                cl->comment_num = 0L;
            }
        }
        if (cl->comment_num) {
            if (!endwritten) {
                if ((th->size >= (Numlines - 6)) || Author) {
                    if (th->author)
                        user_name(th->author, aname);
                    
                    /* 2025-08-10, PL: cleanup footer author: trim CR/LF/space and strip quotes before '<' */
                    {
                        size_t L__ = strlen(aname);
                        while (L__ && (aname[L__-1] == '\r' || aname[L__-1] == '\n' || aname[L__-1] == ' ' || aname[L__-1] == '\t'))
                            aname[--L__] = '\0';
                        char *lt__ = strchr(aname, '<');
                        char *end__ = lt__ ? lt__ : aname + strlen(aname);
                        char *q__ = aname;
                        while ((q__ = strchr(q__, '"')) && q__ < end__) {
                            memmove(q__, q__ + 1, strlen(q__ + 1) + 1);
                        }
                    }

        /* 2025-08-10, PL: RFC2047-decode footer author */
        {
            LINE adec;
            rfc2047_decode(aname, adec, sizeof(adec));
            snprintf(aname, sizeof(aname), "%.*s", (int)sizeof(aname)-1, adec);
        }
 output_ansi_fmt("\n%s " GREEN "%ld "DOT "%s " YELLOW "%s\n" DOT, "\n%s %ld %s %s\n",
                        (th->type == TYPE_TEXT) ? MSG_EOT : MSG_EOSURVEY,
                        th->num, MSG_BY, aname);
                } else
                    output("\n-------\n");
                endwritten = 1;
            }
            if (!cl->comment_author) {
                sprintf(fname, "%s/%d/%ld", SKLAFF_DB, conf,
                    cl->comment_num);
                if ((fd = open_file(fname, OPEN_QUIET)) == -1) {
                    output("\n%s\n\n", MSG_NOTEXT);
                    return -1;
                }
                if ((buf = read_file(fd)) == NULL) {
                    output("\n%s\n\n", MSG_NOREAD);
                    return -1;
                }
                oldbuf = buf;
                if (close_file(fd) == -1) {
                    return -1;
                }
                buf = get_text_entry(buf, &te2);
                free(oldbuf);
                tb = te2.body;
                while (tb) {
                    if ((ptr = strstr(tb->line, MSG_EMFROM)) != NULL)
                        break;
                    else if ((ptr = strstr(tb->line, MSG_EMFROM2)) != NULL)
                        break;
                    else if ((ptr = strstr(tb->line, MSG_EMFROM3)) != NULL)
                        break;
                    tb = tb->next;
                }
                ptr = ptr + strlen(MSG_EMFROM);
                strcpy(username, ptr);
                free_text_entry(&te2);
            } else {
                user_name(cl->comment_author, username);
            }
            if (output_ansi_fmt("%s " GREEN "%ld " DOT "%s " YELLOW "%s\n" DOT, "%s %ld %s %s\n",  MSG_REPLYIN, cl->comment_num,
                    MSG_BY, username) == -1)
                break;
        }
        cl = cl->next;
    }

    if (!endwritten && ((th->size >= (Numlines - 6)) || Author)) {
        if (th->author)
            user_name(th->author, aname);
	else
            //snprintf(aname, sizeof(aname), "%s", MSG_UNKNOWNU); /* fallback because why not PL 2025-08-10 */

        /* 2025-08-10, PL: strip surrounding quotes in author name */
    {
        size_t L = strlen(aname);
        if (L >= 2 && aname[0] == '"' && aname[L-1] == '"') {
            aname[L-1] = '\0';
            memmove(aname, aname + 1, L - 1);
        }
    }

        /* 2025-08-10, PL: RFC2047-decode footer author */
        {
            LINE adec;
            rfc2047_decode(aname, adec, sizeof(adec));
            snprintf(aname, sizeof(aname), "%.*s", (int)sizeof(aname)-1, adec);
        }
        output_ansi_fmt("\n%s " GREEN "%ld "DOT "%s " YELLOW "%s\n" DOT, "\n%s %ld %s %s\n",
            (th->type == TYPE_TEXT) ? MSG_EOT : MSG_EOSURVEY,
            th->num, MSG_BY, aname);
    }
    output("\n");

    if (te.cl) {
        cl = te.cl;
        while (cl) {
            tmpcl = te.cl;
            savedcl = tmpcl;
            while (tmpcl->next) {
                savedcl = tmpcl;
                tmpcl = tmpcl->next;
            }
            if (stack && Current_conf && tmpcl->comment_num &&
                (!check_if_read(tmpcl->comment_num, Current_conf)))
                push_comment(tmpcl->comment_num);
            if (savedcl == tmpcl)
                cl = NULL;
            else
                savedcl->next = NULL;
        }
    }
    if (th->type == TYPE_SURVEY && (survey_flag & TAKE_SURVEY)) {
        if (survey_valid)
            if (save_survey_result(num, conf, survey_reply, th->sh.n_questions))
                mark_survey_as_taken(num, conf);
        free(survey_reply);
    }
    free_text_entry(&te);

    return 0;
}

/*
 * parse_text - translate string to textnumber
 * args: pointer to string
 * ret: textnumber or no text (0) or error (-1)
 */

long
parse_text(char *args)
{
    long textnum;
    LINE home, tmpstr, tmpstr2;
    char fname[128];  /* increased from LINE to avoid overflow, modified on 2025-07-12, PL */
    int fd, uid, right;
    char *buf, *oldbuf;
    struct TEXT_ENTRY te;
    struct TEXT_HEADER *th;
    struct CONF_ENTRY *ce = NULL;
    struct USER_LIST *ul;

    if (!args || (*args == '\0')) {
        Rot13 = 0;
        output("\n%s\n\n", MSG_ERRTNUM);
        return 0;
    }
    down_string(args);

    strcpy(tmpstr, MSG_LASTREAD);
    strcpy(tmpstr2, MSG_REPLIED);
    if (strstr(tmpstr, args) == tmpstr) {
        Last_conf = Current_conf;
        if (Current_text) {
            textnum = Current_text;
        } else {
            Rot13 = 0;
            output("\n%s\n\n", MSG_NOLASTTEXT);
            return 0;
        }
    } else if (strstr(tmpstr2, args) == tmpstr2) {
        if (!Last_text) {
            Rot13 = 0;
            output("\n%s\n\n", MSG_NOLASTTEXT);
            return 0;
        }
        if (Last_conf > 0) {
            sprintf(fname, "%s/%d/%ld", SKLAFF_DB, Last_conf, Last_text);
        } else {
            if (Last_conf) {
                uid = Last_conf - (Last_conf * 2);
            } else {
                uid = Uid;
            }
            mbox_dir(uid, home);
            sprintf(fname, "%s/%ld", home, Last_text);
        }

        if ((fd = open_file(fname, OPEN_QUIET)) == -1) {
            Rot13 = 0;
            output("\n%s\n\n", MSG_NOTEXT);
            return 0;
        }
        if ((buf = read_file(fd)) == NULL) {
            Rot13 = 0;
            output("\n%s\n\n", MSG_NOREAD);
            return 0;
        }
        oldbuf = buf;

        if (close_file(fd) == -1) {
            Rot13 = 0;
            return 0;
        }
        buf = get_text_entry(buf, &te);

        free(oldbuf);
        th = &te.th;
        textnum = th->comment_num;

        if (th->comment_conf) {
            ul = get_confrc_struct(th->comment_conf);
            ce = get_conf_struct(th->comment_conf);
            right = conf_right(ul, Uid, ce->type, ce->creator);
            free_userlist(ul);
            if (!right)
                Last_conf = th->comment_conf;
            else
                textnum = 0;
        } else if (textnum) {
            if (Last_conf < 1) {
                if (th->author != Uid) {
                    Last_conf = th->author - (th->author * 2);
                } else {
                    Last_conf = 0;
                }
            }
        }
        free_text_entry(&te);

        if (!textnum) {
            Rot13 = 0;
            output("\n%s\n\n", MSG_NOREPLY);
        }
    } else {
        Last_conf = Current_conf;
        textnum = atol(args);
        if (textnum == 0) {
            Rot13 = 0;
            output("\n%s\n\n", MSG_ERRTNUM);
            return 0;
        }
    }
    return textnum;
}

/*
 * stack_text - add comments to text to comment stack
 * args: text (num)
 * ret: ok (0) or failure (-1)
 */

int
stack_text(long num)
{
    char fname[128];  /* increased from LINE to avoid overflow, modified on 2025-07-12, PL */
    int fd;
    char *buf, *oldbuf;
    struct TEXT_ENTRY te;
    struct COMMENT_LIST *cl, *tmpcl, *savedcl;

    sprintf(fname, "%s/%d/%ld", SKLAFF_DB, Current_conf, num);

    if ((fd = open_file(fname, OPEN_QUIET)) == -1) {
        return -1;
    }
    if ((buf = read_file(fd)) == NULL) {
        return -1;
    }
    oldbuf = buf;

    if (close_file(fd) == -1) {
        return -1;
    }
    buf = get_text_entry(buf, &te);
    free(oldbuf);

    cl = te.cl;
    while (cl) {
        sprintf(fname, "%s/%d/%ld", SKLAFF_DB, Current_conf,
            cl->comment_num);
        if (file_exists(fname) == -1) {
            mark_as_read(cl->comment_num, Current_conf);
            cl->comment_num = 0L;
        }
        cl = cl->next;
    }

    if (te.cl) {
        cl = te.cl;
        while (cl) {
            tmpcl = te.cl;
            savedcl = tmpcl;
            while (tmpcl->next) {
                savedcl = tmpcl;
                tmpcl = tmpcl->next;
            }
            if (tmpcl->comment_num)
                push_comment(tmpcl->comment_num);
            if (savedcl == tmpcl)
                cl = NULL;
            else
                savedcl->next = NULL;
        }
    }
    free_text_entry(&te);
    return 0;
}

/*
 * tree_top - find first text in tree
 * args: text (text)
 * ret: top text or no text (0)
 */

int
tree_top(long text)
{
    char fname[128];  /* increased from LINE to avoid overflow, modified on 2025-07-12, PL */
    int fd;
    long top;
    char *buf, *oldbuf;
    struct TEXT_ENTRY te;
    struct TEXT_HEADER *th;

    top = text;
    while (text) {
        sprintf(fname, "%s/%d/%ld", SKLAFF_DB, Current_conf, text);

        if ((fd = open_file(fname, OPEN_QUIET)) == -1) {
            return top;
        }
        if ((buf = read_file(fd)) == NULL) {
            return 0;
        }
        oldbuf = buf;

        if (close_file(fd) == -1) {
            return 0;
        }
        buf = get_text_entry(buf, &te);
        free(oldbuf);

        th = &te.th;
        if (th->comment_num && !th->comment_conf) {
            text = th->comment_num;
        } else {
            text = 0;
        }

        sprintf(fname, "%s/%d/%ld", SKLAFF_DB, Current_conf, text);
        if (file_exists(fname) != -1) {
            top = text;
        }
        free_text_entry(&te);
    }
    return top;
}

/*
 * list_subj - list text subjects
 * args: pointer to string searched for
 * ret: ok (0) or failure (-1)
 */

int
list_subj(char *str)
{
    LINE subject, author, subj_disp, subj_dec;
    char fname[128];  /* increased from LINE to avoid overflow, modified on 2025-07-12, PL */
    char *buf, *oldbuf; char c;
    char *ptr2 = NULL;   /* modified on 2025-07-12, PL */
    long firsttext, current_text;
    int dot_count, wait_count, strlgth, xit, from, fd;
    struct TEXT_ENTRY te;
    struct TEXT_BODY *tb;
    struct TEXT_HEADER *th;

    if (Current_conf)
        output("\n%7s  %-30s    %s\n\n", MSG_TEXTNAME, MSG_WRITTENBY,
            MSG_SUBJECT2);
    else
        output("\n%7s  %-30s  %s\n\n", MSG_TEXTNAME, MSG_TOFROM,
            MSG_SUBJECT2);

    rtrim(str);
    up_string(str);
    strlgth = strlen(str);
    current_text = last_text(Current_conf, Uid);
    firsttext = first_text(Current_conf, Uid);
    xit = 0;
    wait_count = 0;
    dot_count = 0;

    do {
        if (Current_conf) {
            sprintf(fname, "%s/%d/%ld", SKLAFF_DB, Current_conf, current_text);
        } else {
           snprintf(fname, sizeof(fname), "%s/%ld", Mbox, current_text);  /* modified on 2025-07-12, PL */
        }
        if ((fd = open_file(fname, OPEN_QUIET)) != -1) {
            if ((buf = read_file(fd)) == NULL) {
                sys_error("list_subj", 1, "read_file");
                return -1;
            }
            if ((fd = close_file(fd)) == -1) {
                free(buf);
                sys_error("list_subj", 2, "close_file");
                return -1;
            }
            oldbuf = buf;
            buf = get_text_entry(buf, &te);
            free(oldbuf);

            th = &te.th;

            from = 0;
            if (!Current_conf && (th->author == Uid) && th->comment_author) {
                user_name(th->comment_author, author);
                from = 1;
            } else if ((th->author == Uid) && !Current_conf) {
                if (te.body != NULL &&
                    (ptr2 = strstr(te.body->line, MSG_COPYTO))) {
                    strcpy(author, (ptr2 + 1 + strlen(MSG_COPYTO)));
                    author[strlen(author) - 1] = 0;
                    from = 1;
                } else
                    user_name(th->author, author);
            } else if (th->author)
                user_name(th->author, author);
            else {
                tb = te.body;
                while (tb) {
                    if ((ptr2 = strstr(tb->line, MSG_EMFROM)) != NULL)
                        break;
                    else if ((ptr2 = strstr(tb->line, MSG_EMFROM2)) != NULL)
                        break;
                    else if ((ptr2 = strstr(tb->line, MSG_EMFROM3)) != NULL)
                        break;
                    tb = tb->next;
                }
                ptr2 = ptr2 + strlen(MSG_EMFROM);
/* 2025-08-10, PL: humanize From: for listing (prefer display name) */
{
    LINE from_dec, disp;
    rfc2047_decode(ptr2, from_dec, sizeof(from_dec));
    extract_display_name(from_dec, disp, sizeof(disp));
    /* strip surrounding quotes */
    size_t L__ = strlen(disp);
    if (L__ >= 2 && disp[0] == '"' && disp[L__-1] == '"') {
        disp[L__-1] = '\0';
        memmove(disp, disp + 1, L__ - 1);
    }
    if (disp[0]) {
        snprintf(author, sizeof(author), "%s", disp);
    } else {
        /* Fallback to old email extraction if no display name */
        char *p3 = strchr(from_dec, '@');
        if (!p3) p3 = strchr(from_dec, '!');
        if (p3) {
            while ((p3 > from_dec) && (p3[-1] != ' ') && (p3[-1] != '<')) p3--;
            char *p4 = strchr(p3, '>');
            if (!p4) p4 = strchr(p3, ' ');
            if (p4) { char sav__ = *p4; *p4 = '\0'; snprintf(author, sizeof(author), "%s", p3); *p4 = sav__; }
            else snprintf(author, sizeof(author), "%s", p3);
        } else {
            snprintf(author, sizeof(author), "%s", from_dec);
        }
    }
}
}
            /* 2025-08-10, PL: decode/humanize author for listing */
            {
                LINE tmpdec, disp;
                rfc2047_decode(author, tmpdec, sizeof(tmpdec));
                extract_display_name(tmpdec, disp, sizeof(disp));
                /* strip surrounding quotes, e.g. "Carlos E.R." -> Carlos E.R. */
                size_t L__ = strlen(disp);
                if (L__ >= 2 && disp[0] == '"' && disp[L__-1] == '"') {
                    disp[L__-1] = '\0';
                    memmove(disp, disp + 1, L__ - 1);
                }
                snprintf(author, sizeof(author), "%s", disp);
            }

            /* 2025-08-10, PL: decode subject for search/display (don't mutate th->subject) */
            rfc2047_decode(th->subject, subj_dec, sizeof(subj_dec));
            strcpy(subject, subj_dec);
            up_string(subject);

            /* 2025-08-10, PL: UTF-8 safe author truncation to 30 cols */
            {
                LINE tmpa;
                utf8_trunc_cols(author, 30, tmpa, sizeof(tmpa));
                snprintf(author, sizeof(author), "%s", tmpa);
            }
            utf8_trunc_cols(subj_dec, 36, subj_disp, sizeof(subj_disp));
            if (strncmp(subject, str, strlgth) == 0) {
                while (dot_count > 0) {
                    output("\b \b");
                    dot_count--;
                }
                wait_count = 0;

                if (Current_conf) {
                    if (th->comment_num)
                        c = ' ';
                    else
                        c = '*';
                    if (th->type == TYPE_SURVEY)
                        c = '#';
                    if (output("%7ld  %-30s  %c %s\n",
                            th->num, author, c, subj_disp
                        ) == -1) {
                        xit = 1;
                    }
                } else {
                    if (from) {
                        if (output("%7ld  %s%-24s  %s\n",
                                th->num, MSG_TOSUB, author, subj_disp
                            ) == -1) {
                            xit = 1;
                        }
                    } else {
                        if (output("%7ld  %s%-24s  %s\n",
                                th->num, MSG_FROMSUB, author, subj_disp
                            ) == -1) {
                            xit = 1;
                        }
                    }
                }
            } else {
                wait_count++;
                if (wait_count == 15) {
                    if (dot_count < 79) {
                        output(".");
                        fflush(stdout);
                        dot_count++;
                    } else {
                        while (dot_count > 0) {
                            output("\b \b");
                            dot_count--;
                        }
                    }
                    wait_count = 0;
                }
            }
            free_text_entry(&te);
        }
        current_text--;
    } while ((!xit) && (current_text >= firsttext));
    while (dot_count > 0) {
        output("\b \b");
        dot_count--;
    }
    output("\n");
    return 0;
}

/*
 * age_to_textno - return text number of texts not older than age
 * args: age (seconds)
 * ret: text no (>0) or failure (0)
 */

long
age_to_textno(long age)
{
    char *buf, *oldbuf;
    long textno, current_text;
    time_t now, texttime;
    int fd;
    struct TEXT_ENTRY te;
    struct TEXT_HEADER *th;
    char fname[128];  /* increased from LINE to avoid overflow, modified on 2025-07-12, PL */;

    now = time(NULL);

    current_text = last_text(Current_conf, Uid);

    textno = -1;
    texttime = now;
    do {
        if (Current_conf) {
            sprintf(fname, "%s/%d/%ld", SKLAFF_DB, Current_conf, current_text);
        } else {
         /* sprintf(fname, "%s/%ld", Mbox, current_text); */   
            snprintf(fname, sizeof(fname), "%s/%ld", Mbox, current_text);  /* modified on 2025-07-12, PL */
}
        if ((fd = open_file(fname, OPEN_QUIET)) != -1) {
            if ((buf = read_file(fd)) == NULL) {
                sys_error("list_subj", 1, "read_file");
                return -1;
            }
            if ((fd = close_file(fd)) == -1) {
                free(buf);
                sys_error("list_subj", 2, "close_file");
                return -1;
            }
            oldbuf = buf;
            buf = get_text_entry(buf, &te);
            free(oldbuf);

            th = &te.th;
            texttime = th->time;

            free_text_entry(&te);
        }
        current_text--;
        textno++;
    } while (texttime > now - age);

    return textno;
}


/*
 * save_text - save text into database
 * args: filename to save (fname), text header (th), conference (conf)
 * ret: number of text saved
 */

long
save_text(char *fname, struct TEXT_HEADER * th, int conf)
{
    int fd, fdinfile, fdoutfile, usernum, oldconf, ml;
    char *buf, *oldbuf, *nbuf, *inbuf, *fbuf;
    char conffile[256], confdir[256], textfile[256], home[256];  /* increased size to prevent truncation, modified on 2025-07-12, PL */ 
    struct CONF_ENTRY ce;

    oldconf = conf;
    ml = 0;
    if (conf < 0) {
        usernum = conf - (conf * 2);
        ml = usernum;
        mbox_dir(usernum, home);
     /* sprintf(conffile, "%s%s", home, MAILBOX_FILE); */ // Original code
	snprintf(conffile, sizeof(conffile), "%.200s%s", home, MAILBOX_FILE);  /* modified on 2025-07-12, PL */
     /* sprintf(confdir, "%s/", home); */ // Original code
        snprintf(confdir, sizeof(confdir), "%.252s/", home); /* modified on 2025-07-12, PL */
        conf = 0;
    } else {
        strcpy(conffile, CONF_FILE);
        sprintf(confdir, "%s/%d/", SKLAFF_DB, conf);
    }

    if ((fd = open_file(conffile, 0)) == -1) {
        return -1L;
    }
    if ((buf = read_file(fd)) == NULL) {
        return -1L;
    }
    oldbuf = buf;

    while ((buf = get_conf_entry(buf, &ce))) {
        if (ce.num == conf)
            break;
    }

    if (ce.num == conf) {
        ce.last_text++;
        nbuf = replace_conf(&ce, oldbuf);
        if (!nbuf) {
            output("\n%s\n\n", MSG_CONFMISSING);
            return -1L;
        }
    } else {
        output("\n%s\n\n", MSG_CONFMISSING);
        return -1L;
    }

 /* sprintf(textfile, "%s%ld", confdir, ce.last_text); */
    snprintf(textfile, sizeof(textfile), "%.240s%ld", confdir, ce.last_text); /* modified on 2025-07-12, PL */

    if ((fdoutfile = open_file(textfile, OPEN_QUIET | OPEN_CREATE)) == -1) {
        output("\n%s\n\n", MSG_ERRCREATET);
        return -1L;
    }
    if ((fdinfile = open_file(fname, 0)) == -1) {
        return -1L;
    }
    if ((inbuf = read_file(fdinfile)) == NULL) {
        return -1L;
    }
    if (close_file(fdinfile) == -1) {
        return -1L;
    }
    fbuf = (char *) malloc(strlen(inbuf) + sizeof(LONG_LINE));
    if (fbuf == NULL) {
        sys_error("save_text", 1, "malloc");
        return -1L;
    }
    memset(fbuf, 0, strlen(inbuf) + sizeof(LONG_LINE));
    if (th->type == TYPE_TEXT)
        sprintf(fbuf, "%ld:%d:%lld:%ld:%d:%d:%d:%d\n", ce.last_text, th->author,
            (long long) th->time, th->comment_num, th->comment_conf,
            th->comment_author, th->size, th->type);
    else
        sprintf(fbuf, "%ld:%d:%lld:%ld:%d:%d:%d:%d:%d:%lld\n", ce.last_text, th->author,
            (long long) th->time, th->comment_num, th->comment_conf,
            th->comment_author, th->size, th->type,
            th->sh.n_questions, (long long) th->sh.time);
    strcat(fbuf, th->subject);
    strcat(fbuf, "\n");
    strcat(fbuf, inbuf);

    free(inbuf);

    critical();
    if (write_file(fdoutfile, fbuf) == -1) {
        return -1L;
    }
    if (close_file(fdoutfile) == -1) {
        return -1L;
    }
    if (write_file(fd, nbuf) == -1) {
        return -1L;
    }
    if (close_file(fd) == -1) {
        return -1L;
    }
    non_critical();

    /* Save copy if flag set and in mailbox */

    usernum = oldconf - (oldconf * 2);
    if ((oldconf < 0) && Copy && (usernum != Uid)) {
        sprintf(conffile, "%s%s", Mbox, MAILBOX_FILE);
        sprintf(confdir, "%s/", Mbox);
        oldconf = 0;

        if ((fd = open_file(conffile, 0)) == -1) {
            return -1L;
        }
        if ((buf = read_file(fd)) == NULL) {
            return -1L;
        }
        oldbuf = buf;

        while ((buf = get_conf_entry(buf, &ce))) {
            if (!ce.num)
                break;
        }

        if (!ce.num) {
            ce.last_text++;
            nbuf = replace_conf(&ce, oldbuf);
            if (!nbuf) {
                output("\n%s\n\n", MSG_CONFMISSING);
                return -1L;
            }
        } else {
            output("\n%s\n\n", MSG_CONFMISSING);
            return -1L;
        }

        snprintf(textfile, sizeof(textfile), "%.240s%ld", confdir, ce.last_text); /* modified on 2025-07-12, PL */

        if ((fdoutfile = open_file(textfile,
                    OPEN_QUIET | OPEN_CREATE)) == -1) {
            output("\n%s\n\n", MSG_ERRCREATET);
            return -1L;
        }
        if ((fdinfile = open_file(fname, 0)) == -1) {
            return -1L;
        }
        if ((inbuf = read_file(fdinfile)) == NULL) {
            return -1L;
        }
        if (close_file(fdinfile) == -1) {
            return -1L;
        }
        fbuf = (char *) malloc(strlen(inbuf) + LONG_LINE_LEN);
        if (fbuf == NULL) {
            sys_error("save_text", 1, "malloc");
            return -1L;
        }
        memset(fbuf, 0, strlen(inbuf) + LONG_LINE_LEN);
        th->comment_num = 0;
        th->comment_conf = 0;
        th->comment_author = usernum;

        /* Note, mail copy is never a survey */

        sprintf(fbuf, "%ld:%d:%lld:%ld:%d:%d:%d:%d\n", ce.last_text, th->author,
            (long long) th->time, th->comment_num, th->comment_conf,
            th->comment_author, th->size, th->type);
        strcat(fbuf, th->subject);
        strcat(fbuf, "\n");
        strcat(fbuf, inbuf);

        free(inbuf);

        critical();
        if (write_file(fdoutfile, fbuf) == -1) {
            return -1L;
        }
        if (close_file(fdoutfile) == -1) {
            return -1L;
        }
        if (write_file(fd, nbuf) == -1) {
            return -1L;
        }
        if (close_file(fd) == -1) {
            return -1L;
        }
        non_critical();

        mark_as_read(ce.last_text, 0);

    }
    /* End of save-copy section */

    unlink(fname);
    if (ml) {
        if (user_is_active(ml))
            notify_user(ml, SIGNAL_NEW_TEXT);
    } else
        notify_all_processes(SIGNAL_NEW_TEXT);
    return ce.last_text;
}


/*
 * more_comment - check for more comment in comment stack
 * ret:	no_more	(0) or more (1)
 */

int
more_comment(void)
{
    if (cstack)
        return 1;
    else
        return 0;
}


/*
 * more_text - check for more texts in conf
 * ret:	no_more (0) or more (1)
 */

int
more_text(void)
{
    if (next_text(Current_conf)) {
        return 1;
    } else {
        return 0;
    }
}


/*
 *  free_text_entry - free text entry
 */

void
free_text_entry(struct TEXT_ENTRY * te)
{
    struct TEXT_BODY *tmptb;
    struct COMMENT_LIST *tmpcl;

    while (te->body) {
        tmptb = te->body->next;
        free(te->body);
        te->body = tmptb;
    }
    while (te->cl) {
        tmpcl = te->cl->next;
        free(te->cl);
        te->cl = tmpcl;
    }
    return;
}

/*
 * save_mailcopy - saves copy of outgoing email
 * args: name of email receiver (rec), subject (subject),
 *       buffer to save (inbuf)
 * ret: number of text saved
 */

long
save_mailcopy(char *rec, char *subject, char *inbuf)
{
    char conffile[256], confdir[256], textfile[256], copymsg[256];  /* promoted to 256 to avoid truncation, modified on 2025-07-12, PL */
    char *buf, *oldbuf, *nbuf, *fbuf, *ptr;
    int fd, fd2;
    struct TEXT_HEADER th;
    struct CONF_ENTRY ce;

    sprintf(conffile, "%s%s", Mbox, MAILBOX_FILE);
    sprintf(confdir, "%s/", Mbox);

    if ((fd = open_file(conffile, 0)) == -1)
        return -1L;
    if ((buf = read_file(fd)) == NULL)
        return -1L;
    oldbuf = buf;
    while ((buf = get_conf_entry(buf, &ce)))
        if (!ce.num)
            break;

    if (!ce.num) {
        ce.last_text++;
        nbuf = replace_conf(&ce, oldbuf);
        if (!nbuf) {
            output("\n%s\n\n", MSG_CONFMISSING);
            return -1L;
        }
    } else {
        output("\n%s\n\n", MSG_CONFMISSING);
        return -1L;
    }

 /* sprintf(textfile, "%s%ld", confdir, ce.last_text); */
    snprintf(textfile, sizeof(textfile), "%.240s%ld", confdir, ce.last_text); /* modified on 2025-07-12, PL */

    if ((fd2 = open_file(textfile, OPEN_QUIET | OPEN_CREATE)) == -1) {
        output("\n%s\n\n", MSG_ERRCREATET);
        return -1L;
    }
    fbuf = (char *) malloc(strlen(inbuf) + 240);
    if (fbuf == NULL) {
        sys_error("save_mailcopy", 1, "malloc");
        return -1L;
    }
    memset(fbuf, 0, strlen(inbuf) + 240);

    th.author = Uid;
    th.time = time(0);
    th.comment_num = 0;
    th.comment_conf = 0;
    th.comment_author = 0;
    th.size = 0;
    th.type = TYPE_TEXT;
    ptr = inbuf;
    while (1) {
        ptr = strchr(ptr, '\n');
        if (ptr) {
            th.size++;
            ptr++;
        } else
            break;
    }
    th.size += 2;
    sprintf(copymsg, "<%s %s>", MSG_COPYTO, rec);

    /* Note, don't worry about saving survey info */

    sprintf(fbuf, "%ld:%d:%lld:%ld:%d:%d:%d:%d\n", ce.last_text, th.author,
        (long long) th.time, th.comment_num, th.comment_conf,
        th.comment_author, th.size, th.type);
    strcat(fbuf, subject);
    strcat(fbuf, "\n");
    strcat(fbuf, copymsg);
    strcat(fbuf, "\n\n");
    strcat(fbuf, inbuf);

    critical();

    if (write_file(fd2, fbuf) == -1)
        return -1L;
    if (close_file(fd2) == -1)
        return -1L;

    if (write_file(fd, nbuf) == -1)
        return -1L;
    if (close_file(fd) == -1)
        return -1L;

    non_critical();

    mark_as_read(ce.last_text, 0);

    return ce.last_text;
}


/*
 * cnvcat - convert characters in string - by Daniel Gr|njord
 */

void
cnvnat(char *str, int ch)
{
    char *c;

    if (!Ibm && !Iso8859 && !Mac)
        return;

    c = str;
    while (*c) {
        if (Ibm) {
            switch (*c) {
            case '}':
                *c = (char) 134;
                break;
            case '{':
                *c = (char) 132;
                break;
            case '|':
                *c = (char) 148;
                break;
            case ']':
                *c = (char) 143;
                break;
            case '[':
                *c = (char) 142;
                break;
            case '\\':
                *c = (char) 153;
                break;
            }
        } else if (Iso8859) {
            switch (*c) {
            case '}':
                *c = (char) 229;
                break;
            case '{':
                *c = (char) 228;
                break;
            case '|':
                *c = (char) 246;
                break;
            case ']':
                *c = (char) 197;
                break;
            case '[':
                *c = (char) 196;
                break;
            case '\\':
                *c = (char) 214;
                break;
            }
        } else if (Mac) {
            switch (*c) {
            case '}':
                *c = (char) 140;
                break;
            case '{':
                *c = (char) 138;
                break;
            case '|':
                *c = (char) 154;
                break;
            case ']':
                *c = (char) 129;
                break;
            case '[':
                *c = (char) 128;
                break;
            case '\\':
                *c = (char) 133;
                break;
            }
        }
        c++;
    }
}

/*
 * int2ms - Convert integer to Microsoft Basic float. Urk.
 */

void
int2ms(int i, char c[4])
{
    int m, e;

    if (i == 0) {
        c[0] = c[1] = c[2] = 0;
        c[3] = 0x80;
        return;
    }
    e = 152;
    m = 0x7fffff & i;

    while (!(0x800000 & m)) {
        m <<= 1;
        e--;
    }
    c[0] = 0xff & m;
    c[1] = 0xff & (m >> 8);
    c[2] = 0x7f & (m >> 16);
    c[3] = 0xff & e;
}


/*
 * display_survey_result - display text
 * args: conference (conf), text (num), add to comment stack (stack)
 *       absolute date (dtype)
 * ret: ok (0) or failure (-1)
 */

int
display_survey_result(int conf, long num)
{
    LINE username, confdir, home, time_val, c;
    char fname[128];  /* increased from LINE to avoid overflow, modified on 2025-07-12, PL */
    char resfile[128];  /* increased from LINE to avoid overflow, modified on 2025-07-12, PL */
    int fd, n, i;
    char *buf, *oldbuf, *s, *s2;
    struct TEXT_ENTRY te;
    struct TEXT_HEADER *th;

    if (num == 0) {
        output("\n%s\n\n", MSG_NOTEXTNUM);
        return -1;
    }
    if (conf > 0) {
        sprintf(fname, "%s/%d/%ld", SKLAFF_DB, conf, num);
    } else {
        output("\nInga enk{ter i brevl}dan\n\n");
        return 0;
    }

    if ((fd = open_file(fname, OPEN_QUIET)) == -1) {
        output("\n%s\n\n", MSG_NOTEXT);
        return -1;
    }
    if ((buf = read_file(fd)) == NULL) {
        output("\n%s\n\n", MSG_NOREAD);
        return -1;
    }
    oldbuf = buf;

    if (close_file(fd) == -1) {
        return -1;
    }
    buf = get_text_entry(buf, &te);
    free(oldbuf);
    output("\n");
    th = &te.th;
    strcpy(Sub, th->subject);
    th->num = num;

    if (th->type != TYPE_SURVEY) {
        output("%s\n\n", MSG_NOTASURVEY);
        return 0;
    }
    if (time(0) < th->sh.time) {/* Time to show result? */
        output("%s\n\n", MSG_NOTREPORTED);
        return 0;
    }
    user_name(th->author, username);

    if (Clear)
        cmd_cls(home);

    /* Get survey results */

    sprintf(confdir, "%s/%d/", SKLAFF_DB, conf);
    sprintf(resfile, "%s%ld.result", confdir, num);

    if ((fd = open_file(resfile, OPEN_QUIET)) == -1) {
        n = 0;
        buf = NULL;
    } else {

        if ((buf = read_file(fd)) == NULL) {
            sys_error("display_survey_result", 1, "read_file");
            return 0;
        }
        if (close_file(fd) == -1) {
            sys_error("display_survey_result", 1, "close_file");
            return 0;
        }
        /* Get number of replies */

        s = buf;
        i = 0;
        s2 = strchr(s, '\n');
        n = 0;
        while (s2) {
            i++;
            if ((i % th->sh.n_questions) == 0)
                n++;
            s = s2 + 1;
            s2 = strchr(s, '\n');
        }
    }
    /* Display header */

    output("%s %d; %s %s;", MSG_RESULTFOR,
        th->num, MSG_WRITTENBY, username);
    switch (n) {
    case 0:
        output(" %s\n", MSG_NOREPLIES);
        break;
    case 1:
        output(" %s\n", MSG_ONEANSWER);
        break;
    default:
        output(" %d %s\n", n, MSG_ANSWER);
        break;
    }
    time_string(th->sh.time, time_val, Date);
    output("%s: %d; %s: %s\n", MSG_NQUESTIONS, th->sh.n_questions,
        MSG_REPORTRESULT, time_val);
    output("%s%s\n", MSG_SUBJECT, th->subject);
    for (i = 0; i < (strlen(th->subject) + 8); i++)
        c[i] = '-';
    c[i] = '\0';
    output("%s\n", c);

    if (n > 0) {
        s = buf;
        i = 0;
        s2 = strchr(s, '\n');
        while (s2) {
            *s2 = '\0';
            if (output("%s\n", s) == -1)
                break;
            i++;
            if ((i % th->sh.n_questions) == 0)
                if (output("%s\n", MSG_SURVDELIMIT2) == -1)
                    break;
            s = s2 + 1;
            s2 = strchr(s, '\n');
        }
        free(buf);
    }
    output("\n%s %ld %s %s\n\n", MSG_EORESULT,
        th->num, MSG_BY, username);
    free_text_entry(&te);

    return 0;
}
