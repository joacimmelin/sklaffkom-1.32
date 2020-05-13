/* survey.c */

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

/* #define SURVEYDEBUG */

#include <math.h>
#include "sklaff.h"
#include "ext_globals.h"

struct RESSHOW {
  char *entry;
  int val;
};


#ifdef __STDC__

/* static int resshowcompare(struct RESSHOW *, struct RESSHOW *); */
static int parse_survey_line(char *);
static int input_survey_quest(char *, char *, int, int);

#else

static int resshowcompare();
static int parse_survey_line();
static int input_survey_quest();
static int repbufcompare();

#endif


int
make_survey(reply, quest, ll, flag)
char *reply;
int *quest;
LINE ll;
int flag;
{
  int ret, qtype;

  qtype = parse_survey_line(ll);
  if (qtype >= 0) {
    ret = input_survey_quest(ll, reply, qtype, flag);
    (*quest)++;
  } else {
    ret = output("%s\n", ll);
  }

  return ret;
}



static int 
resshowcompare(r1, r2)
struct RESSHOW *r1, *r2;
{
  return(r1->val - r2->val);
}

int 
save_survey_result(survey, conf, survey_result, n_quest)
long survey;
int conf;
char *survey_result;
int n_quest;
{
  int fd, i, j, ret, n;
  LINE confdir, resfile, home;
  char *buf, *fbuf, *t;
  char saveanswer[4], defans[2];
  struct RESSHOW *resbuf;
#ifdef SURVEYDEBUG
  LINE debfile, tist;
  char *debugbuf;
#endif

  ret = 0;
  
  /* Show answers and ask for confirmation */

  output("%s:\n\n", MSG_SURVEYANS);

  for (i=0; i<n_quest; i++)
    output("%d. %s\n", i+1, survey_result + i*LINE_LEN);
  output("\n%s (%c/n):", MSG_SAVESURVANS, MSG_YESANSWER);
  input("", saveanswer, 4, 0, 0, 0);
      
  if (tolower(*saveanswer) == MSG_YESANSWER) {
    
    /* Save answers */

    if (conf >= 0) {
      sprintf(confdir, "%s/%d/", SKLAFF_DB, conf);
      sprintf(resfile, "%s%ld.result", confdir, survey);

      if ((fd = open_file(resfile, OPEN_QUIET)) == -1) {
	if ((fd = open_file(resfile, OPEN_CREATE)) == -1) {
	  sys_error("save_survey_result", 1, "open_file");
	  return -1L;
	}
	
	/* First answer */

	fbuf = (char *) malloc(n_quest*LINE_LEN);

	if (fbuf == NULL) {
	  sys_error("save_survey_result", 1, "malloc");
	  return -1;
	}
	
	bzero(fbuf, n_quest*LINE_LEN);

	for (i=0; i<n_quest; i++) {
	  strcat(fbuf, survey_result + i*LINE_LEN);
	  strcat(fbuf, "\n");
	}
      } else {
	/* Subsequent answers */

	if ((buf = read_file(fd)) == NULL) {
	  sys_error("save_survey_result", 2, "read_file");
	  return -1;
	}

	fbuf = (char *) malloc(strlen(buf) + n_quest*LINE_LEN);

	if (fbuf == NULL) {
	  sys_error("save_survey_result", 1, "malloc");
	  return -1;
	}

	for (n=0, t=strchr(buf,'\n'); t; t = strchr(t+1, '\n'), n++)
	  ;
	
	n = (n+0.25) / n_quest + 1;
	
	resbuf = (struct RESSHOW *) malloc(n * sizeof(struct RESSHOW));
	if (resbuf == NULL) {
	  sys_error("save_survey_result", 1, "malloc");
	  return 0;
	}
	
	resbuf[0].entry = survey_result;
	resbuf[0].val = rand();
	
	t=buf;
	for (i=1; i<n; i++) {
	  resbuf[i].entry = t;
	  resbuf[i].val = rand();
	  for (j=0; j<n_quest; j++)
	    t = strchr(t, '\n')+1;
	  *(t-1) = '\0';
	}

	/* Sort to get random order of results */
	
	qsort(resbuf, n, sizeof(struct RESSHOW), resshowcompare);
	bzero(fbuf, strlen(buf) + n_quest*LINE_LEN);

	for (i=0; i<n; i++) {
	  if (resbuf[i].entry == survey_result) {
	    for (j=0; j<n_quest; j++) {
	      strcat(fbuf, survey_result + j*LINE_LEN);
	      strcat(fbuf, "\n");
	    }
	  } else {
	    strcat(fbuf, resbuf[i].entry);
	    strcat(fbuf, "\n");
	  }
	}
	free(buf);
	free(resbuf);
      }

#ifdef SURVEYDEBUG
      debugbuf = (char *) malloc(strlen(fbuf)+2*LINE_LEN);
      bzero(debugbuf, strlen(fbuf)+2*LINE_LEN);
      memcpy(debugbuf, fbuf, strlen(fbuf));
#endif

      critical(); 

      if (write_file(fd, fbuf) == -1) {
	return -1L;
      }
      
      if (close_file(fd) == -1) {
	return -1L;
      }
      
      non_critical();

#ifdef SURVEYDEBUG
      sprintf(debfile, "%s%ld.%ld.res", confdir, survey, Uid);

      time_string(time(0), tist, 1);
      strcat(debugbuf, tist);
      strcat(debugbuf, "\n");
      sprintf(tist, "n=%d, n_quest=%d", n, n_quest); 
      strcat(debugbuf, tist);
      strcat(debugbuf, "\n");
      if ((fd = open_file(debfile, OPEN_CREATE)) == -1) {
	sys_error("save_survey_result", 1, "open_file");
	return -1L;
      }
      critical(); 

      if (write_file(fd, debugbuf) == -1) {
	return -1L;
      }
      
      if (close_file(fd) == -1) {
	return -1L;
      }
      
      non_critical();
#endif

    }

    output("\n%s.\n\n", MSG_SURVSAVED);
    ret = -1;
  } else
    output("\n%s.\n\n", MSG_SURVNOSAVE);
  return ret;
}

 
static int 
repbufcompare(r1, r2)
double *r1, *r2;
{
  int rr = (*r1 > *r2) ? 1 : ( (*r1 < *r2) ? -1 : 0 );
  return(rr);
}

