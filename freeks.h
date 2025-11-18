/* --- start of configurable section --- */
/*
 * HASHTABLESIZE must be prime, such as 61,113,251,509,1021 To save memory
 * choose a value close to the number of users on your system.
 *
 * Note:
 * This is not a full-fledged hashing library, just some code stolen from
 * "Algorithms in C" by Robert Sedgewick which has been refined to avoid hash
 * collisions. The algorithm which does this is very simple and probably not
 * worth a lot in more complex programs. (It does not handle removal from the
 * list, because that won't happen in this application.)
 */
#ifndef HASHTABLESIZE
#define HASHTABLESIZE 1021
#endif

/* NO_STRDUP:
 * if your system doesn't have the strdup routine, then uncomment the
 * definition below.  
 */
/* #define NO_STRDUP */

/* WANTGECOS:
 * If you want to use GECOS entries ("real names") in 'freeks -t' output
 * define this
 */
#ifndef WANTGECOS
#define WANTGECOS
#endif

/* DEBUG:
 * If you want to get more debugging output on standard error, then
 * uncomment this here or look at the debugflags in the Makefile
 */
/* #define DEBUG */

/* CONFIGFILE */
/* System V users might want to create a configfile containing the   */
/* names of additional usernames that create a shutdown, like `halt' */
/* or `powerdown'. Each username must appear separately on a line in */
/* the file                                                          */
/* #define CONFIGFILE "/usr/local/lib/freeks/config" */

/* --- you shouldn't need to change much below this --- */

#if defined(__hpux)
#ifndef SYSV
#define _CLASSIC_ID_TYPES
#endif
#endif

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/file.h>
#include <ctype.h>
#ifndef SYSV
#include <utmp.h>
#include <strings.h>
#else
#include <utmpx.h>
#include <string.h>
#ifndef ut_name
#define ut_name ut_user
#endif
#endif

#ifdef WANTGECOS
#include <pwd.h>
/***************************************************************/
/* MAX_GECOS_LEN is the maximum length of a gecos entry in the */
/* password database for use in topten ( option '-t' ) mode    */
/* Default: 20                                                 */
/***************************************************************/
#ifdef	__bsdi__
#define MAX_GECOS_LEN 32
#else
#define MAX_GECOS_LEN 20
#endif
#define minimum(a,b)  (a < b) ? a : b 
#endif /* WANTGECOS */

struct utmp utmp_dummy;
#ifndef	UT_NAMESIZE
#define UT_NAMESIZE sizeof (utmp_dummy.ut_name)
#endif
#ifndef	UT_LINESIZE
#define UT_LINESIZE sizeof (utmp_dummy.ut_line)
#endif

#define TRUE 1
#define FALSE 0

#define timefmt(time) \
printf(" %4ld:%02ld:%02ld", (time) / 3600, ((time) / 60) % 60, (time) % 60)

#ifndef NO_STRDUP
#if (defined(__ultrix__) || defined(nextstep3))
#define NO_STRDUP
#endif /* ultrix or nextstep */
#endif

typedef struct anUser
{
  char           *name;
  int             current_logins;
  int             max_conc_logins;
  int             logins;
  long            logintimebuf;
  int             needs_timeset;
  long            time;
  long            total;
  long            max_time;
}               USER;

typedef struct aTty
{
  char           *tty;
  int             used;
  int             user_hash;
  long            time_in;
  struct aTty    *next;
}               TTY;

#ifdef CONFIGFILE
typedef struct priv_list {
  char             *user;
  struct priv_list *next;
}               PRIVILEGED ;
#endif

