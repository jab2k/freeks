# freeks - extended login accounting

## Project Info

*freeks* produces a report on system usage based on the contents of `wtmp` and sorted by the actual time different users have spent on a *Unix* system. The first few lines contain general information about the system.

Originally published in July 1994 on Usenet **comp.sources.unix** in Volume 28 Issue 83

## How to use freeks

freeks [ −w wtmp ] [ −t] [ −c configfile ]

- The default wtmp path is `/usr/adm/wtmp`.  
- `-t` shows rank, username, total login time, and number of logins.  
- `-c` specifies a config file that lists alternative shutdown-user names (mainly relevant for System V).

## Notes / Bugs

Freeks is mostly limited to BSD-style wtmp files (see TODO).  
It has a few known issues:

- If a reboot entry appears without a preceding shutdown, Freeks assumes a crash and uses the last entry before the reboot as the shutdown time. This may cause minor inaccuracies in uptime and login statistics.
- Changing the system time in single-user mode is not logged. This can break time accounting. Avoid doing it or delete wtmp afterward.
- System V uses a different wtmp format, but Freeks may still work if shutdowns are rare or a dedicated shutdown user exists.  
  - If that user is named `shutdown`, no config file is needed.  
  - Otherwise, specify their name via the config file (when compiled with `CONFIGFILE` enabled).

## Installation

1. Edit the Makefile to suit your system.
2. Check whether changes are needed in `freeks.h`.

   If your system has **more** than `HASHTABLESIZE` users, increase `HASHTABLESIZE` (preferably to a prime number).  
   Otherwise, Freeks will fail with *Hash table full*.


3. `make`
4. Test-run freeks.
5. `make install`
6. `make clean`

## Portability

Tested on:

| Machine Type     | Operating System        | Compiler            |
|------------------|--------------------------|----------------------|
| Decsystem 5200   | Ultrix 4.0              | gcc 2.2.2           |
| Nextstation m68k | Nextstep 3.2            | cc (gcc 2.2.2)      |
| Sparcstation 1   | SunOS 4.1.3             | gcc 2.5.8           |
| SparcClassic     | SunOS 5.3 (Solaris 2.3) | gcc 2.5.8           |
| Intel 486        | NetBSD 0.9B             | gcc 2.4.5           |
| Intel x86        | BSD/386                 | ??                  |

## Thanks

Andy Wick, Bruce Schuchardt, Wietse Venema, Terry Kennedy and Mark Delany for their patches and feedback.

Copyright (c) 1994 by Apostolos Lytras.