double 
my_atof(s)
char *s;
{
  char *t, *s1, *s2;
  LINE tmp;
  double d;

  if (strlen(s) == 0)
    return 0.0;

  strcpy(tmp, s);

  /* Note, this atof() is only applicable on NUMBER & INTERVAL.
     We hence know apriori that no spaces, commas, etc. are present. */

  t = strchr(tmp+1, '-');      /* Check for "2-3" constructions */
  
  if (t == NULL) {
    t = strchr(tmp+1, '/');      /* Check for "2/3" constructions */
    if (t == NULL)
      return atof(tmp);

    printf("Found 2/3 constr: [%s] [%s]", tmp, t);
    *t='\0';
    d = my_atof(t+1);
    if (d != 0)
      d = my_atof(tmp) / d;
    else
      d = my_atof(tmp);
    printf(" -> [%s] [%s] -> %g\n", tmp, t+1, d);
    return d;
  }
  printf("Found 2-3 constr: [%s] [%s]", tmp, t);
  *t='\0';
  d = (my_atof(tmp) + my_atof(t+1)) / 2.0;
  printf(" -> [%s] [%s] -> %g\n", tmp, t+1, d);
  return d;
}

static struct TEXT_BODY
*outline(tb, lin, totlen)
struct TEXT_BODY *tb;
LINE lin;
long *totlen;
{
  if (tb != NULL) {
    strcpy(tb->line, lin); 
    tb->next = (struct TEXT_BODY *) malloc (sizeof (struct TEXT_BODY));
    if (tb->next != NULL)
      tb->next->next = NULL;
  }
  *totlen += strlen(lin);
  return tb->next;
}


