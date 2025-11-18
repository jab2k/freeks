/********************************************************************/
/*                                                                  */
/* freeks.c - login accounting                                      */
/* Copyright (C) 1994 by Apostolos Lytras                           */
/* Version 1.33, July 10, 1994                                      */
/*                                                                  */
/*            1.33  NetBSD, BSD/386 patches from Mark Delany and    */
/*                  Terry Kennedy, who also found and fixed a bug   */
/*                  in gecos.c                                      */
/*            1.32  SysV support almost in. fixed more bugs.        */
/*            1.31  time handling still not okay.                   */
/*                  finally got some SysV help from Wietse Venema   */
/*            1.30  first attempt to include SysV support           */
/*            1.22  included hints for HP/UX 9.0x with              */
/*                  CLASSIC_ID_TYPES                                */ 
/*                  suggested by Bruce Schuchardt                   */
/*            1.21  included bugfix for J_hash suggested by         */
/*                  Andy Wick                                       */
/*                  posted to alt.sources March 30, 1994            */
/*            1.20  first edition, posted to alt.sources            */
/*                  March 29, 1994                                  */
/*                                                                  */
/*                                                                  */
/* See the manual page (freeks.6) distributed with this file, for   */
/* restriction, warranty, distribution and copyright information.   */
/*                                                                  */
/********************************************************************/

#include "freeks.h"

#ifdef NO_STRDUP
char           *
strdup(char *s)
{
  char           *t = (char *) malloc(strlen(s) + 1);

  return t ? strcpy(t, s) : t;
}
#else
extern char    *strdup();
#endif


USER            userlist[HASHTABLESIZE];
TTY            *ttylist;

long            s_uptime;
long            s_start;
long            s_corr = 0;
long            t_corr = 0;
long            s_stop;
unsigned int    s_reboots = 0;
unsigned int    s_shutdowns = 0;
unsigned int    s_crashes = 0;
unsigned int    s_logouts = 0;
unsigned int    s_logins = 0;
unsigned int    toptenmode = 0;
long            now = 0;
static int      shutd_state = FALSE;

#ifdef __STDC__
void J_init_all(void)
#else
J_init_all()
#endif
{ int i;
  for ( i = 0 ; i <= HASHTABLESIZE ; i++)
  {
   if (userlist[i].name != NULL)
    { 
    userlist[i].name = NULL; 
    userlist[i].current_logins = 0;
    userlist[i].max_conc_logins = 0;
    userlist[i].logins = 0;
    userlist[i].logintimebuf = 0;
    userlist[i].needs_timeset = FALSE;
    userlist[i].time = 0;
    userlist[i].total = 0;
    userlist[i].max_time = 0;
    }
  }
  ttylist = NULL;
  s_uptime = 0;
  s_start = 0;
  s_corr = 0;
  t_corr =0;
  s_stop = 0;
  s_reboots = 0;
  s_shutdowns = 0;
  s_crashes = 0;
  s_logouts = 0;
  s_logins = 0;
  shutd_state = FALSE;
  now = 0;
}

#ifdef CONFIGFILE

PRIVILEGED *shutdownusers;

PRIVILEGED *config_init(filename)
char *filename;
{
  FILE *fp;
  PRIVILEGED *tmp = (PRIVILEGED *)NULL;
  char priv_user[UT_NAMESIZE];

  fp =  (FILE *)fopen(filename,"r");
  if (fp == NULL) { 
       fprintf(stderr,"Warning: Could not open config file (%s)\n", 
               filename ); return((PRIVILEGED *)NULL);
  }
  while (fscanf(fp,"%s",&priv_user) != EOF) {
    priv_user[UT_NAMESIZE] = '\0';
    if ((tmp = (PRIVILEGED *)malloc(sizeof(PRIVILEGED))) == (PRIVILEGED *)NULL)
      fprintf(stderr,("Fatal: malloc failed in config\n"));
    else {
       tmp->next = shutdownusers;
       tmp->user = (char *) strdup (priv_user);
       shutdownusers = tmp;
    }
  }
  if (fclose(fp) == 0)
  {
    return(tmp);
  }
  else
  {
    fprintf(stderr, "Fatal: couldn't close %s\n", filename);
    exit(1);
  }
}

