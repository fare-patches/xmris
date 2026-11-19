/* Copyright (C) 1993, 1992 Nathan Sidwell */
/* RCS $Id: draw.c 1.2 Tue, 16 Mar 1999 11:28:16 +0000 nathan $ */
#include "xmris.h"
#include "time.h"
/*{{{  board load error masks*/
#define ERROR_FILL              0x0001
#define ERROR_BACKGROUND        0x0002
#define ERROR_APPLES            0x0004
#define ERROR_APPLE_POSITION    0x0008
#define ERROR_PATH              0x0010
#define ERROR_CHAR              0x0020
#define ERROR_STRING            0x0040
#define ERROR_INCONSISTANT      0x0080
#define ERROR_NO_CHERRIES       0x0100
#define ERROR_TOO_FEW_APPLES    0x0200
#define ERROR_NO_DEN            0x0400
#define ERROR_NO_PLAYER         0x0800
#define ERROR_BAD_APPLES        0x1000
#define ERROR_SYNTAX            0x2000
#define ERROR_EOF               0x4000
#define ERROR_READING           0x8000
#define ERROR_MALLOC           0x10000
#define ERROR_OPEN             0x20000
#define ERROR_TOO_MANY         0x40000
#define ERROR_FIX_MASK          0x0DFF
#define ERROR_FATAL_MASK       0x7E000
#define ERROR_KEEP_MASK        0x7C000
/*}}}*/
#define INCLUDE_LIMIT 16
#define FILE_SUFFIX ".gdn"
#define FILE_SUFFIX_LEN 4
/*{{{  typedef struct Include*/
typedef struct Include
{
  char CONST *name;   /* filename NULL for internal */
  long      seek;     /* seek to next line */
  unsigned  error;    /* error line */
  unsigned  line;     /* line number */
  unsigned  start;    /* start of garden line */
  unsigned  garden;   /* garden number */
} INCLUDE;
/*}}}*/
/*{{{  parse*/
static struct
{
  INCLUDE   stack[INCLUDE_LIMIT]; /* include information */
  unsigned  depth;    /* current include depth */
  size_t    limit;    /* size of buffer */
  char      *text;    /* line text */
} parse;
/*}}}*/
/*{{{  static BOARD internal_boards[] =*/
/*
 * 0-9A-F selected apples on background.
 * X      fully connected path w/o cherry
 * x      fully connected path w cherry
 * H-W    explicitly connected path w/o cherry
 * h-w    explicitly connected path w cherry
 * @      cherry on background
 * +      background denying random apple
 * .      background permitting random apple
 */