char
*show_survey_result(survey, conf, tbstart, n_quest)
long survey;
int conf;
struct TEXT_BODY *tbstart;
int n_quest;
{
  struct TEXT_BODY *tb, *tbut;
  struct TEXT_ENTRY te;
  int fd, i, j, k, n, quest, qtype, nalt, bryt, n2;
  LINE confdir, resfile, infofile, home, utlinje, uttmp;
  char *buf, *t, *utbuf, *buf2;
  struct RESSHOW *resbuf;
  long *repbuf;
  double *repbufd;
  long totlen, repi;
  double rep, mn, mx, sm, sm2;
  int fd2;

  te.cl = NULL; /* No comment list, just internal */
  te.body = (struct TEXT_BODY *) malloc (sizeof (struct TEXT_BODY));
  tbut = te.body;
  totlen = 0;

  sprintf(confdir, "%s/%d/", SKLAFF_DB, conf);
  sprintf(resfile, "%s%ld.result", confdir, survey);

  n=-1;
  if ((fd = open_file(resfile, OPEN_QUIET)) == -1) {
    n = 0;
  }

  if (n != 0) {
    if ((buf = read_file(fd)) == NULL) {
      sys_error("show_survey_result", 2, "read_file");
      return NULL;
    }
    if (close_file(fd) == -1) {
      sys_error("show_survey_result", 3, "close_file");
      return NULL;
    }
    
    for (n=0, t=strchr(buf,'\n'); t; t = strchr(t+1, '\n'), n++)
      ;
    
    n = (n+0.25) / n_quest;

    /* Consistency check start */

    sprintf(infofile, "%s%ld.users", confdir, survey);
    if ((fd2 = open_file(infofile, OPEN_QUIET)) == -1) {
      if (n != 0) {
	sprintf(utlinje, "Varning: inkonsistent antal svar (ninfo=0)\n");
	tbut = outline(tbut, utlinje, &totlen);
      }
    } else {
      if ((buf2 = read_file(fd2)) == NULL) {
	sys_error("show_survey_result", 2, "read_file");
	return NULL;
      }
      if (close_file(fd2) == -1) {
	sys_error("show_survey_result", 3, "close_file");
	return NULL;
      }
      for (n2=0, t=buf2; t ;t=strchr(t+1, ']'))
	n2++;
      if (n2-1 != n) {
	sprintf(utlinje, "Varning: inkonsistent antal svar (ninfo=%d)\n", n2-1);
	tbut = outline(tbut, utlinje, &totlen);
      }
    }

    /* Consistency check end */

    resbuf = (struct RESSHOW *) malloc(n * sizeof(struct RESSHOW));
    if (resbuf == NULL) {
      sys_error("show_survey_result", 1, "malloc");
      return NULL;
    }
    
    t=buf;
    for (i=0; i<n; i++) {
      resbuf[i].entry = t;
      for (j=0; j<n_quest; j++) {
	t = strchr(t, '\n')+1;
	*(t-1) = '\0';
      }
    }
  }
  tb = tbstart; quest=0;

  sprintf(utlinje, "%s: %d\n\n", MSG_TOTANSWERS, n);
  tbut = outline(tbut, utlinje, &totlen);
  while (tb) {
    if (tb->line[0] == '\f')  {
      tb->line[0] = '\0';
      Lines = Numlines;
    }
    
    qtype = parse_survey_line(tb->line);
    if (qtype >= 0) {
      sprintf(utlinje, "%s\n", MSG_SURVDELIMIT1);
      tbut = outline(tbut, utlinje, &totlen);

      if (n>0) {
	nalt = -1;
	if (qtype == SURVEY_SINGLECHOICE || qtype == SURVEY_MULTIPLECHOICE) {
	  nalt = atoi(strchr(tb->line, ':') + 1);
	  repbuf = (long *) malloc(nalt * sizeof(long));
	  for (i=0; i<nalt; i++)
	    repbuf[i] = 0;
	}
	
	switch (qtype) {
	case SURVEY_FREETEXT:
	  for (i=0; i<n; i++) {
	    t = resbuf[i].entry;
	    for (j=0; j<quest; j++) 
	      t = t + strlen(t)+1;
	    sprintf(utlinje, "%s\n", t);
	    tbut = outline(tbut, utlinje, &totlen);
	  }
	  break;
	case SURVEY_SINGLECHOICE:
	  for (i=0; i<n; i++) {
	    t = resbuf[i].entry;
	    for (j=0; j<quest; j++) 
	      t = t + strlen(t)+1;
	    repi = atol(t);
	    if (repi>0) 
	      repbuf[repi-1]++;
	  }
	  j=0;
	  for (i=0; i<nalt; i++) {
	    sprintf(utlinje, "%d. %3ld st (%3d%c)\n", i+1, repbuf[i], (int) (100*repbuf[i]/n +0.5), '%');
	    tbut = outline(tbut, utlinje, &totlen);
	    j += repbuf[i];
	  }
	  sprintf(utlinje, "\n%s: %3d st (%3d%c)\n",
		  MSG_BLANKVOTES,
		  n - j, (int) (100*(n-j)/n +0.5), '%');
	  tbut = outline(tbut, utlinje, &totlen);
	  free(repbuf);
	  break;
	case SURVEY_MULTIPLECHOICE:
	  for (i=0; i<n; i++) {
	    t = resbuf[i].entry;
	    for (j=0; j<quest; j++) 
	      t = t + strlen(t)+1;
	    for (j=0; j<nalt; j++)
	      if (strchr(t, 'a'+j))
		repbuf[j]++;
	  }
	  for (i=0; i<nalt; i++) {
	    sprintf(utlinje, "%c. %3ld st (%3d%c)\n", 'a'+i, repbuf[i], (int) (100*repbuf[i]/n +0.5), '%');
	    tbut = outline(tbut, utlinje, &totlen);
	  }
	  free(repbuf);
	  break;
	case SURVEY_FREENUMBER:
	case SURVEY_INTERVAL:
	  mn = 1e10; mx = -mn; sm=0.0; sm2=0.0; nalt=n;
	  for (i=0; i<n; i++) {
	    t = resbuf[i].entry;
	    for (j=0; j<quest; j++) 
	      t = t + strlen(t)+1;
	    if (strlen(t) != 0) {
	      rep = my_atof(t);
	      if (rep < mn) mn = rep;
	      if (rep > mx) mx = rep;
	      sm += rep; sm2 += rep*rep;
	    } else
	      nalt--;
	  }
	  sprintf(utlinje, "%s: %3d st (%3d%c)\n",
		  MSG_NOANSWERS,
		  nalt, (int) (100*(nalt)/n +0.5), '%');
	  tbut = outline(tbut, utlinje, &totlen);
	  sprintf(utlinje, "%s: %3d st (%3d%c)\n",
		  MSG_BLANKVOTES,
		  n - nalt, (int) (100*(n-nalt)/n +0.5), '%');
	  tbut = outline(tbut, utlinje, &totlen);

	  if (nalt > 0) {
	    repbufd = (double *) malloc(nalt * sizeof(double));
	    k=0;
	    for (i=0; i<n; i++) {
	      t = resbuf[i].entry;
	      for (j=0; j<quest; j++) 
		t = t + strlen(t)+1;
	      if (strlen(t) != 0) {
		repbufd[k]= my_atof(t);
		k++;
	      } 
	    }

	    qsort(repbufd, nalt, sizeof(double), repbufcompare);
	    
	    sprintf(utlinje, "%s:\n", MSG_ANSWER2);
	    tbut = outline(tbut, utlinje, &totlen);
	    utlinje[0]='\0';
	    for (i=0; i<nalt; i++) {
	      sprintf(uttmp, "%10g", repbufd[i]);
	      strcat(utlinje, uttmp);
	      if (((i+1) % 6) == 0) {
		strcat(utlinje, "\n");
		tbut = outline(tbut, utlinje, &totlen);
		utlinje[0]='\0';
	      }
	    }
	    if (((i) % 6) != 0) {
	      strcat(utlinje, "\n");
	      tbut = outline(tbut, utlinje, &totlen);
	    }
	    sprintf(utlinje, "\nMin: %g,  Max: %g\n", mn, mx);
	    tbut = outline(tbut, utlinje, &totlen);
	    sprintf(utlinje, "%s: %.4g,  Median: %.4g\n", 
		    MSG_MEAN, (double) sm / nalt,
		    (double) (((nalt % 2) == 0) ? 
			      (repbufd[nalt/2]+repbufd[nalt/2-1])/2 :
			      repbufd[(nalt-1)/2]));
	    tbut = outline(tbut, utlinje, &totlen);
	    if (nalt > 1) {
	      sprintf(utlinje, "%s: %.4g\n", MSG_STD,
		      sqrt( (double) (nalt*sm2-sm*sm) / (nalt*(nalt-1)) ));
	      tbut = outline(tbut, utlinje, &totlen);
	    }
	    free(repbufd);
	  }
	  break;
	}
      }
      sprintf(utlinje, "%s\n", MSG_SURVDELIMIT2);
      tbut = outline(tbut, utlinje, &totlen);
      quest++;
    } else {
      sprintf(utlinje, "%s\n", tb->line);
      tbut = outline(tbut, utlinje, &totlen);
    }
    tb = tb->next;
  }
  utbuf = (char *) malloc(totlen+1);

  if (utbuf == NULL) {
    sys_error("show_survey_result", 2, "malloc");
    return NULL;
  }

  tbut = te.body;
  utbuf[0]=0;
  while(tbut->next) {
    strcat(utbuf, tbut->line);
    tbut = tbut->next;
  }

  free_text_entry(&te);

  if (n>0) {
    free(buf);
    free(resbuf);
  }
  return utbuf;
}

