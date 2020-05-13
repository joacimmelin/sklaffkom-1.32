/* sklaff.h */

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

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <time.h>

/* Define target system, one of: BSD, SUNOS, SYSV, DG_UX, UNIXWARE,
                                 LINUX, ULTRIX, or MT_XINU 	*/

/* #define BSD */ 		/* BSD something or other 	*/
#define FREEBSD			/* FreeBSD			*/
/* #define SUNOS */		/* SunOS			*/
/* #define SOLARIS */ 		/* Solaris			*/
/* #define SYSV */		/* ISC3 or other basic SYSVR3	*/
/* #define DG_UX */		/* DG/UX or other SYSVR4	*/
/* #define UNIXWARE */		/* Unixware			*/
/* #define LINUXOLD */ 		/* Linux			*/
/* #define LINUXELF */ 		/* Linux with ELF		*/
/* #define ULTRIX */		/* Ultrix			*/
/* #define MT_XINU */		/* MT_XINU for VAXen		*/

/* Define language desired, one of: SWEDISH or ENGLISH */

#define SWEDISH
/* #define ENGLISH */

/* Define group ids and terminaltypes for modem_pool */

#define MODEM_POOL 	"silly"
#define MODEM_GROUP	50
#define INET_GROUP	60

/* Allow news postings in news-conferences */

/* #undef POSTING_OK */
/* #define POSTING_OK */

/* Machine name for news postings */

#define MACHINE_NAME	"skom.se"

/* User to mail for new accounts */

#define SKLAFF_ACCT	"sklaff"

/* For "batch" command */

#define SKLAFF_ID	"SKOM II"
#define SKLAFF_LOC	"Stockholm, Sweden"
#define SKLAFF_NUM	"+46-8-7021174"
#define SKLAFF_SYSOP	"Carl Sundbom"

/* User ID of survey reporter account */

#define SKLAFF_SURVEY_REPORTER 5000

/* Default terminal used for clear screen if TERM not correct */

#define NO_TERM	        "unknown"
#define SKLAFF_TERM	"vt100"

/* Default number of days before texts expire */

#define EXP_DEF		10		/* Ordinary confs */
#define EXP_DEF_NEWS	 3		/* News confs	  */

/* System files used by SklaffKOM */

#define UTMP_REC 	"/var/run/utmp"
#define MAIL_LIB	"/var/mail"
#define NEWS_SPOOL	"/var/spool/news"
#define NEWS_GROUPS	"/var/lib/news/active"

/* Programs used by SklaffKOM */

#define SKLAFFSHELL	"/bin/sh"
#define SKLAFFPASSWD 	"/usr/bin/passwd"
#define SKLAFFGREP 	"/usr/bin/grep"
#define GREPOPT		"-i -F"		      /* option to grep */
#define SKLAFFAT 	"/usr/bin/at"
#define SKLAFFECHO 	"/bin/echo"
#define UPLOADPRGM	"/usr/bin/rz"
#define ULOPT1          ""
#define DOWNLOADPRGM	"/usr/bin/sz"
#define DLOPT1          ""
#define DLOPT2          ""
#define DLOPT3          ""
#define ZIPPRGM		"/usr/bin/zip -9"
#define LSPRGM		"/bin/ls"
#define LSOPT		"-1"		      /* option to make one column */
#define MVPRGM		"/bin/mv"
#define MAILPRGM	"/usr/sbin/sendmail --"
#define NEWSPRGM	"/usr/bin/inews"
#define SURVREPORT 	"/usr/local/bin/srep"

/* SklaffKOM-files */

#define SKLAFF_DB 	"/usr/local/sklaff/db"
#define USER_DB		"/usr/local/sklaff/user"
#define FILE_DB		"/usr/local/sklaff/files"
#define MBOX_DB		"/usr/local/sklaff/mbox"
#define LOGDIR   	"/usr/local/sklaff/log"

