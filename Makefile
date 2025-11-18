# Makefile for freeks 1.33

# Configure this for your system (default is {SunOS,Ultrix} with gcc)
# 
# NOTE: if your system has far more (or far less) users than 1021, you
#       may wish to change the value for HASHTABLESIZE. That value must
#       be prime. You can change it in freeks.c, or here by adding
#       -DHASHTABLESIZE=2003 (for about 2000 users for example) to CFLAGS.
#       If you have a lot of users and you want to improve performance a bit,
#       it's probably best to test a prime number around (1.5*number_of_users)
#       which should cause less searching for free positions in the hash
#       table (the searching is linear!) when the table gets full.
#
# NOTE FOR SYSTEM V: System V doesn't log shutdowns by default. If you
#       want accurate information about system uptime then create a
#       shutdown user whose login will indicate that the system was shut 
#       down. Then define CONFIGFILE in freeks.c (or call the account
#       "shutdown") and create the configfile in the appropriate
#       place.

# Some general Variables
#
prefix = /usr/local
postfix = 6
MANPATH = ${prefix}/man/man${postfix}
CP = install -c -m
MODE1 = 755
MODE2 = 644
OBJS = freeks.o gecos.o

# BSD/386 1.0 with gcc
#
#CC = cc
#CFLAGS = -O 

# SunOS 4.1.3  / Ultrix 4.0 with gcc
#
CC = gcc
CFLAGS = -Wall -W -Wno-implicit -O 

# SunOS 4.1.3 with Sun C compiler
#
#CC = cc
#CFLAGS = -O

# Slowaris 2.3 with gcc
#
#CC = gcc
#CFLAGS = -Wall -W -O -DSYSV

# HP/UX 9.0x.
#
# CC = cc
# CFLAGS = -Aa -O

# Nextstep 3.x with NeXT's (g)cc
#
# CC = cc
# CFLAGS = -Wall -W -Dnextstep3 -O

# more flags
# DEBUGFLAGS = -DDEBUG -g -ansi -pedantic -DWANTGECOS
DEBUGFLAGS = 
# LDFLAGS = -s
LDFLAGS =

# -------- from here on you shouldn't need to change anything ---

all: 	freeks  

freeks: ${OBJS}
	${CC} ${CFLAGS} ${DEBUGFLAGS} ${LDFLAGS} -o $@ ${OBJS}

freeks.o:
	${CC} ${CFLAGS} ${DEBUGFLAGS} -c $*.c

gecos.o:
	 ${CC} ${CFLAGS} ${DEBUGFLAGS} -c $*.c

install: all
	${CP} ${MODE1} freeks ${prefix}/bin; \
	${CP} ${MODE2} freeks.6 ${MANPATH}/freeks.${postfix}

clean:
	rm -f freeks *~ core *.o