int privilege_check(username,list)
char *username;
PRIVILEGED *list;
{
  PRIVILEGED *T;

  if (list != NULL) {
   for (T=list ; T != NULL ; T = T->next) {
#ifdef DEBUG
    fprintf(stderr,"Debug: privileges test %s for %s\n",list->user, username);
#endif /* DEBUG */
    if (!strncmp(T->user,username,UT_NAMESIZE))  return TRUE;
   }
  };
  return FALSE;
}

#endif /* CONFIGFILE */

static int
SedgeHash(arg)
char           *arg;
{
  int             h;

  for (h = 0; *arg != '\0'; arg++)
  {
    h = (64 * h + *arg) % HASHTABLESIZE;
  }
  return h;
}

static int
J_hash(arg)
char           *arg;
{
  int             Index, i, count=0;

  Index = (int) SedgeHash(arg);

  if (userlist[Index].name == NULL)
  {
    return Index;
  }
  for (i = Index; userlist[i].name != NULL; i++)
  {
    count++;
    if (i == HASHTABLESIZE)
    {
      i = 0;
      if (userlist[i].name == NULL)
	return i;
    }
    else if (count == HASHTABLESIZE)
    {
      fprintf(stderr, "Fatal: Hash table is full.\n");
      exit(2);
    };
    if (strcmp(userlist[i].name, arg) == 0)
      return i;
  }
  return i;
}

int
ucompare(i, j)
USER           *i, *j;
{
  if (i->time > j->time)
    return -1;
  else if (i->time < j->time)
    return 1;
  return 0;
}

static TTY     *
J_gettty(thetty)
char           *thetty;
{
  register TTY   *cur, *T;

  if (ttylist != NULL)
  {
    for (T = ttylist; T != NULL; T = T->next)
    {
      if (strncmp(thetty, T->tty, UT_LINESIZE) == 0)
      {
	return (T);
      };
    }
  };
  if ((cur = (TTY *) malloc(sizeof(TTY))) == NULL)
  {
    fprintf(stderr, "Fatal: malloc failed !\n");
    exit(1);
  }
  cur->next = ttylist;
  cur->tty = (char *) strdup(thetty);
  ttylist = cur;
  return (cur);
}

static int
J_countlogins(uhash)
int             uhash;
{
  register TTY   *curt;
  int             count = 0;

  if (ttylist != NULL)
  {
    for (curt = ttylist; curt != NULL; curt = curt->next)
    {
      if ((curt->used) && (uhash == curt->user_hash))
      {
	count++;
      }
      else
	continue;
    }
  };
  return count;
}

static void
J_logout(logouttime, cur)
long            logouttime;
TTY            *cur;
{
  long            stayed;
  long            definite;
  int             current;

  if (cur->used == 0)
  {
    return;
  }
  else
  {
    cur->used = 0;
    if ((logouttime != now) || (now == 0))
      /* avoid counting users still logged in */ 
    { 
      s_logouts++;
    }
    current = J_countlogins(cur->user_hash);
    userlist[cur->user_hash].current_logins = current;
    stayed = logouttime - cur->time_in;
    if (stayed < 0) 
    {
      fprintf(stderr,"Error: user %s has a negative session time.\n",
                      userlist[cur->user_hash].name);
    };
    if (stayed > userlist[cur->user_hash].max_time)
    {
      userlist[cur->user_hash].max_time = stayed;
    };
    userlist[cur->user_hash].total += stayed;
    if (current == 0)
    {
      definite = logouttime - userlist[cur->user_hash].logintimebuf;
      userlist[cur->user_hash].time += definite;
    };
  }
}

