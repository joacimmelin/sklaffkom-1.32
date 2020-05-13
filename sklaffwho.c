/* sklaffadm.c */

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
#include "globals.h" 

int main(int argc, char *argv[])
{
  LINE namearg, expname;
  char *t;
  int what, num;

  /*      int i;
  for (i=0; i<argc; i++)
    printf("%d. [%s]\n", i,  argv[i]);
   
  printf("TEST: argc=%d\n", argc);
  */

  Iso8859 = 1;

  if (argc == 2)
    list_who(WHO_EXTERNAL);
  else {
    strcpy(namearg, argv[2]);
    rtrim(namearg);
    for (t=namearg; *t; t++)
      if (*t =='.')
	*t=' ';
    what=0;
    t = expand_name(namearg, USER, 1, &what);

    if (!t)
      exit(0);
    strcpy(expname, t);
    num = user_uid(expname);
    show_status(num, USER, STATUS_EXTERNAL);
  }

  /*  for (i=0; i<10; i++)
    namearg[i]=160+i;
  namearg[10]=0;
  
  printf("TEST2: [%s]\n", namearg); */

    exit(0);
}



