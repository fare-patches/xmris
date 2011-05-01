/* Copyright (C) 1993, 1992 Nathan Sidwell */
#include "xmris.h"
/*{{{  other includes*/
#include <pwd.h>
#include <sys/stat.h>
/*}}}*/
/*{{{  file locking*/
#ifndef SYSV
#ifdef linux
#include <sys/file.h>
#endif
#define lock_file(stream) flock(fileno(stream), LOCK_EX)
#define unlock_file(stream) flock(fileno(stream), LOCK_UN)
#else
#define lock_file(stream) lockf(fileno(stream), F_LOCK, 0L)
#define unlock_file(stream) lockf(fileno(stream), F_ULOCK, 0L)
#endif /* SYSV */
/*}}}*/
/*{{{  static*/
static CONST char date_formats[4] = "DMY";
static char     *score_file = "/var/games/xmris.score"; /* score file name */
static char     *personal_file = NULL;  /* personal in home dir */
static FILE     *score_stream = NULL;
static FILE     *personal_stream = NULL;
static char     date_format[4] = "   ";
static char     *alternate = NULL;    /* alternative name */
static HIGH_SCORE *CONST tables[] =
  {scoring.high, scoring.today, scoring.personal};
/*}}}*/
/*{{{  prototypes*/
static unsigned expire PROTOARG((time_t, time_t));
static unsigned file_changed PROTOARG((char CONST *));
static unsigned get_lock PROTOARG((FILE *));
static VOIDFUNC get_unlock PROTOARG((FILE *));
static unsigned insert_personal PROTOARG((HIGH_SCORE CONST *));
static unsigned insert_score
    PROTOARG((HIGH_SCORE *, HIGH_SCORE *, unsigned));
static VOIDFUNC load_check_expire_insert PROTOARG((unsigned));
static VOIDFUNC make_unique PROTOARG((HIGH_SCORE *));
static unsigned merge_personal PROTOARG((FILE *));
static unsigned merge_scores PROTOARG((FILE *));
static unsigned long read_line
    PROTOARG((FILE *, HIGH_SCORE *, unsigned long));
static VOIDFUNC remove_scores PROTOARG((HIGH_SCORE *, unsigned));
static VOIDFUNC retire_scores PROTOARG((void));
static VOIDFUNC write_personal PROTOARG((FILE *));
static VOIDFUNC write_scores PROTOARG((FILE *));
static unsigned long write_table
    PROTOARG((FILE *, HIGH_SCORE *, unsigned long));