#define USER_FILE 	"/usr/local/sklaff/etc/user"
#define	ACTIVE_FILE 	"/usr/local/sklaff/etc/active"
#define CONF_FILE 	"/usr/local/sklaff/etc/conf"
#define NEWS_FILE 	"/usr/local/sklaff/etc/news"
#define INFO_FILE	"/usr/local/sklaff/etc/info"
#define LICENS_FILE	"/usr/local/sklaff/etc/COPYING"
#define DOWN_FILE 	"/usr/local/sklaff/etc/down"
#define PAY_FILE	"/usr/local/sklaff/etc/pay"
#define INET_FILE	"/usr/local/sklaff/etc/inet"
#define ACCT_FILE	"/usr/local/sklaff/etc/newacct"
#define ACCT_LOG	"/usr/local/sklaff/etc/acctlog"
#define STD_CONFS 	"/usr/local/sklaff/etc/stdconfs"
#define GLOBAL_SKLAFFRC "/usr/local/sklaff/etc/sklaffrc"
#define STD_SKLAFFRC 	"/usr/local/sklaff/etc/stdsklaffrc"
#define POST_INFO	"/usr/local/sklaff/etc/postnews"

#ifdef SWEDISH
#define PARSE_FILE 	"/usr/local/sklaff/etc/parse.swe"
#define STD_MAILBOX 	"/usr/local/sklaff/etc/stdmailbox.swe"
#define HELP_DIR	"/usr/local/sklaff/etc/help.swe"
#define HELP_FILE	"/usr/local/sklaff/etc/help.swe/general.help"
#else
#define PARSE_FILE 	"/usr/local/sklaff/etc/parse.eng"
#define STD_MAILBOX 	"/usr/local/sklaff/etc/stdmailbox.eng"
#define HELP_DIR	"/usr/local/sklaff/etc/help.eng"
#define HELP_FILE	"/usr/local/sklaff/etc/help.eng/general.help"
#endif

#define CONFRC_FILE 	"/confrc"

#define INDEX_FILE	"/.index"
#define NEWINDEX_FILE	"/.newindex"

#define MSG_FILE 	"/msg"
#define CONFS_FILE 	"/confs"
#define SKLAFFRC_FILE 	"/sklaffrc"
#define MAILBOX_FILE 	"/mailbox"
#define EDIT_FILE 	"/text.sklaff"
#define DEAD_FILE 	"/dead.sklaff"
#define TMP_NOTE 	"/tmplapp"
#define ACT_FILE        "/act"

/* Log level (lower means less logs, 0 = no logs) */

#define LOGLEVEL 6

/* Change nothing below this line (unless it won't compile) */

#ifndef MT_XINU
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#endif

#ifdef LINUXELF
#define LINUX
#endif

#ifdef LINUXOLD
#define LINUX
#endif

#ifdef DG_UX
#define SYSV
#endif

#ifdef UNIXWARE
#define SYSV
#endif

#ifdef SYSV
#define SYSVTERMIO
#else
#define BSDTERMIO
#endif

#ifdef SUNOS
#define BSD
#include <syscall.h>
#endif

#ifdef SOLARIS
#define BSD
#define BSD_COMP
#include <sys/syscall.h>
#endif

#ifdef FREEBSD
#define BSD
#define COMPAT_SUNOS
#include <sys/ioctl.h>
#include <termios.h>
#include <sys/ttycom.h>
#endif

#ifdef LINUX
#undef BSDTERMIO
#define SYSVTERMIO
#define BSD
#include <syscall.h>
#endif

#ifdef ULTRIX
#undef BSDTERMIO
#define SYSVTERMIO
#define BSD
#endif

#ifdef MT_XINU
#define BSD
typedef unsigned short mode_t;
#endif

#ifdef BSD
#define index strchr
#define rindex strrchr
#endif

#ifdef SYSV
#ifndef DG_UX
#ifndef UNIXWARE
typedef unsigned short mode_t;
#endif
#endif
#endif

#ifdef SYSV
#define GETCWD
#endif
#ifdef LINUX
#define GETCWD
#endif

/********************/
/* This is a kludge */
/********************/

#ifdef SYSV
#ifndef DG_UX
#define ut_host ut_line
#ifdef __STDC__
#define L_ctermid 9
#endif
#endif
#endif

/* End of kludge */

/* Misc defines */

#define LINE_LEN 	79
#define SUBJECT_LEN 	70
#define LONG_LINE_LEN 	2 * LINE_LEN
#define HUGE_LINE_LEN 	32768
#define MAX_COMMANDS 	256
#define HISTORY_SIZE    20
#define NEW_FILE_MODE 	0600
#define NEW_DIR_MODE 	0700
#define SIGNAL_NEW_TEXT SIGUSR1
#define SIGNAL_NEW_MSG  SIGUSR2
#define FROM_FIELD_LEN  28
#define IDLE_RESOLUTION 60
#define IDLE_LIMIT      3