static BOARD internal_boards[] =
{
#include "digits.gdn"
};
/*}}}*/
/*{{{  static char CONST *text_error[] =*/
static char CONST *text_error[] =
{
  "Bad fill pattern",
  "Bad background colour",
  "Too many apples",
  "Bad apple position",
  "Required paths missing",
  "Bad character",
  "String wrong length",
  "Inconistant paths",
  "No cherries",
  "Too few apple locations",
  "No den",
  "No player",
  "Inconsistant apples",
  "Syntax error",
  "Unexpected EOF",
  "Error reading file",
  "Cannot malloc",
  "Cannot open file",
  "Too many includes",
};
/*}}}*/
/*{{{  prototypes*/
static unsigned check_board PROTOARG((BOARD *));
static VOIDFUNC draw_ball PROTOARG((unsigned));
static VOIDFUNC plot_ball PROTOARG((unsigned, int, int));
static VOIDFUNC read_boards PROTOARG((char CONST *, char CONST *));
static unsigned read_line PROTOARG((FILE *, INCLUDE *));
/*}}}*/
/*{{{  void add_background(x, y, w, h)*/
extern VOIDFUNC add_background
FUNCARG((x, y, width, height),
	int       x
ARGSEP  int       y
ARGSEP  unsigned  width
ARGSEP  unsigned  height
)
/*
 * adds an area to the background update list
 */
{
  BACKGROUND *bptr;

  assert(update.back.backs != BACK_UPDATES);
  bptr = &update.back.list[update.back.backs++];
  bptr->place.x = x;
  bptr->place.y = y;
  bptr->size.x = width;
  bptr->size.y = height;
  return;
}
/*}}}*/
/*{{{  ANIMATE animate_zoom(next)*/
extern FUNCANIMATE(animate_zoom, next)
/*
 * zooms out on the initial board
 * and then return from whence I came
 */
{
  static PROTOANIMATE((*rts));
  static unsigned demo;
  PROTOVOID(*then);
  
  then = (PROTOVOID(*))animate_zoom;
  if(next)
    /*{{{  start*/
    {
      rts = (PROTOANIMATE((*)))next;
      video_fill(display.copy, GCN(GC_CLEAR),
	  BORDER_LEFT + 1, BORDER_TOP + 1, BOARD_WIDTH - 2, BOARD_HEIGHT - 2);
      global.missed = 0;
      refresh_window();
      video_copy(display.back, display.copy, GCN(GC_COPY),
	  BORDER_LEFT + 1, BORDER_TOP + 1, BOARD_WIDTH - 2, BOARD_HEIGHT - 2,
	  BORDER_LEFT + 1, BORDER_TOP + 1);
      /*{{{  do the apple sprites*/
      {
	int       i;
	APPLE     *aptr;
	    
	for(aptr = apple.list, i = apple.apples; i--; aptr++)
	  {
	    SPRITE    *sptr;
	    
	    sptr = &sprites[SPRITE_APPLE];
	    video_copy(sptr->mask, display.copy, GCN(GC_MASK),
		0, 0, CELL_WIDTH, CELL_HEIGHT, aptr->pixel.x, aptr->pixel.y);
	    video_copy(sptr->image, display.copy, GCN(GC_OR),
		0, 0, CELL_WIDTH, CELL_HEIGHT, aptr->pixel.x, aptr->pixel.y);
	  }
      }
      /*}}}*/
      demo = global.state == MODE_DEMO;
      timer_set(ZOOM_RATE, TIMING_PAUSE);
      global.count = 0;
      global.state = MODE_ZOOM;
    }
    /*}}}*/
  if(global.count >= BOARD_HEIGHT / (2 * ZOOM_Y) || global.throw == 1 ||
      (demo && (global.pause ||
      global.pressed & (1 << KEY_QUIT | 1 << KEY_KEYBOARD))))
    /*{{{  end*/
    {
      if(!demo && global.count < BOARD_HEIGHT / (2 * ZOOM_Y))
	present_area(BORDER_LEFT + 1, BORDER_TOP + 1,
	    BOARD_WIDTH - 2, BOARD_HEIGHT - 2);
      if(!demo && global.throw == 1)
	global.throw = 2;
      assert(rts != (PROTOANIMATE((*)))NULL);
      then = (*rts)((PROTOVOID(*))NULL);
    }
    /*}}}*/
  else
    /*{{{  animate*/
    {
      unsigned  x, y;
      
      x = global.count * ZOOM_X;
      y = global.count * ZOOM_Y;
      present_area(CENTER_X - ZOOM_X - (int)x, CENTER_Y - ZOOM_Y - (int)y,
	  ZOOM_X, 2 * y + 2 * ZOOM_Y);
      present_area(CENTER_X + (int)x, CENTER_Y - ZOOM_Y - (int)y,
	  ZOOM_X, 2 * y + 2 * ZOOM_Y);
      present_area(CENTER_X - (int)x, CENTER_Y - ZOOM_Y - (int)y,
	  2 * x, ZOOM_Y);
      present_area(CENTER_X - (int)x, CENTER_Y + (int)y,
	  2 * x, ZOOM_Y);
      global.count++;
    }
    /*}}}*/
  return then;
}
/*}}}*/
/*{{{  void blank_board(map, cycle)*/
extern VOIDFUNC blank_board
FUNCARG((map, cycle),
	BOARD CONST *map
ARGSEP  unsigned cycle
)
/*
 * sets up a blank board
 */
{
  memset(garden, 0, sizeof(garden));
  /*{{{  draw blank background*/
  {
    unsigned long foreground, background;
      Pixmap stipple;
    unsigned  back;
  
    video_fill(display.back, GCN(GC_CLEAR),
	0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    back = (map->colors + cycle) % BACKGROUNDS;
    if(data.mono == False)
      color_set((int)back);
    /*{{{  draw blank board*/
    {
      background = data.mono != False ? display.white :
	  colors[display.dynamic ? COLOR_DYNAMIC :
	  backgrounds[back][0]].pixel;
      stipple = fills[map->fill].mask;
      foreground = data.mono != False ?
	  display.black : colors[display.dynamic ?
	  COLOR_DYNAMIC + 1 : backgrounds[back][1]].pixel;
      video_pattern(foreground, background, stipple);
      video_fill(display.back, GCN(GC_BOARD),
	  BORDER_LEFT, BORDER_TOP, BOARD_WIDTH, BOARD_HEIGHT);
    }
    /*}}}*/
    video_line(display.back, GCN(GC_BORDER),
	BORDER_LEFT, BORDER_TOP, BORDER_LEFT + BOARD_WIDTH, BORDER_TOP);
    video_line(display.back, GCN(GC_BORDER),
	BORDER_LEFT, BORDER_TOP + BOARD_HEIGHT - 1,
	BORDER_LEFT + BOARD_WIDTH, BORDER_TOP + BOARD_HEIGHT - 1);
    video_line(display.back, GCN(GC_BORDER),
	PIXELX(4, -1), PIXELY(-1, 0), PIXELX(4, -1), PIXELY(-1, CELL_HEIGHT));
    video_line(display.back, GCN(GC_BORDER),
	PIXELX(4, XTRA_SPACING * 4 + CELL_WIDTH), PIXELY(-1, 0),
	PIXELX(4, XTRA_SPACING * 4 + CELL_WIDTH), PIXELY(-1, CELL_HEIGHT));
  }
  /*}}}*/
  /*{{{  add the xtra*/
  {
    unsigned  i;
	
    extra.escape = 0;
    for(i = 5; i--;)
      draw_extra_letter(i);
  }
  /*}}}*/
  /*{{{  initialize stuff*/
  {
    update.back.backs = 0;
    update.score.scores = 0;
    player.old_ball.state = 0;
    player.old_ball.count = 8;
    player.ball.state = 0;
    player.ball.count = 8;
    monster.monsters = 0;
    apple.apples = 0;
  }
  /*}}}*/
  return;
}
/*}}}*/
/*{{{  void bounding_box(x, y, width, height)*/
extern VOIDFUNC bounding_box
FUNCARG((x, y, width, height),
	int       x
ARGSEP  int       y
ARGSEP  unsigned  width
ARGSEP  unsigned  height
)
/*
 * updates the update box so that it includes the
 * supplied rectangle
 * remember to empty the update box to the background list
 */
{
  if(!update.set)
    {
      update.br.x = (update.tl.x = x) + width;
      update.br.y = (update.tl.y = y) + height;
      update.set = 1;
    }
  else
    {
      if(update.tl.x > x)
	update.tl.x = x;
      if(update.tl.y > y)
	update.tl.y = y;
      if(update.br.x < (int)(x + width))
	update.br.x = (int)(x + width);
      if(update.br.y < (int)(y + height))
	update.br.y = (int)(y + height);
    }
  return;
}
/*}}}*/
/*{{{  unsigned check_board(map)*/
static unsigned check_board
FUNCARG((map),
	BOARD     *map
)
/* checks a board is ok
 * and fix if possible
 * returns a mask of the errors
 */
{
  unsigned  error;
  
  error = 0;
  /*{{{  check characters*/
  {
    char      *cptr;
    unsigned  ix;
    
    for(ix = CELLS_DOWN; ix--;)
      for(cptr = map->map[ix]; *cptr; cptr++)
	if(*cptr != GARDEN_RANDOM && *cptr != GARDEN_NOAPPLE &&
	    *cptr != GARDEN_CHERRY && !ISPATH(*cptr) && !ISAPPLE(*cptr))
	  {
	    error |= ERROR_CHAR;
	    *cptr = GARDEN_RANDOM;
	  }
  }
  /*}}}*/
  /*{{{  check fill*/
  if(map->fill >= FILLS)
    {
      error |= ERROR_FILL;
      map->fill %= FILLS;
    }
  /*}}}*/
  /*{{{  check background*/
  if(map->colors >= BACKGROUNDS)
    {
      error |= ERROR_BACKGROUND;
      map->colors %= BACKGROUNDS;
    }
  /*}}}*/
  /*{{{  check apples number*/
  if(map->apples > APPLE_LIMIT)
    {
      error |= ERROR_APPLES;
      map->apples = APPLE_LIMIT;
    }
  /*}}}*/
  /*{{{  check apple positioning*/
  {
    char      *cptr;
    
    for(cptr = map->map[CELLS_DOWN - 1]; *cptr; cptr++)
      if(ISAPPLE(*cptr) || *cptr == GARDEN_RANDOM)
	{
	  *cptr = GARDEN_NOAPPLE;
	  error |= ERROR_APPLE_POSITION;
	}
  }
  /*}}}*/
  /*{{{  check required paths*/
  {
    static CONST unsigned path[][2] =
      {
	{4, 0x2}, {5, 0x2}, {6, 0x2}, {7, 0x0},
      };
    unsigned  ix;
    
    for(ix = XtNumber(path); ix--;)
      {
	char      *cptr;
	unsigned  c;
	
	cptr = (char *)(map->map) + path[ix][0];
	c = *cptr;
	if(!ISPATH(c))
	  {
	    error |= ERROR_PATH;
	    *cptr = GARDEN_PATH + path[ix][1];
	  }
	else if((GARDENPATH(c) & path[ix][1]) != path[ix][1])
	  {
	    error |= ERROR_PATH;
	    *cptr = (GARDENPATH(c) | path[ix][1]) + GARDEN_PATH;
	  }
      }
  }
  /*}}}*/
  /*{{{  check integrity*/
  {
    unsigned  row;
    char      *cptr;
    
    for(row = CELLS_DOWN; row--;)
      for(cptr = map->map[row]; *cptr; cptr++)
	/*{{{  check a cell*/
	{
	  char      c;
	  
	  c = *cptr;
	  if(ISPATH(c))
	    {
	      unsigned  paths;
	      
	      paths = 0;
	      if(ISPATH(cptr[1]))
		paths |= 2;
	      if(row != CELLS_DOWN - 1 &&
		  ISPATH(cptr[CELLS_ACROSS + 1]))
		paths |= 1;
	      if((GARDENPATH(c) & paths) != (GARDENPATH(c) & 3))
		{
		  error |= ERROR_INCONSISTANT;
		  *cptr = ((GARDENPATH(c) & 0xC) | paths) + GARDEN_PATH;
		}
	    }
	}
	/*}}}*/
  }
  /*}}}*/
  /*{{{  check totals*/
  {
    char      *cptr;
    unsigned  count;
    unsigned  totals[8];
    
    for(count = 8; count--;)
      totals[count] = 0;
    for(count = (CELLS_ACROSS + 1) * CELLS_DOWN,
	cptr = (char *)map->map; count--; cptr++)
      if(ISPATHCHERRY(*cptr) || *cptr == GARDEN_CHERRY)
	totals[4]++;
      else if(ISPATHDEN(*cptr))
	totals[5]++;
      else if(ISPATHPLAYER(*cptr))
	totals[6]++;
      else if(ISAPPLE(*cptr))
	{
	  unsigned  value;
	  unsigned  ix;
	  
	  value = GARDENAPPLE(*cptr);
	  for(ix = 4; ix--; value >>= 1)
	    if(value & 1)
	      totals[ix]++;
	  totals[7]++;
	}
      else if(*cptr == GARDEN_RANDOM)
	totals[7]++;
    do
      {
	char    c;
	
	cptr = NULL;
	c = 0;
	if(!totals[5])
	  {
	    error |= ERROR_NO_DEN;
	    cptr = &map->map[DEFAULT_DEN_Y][DEFAULT_DEN_X];
	    c = GARDEN_PATH_DEN;
	  }
	if(!totals[6])
	  {
	    error |= ERROR_NO_PLAYER;
	    cptr = &map->map[DEFAULT_PLAYER_Y][DEFAULT_PLAYER_X];
	    c = GARDEN_PATH_PLAYER;
	  }
	if(!totals[4])
	  {
	    error |= ERROR_NO_CHERRIES;
	    cptr = &map->map[DEFAULT_CHERRY_Y][DEFAULT_CHERRY_X];
	    c = GARDEN_PATH_CHERRY;
	  }
	if(cptr)
	  {
	    if(*cptr == GARDEN_CHERRY || ISPATHCHERRY(*cptr))
	      totals[4]--;
	    if(ISPATH(*cptr))
	      c = (GARDENPATH(*cptr) & 3) + c + GARDEN_PATH;
	    else if(c == GARDEN_PATH_CHERRY)
	      c = GARDEN_CHERRY;
	    else
	      c += GARDEN_PATH;
	    *cptr = c;
	  }
	if(totals[7] < map->apples)
	  error |= ERROR_TOO_FEW_APPLES;
      }
    while(cptr);
    for(count = 4; count--;)
      if(totals[count] != map->apples)
	error |= ERROR_BAD_APPLES;
  }
  /*}}}*/
  return error;
}
/*}}}*/
/*{{{  void create_boards()*/
extern VOIDFUNC create_boards FUNCARGVOID
/* initializes the boards
 * either from the internal ones, or
 * by reading the user specified file
 */
{
  read_boards(data.boards, NULL);
  if(!board.boards)
    {
      fputs("No gardens found, using default\n", stderr);
      parse.depth = 0;
      read_boards(NULL, NULL);
    }
  if(parse.text)
    free(parse.text);
  return;
}
/*}}}*/
/*{{{  void create_xtra_monster(ix)*/
extern VOIDFUNC create_xtra_monster
FUNCARG((ix),
	unsigned  ix
)
/*
 * create an xtra monster sprite, by taking the virgin one, and
 * slapping a letter on its chest
 */
{
  SPRITE    *dptr;
  SPRITE    *sptr;
  SPRITE    *lptr;
  unsigned  i;
  
  sptr = &sprites[SPRITE_XTRA];
  lptr = &sprites[SPRITE_EXTRA];
  for(dptr = &sprites[SPRITE_XTRA], sptr = &sprites[SPRITE_XTRA_SOURCE],
      i = MONSTER_IMAGES; i--; dptr++, sptr++)
    {
      video_copy(sptr->image, dptr->image, GCN(GC_COPY),
	  0, 0, CELL_WIDTH, CELL_HEIGHT, 0, 0);
      video_copy(lptr->mask, dptr->image, GCN(GC_MASK),
	  (int)ix * (CELL_WIDTH / 2), 0, CELL_WIDTH / 2, CELL_HEIGHT / 2,
	  XTRA_LETTER_X, XTRA_LETTER_Y);
      video_copy(lptr[!(extra.got & 1 << ix)].image,
	  dptr->image, GCN(GC_OR),
	  (int)ix * (CELL_WIDTH / 2), 0, CELL_WIDTH / 2, CELL_HEIGHT / 2,
	  XTRA_LETTER_X, XTRA_LETTER_Y);
    }
  return;
}
/*}}}*/
/*{{{  void draw_ball(flag)*/
static VOIDFUNC draw_ball
FUNCARG((flag),
	unsigned  flag
)
/*
 * draws the old ball on or off the window
 */
{
  switch(player.old_ball.state)
  {
    /*{{{  case 0:*/
    case 0:
    {
      if(player.old_ball.count < 8)
	{
	  COORD CONST *hold;
	   
	  hold = &ball_hold[player.old_ball.count * MONSTER_IMAGES +
	      player.old_ball.image];
	  plot_ball(flag, player.old_ball.pixel.x + hold->x,
	      player.old_ball.pixel.y + hold->y);
	}
      break;
    }
    /*}}}*/
    /*{{{  case 1:*/
    case 1:
    {
      plot_ball(flag, player.old_ball.pixel.x, player.old_ball.pixel.y);
      break;
    }
    /*}}}*/
    /*{{{  case 2: case 4:*/
    case 2: case 4:
    {
      COORD     offset;
      COORD     pixel;
      unsigned  bits;
	
      pixel.x = player.old_ball.pixel.x;
      pixel.y = player.old_ball.pixel.y;
      if(player.old_ball.count)
	{
	  offset.x = player.old_ball.count * BALL_EX;
	  offset.y = player.old_ball.count * BALL_EY;
	  bits = 0xFF;
	  /*{{{  set clips*/
	  {
	    if(pixel.x < offset.x)
	      bits &= 0xF8;
	    if(pixel.y < offset.y)
	      bits &= 0x3E;
	    if(pixel.x + offset.x > BOARD_WIDTH + BALL_WIDTH)
	      bits &= 0x8F;
	    if(pixel.y + offset.y > BOARD_HEIGHT + BALL_HEIGHT)
	      bits &= 0xE3;
	  }
	  /*}}}*/
	  /*{{{  do inner bits*/
	  {
	    if(bits & 0x01)
	      plot_ball(flag, pixel.x - offset.x, pixel.y - offset.y);
	    if(bits & 0x02)
	      plot_ball(flag, pixel.x - offset.x, pixel.y);
	    if(bits & 0x04)
	      plot_ball(flag, pixel.x - offset.x, pixel.y + offset.y);
	    if(bits & 0x08)
	      plot_ball(flag, pixel.x, pixel.y + offset.y);
	    if(bits & 0x10)
	      plot_ball(flag, pixel.x + offset.x, pixel.y + offset.y);
	    if(bits & 0x20)
	      plot_ball(flag, pixel.x + offset.x, pixel.y);
	    if(bits & 0x40)
	      plot_ball(flag, pixel.x + offset.x, pixel.y - offset.y);
	    if(bits & 0x80)
	      plot_ball(flag, pixel.x, pixel.y - offset.y);
	  }
	  /*}}}*/
	  offset.x *= 2;
	  offset.y *= 2;
	  /*{{{  set clips*/
	  {
	    if(pixel.x < offset.x)
	      bits &= 0xF8;
	    if(pixel.y < offset.y)
	      bits &= 0x3E;
	    if(pixel.x + offset.x > BOARD_WIDTH + BALL_WIDTH)
	      bits &= 0x8F;
	    if(pixel.y + offset.y > BOARD_HEIGHT + BALL_HEIGHT)
	      bits &= 0xE3;
	  }
	  /*}}}*/
	  /*{{{  do inner bits*/
	  {
	    if(bits & 0x01)
	      plot_ball(flag, pixel.x - offset.x, pixel.y - offset.y);
	    if(bits & 0x02)
	      plot_ball(flag, pixel.x - offset.x, pixel.y);
	    if(bits & 0x04)
	      plot_ball(flag, pixel.x - offset.x, pixel.y + offset.y);
	    if(bits & 0x08)
	      plot_ball(flag, pixel.x, pixel.y + offset.y);
	    if(bits & 0x10)
	      plot_ball(flag, pixel.x + offset.x, pixel.y + offset.y);
	    if(bits & 0x20)
	      plot_ball(flag, pixel.x + offset.x, pixel.y);
	    if(bits & 0x40)
	      plot_ball(flag, pixel.x + offset.x, pixel.y - offset.y);
	    if(bits & 0x80)
	      plot_ball(flag, pixel.x, pixel.y - offset.y);
	  }
	  /*}}}*/
	}
      else
	plot_ball(flag, pixel.x, pixel.y);
      break;
    }
    /*}}}*/
    /*{{{  case 3:*/
    case 3:
      break;
    /*}}}*/
  }
  return;
}
/*}}}*/
/*{{{  void draw_extra()*/
extern VOIDFUNC draw_extra FUNCARGVOID
/*
 * whack the current extra monster onto the background
 */
{
  int       x;
  
  x = PIXELX(4, extra.select * XTRA_SPACING);
  back_sprite(SPRITE_XTRA + (chaotic() & 1), 1, x, PIXELY(-1, 0));
  add_background(x, PIXELY(-1, 0), CELL_WIDTH, CELL_HEIGHT);
  return;
}
/*}}}*/
/*{{{  void draw_extra_letter(ix)*/
extern VOIDFUNC draw_extra_letter
FUNCARG((ix),
	unsigned  ix
)
/*
 * draw one of the extra letters on the background
 * and add it to the update list
 */
{
  SPRITE    *lptr;
  int       x;
  
  lptr = &sprites[SPRITE_EXTRA];
  x = PIXELX(4, ix * XTRA_SPACING);
  video_fill(display.back, GCN(GC_CLEAR),
      x, PIXELY(-1, 0), CELL_WIDTH, CELL_HEIGHT);
  if(display.background != COLOUR_ZERO)
    video_copy(lptr->mask,
	display.back, GCN(GC_MASK), (int)ix * (CELL_WIDTH / 2), 0,
	CELL_WIDTH / 2, CELL_HEIGHT / 2,
	x + XTRA_LETTER_X, PIXELY(-1, XTRA_LETTER_Y));
  video_copy(lptr[!(extra.got & 1 << ix)].image,
      display.back, GCN(GC_OR), (int)ix * (CELL_WIDTH / 2), 0,
      CELL_WIDTH / 2, CELL_HEIGHT / 2,
      x + XTRA_LETTER_X, PIXELY(-1, XTRA_LETTER_Y));
  add_background(x, PIXELY(-1, 0), CELL_WIDTH, CELL_HEIGHT);
  return;
}
/*}}}*/
/*{{{  void new_board()*/
extern VOIDFUNC new_board FUNCARGVOID
/*
 * sets up a new board
 */
{
  BOARD CONST *map;
  int       spaces;

  map = board.list[(global.screen - 1) % board.boards];
  blank_board(map, (global.screen - 1) / board.boards);
  apple.apples = 0;
  spaces = 0;
  global.cherries = 0;
  global.dens = 0;
  /*{{{  copy the board*/
  {
    unsigned  mask;
	
    mask = 1 << (chaotic() % 4);
    /*{{{  copy the text map*/
    {
      CELL      *cptr;
      char CONST *mptr;
      unsigned  y, x;
      
      mptr = (char CONST *)map->map;
      for(y = 0, cptr = BOARDCELL(0, 0);
	  y != CELLS_DOWN; y++, cptr += CELL_STRIDE - CELLS_ACROSS, mptr++)
	for(x = 0; x != CELLS_ACROSS; x++, mptr++, cptr++)
	  {
	    char    c;
	  
	    c = *mptr;
	    /*{{{  cherry?*/
	    if(c == GARDEN_CHERRY || ISPATHCHERRY(c))
	      {
		cptr->sprite = SPRITE_CHERRY;
		global.cherries++;
	      }
	    /*}}}*/
	    if(ISAPPLE(c))
	      /*{{{  explicit apple*/
	      {
		cptr->xtra = data.random != False || !(mask & GARDENAPPLE(c));
		if(!cptr->xtra && apple.apples < APPLE_LIMIT)
		  spawn_apple(x, y, 0, 0);
	      }
	      /*}}}*/
	    else if(ISPATH(c))
	      /*{{{  explicitly connected path*/
	      {
		unsigned  bits;
		
		bits = GARDENPATH(c) & 3;
		cptr->visit = 1;
		if(bits & 1)
		  {
		    cptr[0].depths[1] = CELL_HEIGHT + GAP_HEIGHT;
		    cptr[CELL_STRIDE].depths[0] = -(CELL_HEIGHT + GAP_HEIGHT);
		  }
		if(bits & 2)
		  {
		    cptr[0].depths[3] = CELL_WIDTH + GAP_WIDTH;
		    cptr[1].depths[2] = -(CELL_WIDTH + GAP_WIDTH);
		  }
		if(ISPATHDEN(c))
		  {
		    cptr->den = 1;
		    global.dens++;
		  }
		else if(ISPATHPLAYER(c))
		  {
		    global.start.x = x;
		    global.start.y = y;
		  }
	      }
	      /*}}}*/
	    else if(c == GARDEN_RANDOM)
	      cptr->xtra = 1;
	    spaces += cptr->xtra;
	  }
    }
    /*}}}*/
  }
  /*}}}*/
  if(apple.apples < map->apples)
    /*{{{  add the apples*/
    {
      unsigned  count;
      
      for(count = map->apples - apple.apples; spaces && count--;)
	{
	  CELL      *cptr;
	  
	  /*{{{  select a location*/
	  {
	    unsigned  count;
	    
	    do
	      count = chaotic();
	    while(count >= 256 / spaces * spaces);
	    count %= spaces;
	    for(cptr = BOARDCELL(0, 0); !cptr->xtra || !count--; cptr++)
	      /* EMPTY */;
	  }
	  /*}}}*/
	  cptr->apple = 1;
	  cptr->xtra = 0;
	  spaces--;
	  /*{{{  clear surround*/
	  {
	    static CONST int offsets[] = {-1, 1, CELL_STRIDE, -CELL_STRIDE};
	    unsigned ix;
	    
	    for(ix = sizeof(offsets) / sizeof(offsets[0]); ix--;)
	      {
		CELL    *optr;
		
		optr = cptr + offsets[ix];
		spaces -= (int)optr->xtra;
		optr->ball |= optr->xtra;
		optr->xtra = 0;
	      }
	  }
	  /*}}}*/
	  /*{{{  run out of spaces?*/
	  if(spaces <= 0 && count)
	    {
	      unsigned  count;
	      
	      for(count = CELLS_DOWN * CELL_STRIDE, cptr = BOARDCELL(0, 0);
		  count--; cptr++)
		if(cptr->ball)
		  {
		    cptr->ball = 0;
		    cptr->xtra = 1;
		    spaces++;
		  }
	    }
	  /*}}}*/
	  spawn_apple((cptr - BOARDCELL(0, 0)) % CELL_STRIDE,
	      (cptr - BOARDCELL(0, 0)) / CELL_STRIDE, 0, 0);
	}
    }
    /*}}}*/
  video_fill(display.back, GCN(GC_CLEAR),
      BORDER_LEFT + (CELL_WIDTH + GAP_WIDTH) * 4 + GAP_WIDTH, BORDER_TOP + 1,
      XTRA_SPACING * 4 + CELL_WIDTH, GAP_HEIGHT * 2 - 1);
  draw_extra();
  /*{{{  add the screen number*/
  {
    static char text[] = "Garden 000";
    int       length;
    
    length = 7 + itoa(text + 7, (unsigned long)global.screen, 0);
    video_opaque_text(display.back, GCN(GC_TEXT),
	PIXELX(8, (int)font.width * 2),
	PIXELY(-1, CELL_HEIGHT / 2 + font.center), text, length);
  }
  /*}}}*/
  add_score(0, 0, 0);
  /*{{{  add lives*/
  if(global.lives)
    {
      unsigned  lives;
	
      for(lives = global.lives - 1; lives--;)
	back_sprite(SPRITE_PLAYER + 6, 0,
	    PIXELX((int)lives, 0), PIXELY(CELLS_DOWN, 0));
    }
  /*}}}*/
  /*{{{  cut the background*/
  {
    unsigned  y, x;
    COORD     cell;
    CELL      *cptr;
	
    cptr = BOARDCELL(0, 0);
    cell.x = PIXELX(0, 0);
    cell.y = PIXELY(0, 0);
    for(y = CELLS_DOWN; y--; cptr += CELL_STRIDE - CELLS_ACROSS,
	cell.x -= (CELL_WIDTH + GAP_WIDTH) * CELLS_ACROSS,
	cell.y += CELL_HEIGHT + GAP_HEIGHT)
      for(x = CELLS_ACROSS; x--; cptr++, cell.x += CELL_WIDTH + GAP_WIDTH)
	{
	  if(cptr->visit)
	    munch_hole(cptr, cell.x, cell.y);
	  if(cptr->sprite)
	    back_sprite(cptr->sprite, 1, cell.x, cell.y);
	  if(cptr->den)
	    video_fill(display.back,
		GCN(GC_CLEAR), cell.x - DEN_WIDTH,
		cell.y - DEN_HEIGHT,
		CELL_WIDTH + DEN_WIDTH * 2, CELL_HEIGHT + DEN_HEIGHT * 2);
	}
  }
  /*}}}*/
  video_copy(display.back, display.copy, GCN(GC_COPY),
      0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0);
  return;
}
/*}}}*/
/*{{{  void plot_ball(flag, x, y)*/
static VOIDFUNC plot_ball
FUNCARG((flag, x, y),
	unsigned  flag
ARGSEP  int       x
ARGSEP  int       y
)
/*
 * plots the ball centered on the given coordinate
 * flag is 0 for unplot, 1 for plot
 */
{
  if(flag)
    {
      video_copy(sprites[SPRITE_BALL].mask, display.copy,
	  GCN(GC_MASK), 0, 0, BALL_WIDTH, BALL_HEIGHT,
	  x - BALL_WIDTH / 2, y - BALL_HEIGHT / 2);
      video_copy(sprites[SPRITE_BALL].image, display.copy,
	  GCN(GC_OR), 0, 0, BALL_WIDTH, BALL_HEIGHT,
	  x - BALL_WIDTH / 2, y - BALL_HEIGHT / 2);
    }
  add_background(x - BALL_WIDTH / 2, y - BALL_HEIGHT / 2,
      BALL_WIDTH, BALL_HEIGHT);
  return;
}
/*}}}*/
/*{{{  void read_boards()*/
static VOIDFUNC read_boards
FUNCARG((name, parent),
	char CONST *name
ARGSEP  char CONST *parent
)
/* read a board file
 * recurses to deal with include
 */
{
  INCLUDE   *sptr;
  FILE      *stream;
  unsigned  error;
  BOARD   *bptr;

  sptr = &parse.stack[parse.depth];
  sptr->name = NULL;
  error = 0;
  sptr->line = 0;
  sptr->start = 0;
  sptr->garden = 0;
  stream = NULL;
  /*{{{  open?*/
  if(name)
    {
      char      *filename;
      size_t    length;
      
      length = strlen(name);
      {
        char CONST *slash = *name != '/' && parent ? strrchr(parent, '/') : NULL;
        size_t offset = slash ? (size_t)(slash - parent + 1) : 0;

        filename = malloc(offset + length + FILE_SUFFIX_LEN + 1);
        if(!filename)
          error = ERROR_MALLOC;
        else
          {
            if(offset)
              memcpy(filename, parent, offset);
            memcpy(filename + offset, name, length);
            strcpy(filename + offset + length, FILE_SUFFIX);
            stream = fopen(filename, "r");
            if(!stream)
              {
                filename[offset + length] = 0;
                stream = fopen(filename, "r");
              }
            if(!stream && *name != '/')
              {
                char *resource = garden_resource(name, NULL);
                if(!resource)
                  resource = garden_resource(name, FILE_SUFFIX);
                if(resource)
                  {
                    free(filename);
                    filename = resource;
                    stream = fopen(filename, "r");
                  }
              }
          }
      }
      sptr->name = filename;
      if(filename && !stream)
	{
	  perror(filename);
	  error = ERROR_OPEN;
	}
    }
  else
    stream = 0;
  /*}}}*/
  do
    /*{{{  read a board*/
    {
      bptr = NULL;
      if(error)
	/* EMPTY */;
      else if(sptr->name)
	/*{{{  read board*/
	{
	  /* state 0    new block
	   * state 1    comment
	   * state 2-4  numbers
	   * state 5    open brace
	   * state 6    close brace
	   * state 7    close brace
	   * state 10+  lines
	   */
	  unsigned  state;
	  
	  state = 0;
	  sptr->start = sptr->line + 1;
	  do
	    {
	      char      *ptr;
	  
	      /*{{{  read line*/
	      {
		unsigned  temp;
	      
		temp = read_line(stream, sptr);
		if(!error)
		  sptr->error = sptr->line;
		if(temp)
		  {
		    error |= temp;
		    break;
		  }
	      }
	      /*}}}*/
	      ptr = parse.text;
	      if(!*ptr)
		{
		  if(state)
		    error |= ERROR_EOF;
		  break;
		}
	      while(*ptr == ' ' || *ptr == '\t')
		ptr++;
	      if(*ptr == '\n')
		continue;
	      /*{{{  new block*/
	      if(state == 0)
		{
		  if(ptr[0] == '/' && ptr[1] == '*')
		    {
		      ptr += 2;
		      state = 1;
		    }
		  else if(!strncmp(ptr, "#include", 8))
		    /*{{{  include*/
		    {
		      ptr += 8;
		      while(*ptr == ' ' || *ptr == '\t')
			ptr++;
		      if(*ptr == '\"')
			{
			  char      *name;
		      
			  name = ++ptr;
			  while(*ptr && *ptr != '\"')
			    ptr++;
			  *ptr = 0;
			  if(ptr == name)
			    name = NULL;
			  if(parse.depth == INCLUDE_LIMIT)
			    {
			      error = ERROR_TOO_MANY;
			      break;
			    }
			  sptr->seek = ftell(stream);
			  fclose(stream);
			  parse.depth++;
			  read_boards(name, sptr->name);
			  parse.depth--;
			  stream = fopen(sptr->name, "r");
			  if(!stream || fseek(stream, sptr->seek, SEEK_SET))
			    {
			      error = ERROR_OPEN;
			      perror(sptr->name);
			      break;
			    }
			}
		      else
			{
			  error = ERROR_SYNTAX;
			  break;
			}
		    }
		    /*}}}*/
		  else if(*ptr == '{')
		    {
		      ptr++;
		      state = 2;
		      bptr = malloc(sizeof(BOARD));
		      if(!bptr)
			{
			  error = ERROR_MALLOC;
			  state = 7;
			}
		    }
		  else
		    error = ERROR_SYNTAX;
		}
	      /*}}}*/
	      /*{{{  in comment*/
	      if(state == 1)
		{
		  while(*ptr && state)
		    {
		      while(*ptr && *ptr != '*')
			ptr++;
		      if(*ptr)
			{
			  if(ptr[1] == '/')
			    {
			      ptr += 2;
			      state = 0;
			      break;
			    }
			  else
			    ptr++;
			}
		    }
		}
	      /*}}}*/
	      /*{{{  reading board*/
	      if(state >= 2)
		{
		  while(state)
		    {
		      if(*ptr == ' ' || *ptr == '\t')
			ptr++;
		      if(*ptr == '\n')
			break;
		      switch(state)
		      {
			/*{{{  case 2: case 3: case 4:*/
			case 2: case 3: case 4:
			{
			  char      *eptr;
			  unsigned  value;
			  
			  value = strtol(ptr, &eptr, 0);
			  if(eptr == ptr)
			    {
			      error = ERROR_SYNTAX;
			      state = 7;
			    }
			  else
			    {
			      switch(state)
			      {
				case 2:
				  bptr->fill = value;
				  /* CASCADE */
				case 3:
				  bptr->colors = value;
				  /* CASCADE */
				case 4:
				  bptr->apples = value;
				  /* CASCADE */
			      }
			      ptr = eptr;
			      state++;
			    }
			  break;
			}
			/*}}}*/
			/*{{{  case 5:*/
			case 5:
			  if(*ptr == '{')
			    {
			      state = 10;
			      ptr++;
			    }
			  else
			    {
			      state = 7;
			      error = ERROR_SYNTAX;
			    }
			  break;
			/*}}}*/
			/*{{{  case 6:*/
			case 6:
			  if(*ptr++ == '}')
			    state = 7;
			  else
			    error = ERROR_SYNTAX;
			  break;
			/*}}}*/
			/*{{{  case 7:*/
			case 7:
			  if(*ptr == '}')
			    {
			      state = 0;
			      ptr++;
			    }
			  else if(*ptr++ == '{')
			    state = 6;
			  else
			    error = ERROR_SYNTAX;
			  break;
			/*}}}*/
			/*{{{  default:*/
			default:
			{
			  char      *eptr;
			  
			  assert(state >= 10 && state < 10 + CELLS_DOWN);
			  eptr = strchr(ptr + 1, '\"');
			  if(*ptr != '\"' || !eptr ||
			      eptr - ptr != CELLS_ACROSS + 1)
			    {
			      error = ERROR_SYNTAX;
			      state = 6;
			    }
			  else
			    {
			      strncpy(bptr->map[state - 10], ptr + 1,
				  CELLS_ACROSS);
			      bptr->map[state - 10][CELLS_ACROSS] = 0;
			      state++;
			      ptr = eptr + 1;
			      if(state == 10 + CELLS_DOWN)
				state = 6;
			    }
			  break;
			}
			/*}}}*/
		      }
		      if(error && bptr)
			{
			  free(bptr);
			  bptr = NULL;
			}
		      if(*ptr == ',')
			ptr++;
		    }
		}
	      /*}}}*/
	    }
	  while(state);
	  if(!error && !*parse.text)
	    break;
	}
	/*}}}*/
      else if(sptr->garden != sizeof(internal_boards) /
	  sizeof(internal_boards[0]))
	bptr = &internal_boards[sptr->garden];
      else
	break;
      /*{{{  a new garden?*/
      if(bptr)
	{
	  if(!(error & ERROR_FATAL_MASK))
	    error |= check_board(bptr);
	  if(board.boards == BOARD_LIMIT)
	    error |= ERROR_TOO_MANY;
	  if(!(error & ERROR_FATAL_MASK))
	    board.list[board.boards++] = bptr;
	  else
	    {
	      if(sptr->name)
		free(bptr);
	      bptr = NULL;
	    }
	}
      /*}}}*/
      /*{{{  error*/
      if(error)
	{
	  unsigned  depth;
	  int       ix;
	  
	  fprintf(stderr, "In file %s", sptr->name ? sptr->name :
	      "<default>");
	  for(depth = parse.depth; depth--;)
	    fprintf(stderr, " included from %s:%d",
		parse.stack[depth].name, parse.stack[depth].line);
	  fprintf(stderr, " line %d", sptr->error);
	  if(sptr->start != sptr->line)
	    fprintf(stderr, " garden %d (lines %d-%d)",
		sptr->garden + 1, sptr->start, sptr->line);
	  fputc('\n', stderr);
	  for(ix = 16; ix--;)
	    if(error & 1 << ix)
	      fprintf(stderr, "%s%s\n", text_error[ix],
		  (1 << ix) & ERROR_FIX_MASK ? " (fixed)" : ""); 
	  error &= ERROR_KEEP_MASK;
	}
      /*}}}*/
      if(bptr)
	sptr->garden++;
    }
    /*}}}*/
  while(!error);
  if(stream)
    fclose(stream);
  free((char *)sptr->name);
  return;
}
/*}}}*/
/*{{{  unsigned read_line(stream)*/
static unsigned read_line
FUNCARG((stream, sptr),
	FILE    *stream
ARGSEP  INCLUDE *sptr
)
{
  size_t    length;
  char      *ptr;
  unsigned  error;
  char      *line;
  
  sptr->line++;
  error = 0;
  if(!parse.text)
    {
      parse.limit = 128;
      parse.text = malloc(128);
      if(!parse.text)
	return ERROR_MALLOC;
    }
  length = 0;
  ptr = parse.text;
  /*{{{  read line*/
  while((line = fgets(ptr, parse.limit - length, stream)))
    {
      ptr += strlen(ptr);
      length = ptr - parse.text;
      if(length && *(ptr - 1) == '\n')
	break;
      if(length + 1 < parse.limit)
	break;
      ptr = realloc(parse.text, parse.limit + 128);
      if(ptr)
	{
	  parse.limit += 128;
	  parse.text = ptr;
	  ptr += length;
	}
      else
	{
	  error = ERROR_MALLOC;
	  break;
	}
    }
  /*}}}*/
  /*{{{  error?*/
  if(!line)
    {
      if(feof(stream))
	*parse.text = 0;
      else
	{
	  perror(sptr->name);
	  error = ERROR_READING;
	}
    }
  /*}}}*/
  return error;  
}
/*}}}*/
/*{{{  void refresh_window()*/
extern VOIDFUNC refresh_window FUNCARGVOID
/*
 * refreshes the display window
 */
{
  if(global.state == MODE_ZOOM)
    {
      unsigned  x, y;
      
      x = global.count * ZOOM_X;
      y = global.count * ZOOM_Y;
      present_area(0, 0,
	  WINDOW_WIDTH, BORDER_TOP + 1);
      present_area(0, BORDER_TOP + BOARD_HEIGHT - 1,
	  WINDOW_WIDTH, BORDER_BOTTOM + 1);
      if(BORDER_LEFT > 0)
	present_area(0, BORDER_TOP,
	    BORDER_LEFT + 1, BOARD_HEIGHT);
      if(BORDER_RIGHT > 0)
	present_area(BORDER_LEFT + BOARD_WIDTH - 1, BORDER_TOP,
	    BORDER_RIGHT + 1, BOARD_HEIGHT);
      present_area(CENTER_X - (int)x, CENTER_Y - (int)y,
	  2 * x, 2 * y);
    }
  else
    present_area(0, 0,
	WINDOW_WIDTH, WINDOW_HEIGHT);
  if(global.missed)
    present_load(WINDOW_WIDTH - (int)global.missed, WINDOW_WIDTH);
  if(global.dilation != FRAME_SCALE)
    present_load(0, WINDOW_WIDTH - (int)((unsigned long)WINDOW_WIDTH * FRAME_SCALE /
	    global.dilation));
  return;
}
/*}}}*/
/*{{{  void set_back_sprite(ix)*/
extern VOIDFUNC set_back_sprite
FUNCARG((ix, cell_x, cell_y),
	unsigned  ix
ARGSEP  unsigned  cell_x
ARGSEP  unsigned  cell_y
)
/*
 * sets a den/prize sprite and draws it on the background
 */
{
  int       x;
  int       y;
  
  BOARDCELL(cell_x, cell_y)->sprite = ix;
  x = PIXELX(cell_x, 0);
  y = PIXELY(cell_y, 0);
  add_background(x, y, CELL_WIDTH, CELL_HEIGHT);
  video_fill(display.back, GCN(GC_CLEAR),
      x, y, CELL_WIDTH, CELL_HEIGHT);
  back_sprite(ix, 0, x, y);
  return;
}
/*}}}*/
/*{{{  void show_updates()*/
extern VOIDFUNC show_updates FUNCARGVOID
/*
 * shows all the updates on the update list
 * and delete the dead apples and monsters.
 * remember to keep the monster lists intact.
 * the monsters a drawn in reverse order, so that the player
 * ends up on top of everybody.
 * We must be careful about the order of the update, so that there
 * isn't excessive flicker. That's why we have a copy window to do the
 * updates first, and then copy the relevant areas onto the display window
 */
{
  draw_ball(0);
  /*{{{  backgrounds to copy*/
  {
    unsigned  i;
    BACKGROUND *bptr;
	
    for(bptr = update.back.list, i = update.back.backs; i--; bptr++)
      video_copy(display.back, display.copy, GCN(GC_COPY),
	  bptr->place.x, bptr->place.y, bptr->size.x, bptr->size.y,
	  bptr->place.x, bptr->place.y);
  }
  /*}}}*/
  if(global.state != MODE_ZOOM)
    {
      /*{{{  do the monster backgrounds*/
      {
	int       i;
	MONSTER   *mptr;
	MONSTER   *nptr;
	    
	for(mptr = nptr = monster.list, i = monster.monsters; i--; mptr++)
	  {
	    int   new;
	    
	    update.set = 0;
	    if(mptr->type == 5)
	      new = 0;
	    else if(mptr->type == 6)
	      new = SPRITE_DIAMOND + mptr->image;
	    else if(mptr->type > 5)
	      new = mptr->type;
	    else if(mptr->chew)
	      new = SPRITE_CHOMP + mptr->image;
	    else if(mptr->face >= 16)
	      new = SPRITE_SQUISHED - 16 + mptr->type * 2 + mptr->face;
	    else
	      new = SPRITE_MONSTERS + mptr->type * (6 * MONSTER_IMAGES) +
		  mptr->face * MONSTER_IMAGES + mptr->image;
	    if(!mptr->back && mptr->old_sprite && (new != mptr->old_sprite ||
		mptr->pixel.x != mptr->old_pixel.x ||
		mptr->pixel.y != mptr->old_pixel.y ||
		(mptr == &monster.list[0] &&
		  !player.old_ball.state && player.ball.state)) &&
		mptr->on)
	      {
		video_copy(display.back, display.copy,
		    GCN(GC_COPY),
		    mptr->old_pixel.x, mptr->old_pixel.y,
		    CELL_WIDTH, CELL_HEIGHT,
		    mptr->old_pixel.x, mptr->old_pixel.y);
		if(mptr->pixel.x != mptr->old_pixel.x ||
		    mptr->pixel.y != mptr->old_pixel.y || !new)
		  bounding_box(mptr->old_pixel.x, mptr->old_pixel.y,
		      CELL_WIDTH, CELL_HEIGHT);
	      }
	    mptr->old_sprite = new;
	    if(new)
	      {
		mptr->old_pixel.x = mptr->pixel.x;
		mptr->old_pixel.y = mptr->pixel.y;
		mptr->on = INRANGE(mptr->pixel.x,
		    1 - CELL_WIDTH, WINDOW_WIDTH) &&
		    INRANGE(mptr->pixel.y, 1 - CELL_HEIGHT, WINDOW_HEIGHT);
		if(!mptr->back && mptr->on)
		  bounding_box(mptr->old_pixel.x, mptr->old_pixel.y,
		      CELL_WIDTH, CELL_HEIGHT);
		mptr->tptr = nptr;
		nptr++;
	      }
	    else
	      mptr->tptr = NULL;
	    mptr->back = 0;
	    if(update.set)
	      add_background(update.tl.x, update.tl.y,
		  (unsigned)(update.br.x - update.tl.x),
		  (unsigned)(update.br.y - update.tl.y));
	  }
      }
      /*}}}*/
      /*{{{  do the apple backgrounds*/
      {
	int       i;
	APPLE     *aptr;
	APPLE     *dptr;
	    
	for(aptr = dptr = apple.list, i = apple.apples; i--; aptr++)
	  {
	    int   new;
	    
	    update.set = 0;
	    new = aptr->state;
	    if(!aptr->back && (new != aptr->old_state ||
		aptr->pixel.x != aptr->old_pixel.x ||
		aptr->pixel.y != aptr->old_pixel.y))
	      {
		APPLE_SIZE CONST *asp;
		int       x, y;
		unsigned  width, height;
	    
		asp = &apple_sizes[aptr->old_state];
		x = aptr->old_pixel.x + asp->offset.x;
		y = aptr->old_pixel.y + asp->offset.y;
		width = asp->size.x;
		height = asp->size.y;
		video_copy(display.back, display.copy,
		    GCN(GC_COPY), x, y, width, height, x, y);
		bounding_box(x, y, width, height);
	      }
	    if(new != 6)
	      {
		APPLE_SIZE CONST *asp;
		  
		aptr->old_pixel.x = aptr->pixel.x;
		aptr->old_pixel.y = aptr->pixel.y;
		aptr->old_state = new;
		asp = &apple_sizes[new];
		if(!aptr->back)
		  bounding_box(aptr->old_pixel.x + asp->offset.x,
		      aptr->old_pixel.y + asp->offset.y,
		      asp->size.x, asp->size.y);
		if(aptr->list)
		  {
		    assert(aptr->list->tptr);
		    aptr->list = aptr->list->tptr;
		  }
		aptr->back = 0;
		if(aptr != dptr)
		  memcpy(dptr, aptr, sizeof(APPLE));
		dptr++;
	      }
	    else
	      apple.apples--;
	    if(update.set)
	      add_background(update.tl.x, update.tl.y,
		  (unsigned)(update.br.x - update.tl.x),
		  (unsigned)(update.br.y - update.tl.y));
	  }
      }
      /*}}}*/
      /*{{{  do the apple sprites*/
      {
	int       i;
	APPLE     *aptr;
	    
	for(aptr = apple.list, i = apple.apples; i--; aptr++)
	  {
	    SPRITE    *sptr;
	    APPLE_SIZE CONST *asp;
	    int       x, y;
	    unsigned  width, height;
	    
	    asp = &apple_sizes[aptr->old_state];
	    sptr = &sprites[(aptr->ghost && aptr->old_state < 3 ?
		SPRITE_GHOST : SPRITE_APPLE) + aptr->old_state];
	    x = aptr->old_pixel.x + asp->offset.x;
	    y = aptr->old_pixel.y + asp->offset.y;
	    width = asp->size.x;
	    height = asp->size.y;
	    video_copy(sptr->mask, display.copy, GCN(GC_MASK),
		0, 0, width, height, x, y);
	    video_copy(sptr->image, display.copy, GCN(GC_OR),
		0, 0, width, height, x, y);
	  }
      }
      /*}}}*/
      /*{{{  do the monster sprites*/
      {
	int       i;
	MONSTER   *mptr;
	    
	for(mptr = &monster.list[monster.monsters - 1], i = monster.monsters;
	    i--; mptr--)
	  if(mptr->old_sprite)
	    {
	      if(mptr->on)
		{
		  SPRITE    *sptr;
		  
		  sptr = &sprites[mptr->old_sprite];
		  video_copy(sptr->mask, display.copy,
		      GCN(GC_MASK), 0, 0, CELL_WIDTH, CELL_HEIGHT,
		      mptr->old_pixel.x, mptr->old_pixel.y);
		  if(mptr->ghosting)
		    /*{{{  ghosting*/
		    {
		      SPRITE    *dptr;
		      COORD     offset;
		      
		      dptr = &sprites[SPRITE_GHOSTING];
		      offset.x = mptr->ghosting & 0xF;
		      offset.y = (mptr->ghosting >> 4) & 0xF;
		      video_copy(sptr->image, dptr->image,
			  GCN(GC_COPY), 0, 0, CELL_WIDTH, CELL_HEIGHT, 0, 0);
		      video_copy(dptr->mask, dptr->image,
			  GCN(GC_AND), offset.x, offset.y,
			  CELL_WIDTH, CELL_HEIGHT, 0, 0);
		      video_copy(dptr->image, display.copy,
			  GCN(GC_OR), 0, 0, CELL_WIDTH, CELL_HEIGHT,
			  mptr->old_pixel.x, mptr->old_pixel.y);
		      video_fill(dptr->image,
			  GCN(GC_CLEAR), 0, 0, CELL_WIDTH, CELL_HEIGHT);
		      video_copy(dptr->mask, dptr->image,
			  GCN(GC_MASK), offset.x, offset.y,
			  CELL_WIDTH, CELL_HEIGHT, 0, 0);
		      video_copy(sptr->mask, dptr->image,
			  GCN(GC_AND), 0, 0, CELL_WIDTH, CELL_HEIGHT, 0, 0);
		      sptr = dptr;
		      if(!mptr->cycle)
			/*{{{  drift*/
			{
			  unsigned  temp;
			  
			  temp = chaotic();
			  if(!(temp & 0x3) || offset.x == (mptr->ghosting &
			      0x200 ? GHOSTING_WIDTH - 1 - CELL_WIDTH : 0))
			    mptr->ghosting ^= 0x200;
			  if(!(temp & 0xC) || offset.y == (mptr->ghosting &
			      0x400 ? GHOSTING_HEIGHT - 1 - CELL_HEIGHT : 0))
			    mptr->ghosting ^= 0x400;
			  offset.x += mptr->ghosting & 0x200 ? 1 : -1;
			  offset.y += mptr->ghosting & 0x400 ? 1 : -1;
			  mptr->ghosting = (mptr->ghosting & 0x700) |
			      (offset.y << 4) | offset.x;
			}
			/*}}}*/
		    }
		    /*}}}*/
		  video_copy(sptr->image, display.copy,
		      GCN(GC_OR), 0, 0, CELL_WIDTH, CELL_HEIGHT,
		      mptr->old_pixel.x, mptr->old_pixel.y);
		}
	      if(mptr->squished)
		{
		  if(mptr->list)
		    {
		      assert(mptr->list->tptr ||
			(mptr->type == 4 && mptr->list->type == 5));
		      mptr->list = mptr->list->tptr;
		    }
		}
	      else
		mptr->list = NULL;
	    }
      }
      /*}}}*/
    }
  memcpy(&player.old_ball, &player.ball, sizeof(BALL));
  draw_ball(1);
  /*{{{  scores to copy*/
  {
    unsigned  i;
    SCORE     *sptr;
	
    for(sptr = update.score.list, i = update.score.scores; i--; sptr++)
      {
	video_copy(sptr->mask, display.copy, GCN(GC_MASK),
	    0, 0, DIGIT_WIDTH * 4, DIGIT_HEIGHT,
	    sptr->place.x, sptr->place.y);
	video_copy(sptr->image, display.copy, GCN(GC_OR),
	    0, 0, DIGIT_WIDTH * 4, DIGIT_HEIGHT,
	    sptr->place.x, sptr->place.y);
      }
  }
  /*}}}*/
  /*{{{  areas to window*/
  {
    unsigned  i;
    BACKGROUND *bptr;
	
    for(bptr = update.back.list, i = update.back.backs; i--; bptr++)
      present_area(bptr->place.x, bptr->place.y,
	  bptr->size.x, bptr->size.y);
  }
  /*}}}*/
  /*{{{  scores to window*/
  {
    unsigned  i;
    SCORE     *sptr;
	
    for(sptr = update.score.list, i = update.score.scores; i--; sptr++)
      present_area(sptr->place.x, sptr->place.y,
	  DIGIT_WIDTH * 4, DIGIT_HEIGHT);
  }
  /*}}}*/
  /*{{{  delete monsters*/
  {
    MONSTER   *mptr;
    unsigned  count;
    
    for(mptr = monster.list, count = monster.monsters; count--; mptr++)
      if(!mptr->old_sprite)
	monster.monsters--;
      else if(mptr != mptr->tptr)
	memcpy(mptr->tptr, mptr, sizeof(MONSTER));
  }
  /*}}}*/
  update.back.backs = 0;
  video_sync();
  return;
}
/*}}}*/
