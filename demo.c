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
/*{{{  prototypes*/
static int demo_board PROTOARGLIST((void));
static int demo_keys PROTOARGLIST((void));
static int move_demo PROTOARGLIST((void));
static void move_mris PROTOARGLIST((void));
/*}}}*/
/*{{{  int demo_board()*/
static int demo_board FUNCARGVOID
{
  unsigned  den;
  unsigned  count;
  unsigned  quit;

  quit = 0;
  player.screen = random() % 10;
  extra.select = random() % 5;
  extra.got = random() & 0x1F;
  create_xtra_monster(extra.select);
  new_board();
  zoom_board();
  draw_center(SPRITE_DEN);
  monster.monsters = 0;
  spawn_monster(4, 2, 2, PLAYER_START_X, PLAYER_START_Y, 0, 0);
  monster.list[0].stop = 1;
  monster.den = 0;
  player.ball.state = 0;
  player.ball.count = 8;
  player.old_ball.state = 0;
  /*{{{  plonk on M R I S*/
  for(count = 4; count--;)
    {
      unsigned  x, y;
      unsigned  j;
      CELL      *cptr;
      
      do
	{
	  do
	    j = random();
	  while(j >= CELLS_ACROSS * CELLS_DOWN);
	  x = j % CELLS_ACROSS;
	  y = j / CELLS_ACROSS;
	  cptr = BOARDCELL(x, y);
	}
      while(!cptr->visit);
      spawn_monster(SPRITE_MRIS + count + (random() & 4), 0, 0, x, y, 0, 0);
    }
  /*}}}*/
  refresh_window();
  count = DISPLAY_HOLD;
  den = 4;
  timer_start(FRAME_RATE);
  while(count)
    {
      quit = process_xevents(0);
      if(quit)
	{
	  player.button = 1;
	  break;
	}
      /*{{{  calc distances?*/
      if(den != monster.den)
	{
	  den = monster.den;
	  monster.list[0].cell.y = 0;
	  monster.list[0].cell.x = 4 + den;
	  calc_distances();
	  monster.list[0].cell.y = PLAYER_START_Y;
	  monster.list[0].cell.x = PLAYER_START_X;
	}
      /*}}}*/
      move_mris();
      if(monster.den == 4)
	count--;
      if(player.button)
	count = 0;
      show_updates();
	timer_wait();
      }
  timer_stop();
  return quit;
}
/*}}}*/
/*{{{  int demo_keys()*/
static int demo_keys FUNCARGVOID
{
  unsigned  quit;
  unsigned  count;

  monster.monsters = 0;
  apple.apples = 0;
  player.ball.state = 0;
  player.ball.count = 8;
  XFillRectangle(display.display, display.back, GCN(GC_CLEAR),
      0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
  /*{{{  put on M R I S*/
  {
    unsigned  missing;
    unsigned  index;
    int       x, y;
      
    missing = random() & 3;
    for(index = 4; index--;)
      {
	SPRITE    *sptr;
      
	sptr = &sprites[SPRITE_MRIS + index + 4 * (index == missing)];
	XCopyArea(display.display, sptr->mask, display.back, GCN(GC_MASK),
	    0, 0, CELL_WIDTH, CELL_HEIGHT,
	    PIXELX(4 + index, 0), PIXELY(-1, 0));
	XCopyArea(display.display, sptr->image, display.back, GCN(GC_OR),
	    0, 0, CELL_WIDTH, CELL_HEIGHT,
	    PIXELX(4 + index, 0), PIXELY(-1, 0));
      }
    y = random() % (CELLS_DOWN + 2) * (CELL_HEIGHT + GAP_HEIGHT);
    x = random() & 1 ? CELLS_ACROSS * (CELL_WIDTH + GAP_WIDTH) :
	-CELLS_ACROSS * (CELL_WIDTH + GAP_WIDTH);
    spawn_monster(SPRITE_MRIS + missing, 0, 0, 4 + missing, -1, x, y);
  }
  /*}}}*/
  XCopyArea(display.display, display.back, display.copy, GCN(GC_COPY),
      0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0);
  /*{{{  put on the title text*/
  {
    char const **tptr;
    unsigned  length;
    unsigned  index;
    unsigned  gnome;
      
    gnome = 0;
    for(tptr = title_text, index = 0; *tptr; tptr++, index++)
      {
	length = strlen(*tptr);
	if(length)
	  {
	    TEXT      info;
	    char  const *ptr;
	    unsigned  shift;
	      
	    ptr = strchr(*tptr, '-');
	    if(ptr)
	      {
		text_size(*tptr, ptr - *tptr + 1, &info);
		shift = info.width;
	      }
	    else
	      shift = 0;
	    text_size(*tptr, length, &info);
	    XDrawImageString(display.display, display.back, GCN(GC_TEXT),
		WINDOW_WIDTH / 2 - (shift ? shift : info.width / 2),
		PIXELY(index, CELL_HEIGHT / 2) +
		(info.ascent - info.descent) / 2, *tptr, length);
	    /*{{{  spawn monster*/
	    {
	      unsigned  type;
	      int       cellx;
	      int       offsetx;
				
	      do
		{
		  type = random() & 3;
		  if(type & 2)
		    type++;
		}
	      while(type == 4 && gnome);
	      if(type == 4)
		gnome = 1;
	      if(random() & 1)
		{
		  cellx = -2;
		  offsetx = (CELLS_ACROSS + 2) * (CELL_WIDTH + GAP_WIDTH);
		}
	      else
		{
		  cellx = CELLS_ACROSS + 1;
		  offsetx = -(CELLS_ACROSS + 5) * (CELL_WIDTH + GAP_WIDTH);
		}
	      offsetx += CELL_WIDTH * (random() & 3);
	      spawn_monster(type, 0, 0, cellx, index, offsetx, 0);
	    }
	    /*}}}*/
	  }
      }
  }
  /*}}}*/
  count = DISPLAY_HOLD;
  refresh_window();
  timer_start(FRAME_RATE);
  while(count)
    {
      quit = process_xevents(0);
      if(quit)
	{
	  player.button = 1;
	  break;
	}
      if(!move_demo())
	count--;
      if(player.button)
	count = 0;
      show_updates();
      timer_wait();
      }
  timer_stop();
  return quit;
}
/*}}}*/
/*{{{  int demo_mode()*/
extern int demo_mode FUNCARGVOID
{
  unsigned  quit;

  global.state = 6;
  player.keyboard = 0;
  player.button = 0;
  player.pressed = 0;
  player.old_ball.state = 0;
  quit = 0;
  while(!player.button)
    {
      if(!player.button)
	quit = demo_keys();
      if(!player.button)
	quit = demo_board();
    }
  player.button = 0;
  return quit;
}
/*}}}*/
/*{{{  void extra_life()*/
extern void extra_life FUNCARGVOID
/* does the extra life senario */
{
  static char const *text[] = {"Congratulations", "You win extra", NULL};
  unsigned  thrown;
  
  add_background(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
  XFillRectangle(display.display, display.back, GCN(GC_CLEAR),
      BORDER_LEFT + 1, BORDER_TOP + 1, BOARD_WIDTH - 2, BOARD_HEIGHT - 2);
  /*{{{  display some text*/
  {
    unsigned  line;
    TEXT      info;
    char const **tptr;
    unsigned  length;
      
    line = BORDER_TOP + CELL_HEIGHT;
    for(tptr = text; *tptr; tptr++)
      {
	length = strlen(*tptr);
	text_size(*tptr, length, &info);
	XDrawImageString(display.display, display.back, GCN(GC_TEXT),
	    WINDOW_WIDTH / 2 - info.width / 2,
	    line + info.ascent, *tptr, length);
	line += info.ascent + info.descent;
      }
  }
  /*}}}*/
  monster.monsters = 0;
  apple.apples = 0;
  spawn_monster(4, 3, 3, 3, DEN_Y, -(CELL_WIDTH + GAP_WIDTH) * 3, 0);
  spawn_monster(0, 2, 2, 0, DEN_Y,
      (CELLS_ACROSS - 1) * (CELL_WIDTH + GAP_WIDTH), 0);
  /*{{{  add m r i s*/
  {
    unsigned  i;
    for(i = 4; i--;)
      {
	XCopyArea(display.display, sprites[SPRITE_MRIS + 4 + i].mask,
	    display.back, GCN(GC_MASK), 0, 0, CELL_WIDTH, CELL_HEIGHT,
	    PIXELX(CELLS_ACROSS / 2 - 2 + i, 0), PIXELY(2, 0));
	XCopyArea(display.display, sprites[SPRITE_MRIS + 4 + i].image,
	    display.back, GCN(GC_OR), 0, 0, CELL_WIDTH, CELL_HEIGHT,
	    PIXELX(CELLS_ACROSS / 2 - 2 + i, 0), PIXELY(2, 0));
	spawn_monster(SPRITE_MRIS + i, 1, 1, CELLS_ACROSS / 2 - 2 + i,
	    2, 0, -(CELL_HEIGHT + GAP_HEIGHT) * 3 - CELL_HEIGHT / 2 * i);
      }
  }
  /*}}}*/
  draw_extra_letter(extra.select);
  player.pressed = 0;
  /*{{{  create a path*/
  {
    unsigned  i;
    CELL      *cptr;
	
    apple.apples = 0;
    for(cptr = BOARDCELL(0, DEN_Y), i = CELLS_ACROSS; i--; cptr++)
      {
	cptr->visit = 1;
	cptr->sprite = 0;
	cptr->distance = CELLS_ACROSS - i;
	cptr->depths[0] = 0;
	cptr->depths[1] = 0;
	cptr->depths[2] = -(CELL_WIDTH + GAP_WIDTH);
	cptr->depths[3] = CELL_WIDTH + GAP_WIDTH;
      }
    BOARDCELL(0, DEN_Y)->depths[2] = 0;
    BOARDCELL(CELLS_ACROSS - 1, DEN_Y)->depths[3] = 0;
  }
  /*}}}*/
  global.state = 5;
  thrown = 0;
  while(thrown != 3)
    {
      process_xevents(1);
      if(!monster.list[0].offset.x && !thrown)
	{
	  thrown = 1;
	  player.throw = 1;
	}
      bounce_ball();
      if(player.ball.state == 3)
	player.ball.state = 4;
      else if(player.ball.state == 4)
	{
	  player.ball.pixel.x = PIXELX(player.lives - 1, 0) + CELL_WIDTH / 2;
	  player.ball.pixel.y = BORDER_TOP + BOARD_HEIGHT + 1 +
	      CELL_HEIGHT / 2 - (CELL_HEIGHT + GAP_HEIGHT) * 3;
	}
      else if(!player.ball.state && thrown == 1)
	{
	  spawn_monster(SPRITE_PLAYER + 6, 0, 1, player.lives - 1, CELLS_DOWN,
	      0, -(CELL_HEIGHT + GAP_HEIGHT) * 3);
	  thrown = 2;
	}
      if(!move_demo() && thrown == 2)
	thrown = 3;
      show_updates();
      timer_wait();
    }
  XCopyArea(display.display, display.copy, display.back, GCN(GC_COPY),
      PIXELX(player.lives - 1, 0), PIXELY(CELLS_DOWN, 0),
      CELL_WIDTH, CELL_HEIGHT,
      PIXELX(player.lives - 1, 0), PIXELY(CELLS_DOWN, 0));
  player.lives++;
  return;
}
/*}}}*/
/*{{{  int move_demo()*/
static int move_demo FUNCARGVOID
/*
 * moves the monsters used in the demo screens
 * we take each monster with a non-zero offset, and move it
 * towards a zero offset (changing x first)
 * it might get blown up by the ball
 * returns the number of objects which moved
 */
{
  MONSTER   *mptr;
  unsigned  i;
  unsigned  moved;

  moved = 0;
  for(mptr = monster.list, i = monster.monsters; i--; mptr++)
    {
      if(mptr->shot)
	mptr->type = 5;
      else
	{
	  if(mptr->offset.x)
	    /*{{{  left or right*/
	    {
	      int       dir;
				
	      moved++;
	      if(mptr->offset.x > 0)
		{
		  dir = 2;
		  mptr->offset.x -= VEL_X;
		  mptr->pixel.x -= VEL_X;
		}
	      else
		{
		  dir = 3;
		  mptr->offset.x += VEL_X;
		  mptr->pixel.x += VEL_X;
		}
	      if(dir != mptr->dir)
		{
		  mptr->dir = dir;
		  new_face(mptr);
		}
				  
	    }
	    /*}}}*/
	  else if(mptr->offset.y)
	    /*{{{  up or down*/
	    {
	      int       dir;
				
	      moved++;
	      if(mptr->offset.y > 0)
		{
		  dir = 0;
		  mptr->offset.y -= VEL_Y;
		  mptr->pixel.y -= VEL_Y;
		}
	      else
		{
		  dir = 1;
		  mptr->offset.y += VEL_Y;
		  mptr->pixel.y += VEL_Y;
		}
	      if(dir != mptr->dir)
		{
		  mptr->dir = dir;
		  new_face(mptr);
		}
				  
	    }
	    /*}}}*/
	  else
	    mptr->stop = 1;
	  if(!mptr->stop)
	    {
	      if(!mptr->cycle)
		{
		  mptr->cycle = MONSTER_CYCLES;
		  mptr->image++;
		  if(mptr->image == MONSTER_IMAGES)
		    mptr->image = 0;
		}
	      mptr->cycle--;
	    }
	}
	
    }
  return moved;
}
/*}}}*/
/*{{{  void move_mris()*/
static void move_mris FUNCARGVOID
/*
 * moves M R I S sprites around the board
 * towards the top
 */
{
  unsigned  i;
  MONSTER   *mptr;

  for(mptr = &monster.list[1], i = monster.monsters - 1; i--; mptr++)
    {
      CELL      *cptr;
	
      assert(mptr->type >= SPRITE_MRIS && mptr->type < SPRITE_MRIS + 8);
      cptr = BOARDCELL(mptr->cell.x, mptr->cell.y);
      if(mptr->offset.x || mptr->offset.y)
	move_movable(mptr, cptr);
      else if(!mptr->cell.y && mptr->cell.x == 4 + i)
	{
	  if(monster.den == i)
	    monster.den = i + 1;
	}
      else
	{
	  unsigned  valid;
	  unsigned  temp;
	  
	  valid = valid_directions(mptr, cptr);
	  temp = valid & (0xF ^ (1 << (mptr->dir ^ 1)));
	  if(temp)
	    valid &= temp | 0xF0;
	  if(monster.den == i && (temp = valid & (valid >> 4)))
	    valid = temp;
	  else
	    valid &= 0xF;
	  mptr->dir = choose_direction(valid);
	  move_movable(mptr, cptr);
	}
    }
  return;
}
/*}}}*/
/*{{{  void show_history()*/
extern void show_history FUNCARGVOID
/* shows the history list */
{
  unsigned  count;
  
  add_background(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
  draw_extra_letter(extra.select);
  XFillRectangle(display.display, display.back, GCN(GC_CLEAR),
      BORDER_LEFT + 1, BORDER_TOP + 1, BOARD_WIDTH - 2, BOARD_HEIGHT - 2);
  monster.monsters = 0;
  apple.apples = 0;
  player.ball.state = 0;
  player.ball.count = 8;
  player.old_ball.state = 0;
  /*{{{  add in the parts*/
  {
    unsigned  base;
    unsigned  screens;
    unsigned  index;
    TEXT      info;
    char      text[11];
    unsigned  length;
    static int sprites[3] = {SPRITE_CHERRY, SPRITE_NORMAL + 4, SPRITE_PLAYER + 4};
      
    screens = player.screen < CELLS_DOWN - 2 ? player.screen : CELLS_DOWN - 2;
    base = player.screen - screens;
    text_size("Screen 090", 10, &info);
    for(index = screens; index--;)
      {
	sprintf(text, "Screen %d", base + index + 1);
	length = strlen(text);
	XDrawImageString(display.display, display.back, GCN(GC_TEXT),
	    PIXELX(CELLS_ACROSS / 2, -GAP_WIDTH) - info.width,
	    PIXELY(screens - index, (CELL_HEIGHT + GAP_HEIGHT) / 2) +
	    (info.ascent - info.descent) / 2,
	    text, length);
	if(history.prize & 1 << (screens - 1 - index))
	  spawn_monster(SPRITE_PRIZE_BASE +
	      (base + index) % SPRITE_PRIZES, 0, 0,
	      CELLS_ACROSS / 2 + 2, screens - index,
	      0, index * (2 * CELL_HEIGHT + GAP_HEIGHT) +
	      CELLS_DOWN * GAP_HEIGHT + CELL_HEIGHT / 2 + GAP_HEIGHT +
	      (CELL_HEIGHT + GAP_HEIGHT) * (2 + CELLS_DOWN - screens));
	spawn_monster(sprites[(history.ending >>
	    (screens - 1 - index) * 2) & 3],
	    0, 0, CELLS_ACROSS / 2, screens - index,
	    0, index * (2 * CELL_HEIGHT + GAP_HEIGHT) +
	    CELLS_DOWN * GAP_HEIGHT + 
	    (CELL_HEIGHT + GAP_HEIGHT) * (2 + CELLS_DOWN - screens));
      }
  }
  /*}}}*/
  global.state = 8;
  count = DISPLAY_HOLD;
  while(count)
    {
      process_xevents(1);
      if(!move_demo())
	count--;
      show_updates();
      timer_wait();
    }
  return;
}
/*}}}*/
