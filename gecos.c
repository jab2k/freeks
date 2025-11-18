/* gecos.c                                                             */
/* Apostolos Lytras. June 1994                                         */
/* This little program gets a user name on its command line for which  */
/* it searches /etc/passwd's gecos entry and strips everything but the */
/* first of its (comma delimited) fields...                            */
/* BUGS: maybe the first field is not the user's real name... ?!       */

#include "freeks.h"

#ifndef WANTGECOS

char *J_gecos_parse(string)
char *string;
{ 
   return string;  /* dummy function */
}

#else

char *J_gecos_parse(namen)
char *namen;
{
  char out[MAX_GECOS_LEN], *output, *input;
  int len, i;
  struct passwd *dastruct;

  dastruct = (struct passwd *) malloc (sizeof(struct passwd *));
  if ((dastruct = getpwnam(namen)) == (struct passwd *)NULL )
  { 
#ifdef DEBUG
    fprintf(stderr,"Debug: user %s is not in password file.\n",namen); 
#endif
    return namen;
  };
  if ((input = (char *)strdup(dastruct->pw_gecos)) == (char *)NULL)
  {
    fprintf(stderr,"Fatal: allocation for GECOS failed.\n");
    exit(-2);
  };
/* This starts at 0, so we want -2 (0-based, less 1 for terminator) */
  len = minimum((MAX_GECOS_LEN - 2),strlen(input));
  for ( i = 0; (i <= len) && (input[i] != ',') ; i++, out[i] = '\0')
  {
    out[i] = input[i];
  }
  output = (char *)strdup(out);
  return output;
}

#endif