/*}}}*/
/*{{{  void check_scores()*/
extern VOIDFUNC check_scores FUNCARGVOID
{
  retire_scores();
  if((score_file && file_changed(score_file)) ||
     (personal_file && file_changed(personal_file)))
    load_check_expire_insert(0);
  return;
}
/*}}}*/
/*{{{  unsigned expire(now, then)*/
static unsigned expire
FUNCARG((now, then),
	time_t    now
ARGSEP  time_t    then
)
/*
 * check to see if the score is now too old
 */
{
  struct tm *ptr;
  int now_day, then_day;
  int now_hour, then_hour;
  
  ptr = localtime(&now);
  now_day = ptr->tm_yday;
  now_hour = ptr->tm_hour;
  ptr = localtime(&then);
  then_day = ptr->tm_yday;
  then_hour = ptr->tm_hour;
  return !(now_day == then_day || (now_day == then_day + 1 &&
      then_hour >= 21 && now_hour < 12));
}
/*}}}*/
/*{{{  void file_changed(name)*/
static unsigned file_changed
FUNCARG((name),
	char CONST *name
)
/*
 * check if a score file has been changed since last I looked,
 * so that we pick up the new scores
 */
{
  static  time_t last_time[2];
  struct stat buffer;
  unsigned  changed;
  
  assert(name);
  if(!stat(name, &buffer))
    {
      changed = buffer.st_mtime != last_time[name == score_file];
      last_time[name == score_file] = buffer.st_mtime;
    }
  else
    changed = 0;
  return changed;
}
/*}}}*/
/*{{{  unsigned get_lock(stream)*/
static unsigned get_lock
FUNCARG((stream),
	FILE *stream
)
/*
 * lock the high score file
 */
{
    /*{{{  get lock on the file*/
    while(1)
      {
	int res = lock_file(stream);
	if(res)
	  {
	    if(errno == EINTR)
	      continue;
	    perror("Error locking score file");
	  }

	return res == 0;
      }
    /*}}}*/
}
/*}}}*/
/*{{{  void get_unlock(stream)*/
static VOIDFUNC get_unlock
FUNCARG((stream),
	FILE    *stream
)
/*
 * unlock the high score file
 */
{
  unlock_file(stream);
}
/*}}}*/
/*{{{  void high_score(score, screen, msec)*/
extern VOIDFUNC high_score
FUNCARG((score, screen, msec),
	unsigned long score
ARGSEP  unsigned  screen
ARGSEP  unsigned long msec
)
/*
 * are we worthy of imortallity?
 */
{
  scoring.mine.score = score;
  scoring.mine.screen = screen;
  scoring.mine.elapsed = (unsigned)(msec / 1000);
  scoring.mine.stamp = time((time_t *)NULL);
  scoring.mine.marker = 0;
  retire_scores();
  if(score && (score >= scoring.today[HIGH_SCORES - 1].score ||
      score >= scoring.personal[HIGH_SCORES - 1].score ||
      (score >= SCORE_THRESHOLD &&
	score >= scoring.high[HIGH_SCORES - 1].score)))
    load_check_expire_insert(1);
  return;
}
/*}}}*/
/*{{{  void init_scores()*/
extern VOIDFUNC init_scores FUNCARGVOID
/*
 * check that we have the wherewithall to deal with
 * high scores
 */
{
  char CONST *user;
  char CONST *home;
  char      *reallife;
  
  user = NULL;
  home = NULL;
  reallife = NULL;
  /*{{{  read passwd information*/
  {
    struct passwd *ptr;
    
    ptr = getpwuid(getuid());
    if(ptr)
      {
	user = ptr->pw_name;
	home = ptr->pw_dir;
	if(user && ptr->pw_gecos)
	  /*{{{  get the realname*/
	  {
	  /* attempt to extract a realname from the gecos field.
	   * sometimes this has more than the name in, and sometimes
	   * the name is surname,firstname,
	   * we convert to firstname surname, and strip off
	   * extra fields after the comma.
	   * if the thing before the first comma has spaces in it, then
	   * we assume that's the name.
	   */
	    char      *sptr;
	    char      *cptr;
	    
	    reallife = ptr->pw_gecos;
	    cptr = strchr(reallife, ',');
	    sptr = strchr(reallife, ' ');
	    if(cptr && (!sptr || sptr > cptr))
	      {
		sptr = cptr + 1;
		cptr = strchr(cptr + 1, ',');
	      }
	    else
	      sptr = NULL;
	    if(cptr)
	      *cptr = 0;
	    if(!*reallife)
	      reallife = NULL;
	    else if(sptr)
	      {
		if(!cptr)
		  cptr = reallife + strlen(reallife);
		if(cptr[-1] != ' ')
		  {
		    cptr[0] = ' ';
		    cptr[1] = 0;
		  }
		while(*sptr == ' ')
		  sptr++;
		while(sptr != reallife)
		  {
		    char      c;
		    
		    c = *reallife;
		    for(cptr = reallife; cptr[1]; cptr++)
		      cptr[0] = cptr[1];
		    sptr--;
		    *cptr = c;
		  }
		while(*cptr == ' ' || *cptr == ',')
		  *cptr-- = 0;
	      }
	  }
	  /*}}}*/
      }
  }
  /*}}}*/
  scoring.mine.score = 0;
  /*{{{  set username*/
  {
    if(!user)
      user = getenv("LOGNAME");
    if(!user)
      user = getenv("USER");
    if(!user)
      user = "Unknown";
    if(data.username == False && reallife)
      {
	strncpy(scoring.mine.name, reallife, NAME_LEN);
	reallife = (char *)user;
      }
    else
      strncpy(scoring.mine.name, user, NAME_LEN);
    scoring.mine.name[NAME_LEN] = 0;
    if(reallife)
      {
	/* not everyone has strdup */
	alternate = malloc(strlen(reallife) + 1);
	if(alternate)
	  strcpy(alternate, reallife);
      }
    if(data.username == False)
      scoring.alternate = alternate;
  }
  /*}}}*/
  /*{{{  score directory?*/
    {
      score_stream = fopen(score_file, "r+");
      /* If we cannot open it, try creating it */
      if(!score_stream)
	score_stream = fopen(score_file, "w+");
      if (!score_stream)
	score_file = NULL;
    }
  /*}}}*/
  /*{{{  personal file*/
  {
    if(!home)
      home = getenv("HOME");
    if(home)
      {
	size_t    length;
	
	length = strlen(home);
	personal_file = malloc(length + 15);
	if(personal_file)
	  {
	    strcpy(personal_file, home);
	    strcpy(&personal_file[length], "/.xmris.score");

	    personal_stream = fopen(personal_file, "r+");
	    if(!personal_stream)
	      personal_stream = fopen(personal_file, "w+");
	    if(!personal_stream)
	      {
		free(personal_file);
		personal_file = NULL;
	      }
	  }
      }
  }
  /*}}}*/
  if(data.expire || data.format || data.remove)
    data.scores = True;
  load_check_expire_insert(0);
  return;
}
/*}}}*/
/*{{{  void init_scores()*/
extern VOIDFUNC cleanup_scores FUNCARGVOID
/*
 * Close open score files
 */
{
  if(score_stream)
    {
      fclose(score_stream);
      score_stream = NULL;
    }
  if(personal_stream)
    {
      fclose(personal_stream);
      personal_stream = NULL;
    }
}
/*}}}*/
/*{{{  unsigned insert_personal(sptr)*/
static unsigned insert_personal
FUNCARG((sptr),
	HIGH_SCORE CONST *sptr
)
/* 
 * inserts a score into the personal file
 */
{
  HIGH_SCORE  new;
  char CONST *ptr;
  
  memcpy(&new, sptr, sizeof(new));
  ptr = ctime(&new.stamp);
  memcpy(&new.name[0], &ptr[11], 5);
  memcpy(&new.name[5], &ptr[7], 3);
  memcpy(&new.name[8], &ptr[3], 5);
  memcpy(&new.name[13], &ptr[22], 2);
  new.name[15] = 0;
  return insert_score(scoring.personal, &new, 0);
}
/*}}}*/
/*{{{  unsigned insert_score(sptr, iptr, multiple)*/
static unsigned insert_score
FUNCARG((sptr, iptr, multiple),
	HIGH_SCORE *sptr          /* table */
ARGSEP  HIGH_SCORE *iptr          /* new score */
ARGSEP  unsigned  multiple        /* multiple entries */
)
/*
 * inserts a score into a high score table
 */
{
  unsigned  ix;
  unsigned  inserted;
  
  inserted = 0;
  iptr->marker = 0;
  if(iptr->score)
    {
      for(ix = HIGH_SCORES; ix--; sptr++)
	if(sptr->score > iptr->score)
	  {
	    if(!multiple && !strcmp(sptr->name, iptr->name))
	      break;
	  }
	else if(sptr->score < iptr->score || sptr->stamp < iptr->stamp)
	  {
	    HIGH_SCORE  *eptr;
	    
	    for(eptr = sptr; ix--; eptr++)
	      if(!eptr->score)
		{
		  if(ix)
		    eptr[1].score = 0;
		  break;
		}
	      else if(!multiple && !strcmp(eptr->name, iptr->name))
		break;
	    for(; eptr != sptr; eptr--)
	      memcpy(eptr, eptr - 1, sizeof(HIGH_SCORE));
	    memcpy(sptr, iptr, sizeof(HIGH_SCORE));
	    inserted = 1;
	    break;
	  }
	else
	  {
	    strcpy(sptr->name, iptr->name);
	    break;
	  }
    }
  return inserted;
}
/*}}}*/
/*{{{  void load_check_expire_insert(insert)*/
static VOIDFUNC load_check_expire_insert
FUNCARG((insert),
	unsigned  insert
)
/* this does most of the high score file manipulation
 * it loads up the high score files
 * checks the integrity of the personal score vs high score
 * optionally inserts a new score into the tables
 * optionally expires scores
 * write the files back
 */
{
  unsigned do_score = 0, do_personal = 0;
  unsigned score_lock = 0, personal_lock = 0;

  if(score_stream)
    {
      score_lock = get_lock(score_stream);
      if(score_lock)
	do_score = merge_scores(score_stream);
    }

  if(personal_stream)
    {
      personal_lock = get_lock(personal_stream);
      if(personal_lock)
	do_personal = merge_personal(personal_stream);
    }

  /*{{{  check alternate*/
  if(alternate)
    {
      unsigned  count;
      unsigned  ix;
      HIGH_SCORE *sptr;
      
      for(ix = 2; ix--;)
	for(sptr = tables[ix], count = HIGH_SCORES; count-- && sptr->score;
	    sptr++)
	  if(!strcmp(sptr->name, alternate))
	    {
	      strcpy(sptr->name, scoring.mine.name);
	      do_score = 1;
	    }
    }
  /*}}}*/
  /*{{{  check personal*/
  {
    unsigned  count;
    HIGH_SCORE *sptr;
    HIGH_SCORE *hptr;
    
    for(count = HIGH_SCORES, hptr = scoring.high;
	count-- && hptr->score; hptr++)
      if(!strcmp(hptr->name, scoring.mine.name))
	{
	  if(hptr->score > scoring.personal[0].score)
	    do_personal |= insert_personal(hptr);
	  break;
	}
    if(hptr - scoring.high == HIGH_SCORES || !hptr->score)
      hptr = NULL;
    for(count = HIGH_SCORES, sptr = scoring.personal;
	count-- && sptr->score; sptr++)
      if(sptr->score >= SCORE_THRESHOLD &&
	  score_file && (!hptr || sptr->score > hptr->score))
	{
	  do_personal = 1;
	  sptr->marker = 1;
	}
    remove_scores(scoring.personal, 1);
  }
  /*}}}*/
  /* expire person? */
  if(getuid() != 0)
    {
      if(data.remove)
	fprintf(stderr, "Removing a person can only be done by root");
      if(data.format)
	fprintf(stderr, "Changing the date format can only be done by root");
      data.remove = data.format = NULL;
    }
  else
    {
      /*{{{  set date format?*/
      if(data.format)
	{
	  unsigned  ix;
	  
	  for(ix = 0; ix != 3; ix++)
	    if(!data.format[ix] || !strchr(date_formats, data.format[ix]) ||
		strchr(&data.format[ix + 1], data.format[ix]))
	      break;
	  if(data.format[3] || ix != 3)
	    fprintf(stderr, "Invalid format '%s'", data.format);
	  else
	    {
	      strcpy(date_format, data.format);
	      do_score = 1;
	    }
	  data.format = NULL;
	}
      /*}}}*/
      if(data.remove)
	{
	  unsigned  found;
	  unsigned  index;
	  char CONST *truename;
	  char CONST *home;
	  unsigned  personal;
	  
	  personal = 0;
	  truename = home = NULL;
	  /*{{{  find name & home*/
	  {
	    struct passwd *ptr;
	  
	    ptr = getpwnam(data.remove);
	    if(ptr)
	      {
		home = ptr->pw_dir;
		truename = ptr->pw_gecos;
	      }
	  }
	  /*}}}*/
	  /*{{{  search*/
	  for(found = index = 0; index != 2; index++)
	    {
	      HIGH_SCORE  *sptr;
	      unsigned  count;
	    
	      for(sptr = tables[index], count = HIGH_SCORES; count--; sptr++)
		if(!sptr->score)
		  break;
		else if(!strcmp(sptr->name, data.remove) ||
		    (truename && !strcmp(sptr->name, truename)))
		  {
		    sptr->marker = 1;
		    remove_scores(tables[index], 1);
		    found = 1;
		    break;
		  }
	    }
	  /*}}}*/
	  do_score |= found;
	  /*{{{  try personal files too*/
	  {
	    /*{{{  home directory?*/
	    {
	      if(home)
		{
		  char      *file;
		  size_t    length;
		
		  length = strlen(home);
		  file = malloc(length + 15);
		  if(file)
		    {
		      strcpy(file, home);
		      strcpy(&file[length], "/.xmris.score");
		      if(!unlink(file))
			{
			  personal = 1;
			  found = 1;
			}
		      free(file);
		    }
		}
	    }
	    /*}}}*/
	  }
	  /*}}}*/
	  if(found)
	    printf("Removed '%s'%s\n", data.remove,
		personal ? "" : ", no personal score table");
	  else
	    printf("No entry or personal score table for '%s'\n", data.remove);
	  data.remove = NULL;
	}
    }
  /*{{{  expire date?*/
  if(data.expire)
  {
    /* valid date separators are ' ' ':' '-' '/'
     * day month year in any order,
     * month can be ascii
     * year can be 2 or 4 digits, 00-80 -> 2000-2080
     * 81-99 -> 1981-1999
     * uses date_format to resolve ambiguity
     * or give n{years|months|weeks|days|hours|minutes|seconds},
     *    to expire those older/younger than n days/weeks ago
     * a leading sign specifies older ('+') or younger ('-'). default is older
     */
    /*{{{  month names*/
    static char CONST *CONST months[] =
      {
	"January", "February", "March", "April",
	"May", "June", "July", "August",
	"September", "October", "November", "December",
	NULL
      };
    /*}}}*/
    /*{{{  typedef struct Scale*/
    typedef struct Scale
    {
      char CONST *name;
      unsigned long factor;
    } SCALE;
    /*}}}*/
    /*{{{  time scales*/
    static SCALE CONST scales[] =
      {
	{"years", (365 * 4 + 1) * 6 * 60 * 60},
	{"months", 61 * 12 * 60 * 60},
	{"weeks", 7 * 24 * 60 * 60},
	{"days", 24 * 60 * 60},
	{"hours", 60 * 60},
	{"minutes", 60},
	{"seconds", 1},
	{NULL}
      };
    /*}}}*/
    unsigned long seconds;
    int       values[3];
    int       date[3];
    unsigned  ix;
    char CONST *ptr;
    int       error;
    char      format[4];
    int       rel;
    int       after;
    
    after = 1;
    for(ix = 3; ix--;)
      {
	date[ix] = values[ix] = -1;
	format[ix] = ' ';
      }
    format[3] = 0;
    error = 0;
    ptr = data.expire;
    after = (*ptr == '-') - (*ptr == '+');
    if(after)
      ptr++;
    /*{{{  try relative*/
    {
      char      *tmp;
      
      seconds = strtol(ptr, &tmp, 10);
      if(*tmp)
	{
	  SCALE CONST *sptr;
	  size_t    length;
	    
	  length = strlen(tmp);
	  for(sptr = scales; sptr->name; sptr++)
	    if(!strncmp(tmp, sptr->name, length))
	      {
		seconds *= sptr->factor;
		tmp += length;
		break;
	      }
	}
      else
	seconds *= 24 * 60 * 60;
      rel = !*tmp;
      if(rel)
	{
	  ptr = tmp;
	  after = after > 0;
	}
    }
    /*}}}*/
    if(!rel)
      {
	int       amb;
	
	after = after < 0;
	/*{{{  parse the string*/
	for(ix = 0; ix != 3; ix++)
	  {
	    size_t    length;
	    int       type;
	    int     value;
	   
	    length = 0;
	    value = type = -1;
	    if(isdigit(*ptr))
	      /*{{{  number*/
	      {
		char    *eptr;
		
		value = (int)strtol(ptr, &eptr, 10);
		length = eptr - ptr;
		if(value > 99)
		  type = 2;
		else if(value > 89)
		  {
		    type = 2;
		    value += 1900;
		  }
		else if(value > 31 || !value)
		  {
		    type = 2;
		    value += 2000;
		  }
	      }
	      /*}}}*/
	    else if(isalpha(*ptr))
	      /*{{{  month*/
	      {
		char CONST * CONST *mptr;
		
		while(isalpha(ptr[length]))
		  length++;
		for(mptr = months; *mptr; mptr++)
		  if(length <= strlen(*mptr))
		    {
		      size_t  ix;
		      
		      for(ix = 0; ix != length; ix++)
			if(tolower((*mptr)[ix]) != tolower(ptr[length]))
			  break;
		      if(ix == length)
			break;
		    }
		if(*mptr)
		  {
		    type = 1;
		    value = mptr - months + 1;
		  }
		else
		  error = 1;
	      }
	      /*}}}*/
	    else
	      error = 1;
	    if(type < 0)
	      values[ix] = value;
	    else if(date[type] >= 0)
	      error = 1;
	    else
	      {
		date[type] = value;
		format[ix] = date_formats[type];
	      }
	    ptr += length;
	    if(*ptr && !isalnum(*ptr) && ix != 2)
	      ptr++;
	  }
	/*}}}*/
	amb = strchr(format, ' ') != NULL;
	/*{{{  disambiguate*/
	{
	  int       flag;
	  
	  for(flag = 0; flag != 2; flag++)
	    {
	      /*{{{  use format*/
	      for(ix = 0; ix != 3; ix++)
		if(values[ix] > 0)
		  {
		    if(format[ix] == ' ' &&
			!strchr(format, date_format[ix]))
		      format[ix] = date_format[ix];
		    switch(format[ix])
		    {
		      case 'D':
			date[0] = values[ix];
			values[ix] = -1;
			break;
		      case 'M':
			if(values[0] > 12)
			  format[ix] = ' ';
			else
			  {
			    date[1] = values[ix];
			    values[ix] = -1;
			  }
			break;
		      case 'Y':
			date[2] = values[ix] + 1900;
			if(date[2] < 1990)
			  date[2] += 100;
			values[ix] = -1;
			break;
		      default:
			format[ix] = ' ';
		    }
		  }
	      /*}}}*/
	      if(!flag)
		{
		  char      *ptr;
		  
		  ptr = strchr(format, ' ');
		  if(ptr && !strchr(ptr + 1, ' '))
		    {
		      int   temp;
	      
		      for(temp = ' ', ix = 0; ix != 3; ix++)
			temp += date_formats[ix] - format[ix];
		      *ptr = temp;
		    }
		  else
		    break;
		}
	    }
	}
	/*}}}*/
	if(amb && !strchr(format, ' '))
	  {
	    char    reply[5];
	    
	    printf("Ambiguous date '%s': accept %d %s %d y/n(y)?",
		data.expire, date[0], months[date[1] - 1], date[2]);
	    fflush(stdout);
	    if(!fgets(reply, sizeof(reply), stdin))
	      error = 1;
	    else if(*reply != '\n' && uppercase(*reply) != 'Y')
	      error = 1;
	  }
      }
    error |= *ptr;
    if((!rel && strchr(format, ' ')) || error)
      fprintf(stderr, "Bad or ambiguous date '%s'\n", data.expire);
    else
      {
	time_t    expire;
	unsigned  changed;
	
	changed = 0;
	if(rel)
	  {
	    expire = time((time_t *)NULL);
	    expire -= seconds;
	  }
	else
	  {
	    struct tm date_tm;
	    
	    date_tm.tm_sec = 0;
	    date_tm.tm_min = 0;
	    date_tm.tm_hour = 0;
	    date_tm.tm_yday = -1;
	    date_tm.tm_mday = date[0];
	    date_tm.tm_mon = date[1] - 1;
	    date_tm.tm_year = date[2] - 1900;
	    expire = mktime(&date_tm);
	  }
	if(expire == -1)
	  fprintf(stderr, "Not a valid date\n");
	else
	  /*{{{  expire them*/
	  {
	    unsigned  ix;
	    unsigned  found;
	  
	    for(found = 0, ix = 3; ix--;)
	      {
		HIGH_SCORE  *sptr;
		unsigned  count;
	    
		for(sptr = tables[ix], count = HIGH_SCORES;
		    count-- && sptr->score; sptr++)
		  if(((ix == 2) || !strcmp(sptr->name, scoring.mine.name) ||
		      (alternate && !strcmp(sptr->name, alternate))) &&
		      (after ? expire < sptr->stamp : expire > sptr->stamp))
		    {
		      sptr->marker = 1;
		      found |= 1 << ix;
		    }
		remove_scores(tables[ix], 1);
	      }
	    do_personal |= found & 4;
	    /*{{{  write high_scores?*/
	    if(found & 3 || changed)
	      {
		if(found & 1 &&
		    scoring.personal[0].score >= SCORE_THRESHOLD)
		  {
		    HIGH_SCORE  score;
		    
		    make_unique(&scoring.personal[0]);
		    memcpy(&score, &scoring.personal[0], sizeof(score));
		    scoring.personal->stamp = score.stamp;
		    strcpy(score.name, scoring.mine.name);
		    insert_score(scoring.high, &score, 0);
		  }
		do_score = 1;
	      }
	    /*}}}*/
	  }
	  /*}}}*/
      }
    data.expire = NULL;
  }
  /*}}}*/
  /*{{{  insert?*/
  if(insert)
    {
      HIGH_SCORE *inserted;
      
      make_unique(&scoring.mine);
      inserted = NULL;
      if(insert_personal(&scoring.mine))
	{
	  inserted = scoring.personal;
	  do_personal = 1;
	}
      if(insert_score(scoring.today, &scoring.mine, !score_file))
	{
	  inserted = scoring.today;
	  do_score = 1;
	}
      if(scoring.mine.score >= SCORE_THRESHOLD &&
	  insert_score(scoring.high, &scoring.mine, !score_file))
	{
	  inserted = scoring.high;
	  do_score = 1;
	}
      if(inserted)
	scoring.display = inserted;
    }
  /*}}}*/
  /*{{{  write_score?*/
  if(do_score && score_lock)
    {
      write_scores(score_stream);
      file_changed(score_file);
    }
  /*}}}*/
  /*{{{  write personal?*/
  if(do_personal && personal_lock)
    {
      write_personal(personal_stream);
      file_changed(personal_file);
    }
  /*}}}*/
  if(personal_lock)
    get_unlock(personal_stream);
  if(score_lock)
    get_unlock(score_stream);
  return;
}
/*}}}*/
/*{{{  void make_unique(sptr)*/
static VOIDFUNC make_unique
FUNCARG((hptr),
	HIGH_SCORE *hptr
)
/* makes sure that the score has a unique stamp
 * compared to the high and today score tables
 */
{
  HIGH_SCORE *sptr;
  unsigned  ix;
  unsigned  count;
  unsigned  retry;
  
  do
    {
      retry = 0;
      for(ix = 2; ix--;)
	for(sptr = tables[ix], count = HIGH_SCORES;
	    count-- && sptr->score; sptr++)
	  if(sptr->score == hptr->score && sptr->stamp == hptr->stamp)
	    {
	      hptr->stamp++;
	      retry = 1;
	    }
    }
  while(retry);
  return;
}
/*}}}*/
/*{{{  unsigned merge_personal(stream)*/
static unsigned merge_personal
FUNCARG((stream),
	FILE      *stream
)
/*
 * loads the personal score file into the personal score
 * table. If the score file is corrupted, then we lose the
 * merged entries otherwise we lose the non-loaded entries.
 * The file must have been locked appropriately.
 * returns non-zero if the file is unreadable
 */
{
  unsigned long check;
  unsigned  failed;
  HIGH_SCORE  new;
  
  clearerr(stream);
  rewind(stream);
  check = 0;
  scoring.personal[0].score = 0;
  /*{{{  read file*/
  while(1)
    {
      check = read_line(stream, &new, check);
      if(!new.score)
	break;
      insert_score(scoring.personal, &new, 0);
    }
  /*}}}*/
  failed = ferror(stream) || check != new.stamp;
  if(failed)
    {
      fprintf(stderr, "%s:Your personal score file '%s' has been corrupted.\n",
	  myname, personal_file);
      scoring.personal[0].score = 0;
    }
  return failed;
}
/*}}}*/
/*{{{  unsigned merge_scores(stream)*/
static unsigned merge_scores
FUNCARG((stream),
	FILE      *stream
)
/*
 * merges the high score file into the current high score
 * table. If the score file is corrupted, then we lose the
 * merged entries. The file must have been locked appropriately.
 * returns non-zero if the file is unreadable
 */
{
  unsigned long check;
  unsigned  failed;
  time_t    now;
  HIGH_SCORE  new;
  
  clearerr(stream);
  rewind(stream);
  now = time((time_t *)NULL);
  scoring.high[0].score = 0;
  scoring.today[0].score = 0;
  check = 0;
  /*{{{  read file*/
  while(1)
    {
      check = read_line(stream, &new, check);
      if(!new.score)
	break;
      if(new.score >= SCORE_THRESHOLD)
	insert_score(scoring.high, &new, 0);
      if(!expire(now, new.stamp))
	insert_score(scoring.today, &new, 0);
    }
  /*}}}*/
  failed = ferror(stream) || check != new.stamp;
  if(failed)
    {
      fprintf(stderr, "%s:The high score file '%s' has been corrupted.\n",
	  myname, score_file);
      scoring.high[0].score = 0;
      scoring.today[0].score = 0;
    }
  return failed;
}
/*}}}*/
/*{{{  unsigned long read_line(stream, sptr, check)*/
static unsigned long read_line
FUNCARG((stream, sptr, check),
	FILE      *stream
ARGSEP  HIGH_SCORE *sptr
ARGSEP  unsigned long check
)
{
  char  line[NAME_LEN + 40];
  
  if(!fgets(line, sizeof(line), stream))
    sptr->score = sptr->stamp = 0;
  else if(line[0] == '+')
    {
      char      *ptr;
      size_t    index;
      
      sptr->score = 0;
      sptr->stamp = strtol(&line[1], &ptr, 10);
      while(*ptr == ' ')
	ptr++;
      for(index = 0; isalpha(*ptr) && index < 3; ptr++, index++)
	date_format[index] = *ptr;
      if(*ptr != '\n')
	sptr->stamp = 0;
    }
  else
    {
      size_t    length;
      char      *name;
      char      *ptr;
      
      for(ptr = line; *ptr; ptr++)
	check += *(unsigned char *)ptr;
      sptr->stamp = strtol(line, &ptr, 10);
      while(*ptr == ' ')
	ptr++;
      sptr->score = strtol(ptr, &ptr, 10) / SCORE_ROUND * SCORE_ROUND;
      while(*ptr == ' ')
	ptr++;
      sptr->screen = (unsigned)strtol(ptr, &ptr, 10);
      while(*ptr == ' ')
	ptr++;
      sptr->elapsed = (unsigned)strtol(ptr, &name, 10);
      if(name == ptr)
	sptr->elapsed = 0;
      else
	for(ptr = name; *ptr == ' ';)
	  ptr++;
      name = ptr;
      length = strlen(ptr) - 1;
      if(!sptr->score || !sptr->screen ||
	  ptr[length] != '\n' || length > NAME_LEN)
	sptr->score = 0;
      name[length] = 0;
      strcpy(sptr->name, name);
      sptr->marker = 1;
    }
  return check;
}
/*}}}*/
/*{{{  static VOIDFUNC remove_scores(base, flag)*/
static VOIDFUNC remove_scores
FUNCARG((base, flag),
	HIGH_SCORE *base
ARGSEP  unsigned  flag
)
/*
 * throw out the marked scores, 'cos they're from a corrupt score file
 * or just too darn old
 */
{
  HIGH_SCORE  *ptr;
  HIGH_SCORE  *dest;
  unsigned  ix;
	
  for(ptr = base, ix = HIGH_SCORES; ix--; ptr++)
    {
      if(flag && ptr->marker)
	ptr->score = 0;
      ptr->marker = 0;
    }
  for(ptr = dest = base, ix = HIGH_SCORES; ix--; ptr++)
    if(!ptr->score)
      /* EMPTY */;
    else if(ptr == dest)
      dest++;
    else
      {
	memcpy(dest, ptr, sizeof(HIGH_SCORE));
	dest++;
      }
  for(;dest != ptr; dest++)
    dest->score = 0;
  return;
}
/*}}}*/
/*{{{  static VOIDFUNC retire_scores()*/
static VOIDFUNC retire_scores FUNCARGVOID
/*
 * gracefully retire the wrinkly scores
 */
{
  HIGH_SCORE *ptr;
  unsigned  count;
  time_t    now;
  
  now = time((time_t *)NULL);
  for(ptr = scoring.today, count = 0; count != HIGH_SCORES && ptr->score;
      ptr++, count ++)
    ptr->marker = expire(now, ptr->stamp);
  remove_scores(scoring.today, 1);
  return;
}
/*}}}*/
/*{{{  void write_personal(stream)*/
static VOIDFUNC write_personal
FUNCARG((stream),
      FILE    *stream
)
/*
 * writes out the personal score table to the file.
 * the file must have been locked appropriately.
 */
{
  unsigned long check;

  clearerr(stream);
  rewind(stream);
  check = 0;
  check = write_table(stream, scoring.personal, check);
  if(ferror(stream))
    {
      clearerr(stream);
      rewind(stream);
      check = 0;
    }
  fprintf(stream, "+%lu\n", check);
  fflush(stream);
  return;
}
/*}}}*/
/*{{{  void write_scores(stream)*/
static VOIDFUNC write_scores
FUNCARG((stream),
      FILE    *stream
)
/*
 * writes out the high score table to the file.
 * the file must have been locked appropriately.
 */
{
  unsigned long check;

  clearerr(stream);
  rewind(stream);
  check = 0;
  check = write_table(stream, scoring.high, check);
  check = write_table(stream, scoring.today, check);
  if(ferror(stream))
    {
      clearerr(stream);
      rewind(stream);
      check = 0;
    }
  fprintf(stream, "+%lu %s\n", check, date_format);
  fflush(stream);
  return;
}
/*}}}*/
/*{{{  unsigned long write_table(stream, sptr, check)*/
static unsigned long write_table
FUNCARG((stream, sptr, check),
	FILE      *stream
ARGSEP  HIGH_SCORE *sptr
ARGSEP  unsigned long check
)
{
  unsigned  ix;
  
  for(ix = HIGH_SCORES; ix-- && sptr->score; sptr++)
    {
      char      line[NAME_LEN + 40];
      char      *ptr;
      
      sprintf(line, "%lu %lu %u %u %s\n", (unsigned long)sptr->stamp,
	  sptr->score, sptr->screen, sptr->elapsed, sptr->name);
      fputs(line, stream);
      for(ptr = line; *ptr; ptr++)
	check += *(unsigned char *)ptr;
    }
  return check;
}
/*}}}*/