int 
mark_survey_as_taken(survey, conf)
long survey;
int conf;
{
  int fd;
  LINE confdir, infofile, home;
  char *buf, *fbuf;
#ifdef SURVEYDEBUG
  LINE debfile, tist;
  char *debugbuf;
#endif

  if (conf >= 0) {
    sprintf(confdir, "%s/%d/", SKLAFF_DB, conf);
    sprintf(infofile, "%s%ld.users", confdir, survey);

    if ((fd = open_file(infofile, OPEN_QUIET)) == -1) {
      if ((fd = open_file(infofile, OPEN_CREATE)) == -1) {
	sys_error("mark_survey_as_taken", 1, "open_file");
	return -1L;
      }
      fbuf = (char *) malloc(10);
      if (fbuf == NULL) {
	sys_error("mark_survey_as_taken", 1, "malloc");
	return -1;
      }

      bzero(fbuf, 10);
      sprintf(fbuf, "[%d]", Uid);
    } else {
      
      if ((buf = read_file(fd)) == NULL) {
	sys_error("mark_survey_as_taken", 2, "read_file");
	return -1;
      }
      
      fbuf = (char *) malloc(strlen(buf) + 10);
      if (fbuf == NULL) {
	sys_error("mark_survey_as_taken", 1, "malloc");
	return -1;
      }

      bzero(fbuf, strlen(buf) + 10);
      sprintf(fbuf, "%s\n[%d]", buf, Uid);
      free(buf);
    }

#ifdef SURVEYDEBUG
      debugbuf = (char *) malloc(strlen(fbuf)+LINE_LEN);
      bzero(debugbuf, strlen(fbuf)+LINE_LEN);
      memcpy(debugbuf, fbuf, strlen(fbuf));
#endif

    critical(); 

    if (write_file(fd, fbuf) == -1) {
	return -1L;
    }
    
    if (close_file(fd) == -1) {
	return -1L;
    }

    non_critical();

#ifdef SURVEYDEBUG
      sprintf(debfile, "%s%ld.%ld.usr", confdir, survey, Uid);

      time_string(time(0), tist, 1);
      strcat(debugbuf, "\n");
      strcat(debugbuf, tist);
      strcat(debugbuf, "\n");
      if ((fd = open_file(debfile, OPEN_CREATE)) == -1) {
	sys_error("mark_survey_as_taken", 1, "open_file");
	return -1L;
      }
      critical(); 

      if (write_file(fd, debugbuf) == -1) {
	return -1L;
      }
      
      if (close_file(fd) == -1) {
	return -1L;
      }
      
      non_critical();
#endif
  }    
  return -1;
}