/* Defines for messages */

#define	MSG_SAY		1
#define	MSG_YELL	2
#define	MSG_LOGIN	3
#define MSG_LOGOUT	4
#define	MSG_I           5
#define	MSG_SMS         6
#define	MSG_MY          7

/* Defines for open_file */

#define OPEN_CREATE	1
#define OPEN_QUIET	2
#define OPEN_DEFAULT	0

/* Defines for conf_type */

#define OPEN_CONF	0
#define CLOSED_CONF	1
#define SECRET_CONF	2
#define NEWS_CONF	3

/* Defines for text types */

#define TYPE_TEXT		0
#define TYPE_SURVEY		1

/* Defines for expand_name */

#define USER		0x01	/* to search among users */
#define CONF   		0x02	/* to search among confs. */
#define ACTIVE 		0x04	/* to search among logged in users */
#define SUBSCRIBED	0x08	/* to search among subscribed confs. */
#define UNSUBSCRIBED	0x10	/* to search among unsubscribed confs. */
#define ALSOERASED      0x20    /* to search also among erased confs */

/* Defines for surveys */

#define SURVEY_SINGLECHOICE    0
#define SURVEY_MULTIPLECHOICE  1
#define SURVEY_FREETEXT        2
#define SURVEY_INTERVAL        3
#define SURVEY_FREENUMBER      4

#define TAKE_SURVEY        1
#define SHOW_SURVEY_RESULT 2

/* Other defines */

#define STATUS_INTERNAL   0
#define STATUS_EXTERNAL   1
#define WHO_INTERNAL 0
#define WHO_EXTERNAL 1


/* Typedefs */

typedef char LINE[LINE_LEN + 1];
typedef char LONG_LINE[LONG_LINE_LEN + 1];
typedef char HUGE_LINE[HUGE_LINE_LEN];

#include "struct.h"
#include "lang.h"

#ifdef __STDC__

/* lib-funktioner */

int cmp_strings (char *, char *);
int copy_file (char *, char *);
void critical (void);
char *down_string (char *);
int file_exists (char *);
off_t file_size (int);
char *user_dir (int, char *);
char *in_string (char *, char *);
char *get_hostname (void);
void haffo (int);
void baffo (int);
char *input (char *, char *, int, int, int, int);
int input_extended (char *, char *, int, int, int, int, int, int);
void lock (int);
void non_critical (void);
int output (char *,...);
int outputex (char *,...);
int close_file (int);
int create_file (char *);
int open_file (char *, int);
int parse_strings (char *, char *, int, char *);
char *read_file (int);
int write_file (int, char *);
char *rtrim (char *);
char *ltrim (char *);
void sig_reset (void);
void sig_setup (void);
void sys_error (char *, int, char *);
char *time_string (time_t, char *, int);
void tty_raw (void);
void tty_reset (void);
void unlock (int);
char *up_string (char *);
int wc (char *);
char *fake_string (char *);
char *order_name (char *, char *);
char *real_string (char *);
char *reorder_name (char *, char *);
char *prog_name (char *);
char *mbox_dir (int, char *);

/* Debugging */
/* void *my_malloc (size_t); */
/* void my_free (void *);    */

/* commands.c */

