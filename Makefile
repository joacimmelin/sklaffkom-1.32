#
# Makefile for SklaffKOM
#

SKLAFFBIN=/usr/local/bin
SKLAFFDIR=/usr/local/sklaff

#CC=gcc
#CFLAGS=-O2 -m486
CFLAGS=-O2 -pipe -Wall -g -fcommon

LDFLAGS=-static -Wl,--allow-multiple-definition,--unresolved-symbols=ignore-all

# uncomment for SYSV
#LIBS=-lc_s -lsklaff -ltermcap -lcposix -linet -lm

# uncomment for DG_UX
#LIBS=-lsklaff -ltermcap -lelf -lm

# uncomment for UNIXWARE
#LIBS=-lsklaff -ltermcap -lelf lib/bzero.o -lm

# uncomment for BSD/SUNOS/LINUXOLD/ULTRIX
LIBS=-lsklaff -ltermcap -lm

# uncomment for SOLARIS
#CFLAGS=-g -I/usr/ucbinclude
#LIBS=-L/usr/ucblib -lm -lsklaff -ltermcap -lucb -lsocket -lnsl -lelf -laio

# uncomment for LINUXELF
#LIBS=-lsklaff -ltermcap -lelf -lm

#
# Change nothing below this line
#

OBJS=admin.o buf.o commands.o conf.o edit.o file.o flag.o msg.o parse.o\
survey.o text.o user.o version.o
KOMOBJ=sklaffkom.o
ADMOBJ=sklaffadm.o
WHOOBJ=sklaffwho.o
ACCTOBJ=sklaffacct.o
MTOSSOBJ=mailtoss.o
NTOSSOBJ=newstoss.o
SURVREPOBJ=survreport.o
FTCOBJ=forwardtoconf.o
FTYOBJ=forwardtoyell.o

SKLAFFLIB=lib/libsklaff.a

all: sklafflib sklaffkom sklaffadm sklaffacct mailtoss survreport sklaffwho

$(OBJS): sklaff.h ext_globals.h struct.h lang.h
$(KOMOBJ): sklaff.h globals.h struct.h lang.h
$(ADMOBJ): sklaff.h globals.h struct.h lang.h
$(WHOOBJ): sklaff.h globals.h struct.h lang.h
$(ACCTOBJ): sklaff.h globals.h struct.h lang.h
$(MTOSSOBJ): sklaff.h globals.h struct.h lang.h
$(NTOSSOBJ): sklaff.h globals.h struct.h lang.h
$(SURVREPOBJ): sklaff.h globals.h struct.h lang.h
$(FTCOBJ): sklaff.h globals.h struct.h lang.h
$(FTYOBJ): sklaff.h globals.h struct.h lang.h

sklaffkom: $(SKLAFFLIB) $(KOMOBJ) $(OBJS)
	$(CC) -g -o sklaffkom $(KOMOBJ) $(OBJS) $(LDFLAGS) -Llib $(LIBS)
	#chmod u+s sklaffkom
	#chown sklaff sklaffkom

sklaffadm: $(SKLAFFLIB) $(ADMOBJ) $(OBJS)
	$(CC) -o sklaffadm $(ADMOBJ) $(OBJS) $(LDFLAGS) -Llib $(LIBS)
	strip sklaffadm
	chmod og-rwx sklaffadm

sklaffwho: $(SKLAFFLIB) $(WHOOBJ) $(OBJS)
	$(CC) -o sklaffwho $(WHOOBJ) $(OBJS) $(LDFLAGS) -Llib $(LIBS)
	strip sklaffwho
	#chmod u+s sklaffwho

sklaffacct: $(SKLAFFLIB) $(ACCTOBJ) $(OBJS)
	$(CC) -o sklaffacct $(ACCTOBJ) $(OBJS) $(LDFLAGS) -Llib $(LIBS)
	strip sklaffacct
	#chmod u+s sklaffacct

mailtoss: $(SKLAFFLIB) $(MTOSSOBJ) $(OBJS)
	$(CC) -o mailtoss $(MTOSSOBJ) $(OBJS) $(LDFLAGS) -Llib $(LIBS)
	strip mailtoss

newstoss: $(SKLAFFLIB) $(NTOSSOBJ) $(OBJS)
	$(CC) -o newstoss $(NTOSSOBJ) $(OBJS) $(LDFLAGS) -Llib $(LIBS)
	strip newstoss

survreport: $(SKLAFFLIB) $(SURVREPOBJ) $(OBJS)
	$(CC) -o survreport $(SURVREPOBJ) $(OBJS) $(LDFLAGS) -Llib $(LIBS)
	strip survreport

forwardtoconf: $(SKLAFFLIB) $(FTCOBJ) $(OBJS)
	$(CC) -o forwardtoconf $(FTCOBJ) $(OBJS) $(LDFLAGS) -Llib $(LIBS)
	strip forwardtoconf

forwardtoyell: $(SKLAFFLIB) $(FTYOBJ) $(OBJS)
	$(CC) -o forwardtoyell $(FTYOBJ) $(OBJS) $(LDFLAGS) -Llib $(LIBS)
	strip forwardtoyell

version.c:
	@./version.sh

