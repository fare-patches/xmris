/*{{{  (C) 1992 Nathan Sidwell*/
/*****************************************************************************
			X M R I S V1.01
			---------------
		    (C) 1992 Nathan Sidwell

This program is copyright (C) 1992 Nathan Sidwell. This software and documentation
is in the public domain. Permission is granted to distribute and compile
verbatim copies of this software for non-commercial, non-profit use,
without fee. The software may be modified, provided that both the above copyright
notice and this permission notice appear.

No guarantee is given as to the robustness or suitability of this
software for your computer.

Nathan Sidwell  INMOS UK |                 | nathan@inmos.co.uk       DoD#0390
*****************************************************************************/
/*}}}*/
#include "xmris.h"
#include <time.h>
/*{{{  prototypes*/
static void age_scores PROTOARGLIST((void));
static void parse_args PROTOARGLIST((int, char **));
/*}}}*/
/*{{{  void add_score(increment, x, y)*/
extern void add_score FUNCARGLIST((points, x, y))
int       points  FUNCARGSEP
int       x       FUNCARGSEP
int       y       FUNCARGTERM
/*
 * adds the given score (which may be zero)
 * and displays it at the top of the screen
 * if the coordinate > 0 then add the score into the onboard list
 */
{
  player.score += points;
  /*{{{  text score*/
  {
    unsigned  length;
    char      text[10];
    int       ascent, descent;
    int       direction;
    XCharStruct chars;
    int       x, y;
	
    length = itoa(text, player.score, 0);
    XQueryTextExtents(display.display, font.font, text, length,
	&direction, &ascent, &descent, &chars);
    x = BORDER_LEFT + (CELL_WIDTH + GAP_WIDTH) * 4 -
	CELL_WIDTH / 2 - chars.width;
    y = (CELL_HEIGHT - ascent - descent) / 2 + ascent +
	BORDER_TOP - CELL_HEIGHT;
    XDrawImageString(display.display, display.back, GCN(GC_TEXT),
	x, y, text, length);
    add_background(x, y - ascent, chars.width, ascent + descent);
  }
  /*}}}*/
  /*{{{  board score?*/
  if(y)
    {
      unsigned  length;
      char      text[10];
      int       i;
      SCORE     *sptr;
      SPRITE    *dptr;
	
      dptr = &sprites[SPRITE_DIGITS];
      length = itoa(text, points, 0);
      /*{{{  remove oldest score?*/
      if(update.score.scores == BOARD_SCORES)
	{
	  add_background(update.score.list[0].place.x,
	      update.score.list[0].place.y,
	      DIGIT_WIDTH * 4, DIGIT_HEIGHT);
	  update.score.scores--;
	  memmove(&update.score.list[0], &update.score.list[1],
	      sizeof(SCORE) * update.score.scores);
	}
      /*}}}*/
      sptr = &update.score.list[update.score.scores++];
      sptr->count = SCORE_SHOW;
      sptr->place.x = x - DIGIT_WIDTH * 2;
      sptr->place.y = y - DIGIT_HEIGHT / 2;
      /*{{{  centering*/
      if(length != 4)
	{
	  x = (4 - length) * (DIGIT_WIDTH / 2);
	  XCopyArea(display.display, dptr->image, sptr->image, GCN(GC_COPY),
	      10 * DIGIT_WIDTH, 0, x, DIGIT_HEIGHT, 0, 0);
	  XCopyArea(display.display, dptr->image, sptr->image, GCN(GC_COPY),
	      10 * DIGIT_WIDTH, 0, x, DIGIT_HEIGHT, 4 * DIGIT_WIDTH - x, 0);
	  XCopyArea(display.display, dptr->mask, sptr->mask, GCN(GC_COPY),
	      10 * DIGIT_WIDTH, 0, x, DIGIT_HEIGHT, 0, 0);
	  XCopyArea(display.display, dptr->mask, sptr->mask, GCN(GC_COPY),
	      10 * DIGIT_WIDTH, 0, x, DIGIT_HEIGHT, 4 * DIGIT_WIDTH - x, 0);
	}
      else
	x = 0;
      /*}}}*/
      for(i = 0; i < length; i++, x += DIGIT_WIDTH)
	{
	  XCopyArea(display.display, dptr->image, sptr->image, GCN(GC_COPY),
	      (text[i] - '0') * DIGIT_WIDTH, 0, DIGIT_WIDTH, DIGIT_HEIGHT,
	      x, 0);
	  XCopyArea(display.display, dptr->mask, sptr->mask, GCN(GC_COPY),
	      (text[i] - '0') * DIGIT_WIDTH, 0, DIGIT_WIDTH, DIGIT_HEIGHT,
	      x, 0);
	}
    }
  /*}}}*/
  return;
}
/*}}}*/
/*{{{  void age_scores()*/
static void age_scores FUNCARGVOID
/*
 * ages the onboard scores, and removes the old ones
 */
{
  SCORE     *sptr;
  int       i;

  for(sptr = update.score.list, i = update.score.scores; i--; sptr++)
    if(!sptr->count--)
      {
	Pixmap    image, mask;
	
	add_background(sptr->place.x, sptr->place.y,
	    DIGIT_WIDTH * 4, DIGIT_HEIGHT);
	mask = sptr->mask;
	image = sptr->image;
	memmove(sptr, sptr + 1, i * sizeof(SCORE));
	sptr[i].mask = mask;
	sptr[i].image = image;
	update.score.scores--;
	sptr--;
     }
  return;
}
/*}}}*/
/*{{{  void calc_distances()*/
extern void calc_distances FUNCARGVOID
/*
 * sets the distances from each cell to the player
 * this is so the monsters have non-local knowlegde
 * increment the non-zero cells
 * this proceeds as a sort of flood fill operation, starting
 * from the player's cell and moving outwards
 */
{
  CELL    **aptr, **sptr;
  CELL    *list[2][FLOOD_FILL];
  CELL    *cptr;
  int     toggle;
  int     x, y;
  int     count;

  global.broken = 0;
  for(y = CELLS_DOWN; y--;)
    {
      cptr = BOARDCELL(0, y);
      for(x = CELLS_ACROSS; x--; cptr++)
	cptr->distance = cptr->visit ? 0 : 255;
    }
  toggle = 0;
  cptr = BOARDCELL(monster.list[0].cell.x, monster.list[0].cell.y);
  cptr->distance = count = 1;
  list[0][0] = cptr;
  list[0][1] = NULL;
  while(list[toggle][0])
  {
    sptr = list[toggle];
    toggle = !toggle;
    aptr = list[toggle];
    count++;
    while(cptr = *sptr++)
      {
	CELL      *tptr;
	
	/*{{{  go up?*/
	if(cptr->depths[0])
	  {
	    tptr = cptr - CELL_STRIDE;
	    if(tptr->visit && !tptr->distance)
	      {
		tptr->distance = count;
		*aptr++ = tptr;
	      }
	  }
	/*}}}*/
	/*{{{  go down?*/
	if(cptr->depths[1])
	  {
	    tptr = cptr + CELL_STRIDE;
	    if(tptr->visit && !tptr->distance)
	      {
		tptr->distance = count;
		*aptr++ = tptr;
	      }
	  }
	/*}}}*/
	/*{{{  go left?*/
	if(cptr->depths[2])
	  {
	    tptr = cptr - 1;
	    if(tptr->visit && !tptr->distance)
	      {
		tptr->distance = count;
		*aptr++ = tptr;
	      }
	  }
	/*}}}*/
	/*{{{  go right?*/
	if(cptr->depths[3])
	  {
	    tptr = cptr + 1;
	    if(tptr->visit && !tptr->distance)
	      {
		tptr->distance = count;
		*aptr++ = tptr;
	      }
	  }
	/*}}}*/
	assert(aptr - list[toggle] < FLOOD_FILL);
      }
    *aptr = NULL;
  }
  return;
}
/*}}}*/
/*{{{  void fatal_error(text, ...)*/
extern void fatal_error FUNCARGLIST((text VARARG))
char const *text FUNCARGSEP
FUNCVARARG
{
  va_list   args;
  
  VARARGSET(args, text);
  vfprintf(stderr, text, args);
  fputc('\n', stderr);
  release_resources();
  exit(1);
}
/*}}}*/
/*{{{  int itoa(text, n, width)*/
extern int itoa FUNCARGLIST((text, number, digits))
char    *text   FUNCARGSEP  /* output text (include 0) */
int     number  FUNCARGSEP  /* number to convert */
int     digits  FUNCARGTERM /* field width to convert into */
/*
 * formats an integer to a string
 * in the specified number of digits
 * pads leading zeros to ' '
 * returns the number of characters used
 */
{
  char    reverse[10];
  int     l, length;

  l = 0;
  do
    {
      reverse[l++] = number % 10 + '0';
      number /= 10;
    }
  while(number);
  if(!digits)
    length = 0;
  else if(l < digits)
    {
      length = digits - l;
      memset(text, ' ', length);
    }
  else
    {
      length = 0;
      l = digits;
    }
  while(l)
    text[length++] = reverse[--l];
  text[length] = 0;
  return length;
}
/*}}}*/
/*{{{  int main(argc, argv)*/
extern int main FUNCARGLIST((argc, argv))
int     argc    FUNCARGSEP
char    **argv  FUNCARGTERM
{
#ifndef __STDC__
  sprintf(title_text[1], "%s %s", XMRISVERSION, DATE);
#endif
  /*{{{  set defaults*/
  {
    argv[argc] = NULL;
    display.name = NULL;
    font.name = FONT_NAME;
    flags.gender = GAME_GENDER;
  }
  /*}}}*/
  parse_args(argc, argv);
  /*{{{  help?*/
  if(flags.help)
    {
      ARG const *aptr;
      char const *ptr;
	
      if(!*argv)
	ptr = game_name;
      else
	{
	  ptr = *argv;
	  for(ptr += strlen(ptr) - 1; ptr != *argv; ptr--)
	    if(ptr[-1] == '/')
	      break;
	}
      fprintf(stderr, "%s [option ...]\n", ptr);
      fprintf(stderr, "%s %s\n", game_name, title_text[1]);
      fprintf(stderr, "%s\n", title_text[0]);
      for(aptr = args; aptr->arg; aptr++)
	fprintf(stderr, "  %-8s %s\n", aptr->arg, aptr->help);
      return 0;
    }
  /*}}}*/
  create_resources(argc, argv);
  timer_open();
  XMapWindow(display.display, display.window);
  XSelectInput(display.display, display.window, display.event_mask |
      KeyPressMask | ButtonPressMask | KeyReleaseMask | PointerMotionMask);
  while(!demo_mode())
    /*{{{  game*/
    {
      extra.got = 0;
      extra.select = 0;
      extra.escape = 0;
      extra.score = 0;
      create_xtra_monster(0);
      player.lives = START_LIVES;
      player.score = 0;
      player.screen = 0;
      player.pressed = 0;
      while(player.lives)
	{
	  new_board();
	  zoom_board();
	  refresh_window();
	  history.prize <<= 1;
	  history.ending <<= 2;
	  timer_start(FRAME_RATE);
	  do
	    {
	      int     count;
		  
	      count = SCORE_SHOW;
	      /*{{{  initialize stuff*/
	      {
		player.mouse.x = PLAYER_START_X;
		player.mouse.y = PLAYER_START_Y;
		player.mouse_dir = player.next_dir = 0;
		global.state = 0;
		monster.monsters = 0;
		monster.delay = 0;
		monster.den = monster.normals;
		monster.drones = 0;
		spawn_monster(4, 2, 2, PLAYER_START_X, PLAYER_START_Y, 0, 0);
		monster.list[0].stop = 1;
		player.pressed = 0;
		player.ball.state = 8;
		player.ball.count = 0;
		player.old_ball.state = 0;
	      }
	      /*}}}*/
	      calc_distances();
	      draw_center(SPRITE_DEN);
	      show_updates();
	      player.ball.state = 0;
	      /*{{{  game loop*/
	      while(count)
	      {
		age_scores();
		if(process_xevents(1))
		  {
		    player.lives = 1;
		    monster.list[0].shot = 1;
		    global.state = 4;
		    count = 1;
		  }
		if(global.state != 4)
		  move_player();
		move_monsters();
		bounce_ball();
		move_apples();
		if(!global.state)
		  /*{{{  monster escape?*/
		  {
		    if(!monster.delay && random() < DEN_ESCAPE_PROB)
		      monster.delay = DEN_ESCAPE_DELAY * DEN_ESCAPE_FLASH + 1;
		    if(monster.delay)
		      {
			monster.delay--;
			if(!(monster.delay % DEN_ESCAPE_FLASH))
			  draw_center(monster.delay / DEN_ESCAPE_FLASH &
			      1 ? SPRITE_NORMAL + 4 : SPRITE_DEN);
			if(!monster.delay)
			  {
			    MONSTER   *mptr;
											      
			    mptr = spawn_monster(0, 2, 2, DEN_X, DEN_Y, 0, 0);
			    monster.den--;
			    if(!monster.den)
			      {
				global.state++;
				draw_center(SPRITE_PRIZE_BASE +
				  (player.screen - 1) % SPRITE_PRIZES);
			      }
			  }
		      }
		  }
		  /*}}}*/
		else if(global.state == 2 &&
		    !monster.den && !monster.drones)
		  global.state = 3;
		fall_monsters();
		/*{{{  ending?*/
		if(global.state == 4)
		  {
		    if(update.score.scores || player.ball.state)
		      count++;
		    else
		      {
			APPLE   *aptr;
			int     i;
											
			for(aptr = apple.list, i = apple.apples; i--; aptr++)
			  if(aptr->state)
			    {
			      count++;
			      break;
			    }
		      }
		    count--;
		    if(player.ball.state == 1)
		      {
			player.ball.state = 2;
			player.ball.count = 0;
		      }
		    else if(player.ball.state == 3)
		      player.ball.state = 4;
		  }
		/*}}}*/
		/*{{{  extra stuff*/
		if(!extra.escape)
		  {
		    unsigned  temp;
										    
		    if(random() < NEXT_XTRA_PROB)
		      new_xtra();
		    temp = player.score / 5000;
		    if(temp != extra.score)
		      {
			extra.score = temp;
			extra_escape();
		      }
		  }
		else
		  extra.score = player.score / 5000;
		/*}}}*/
		if(!global.cherries || !monster.normals ||
		    monster.list[0].shot || extra.got == 0x1F)
		  global.state = 4;
		if(global.broken)
		  calc_distances();
		show_updates();
		timer_wait();
	      }
	      /*}}}*/
	      if(extra.got == 0x1F)
		/*{{{  extra life*/
		{
		  extra_life();
		  extra.got = 0;
		  create_xtra_monster(extra.select);
		  monster.list[0].shot = 0;
		  history.ending |= 2;
		}
		/*}}}*/
	      else if(!monster.normals)
		{
		  monster.list[0].shot = 0;
		  history.ending |= 1;
		}
	      else if(!global.cherries)
		monster.list[0].shot = 0;
	      else if(monster.list[0].shot)
		/*{{{  die*/
		{
		  unsigned  i;
		  unsigned  base, offset;
		  MONSTER   *mptr;
										
		  player.ball.count = 8;
		  i = monster.list[0].face;
		  if(i >= 6)
		    i = 2 + (i & 1);
		  i = SPRITE_PLAYER + i * MONSTER_IMAGES;
		  for(base = 8; base--;)
		    if(player_dies[base] == i)
		      break;
		  offset = (base + 1) & 3;
		  base = base & 4;
		  for(i = 0; i != 8; i++)
		    {
		      unsigned  delay;
										
		      if(process_xevents(0))
			player.lives = 1;
		      monster.list[0].type =
			  player_dies[base + ((offset + i) & 3)];
		      show_updates();
		      for(delay = DIE_DELAY; delay--;)
			timer_wait();
		    }
		  for(mptr = monster.list, i = monster.monsters; i--; mptr++)
		    add_background(mptr->pixel.x, mptr->pixel.y,
			CELL_WIDTH, CELL_HEIGHT);
		  monster.monsters = 0;
		  player.lives--;
		  if(player.lives)
		    {
		      XFillRectangle(display.display, display.back, GCN(GC_CLEAR),
			  PIXELX(player.lives - 1, 0), PIXELY(CELLS_DOWN, 0),
			  CELL_WIDTH, CELL_HEIGHT);
		      add_background(PIXELX(player.lives - 1, 0),
			  PIXELY(CELLS_DOWN, 0), CELL_WIDTH, CELL_HEIGHT);
		    }
		  if(extra.escape)
		    {
		      extra.escape = 0;
		      draw_extra();
		    }
		  show_updates();
		}
		/*}}}*/
	      if(!monster.list[0].shot && !(player.screen % HISTORY_SHOW))
		show_history();
	    }
	  while(monster.list[0].shot && player.lives);
	  timer_stop();
	}
    }
    /*}}}*/
  release_resources();
  timer_close();
  return 0;
}
/*}}}*/
/*{{{  void parse_args(argc, argv)*/
static void parse_args FUNCARGLIST((argc, argv))
int     argc    FUNCARGSEP
char    **argv  FUNCARGTERM
{
  char      **vptr;
  static char const *names[2] = {"xmris", "xmsit"};

  vptr = argv;
  /*{{{  determine gender of the game*/
  if(*argv)
    {
      unsigned  length;
      unsigned  other;
      unsigned  index;
	
      length = strlen(*argv);
      for(index = 2; index--;)
	{
	  other = strlen(&names[index][1]);
	  if(length >= other &&
	      !strcmp(&(*argv)[length - other], &names[index][1]))
	    flags.gender = index;
	}
    }
  /*}}}*/
  /*{{{  parse all the arguments*/
  while(*++vptr)
    {
      while(**vptr != '-')
	vptr++;
      if(*vptr)
	{
	  ARG const *aptr;
	
	  for(aptr = args; aptr->arg; aptr++)
	    if(!strcmp(aptr->arg, &(*vptr)[1]))
	      {
		if(aptr->flag >= 0)
		  *(unsigned *)aptr->ptr = aptr->flag;
		else
		  *(char **)aptr->ptr = *++vptr;
		break;
	      }
	}
    }
  /*}}}*/
  game_name = names[flags.gender];
  return;
}
/*}}}*/
/*{{{  int process_xevents(pausable)*/
extern int process_xevents FUNCARGLIST((pausable))
int     pausable  FUNCARGTERM
{
  int       quit;
  int       paused;

  player.motionevent = 0;
  quit = 0;
  paused = 0;
  /*{{{  read the events*/
  while(QLength(display.display) || paused)
    {
      XEvent    event;
	
      XNextEvent(display.display, &event);
      if(event.xany.window != display.window)
	continue;
      switch (event.type)
      {
	/*{{{  case KeyPress:*/
	case KeyPress:
	/*
	 * When a key is pressed, we check to see if it is
	 * a control key. If so, then we set the relevant pressed bit.
	 */
	{
	  char      chr;
	  KeySym    symbol;
	  XComposeStatus status;
					  
	  XLookupString(&event.xkey, &chr, 1, &symbol, &status);
	  if(isupper(chr))
	    chr = tolower(chr);
	  if(chr == 'p' && pausable)
	    paused++;
	  else if(paused && chr == ' ')
	    player.motionevent = paused = 0;
	  else if((paused || !pausable) && chr == 'q')
	    {
	      paused = 0;
	      quit = 1;
	    }
	  else if(global.state > 5)
	    player.keyboard = player.button = 1;
	  else if(!paused && player.keyboard)
	    {
	      int       index;
					
	      for(index = 5; index--;)
		if(chr == keystrokes[index])
		  {
		    if(index == 4)
		      player.button = 1;
		    else
		      player.pressed |= 1 << index;
		    break;
		  }
	    }
	  break;
	}
	/*}}}*/
	/*{{{  case KeyRelease:*/
	case KeyRelease:
	/*
	 * when a key is released, we check to see if it is
	 * the controlling direction key. If so, then we stop
	 */
	{
	  if(player.keyboard)
	    {
	      char      chr;
	      KeySym    symbol;
	      XComposeStatus status;
	      unsigned  index;
					    
	      XLookupString(&event.xkey, &chr, 1, &symbol, &status);
	      if(isupper(chr))
		chr = tolower(chr);
	      for(index = 5; index--;)
		if(chr == keystrokes[index])
		  {
		    if(index == 4)
		      player.button = 0;
		    else
		      player.pressed &= ~(1 << index);
		    break;
		  }
	      break;
	    }
	}
	/*}}}*/
	/*{{{  case MotionNotify:*/
	case MotionNotify:
	/*
	 * for mouse motion, we save the coordinate and process the
	 * final one only
	 */
	{
	  player.raw_mouse.x = event.xmotion.x;
	  player.raw_mouse.y = event.xmotion.y;
	  player.motionevent = 1;
	  break;
	}
	/*}}}*/
	/*{{{  case ButtonPress:*/
	case ButtonPress:
	  if(!player.keyboard)
	    player.button = 1;
	  break;
	/*}}}*/
	/*{{{  case ButtonRelease:*/
	case ButtonRelease:
	  if(!player.keyboard && player.throw)
	    player.button = 0;
	  break;
	/*}}}*/
	/*{{{  case Expose:*/
	case Expose:
	  refresh_window();
	  break;
	/*}}}*/
	/*{{{  case UnmapNotify:*/
	case UnmapNotify:
	  paused++;
	  break;
	/*}}}*/
	/*{{{  case MapNotify:*/
	case MapNotify:
	  if(!pausable)
	    paused = 0;
	  break;
	/*}}}*/
	/*{{{  case PropertyNotify:*/
	case PropertyNotify:
	  if(event.xproperty.atom == display.DEC_icon_atom)
	    paused++;
	  break;
	/*}}}*/
	default:
	  break;
      }
      /*{{{  started to pause?*/
      if(paused == 1)
	{
	  char const *text = "Space to continue, Q to quit";
	  TEXT      info;
	  unsigned  length;
		
	  paused++;
	  length = strlen(text);
	  text_size(text, length, &info);
	  add_background(0, PIXELY(CELLS_DOWN, 0), WINDOW_WIDTH, CELL_HEIGHT);
	  XFillRectangle(display.display, display.copy, GCN(GC_CLEAR),
	      0, PIXELY(CELLS_DOWN, 0), WINDOW_WIDTH, CELL_HEIGHT);
	  XDrawImageString(display.display, display.copy, GCN(GC_TEXT),
	      WINDOW_WIDTH / 2 - info.width / 2,
	      PIXELY(CELLS_DOWN, CELL_HEIGHT / 2) +
	      (info.ascent - info.descent) / 2, text, length);
	  XCopyArea(display.display, display.copy, display.window, GCN(GC_COPY),
	      0, PIXELY(CELLS_DOWN, 0), WINDOW_WIDTH, CELL_HEIGHT,
	      0, PIXELY(CELLS_DOWN, 0));
	  XSync(display.display, False);
	}
      /*}}}*/
    }
  /*}}}*/
  return quit;
}
/*}}}*/
/*{{{  unsigned random()*/
extern unsigned random FUNCARGVOID
/*
 * a simple random number generator
 * it generates 8 new bits of number at each call
 * using a 31 bit maximal length linear feedback shift register
 * the taps are bits 0 and 3
 */
{
  unsigned  bits;

  if(!seed)
    seed = time(NULL);
  bits = ((seed >> 3) ^ seed) & 0xFF;
  seed = (seed >> 8) | (bits << 23);
  return bits;
}
/*}}}*/
