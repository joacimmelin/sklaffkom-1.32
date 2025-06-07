Sklaffkom is a Swedish, command based BBS system (also known as [KOM](https://en.wikipedia.org/wiki/KOM_(bulletin_board_system))) written for SunOS 5.x, later ported to Interactive Unix, NetBSD, FreeBSD and later (sort-of) ported to Linux. 

The BBS system was written by Torbjörn Bååth, Peter Forsberg, Peter Lindberg, Odd Petersson and Carl Sundbom in approximatly 1992-1996.  Later additions was written by Daniel Grönjord and Olof Runborg. 

The code was, and still is, distributed under the GPL 2.0 license from 1991. 

It was never meant to be a complete, permanent BBS system but rather a temporary solution in wait for the grand new big BBS system that arrived 10+ years later in the form of [EasyKOM](https://sv.wikipedia.org/wiki/EasyKOM). 

The reason for keeping this old codebase alive and working on modern operating systems is simple: it is a big part of the cultural heritage of a small BBS community in Sweden. The goal is, of course, to have Sklaffkom support a modern version of a modern operating system to be able to open this BBS system to online users again. Because we can. 

This version, 1.32, is the last version released by the team behind Sklaffkom- It has later been patched to compile on the 32-bit x86 versions of FreeBSD 12 and OpenBSD 6. Complete instructions on how to compile and install Sklaffkom can be found [here](https://github.com/joacimmelin/sklaffkom-1.32/wiki/Install-Instructions). 

At some point, this should probably also be updated again to compile on 64-bit systems and later versions of FreeBSD and OpenBSD (and hopefully Linux, but who knows). Attempts has been made to compile Sklaffkom on OpenBSD 7.4 running on SPARC64 but it didn't work. 

The current code (2025-06-06) will install on 32-bit FreeBSD 14 on x86. Attempts are being made to compile this code in FreeBSD 14.2-RELEASE (64-bit) and macOS 15.5 (64-bit, ARM), the former proving to be a struggle since nlist does not seem to exist on 64-bit systems.  Thoughts and prayers, etc. 

Sklaffkom is dedicated to the memory of Staffan Bergström, a BBS-user who died much too young.   
