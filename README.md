# freeks - extended login accounting

## Project Info

Freeks produces a report on system usage based on the contents of wtmp and sorted by the actual time different users have spent on the system. The first few lines contain general information about the system.

Originally published in July 1994 on Usenet comp.sources.unix in Volume 28 Issue 83

## How to use freeks

freeks [ −w /usr/adm/wtmp ] [ −t] [ −c configfile ]

The default for wtmp is "/usr/adm/wtmp". You may specify an alternate wtmp file on the command line.
With the 't' option the user statistics are modified, showing the rank , the name, the time the user was actually logged in and the number of logins. With the c option you can specify a configfile containing the names of alternative 'shutdown' users (cf. TODO), which is probably interesting to SysV machine owners only.


## Notes / Bugs 

Freeks is (almost) limited to BSD style wtmp files (cf. TODO below). It does not have a lot of fancy options. Freeks does make some mistakes, e.g. when a reboot entry in wtmp happened without a shutdown before it. If the last entry happened more than a day before the reboot entry, freeks presumes that the machine crashed, and will use the time of the last entry before the crash as time of shutdown. This may lead to marginal errors in 'uptime' and the login times of users still logged in at that time.

Another bug happens when time is changed in single user mode which means it doesn't get logged. Don't do that, or delete the wtmp file afterwards.

System V has changed a lot in wtmp but it should be possible to run the program, if you rarely shut your machine down or have a dedicated 'shutdown' user that logs in when you shut down. If this user is called 'shutdown' you needn't do nothing, if his login name is something else, you can specify that in configfile (if, of course, freeks had been compiled with CONFIGFILE defined...).

Copyright (C) 1994 by Apostolos Lytras.

## INSTALLATION:

1) Edit the Makefile to match your system.
2) Check if you have to change anything in 'freeks.h'


Note: If your system has _more_ than HASHTABLESIZE users then you _must_ set 
HASHTABLESIZE to a higher value (preferably a prime) in freeks.h, or
you will get an error (Hash table full) when you run freeks.


3) 'make'
4) Test-run freeks.
5) 'make install'
6) 'make clean'

## Portability

This has been run and tested on the following systems:
Machine Type     Operating System        Compiler
Decsystem 5200   Ultrix 4.0              gcc 2.2.2
Nextstation m68k Nextstep 3.2            cc (Next's Version of gcc 2.2.2)
Sparcstation 1   SunOS 4.1.3             gcc 2.5.8
SparcClassic     SunOS 5.3 (Solaris 2.3) gcc 2.5.8
Intel 486        NetBSD 0.9B             gcc 2.4.5
Intel x86        BSD/386                 ??

## Thanks

Andy Wick, Bruce Schuchardt, Wietse Venema, Terry Kennedy and Mark Delany for their patches and feedback.