static void
J_login(inname, logintime, tty)
char           *inname, *tty;
long            logintime;
{
  int             uhash;
  register TTY   *thistty;

  
  uhash = J_hash(inname);
  if (userlist[uhash].name == NULL)
  {
    userlist[uhash].name = (char *) strdup(inname);
  }
  else
  {
    while (strncmp(userlist[uhash].name, inname, sizeof(inname)) != 0)
    {
      fprintf(stderr, "Fatal: could not avoid hash collision at %d: %s %s\n",
	      uhash, inname, userlist[uhash].name);
      exit(2);
    }
  }
  thistty = J_gettty(tty);
  if (thistty->used)
  { /* the tty is already in use */
    if (strcmp(userlist[thistty->user_hash].name,userlist[uhash].name) == 0)
    { /* the newly logged in user is already logged in */
      return;
    }
    else
    {
      J_logout(logintime, thistty);
	    
#ifdef DEBUG
      fprintf(stderr, "Debug: %s did not log out. Replaced by %s...\n",
	      userlist[thistty->user_hash].name,
	      userlist[uhash].name);
#endif 
    }
  };
  s_logins++;
  userlist[uhash].logins++;
  if (userlist[uhash].current_logins < 0)
  {
    userlist[uhash].current_logins = 1;
  }
  else
  {
    userlist[uhash].current_logins++;
  }
  if (userlist[uhash].current_logins == 1)
  {
    userlist[uhash].logintimebuf = logintime;
  };
  if (userlist[uhash].max_conc_logins < userlist[uhash].current_logins)
  {
    userlist[uhash].max_conc_logins = userlist[uhash].current_logins;
  };
  thistty->time_in = logintime;
  thistty->user_hash = uhash;
  thistty->used = 1;
}

#if __STDC__
static void
J_timeprep(void)
#else
static void
J_timeprep()
#endif
{
  register TTY   *cur;

  if (ttylist != NULL)
  {
    for (cur = ttylist; cur != NULL; cur = cur->next)
    {
      if (cur->used == 1)
      {
	userlist[cur->user_hash].needs_timeset = TRUE;
      };
    }
  };
}

static void
J_timeset(oldtime, newtime)
long            oldtime, newtime;
{
  long            diff;
  register TTY   *cur;

  diff = newtime - oldtime;
  if (ttylist != NULL)
  {
    for (cur = ttylist; cur != NULL; cur = cur->next)
    {
      if (cur->used == 1)
      {
	cur->time_in += diff;
	if (userlist[cur->user_hash].needs_timeset == TRUE)
	{
	  userlist[cur->user_hash].logintimebuf += diff;
	  userlist[cur->user_hash].needs_timeset = FALSE;
	};
      };
    }
  };
  s_corr += diff;
  t_corr += diff;
#ifdef DEBUG
  fprintf(stderr, "Debug: date got set. Difference is: %ld\n", diff);
#endif
}

void
J_cleanup(endtime)
long            endtime;
{
  register TTY   *cur;

  if (ttylist != NULL)
  {
    for (cur = ttylist; cur != NULL; cur = cur->next)
    {
      if (cur->used)
      {
	J_logout(endtime, cur);
      }
      else
	continue;
    }
  };
}

static void
J_reboot(sometime, reboottime, shutdowntime)
long            sometime, reboottime, shutdowntime;
{
  long            thetime, timediff;

  if (shutd_state == TRUE)
  {
    timediff = reboottime - shutdowntime;
    s_corr += timediff;
    if (timediff < 0)
    {
      thetime = reboottime;
    }
    else
    {
      thetime = shutdowntime;
    }
    s_reboots++;
  }
  else
  {
    if (((reboottime - sometime) > 86400) || (sometime > shutdowntime))
    {
      fprintf(stderr, "Warning: You have had a serious crash, I suppose\n");
      thetime = sometime;
      timediff = reboottime - sometime;
      s_corr += timediff;
      s_crashes++;
    }
    else
    {
      thetime = reboottime;
      s_shutdowns++;
      s_reboots++;
    }
  }
  J_cleanup(thetime);
}