int cmd_add_rights (char *);
int cmd_change_conf (char *);
int cmd_comment (char *);
int cmd_create_conf (char *);
int cmd_delete_conf (char *);
int cmd_delete_text (char *);
int cmd_display_commented (char *);
int cmd_display_last (char *);
int cmd_display_rot13 (char *);
int cmd_display_time (char *);
int cmd_end_session (char *);
int cmd_restart (char *);
int cmd_on_flag (char *);
int cmd_off_flag (char *);
int cmd_goto_text (char *);
int cmd_help (char *);
int cmd_info (char *);
int cmd_jump_over (char *);
int cmd_jump_tree (char *);
int cmd_licens (char *);
int cmd_list_confs (char *);
int cmd_list_flags (char *);
int cmd_list_last (char *);
int cmd_list_member (char *);
int cmd_list_news (char *);
int cmd_list_rights (char *);
int cmd_list_subj (char *);
int cmd_list_users (char *);
int cmd_long_help (char *);
int cmd_mail (char *);
int cmd_mod_note (char *);
int cmd_mod_sig (char *);
int cmd_mod_login (char *);
int cmd_mod_pinfo (char *);
int cmd_mod_timeout (char *);
int cmd_next_comment (char *);
int cmd_next_conf (char *);
int cmd_next_text (char *);
int cmd_only (char *);
int cmd_personal (char *);
int cmd_post_text (char *);
int cmd_read_text (char *);
int cmd_whole_text (char *);
int cmd_say (char *);
int cmd_show_status (char *);
int cmd_sub_rights (char *);
int cmd_subscribe (char *);
int cmd_unread_text (char *);
int cmd_unread_tree (char *);
int cmd_unsubscribe (char *);
int cmd_where (char *);
int cmd_who (char *);
int cmd_yell (char *);
int cmd_change_cname (char *);
int cmd_change_comc (char *);
int cmd_change_passwd (char *);
int cmd_cls (char *);
int cmd_grep (char *);
int cmd_global_grep (char *);
int cmd_list_files (char *);
int cmd_download (char *);
int cmd_upload (char *);
int cmd_unlink (char *);
int cmd_describe (char *);
int cmd_I (char *);
int cmd_my (char *);
int cmd_prio (char *);
int cmd_deprio (char *);
int cmd_back_text (char *);
int cmd_readall (char *);
int cmd_readsome (char *);
int cmd_sendbatch (char *);
int cmd_survey_result (char *);
int cmd_unreadsub (char *);
int cmd_jumpsub (char *);
int cmd_jumpuser (char *);
int cmd_answermsg (char *);
int cmd_from (char *);
int cmd_list_says (char *);
int cmd_list_yells (char *);
int cmd_read_last_text (char *);

/* admin.c */

char *display_prompt (char *, char *, int);
void display_welcome (void);
void display_news (void);
void check_open (void);
void out_onoff (int);
int grep (int, char *);
void logout (int);
void exec_login (void);
void timeout (int);
void debuglog(char *, int);
int strip_string(char *, char*);
int show_status (int, int, int);
int list_who(int);

/* buf.c */


char *get_active_entry (char *, struct ACTIVE_ENTRY *);
char *get_conf_entry (char *, struct CONF_ENTRY *);
struct USER_LIST *get_confrc_users (char *);
char *get_confs_entry (char *, struct CONFS_ENTRY *);
char *get_msg_entry (char *, struct MSG_ENTRY *);
char *get_parse_entry (char *, struct PARSE_ENTRY *);
char *get_text_entry (char *, struct TEXT_ENTRY *);
char *get_user_entry (char *, struct USER_ENTRY *);
char *get_file_entry (char *, struct FILE_ENTRY *);

/* conf.c */

char *conf_name (int, char *);
int conf_num (char *);
int can_see_conf (int, int, int, int);
int is_conf_creator (int, int);
int set_conf (int);
int set_first_conf ();
int member_of (int, int);
void free_confs_entry (struct CONFS_ENTRY *);
void free_userlist (struct USER_LIST *);
struct CONF_ENTRY *get_conf_struct (int);
struct USER_LIST *get_confrc_struct (int);
char *replace_conf (struct CONF_ENTRY *, char *);
char *replace_confs (struct CONFS_ENTRY *, char *);
char *stringify_confs_struct (struct CONFS_ENTRY *, char *);
char *stringify_conf_struct (struct CONF_ENTRY *, char *);
long num_unread (int, int, long);
int conf_right (struct USER_LIST *, int, int, int);
int list_confs (int, int);
int more_conf (void);
int force_unsubscribe (void);
long last_text (int, int);
long first_text (int, int);
struct CEN *sort_conf (struct CEL *, int);
int list_news (int);
struct CONF_ENTRY *get_all_confs(void);

/* edit.c */

void abort_edit (int);
int resume_aborted_edit (void);
struct TEXT_HEADER *line_ed (char *, struct TEXT_HEADER *, int, int,
			     int, int *, char *);

/* file.c */

int rebuild_index_file (void);

/* flag.c */

void set_flags (char *);
int check_flag (char *, char *);
int turn_flag (int, char *);

/* msg.c */

void add_to_mlist (struct MSG_ENTRY *);
void list_mlist (int, int);
void newmsg (int);
int display_msg (int);
int send_msg (int, int, char *, int);
int send_msg_to_all (int, char *);
void notify_all_processes (int);
void notify_user (int, int);

/* parse.c */