int 
check_if_survey_taken(survey, conf)
long survey;
int conf;
{
  int fd, ret;
  LINE confdir, infofile, home;
  char *buf, uidbuf[20];

  ret = -1;
  if (conf >= 0) {
    sprintf(confdir, "%s/%d/", SKLAFF_DB, conf);
    sprintf(infofile, "%s%ld.users", confdir, survey);

    /* If file unavailble, assume survey has not been taken */

    if ((fd = open_file(infofile, OPEN_QUIET)) == -1) {
      /*	sys_error("check_if_survey_taken", 1, "open_file"); */
	return 0;
    }
    
    if ((buf = read_file(fd)) == NULL) {
	sys_error("check_if_survey_taken", 2, "read_file");
	return -1;
    }
    
    if (close_file(fd) == -1) {
	sys_error("check_if_survey_taken", 3, "close_file");
	return -1;
    }

    sprintf(uidbuf, "[%d]", Uid);
    

    if (strstr(buf, uidbuf) == NULL) /* Uid not found in file */
      ret = 0;
    
    free(buf);
  }    
  return ret;
}


time_t 
get_survey_time(cur_tim)
time_t  cur_tim;
{
  int ok,  days, hours, minutes;
  LINE repl;
  struct tm ts;
  
  /*
  memcpy(&ts, localtime(&def_tim), sizeof(struct tm));

  sprintf(def_dat, "%.4d-%.2d-%.2d", ts.tm_year+1900, ts.tm_mon+1, ts.tm_mday);
  sprintf(def_clk, "%.2d:%.2d", ts.tm_hour, ts.tm_min);
  */

  output("%s\n", MSG_WHENREPORT);
  ok = 0;
  while (!ok) {
    output("\n%s: ", MSG_WHENFORMAT);
    input("7:0:0", repl, LINE_LEN, 0, 0, 0);
    sscanf(repl, "%d:%d:%d", &days, &hours, &minutes);
    if (days>=0 && hours>=0 && minutes>=0 && days+hours+minutes>0)
      ok=1;
    else
      output("%s\n", MSG_WRONGFORMAT);
  }

  return cur_tim + ((days*24 + hours)*60 + minutes)*60;
}