static void
J_print(list)
USER           *list;
{
  int             i,rank=0;
  long            localv;

  localv = now - (s_start + t_corr);
  if (localv < 0) {
    fprintf(stderr,"Warning: negative time covered!\n");
  };

  s_uptime = now - (s_start + s_corr);
  if (s_uptime < 0) {
    fprintf(stderr,"Warning: negative uptime!\n");
  };

  printf("--- System statistics---\n");
  printf("Start at:     %s", ctime((time_t *)&s_start));
  printf("End at:       %s", ctime((time_t *)&now));
  printf("Time covered:");
  timefmt(localv);
  printf("\n");
  printf("Uptime:      ");
  timefmt(s_uptime);
  printf("  (%.1f %% of total time)\n",
	 (((float) s_uptime / (float) localv) * 100.0));
  printf("Booted:       %d times", s_reboots);
  printf("  (shut down %d times)\n", s_shutdowns);
  printf("Crashed:      %d times\n", s_crashes);
  printf("Logins:       %d\n", s_logins);
  printf("Logouts:      %d\n", s_logouts);
  printf("\n--- User statistics---\n");
  qsort((char *) ((USER *) list), (size_t) HASHTABLESIZE, (size_t) sizeof(struct anUser), ucompare);
 if(toptenmode == 1) {
  for (i = 0; i < HASHTABLESIZE; i++)
  {
    if (list[i].name != NULL)
    {
      rank++;
      printf("%4d %-22s",
             rank,
             (char *)J_gecos_parse(list[i].name));
      timefmt(list[i].time);
      printf(" (%6d logins)\n",list[i].logins);
    };
  } 
 } 
 else {
#ifdef	__bsdi__	/* Actually an SPC-ism */
   printf("user          logins ttys       real      total    longest    average   %%uptime\n");
#else
   printf("user      logins ttys       real      total    longest    average   %%uptime\n");
#endif

  for (i = 0; i < HASHTABLESIZE; i++)
  {
    if (list[i].name != NULL)
    {
#ifdef	__bsdi__	/* Actually an SPC-ism */
      printf("%-13s %6d %4d", list[i].name, list[i].logins,
#else
      printf("%-9s %6d %4d", list[i].name, list[i].logins,
#endif
	     list[i].max_conc_logins);
      timefmt(list[i].time);
      timefmt(list[i].total);
#ifdef DEBUG
      if (list[i].time > list[i].total) 
      {
        fprintf(stderr,"Debug: total time greater than real time: %s\n",
                       list[i].name);
      }
#endif
      timefmt(list[i].max_time);
      timefmt((list[i].total / list[i].logins));
      printf("   %7.2f\n", (((float) list[i].time / (float) s_uptime) * 100));
    };
  }
 }
}

void
J_read_wtmp(fp, def)
FILE           *fp;
int             def;
{
  struct utmp     logbuf, *uptr;
  int             first = TRUE;
  long            timebuf = 0;
  char            curuser[UT_NAMESIZE+1];
  char            curline[UT_LINESIZE+1];
  long            tob = 0, dot = 0;
  
  while (fread((char *) &logbuf, 1, sizeof(logbuf), fp) == sizeof(logbuf))
  {
    s_stop = logbuf.ut_time;
    if (first == TRUE)
    {
      first = FALSE;
      tob = s_stop;
      s_start = s_stop;
    };
    if ( s_stop < s_start ) 
    {
      fprintf(stderr,"Error: time scope vulneration... resetting\n");
      J_init_all();
    };
#ifndef SYSV
    if (!strncmp("reboot", logbuf.ut_name, 6))
#else
    if (logbuf.ut_type == BOOT_TIME)
#endif
    {
      J_reboot(tob, s_stop, dot);
      shutd_state = FALSE;
      continue;
    }
    else if (!strncmp("shutdown", logbuf.ut_name, UT_NAMESIZE)
#ifndef SYSV
	     || logbuf.ut_line[0] == '~' 
#endif
             || shutd_state == TRUE 
#ifdef CONFIGFILE
             || (privilege_check(logbuf.ut_name,shutdownusers) == TRUE) 
#endif
            )
    {
      tob = s_stop;
      if (shutd_state == FALSE)
      {
	shutd_state = TRUE;
	s_shutdowns++;
	dot = logbuf.ut_time;
	continue;
      }
      else
	continue;
    }
#ifndef SYSV
    else if (logbuf.ut_line[0] == '|' && !logbuf.ut_line[1])
#else
    else if (logbuf.ut_type == OLD_TIME)
#endif
  {
      timebuf = logbuf.ut_time;
      J_timeprep();
      tob = s_stop;
      continue;
    }
#ifndef SYSV
    else if (logbuf.ut_line[0] == '{' && !logbuf.ut_line[1])
#else
    else if (logbuf.ut_type == NEW_TIME)
#endif
    {
      tob = s_stop;
      if (timebuf != 0)
      {
#ifdef DEBUG
        fprintf(stderr,"Debug: timesetting %ld\n",(logbuf.ut_time - timebuf));
#endif
	J_timeset(timebuf, logbuf.ut_time);
	continue;
      }
      else
	continue;
    }
    else if ((!strncmp("ftp", logbuf.ut_line, 3)) ||
	     (!strncmp("uucp", logbuf.ut_line, 4)))
    {
      shutd_state = FALSE;
      tob = s_stop;
      continue;
    }
#ifndef SYSV
    else if (isalnum(logbuf.ut_name[0]) != 0)
#else
    else if (logbuf.ut_type == USER_PROCESS)
#endif
    {
      shutd_state = FALSE;
      uptr = &logbuf;
      curuser[UT_NAMESIZE] = '\0';
      curline[UT_LINESIZE] = '\0';

      strncpy(curuser, uptr->ut_name, UT_NAMESIZE);
      strncpy(curline, uptr->ut_line, UT_LINESIZE);
      J_login(curuser, uptr->ut_time, curline);
      tob = s_stop;
      continue;
    }
#ifndef SYSV
    else if (logbuf.ut_line != NULL)
#else
    else if (logbuf.ut_type == DEAD_PROCESS)
#endif
    {
      curline[UT_LINESIZE] = '\0';
      strncpy(curline, logbuf.ut_line, UT_LINESIZE);
      J_logout(logbuf.ut_time, J_gettty(curline));
      tob = s_stop;
      continue;
    }
    else
    {
#ifndef SYSV
      fprintf(stderr, "Warning: Strange entry at %s\n%s %s\n",
              ctime((time_t *)&logbuf.ut_time), 
              logbuf.ut_name, logbuf.ut_line);
#endif
      tob = s_stop;
      continue;
    }
  }
  if (def == 0)
  {
    now = s_stop;
  }
  else
  {
    now = (long) time((time_t *) 0);
  }
  J_cleanup(now);
  J_print(userlist);
}

void
main(argc, argv)
int             argc;
char          **argv;
{
  FILE           *fd;
#ifdef WTMP_FILE
  char           *wtmp = WTMP_FILE;
#else
#ifdef _PATH_WTMP
  char           *wtmp = _PATH_WTMP;
#else
  char           *wtmp = "/usr/adm/wtmp";
#endif
#endif
#ifdef CONFIGFILE
  char           *configfile = CONFIGFILE;
#endif
  int             def = 1;

  while (--argc > 0 && **++argv == '-')
    switch (*++*argv)
    {
    case 'w':
      if (--argc > 0)
      {
	wtmp = *++argv;
	def--;
      }
      continue;
    case 't':
      toptenmode++;
      continue;
#ifdef CONFIGFILE
    case 'c':
      if (--argc > 0)
      {
        configfile = *++argv;
      }
      continue;
#endif
    }
#ifdef CONFIGFILE 
    shutdownusers = config_init(configfile);
#endif 

  fd = (FILE *) fopen(wtmp, "r");
  if (fd == NULL)
  {
    fprintf(stderr, "Fatal: %s ... no such file\n", wtmp);
    exit(1);
  }
  J_read_wtmp(fd, def);
  if (fclose(fd) == 0)
  {
    exit(0);
  }
  else
  {
    fprintf(stderr, "Fatal: couldn't close %s\n", wtmp);
    exit(1);
  }
}