int (*parse (char *, char *)) ();
void buggy_sunOS_fix (int);
int parse_init (char *);
char *expand_name (char*, int, int, int*);

/* sklaffkom.c */

void main (int, char**);

/* text.c */

int check_if_read (long, int);
void clear_comment (void);
void display_header (struct TEXT_HEADER *, int, int, int, char *);
int display_text (int, long, int, int);
int display_survey_result (int, long);
int list_subj (char *);
int mark_as_read (long, int);
int mark_as_unread (long, int);
long next_text (int);
long parse_text (char *);
long pop_comment (void);
int push_comment (long);
long pop_unread (int *);
int push_unread (int, long);
long pop_unread2 (int *);
int push_unread2 (int, long);
long pop_read (int *);
int push_read (int, long);
int stack_text (long);
int tree_top (long);
long save_text (char *, struct TEXT_HEADER *, int);
int more_comment (void);
int more_text (void);
void free_text_entry (struct TEXT_ENTRY *);
long save_mailcopy (char *, char *, char *);
void cnvnat(char *, int);
void int2ms(int, char *);
long age_to_textno(long);

/* user.c */

struct USER_ENTRY *get_user_struct (int);
time_t last_session (int);
int user_is_active (int);
int user_is_avail (int);
char *replace_active (struct ACTIVE_ENTRY *, char *);
int set_avail (int, int);
int set_from (int, char *);
char *user_name (int, char *);
int user_uid (char *);
int setup_new_user (void);
char *stringify_user_struct (struct USER_ENTRY *, char *);
int add_active (void);
int remove_active (void);
struct ACTIVE_ENTRY *check_active (int, struct ACTIVE_ENTRY *);
long active_time (int);
long idle_time (int);
void make_activity_note ();
int disp_note (int);
int list_user (int, int, int);
struct UEN *sort_user (struct UEL *, int);
struct SKLAFFRC *read_sklaffrc (int);
int write_sklaffrc (int, struct SKLAFFRC *);

/* survey.c */

int make_survey(char *, int *, LINE, int);
int save_survey_result(long, int, char *, int);
char *show_survey_result(long, int, struct TEXT_BODY *, int);
int mark_survey_as_taken(long, int);
int check_if_survey_taken(long, int);
time_t get_survey_time(time_t);
int get_no_survey_questions(char *);

#else

/* lib-funktioner */

int cmp_strings ();
int copy_file ();
void critical ();
char *down_string ();
int file_exists ();
off_t file_size ();
char *user_dir ();
char *in_string ();
char *get_hostname ();
void haffo ();
void baffo ();
char *input ();
int input_extended ();
void lock ();
void non_critical ();
int output ();
int outputex ();
int close_file ();
int create_file ();
int open_file ();
int parse_strings ();
char *read_file ();
int write_file ();
char *rtrim ();
char *ltrim ();
void sig_reset ();
void sig_setup ();
void sys_error ();
char *time_string ();
void tty_raw ();
void tty_reset ();
void unlock ();
char *up_string ();
int wc ();
char *fake_string ();
char *order_name ();
char *real_string ();
char *reorder_name ();
char *prog_name ();
char *mbox_dir ();

/* commands.c */

int cmd_add_rights ();
int cmd_change_conf ();
int cmd_comment ();
int cmd_create_conf ();
int cmd_delete_conf ();
int cmd_delete_text ();
int cmd_display_commented ();
int cmd_display_last ();
int cmd_display_rot13 ();
int cmd_display_time ();
int cmd_end_session ();
int cmd_restart ();
int cmd_on_flag ();
int cmd_off_flag ();
int cmd_goto_text ();
int cmd_help ();
int cmd_info ();
int cmd_jump_over ();
int cmd_jump_tree ();
int cmd_licens ();
int cmd_list_confs ();
int cmd_list_flags ();
int cmd_list_last ();
int cmd_list_member ();
int cmd_list_news ();
int cmd_list_rights ();
int cmd_list_subj ();
int cmd_list_users ();
int cmd_long_help ();
int cmd_mail ();
int cmd_mod_note ();
int cmd_mod_sig ();
int cmd_mod_login ();
int cmd_mod_pinfo ();
int cmd_mod_timeout ();
int cmd_next_comment ();
int cmd_next_conf ();
int cmd_next_text ();
int cmd_only ();
int cmd_personal ();
int cmd_post_text ();
int cmd_read_text ();
int cmd_whole_text ();
int cmd_say ();
int cmd_show_status ();
int cmd_sub_rights ();
int cmd_subscribe ();
int cmd_unread_text ();
int cmd_unread_tree ();
int cmd_unsubscribe ();
int cmd_where ();
int cmd_who ();
int cmd_yell ();
int cmd_change_cname ();
int cmd_change_comc ();
int cmd_change_passwd ();
int cmd_cls ();
int cmd_grep ();
int cmd_global_grep ();
int cmd_list_files ();
int cmd_download ();
int cmd_upload ();
int cmd_unlink ();
int cmd_describe ();
int cmd_I ();
int cmd_my ();
int cmd_prio ();
int cmd_deprio ();
int cmd_back_text ();
int cmd_readall ();
int cmd_readsome ();
int cmd_sendbatch ();
int cmd_survey_result ();
int cmd_unreadsub ();
int cmd_jumpsub ();
int cmd_jumpuser ();
int cmd_answermsg ();
int cmd_from ();
int cmd_list_says ();
int cmd_list_yells ();
int cmd_read_last_text ();