int
get_no_survey_questions(fname)
char *fname;
{
  int fd, n;
  char *buf, *sb;

  n = 0;

  if ((fd = open_file(fname, 0)) == -1) {
    sys_error("get_no_survey_questions", 1, "open_file");
    return -1;
  }
  
  if ((buf = read_file(fd)) == NULL) {
    sys_error("get_no_survey_questions", 2, "read_file");
    return -1;
  }
  
  if (close_file(fd) == -1) {
    sys_error("get_no_survey_questions", 3, "close_file");
    return -1;
  }

  sb = strtok(buf, "\n");
  while (sb != NULL) {
    if(parse_survey_line(sb) >= 0)
      n++;
    sb = strtok(NULL, "\n");
  }
  
  free(buf);

  return n;
}


static int
parse_survey_line(lin)
char *lin;
{
  char *s, *s2, *s3;
  int ret;
  long n;

  if (strncmp(lin, "##", 2)) 
    return -1;

  s=lin+2;
  while (*s == ' ')
    s++;
  
  if (tolower(*s) == 't')
    return SURVEY_FREETEXT;

  if (tolower(*s) == 'n')
    return SURVEY_FREENUMBER;

  s2 = strchr(lin, ':');

  if (s2 == NULL)
    return -1;
  
  n = atol(s2+1);
  
  s3 = strchr(s2+1, ':');

  if (s3 != NULL) {
    if (tolower(*s) == 'i' && n < atol(s3+1)) 
      return SURVEY_INTERVAL;
  }

  /*   if (s3 == NULL && (n < 2 || n > 26))      Obsolete 
    return -1;  */

  if (n < 2 || n > 26)
    return -1;

  if (tolower(*s) == 's')
    return SURVEY_SINGLECHOICE;
  if (tolower(*s) == 'm')
    return SURVEY_MULTIPLECHOICE;

  return -1;
}