sklafflib:
	(cd lib; make CC=$(CC) CFLAGS="$(CFLAGS) -I.." )

install: sklaffkom sklaffadm sklaffacct survreport sklaffwho
	@echo Making libraries
	-mkdir $(SKLAFFBIN)
	-mkdir $(SKLAFFDIR)
	-mkdir $(SKLAFFDIR)/etc
	@echo Installing SklaffKOM
	-mv $(SKLAFFBIN)/sklaffkom $(SKLAFFBIN)/sklaffkom.old
	chown sklaff $(SKLAFFBIN)/sklaffkom.old
	chmod u+s $(SKLAFFBIN)/sklaffkom.old
	cp sklaffkom sklaffadm sklaffwho sklaffacct $(SKLAFFBIN)/
	cp survreport $(SKLAFFBIN)/srep
	-chown sklaff $(SKLAFFBIN)/sklaffkom $(SKLAFFBIN)/sklaffadm $(SKLAFFBIN)/sklaffwho \
		      $(SKLAFFBIN)/sklaffacct
	-chown root   $(SKLAFFBIN)/srep
	chmod u+s $(SKLAFFBIN)/sklaffkom $(SKLAFFBIN)/sklaffacct $(SKLAFFBIN)/sklaffwho
	chmod og-rxw $(SKLAFFBIN)/sklaffadm
	-chmod 4755   $(SKLAFFBIN)/srep
	cp newstoss mailtoss $(SKLAFFDIR)/etc
	-chown root $(SKLAFFDIR)/etc/newstoss $(SKLAFFDIR)/etc/ntoss \
                   $(SKLAFFDIR)/etc/mailtoss $(SKLAFFDIR)/etc/mtoss
	-chmod og-rxw $(SKLAFFDIR)/etc/newstoss $(SKLAFFDIR)/etc/ntoss \
                     $(SKLAFFDIR)/etc/mailtoss $(SKLAFFDIR)/etc/mtoss

installdb:
	@echo Installing datafiles
	-mkdir $(SKLAFFDIR)
	chown sklaff $(SKLAFFDIR)
	chmod og-rwx $(SKLAFFDIR)
	-rm -rf $(SKLAFFDIR)/db
	mkdir $(SKLAFFDIR)/db
	chown sklaff $(SKLAFFDIR)/db
	chmod og-rwx $(SKLAFFDIR)/db
	-rm -rf $(SKLAFFDIR)/user
	mkdir $(SKLAFFDIR)/user
	chown sklaff $(SKLAFFDIR)/user
	chmod og-rwx $(SKLAFFDIR)/user
	-rm -rf $(SKLAFFDIR)/mbox
	mkdir $(SKLAFFDIR)/mbox
	chown sklaff $(SKLAFFDIR)/mbox
	chmod og-rwx $(SKLAFFDIR)/mbox
	-rm -rf $(SKLAFFDIR)/files
	mkdir $(SKLAFFDIR)/files
	chown sklaff $(SKLAFFDIR)/files
	chmod og-rwx $(SKLAFFDIR)/files
	-rm -rf $(SKLAFFDIR)/etc
	mkdir $(SKLAFFDIR)/etc
	chown sklaff $(SKLAFFDIR)/etc
	chmod og-rwx $(SKLAFFDIR)/etc
	mkdir $(SKLAFFDIR)/etc/help.swe
	chown sklaff $(SKLAFFDIR)/etc/help.swe
	chmod og-rxw $(SKLAFFDIR)/etc/help.swe
	mkdir $(SKLAFFDIR)/etc/help.eng
	chown sklaff $(SKLAFFDIR)/etc/help.eng
	chmod og-rxw $(SKLAFFDIR)/etc/help.eng
	-cp etc/* $(SKLAFFDIR)/etc/
	cp etc/help.swe/* $(SKLAFFDIR)/etc/help.swe/
	cp etc/help.eng/* $(SKLAFFDIR)/etc/help.eng/
	chown sklaff $(SKLAFFDIR)/etc/*
	chown sklaff $(SKLAFFDIR)/etc/help.swe/*
	chown sklaff $(SKLAFFDIR)/etc/help.eng/*
	chmod og-rxw $(SKLAFFDIR)/etc/*
	chmod og-rxw $(SKLAFFDIR)/etc/help.swe/*
	chmod og-rxw $(SKLAFFDIR)/etc/help.eng/*

clean:
	(cd lib; make clean)
	-rm -f *.o *.a \#* *~ core sklaffkom sklaffadm sklaffacct mailtoss newstoss survreport sklaffwho version.c

distrib:
	echo 0 > .compile
	(cd ..; \
	find sklaff-1.28 -type f -not -name "*.o" -not -name "*.a" \
		-not -name "*~" \( -not -perm +111 -or -name "version.sh" \) \
		-print | sort > /tmp/sklaff; \
        tar --create --file - --files-from /tmp/sklaff | \
		compress > sklaff128.tz; \
	tar --create --file - --files-from /tmp/sklaff | \
		gzip -c > sklaff128.tgz; \
	cat /tmp/sklaff | xargs -iFILE zip sklf128 FILE; \
	rm /tmp/sklaff)