/* admin.c */

char *display_prompt ();
void display_welcome ();
void display_news ();
void check_open ();
void out_onoff ();
int grep ();
void logout ();
void exec_login ();
void timeout ();
void debuglog();
int strip_string();
int show_status();
int list_who();

/* buf.c */

char *get_active_entry ();
char *get_conf_entry ();
struct USER_LIST *get_confrc_users ();
char *get_confs_entry ();
char *get_msg_entry ();
char *get_parse_entry ();
char *get_text_entry ();
char *get_user_entry ();
char *get_file_entry ();

/* conf.c */

char *conf_name ();
int conf_num ();
int can_see_conf ();
int is_conf_creator ();
int set_conf ();
int set_first_conf ();
int member_of ();
void free_confs_entry ();
void free_confrc_entry ();
struct CONF_ENTRY *get_conf_struct ();
struct USER_LIST *get_confrc_struct ();
char *replace_conf ();
char *replace_confs ();
char *stringify_confs_struct ();
char *stringify_conf_struct ();
long num_unread ();
int conf_right ();
int list_confs ();
int more_conf ();
int force_unsubscribe ();
long last_text ();
long first_text ();
struct CEN *sort_conf ();
int list_news ();
struct CONF_ENTRY *get_all_confs();

/* edit.c */

void abort_edit ();
int resume_aborted_edit ();
struct TEXT_HEADER *line_ed ();

/* file.c */

int rebuild_index_file ();

/* flag.c */

void set_flags ();
int check_flag ();
int turn_flag ();

/* msg.c */

void newmsg ();
int display_msg ();
int send_msg ();
int send_msg_to_all ();
void notify_all_processes ();
void notify_user ();

/* parse.c */

int (*parse ()) ();
void buggy_sunOS_fix ();
int parse_init ();
char *expand_name ();

/* sklaffkom.c */

void main ();

/* text.c */

int check_if_read ();
void clear_comment ();
void display_header ();
int display_text ();
int display_survey_result ();
int list_subj ();
int mark_as_read ();
int mark_as_unread ();
long next_text ();
long parse_text ();
long pop_comment ();
int push_comment ();
long pop_unread ();
int push_unread ();
long pop_unread2 ();
int push_unread2 ();
long pop_read ();
int push_read ();
int stack_text ();
int tree_top ();
long save_text ();
int more_comment ();
int more_text ();
void free_text_entry ();
long save_mailcopy ();
void cnvnat();
void int2ms();
long age_to_textno();

/* user.c */

struct USER_ENTRY *get_user_struct ();
time_t last_session ();
int user_is_active ();
int user_is_avail ();
char *replace_active ();
int set_avail ();
char *user_name ();
int user_uid ();
int setup_new_user ();
char *stringify_user_struct ();
int add_active ();
int remove_active ();
struct ACTIVE_ENTRY *check_active ();
long active_time ();
void make_activity_note ();
long idle_time ();
int disp_note ();
int list_user ();
struct UEN *sort_user ();
struct SKLAFFRC *read_sklaffrc ();
int write_sklaffrc ();

/* survey.c */

int make_survey();
int save_survey_result();
char *show_survey_result();
int mark_survey_as_taken();
int check_if_survey_taken();
time_t get_survey_time();
int get_no_survey_questions();

#endif /* __STDC__ */