static int
input_survey_quest(lin, reply, qtype, flag)
LINE lin, reply;
int qtype, flag;
{
  char *s, *strim, *s2;
  long n1, n2;
  int i, j, t, ret;

  ret = 0;
  if (qtype != SURVEY_FREETEXT && qtype != SURVEY_FREENUMBER) {
    s2 = strchr(lin, ':');
    s2 = strchr(s2+1, ':');
  }

  s = strtok(lin, ":");

  switch (qtype) {
  case SURVEY_FREETEXT:
  case SURVEY_FREENUMBER:
    ret = output("? ");
    break;
  case SURVEY_SINGLECHOICE:
    s = strtok(NULL, ":");
    if (s2 != NULL) {
      /*      i = 0;          This is an obsolete format
      while (s) {
	strim = s + strspn(s, " ");
	ret = output("%d. %s\n", i+1, strim);
	if (ret == -1) break;
	s = strtok(NULL, ":");
	i++;
      }
      output("\n");
      */
    } else
      i = atoi(s);
    output("(1 - %d)? ", i);
    break;
  case SURVEY_MULTIPLECHOICE:
    s = strtok(NULL, ":");
    if (s2 != NULL) {
      /* i = 0;        This is an obsolete format
      while (s) {
	strim = s + strspn(s, " ");
	ret = output("%c. %s\n", 'a'+i, strim);
	if (ret == -1) break;
	s = strtok(NULL, ":");
	i++;
      }
      output("\n"); */
    } else
      i = atoi(s);
    output("(a - %c)? ", 'a'+i-1);
    break;
  case SURVEY_INTERVAL:
    s = strtok(NULL, ":");
    n1 = atol(s); 
    s = strtok(NULL, ":");
    n2 = atol(s);
    output("(%ld - %ld)? ", n1, n2);
    break;
  }

  if ((flag & TAKE_SURVEY) && (ret != -1)) {
    switch (qtype) {
    case SURVEY_FREETEXT:
      ret=input_extended("", reply, LINE_LEN, 0, 0, 0, 0, 255);
      break;
    case SURVEY_FREENUMBER:   /* Floats & negatives ok */
      ret=input_extended("", reply, LINE_LEN, 0, 0, 0, '-', '9');
      break;
    case SURVEY_SINGLECHOICE:
      ret=input_extended("", reply, LINE_LEN, 0, 0, 0, '0', '9');
      while ((atol(reply)< 1 || atol(reply) > i)  && *reply != '\0' && ret!=-1) {
	output("(1 - %d)? ", i);
	ret=input_extended("", reply, LINE_LEN, 0, 0, 0, '0', '9');
      } 
      break;
    case SURVEY_MULTIPLECHOICE:
      ret=input_extended("", reply, LINE_LEN, 0, 0, 0, 'a', 'a'+i-1);
      for (;ret!=-1;) {
	t=1;
	/* Check if characters appear more than once */
	for (j=0; j<strlen(reply); j++) {
	  t = reply[j]; reply[j]=' ';
	  if (strchr(reply, t) != NULL) {
	    t=0; break;
	  }
	  reply[j]=t;
	}
	if (t != 0)
	  break;
	output("(a - %c)? ", 'a'+i-1);
	ret=input_extended("", reply, LINE_LEN, 0, 0, 0, 'a', 'a'+i-1);
      }
      break;
    case SURVEY_INTERVAL:  /* Only integers */
      ret=input_extended("", reply, LINE_LEN, 0, 0, 0, '0', '9');
      while ((atol(reply)< n1 || atol(reply) > n2) && *reply != '\0' && ret!=-1) {
	output("(%ld - %ld)? ", n1, n2);
	ret=input_extended("", reply, LINE_LEN, 0, 0, 0, '0', '9');
      } 
      break;
    }
  } else
    output("\n");

  return ret;
}
