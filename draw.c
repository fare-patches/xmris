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
#include "time.h"
/*{{{  prototypes*/
static void xor_ball PROTOARGLIST((void));
/*}}}*/
/*{{{  void add_background(x, y, w, h)*/
extern void add_background FUNCARGLIST((x, y, width, height))
int       x       FUNCARGSEP
int       y       FUNCARGSEP
int       width   FUNCARGSEP
int       height  FUNCARGTERM
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
/*{{{  void bounding_box(x, y, width, height)*/
extern void bounding_box FUNCARGLIST((x, y, width, height))
int       x       FUNCARGSEP
int       y       FUNCARGSEP
unsigned  width   FUNCARGSEP
unsigned  height  FUNCARGTERM
/*
 * updates the update box so that it includes the
 * supplied rectangle/
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
/*{{{  void draw_center(index)*/
extern void draw_center FUNCARGLIST((index))
int   index FUNCARGTERM
/*
 * sets the center sprite and draws it on the background
 */
{
  SPRITE    *sptr;

  BOARDCELL(DEN_X, DEN_Y)->sprite = index;
  add_background(PIXELX(DEN_X, 0), PIXELY(DEN_Y, 0), CELL_WIDTH, CELL_HEIGHT);
  XFillRectangle(display.display, display.back, GCN(GC_CLEAR),
      PIXELX(DEN_X, 0), PIXELY(DEN_Y, 0), CELL_WIDTH, CELL_HEIGHT);
  sptr = &sprites[index];
  if(display.background != COLOUR_ZERO)
    XCopyArea(display.display, sptr->mask, display.back, GCN(GC_MASK),
	0, 0, CELL_WIDTH, CELL_HEIGHT, PIXELX(DEN_X, 0), PIXELY(DEN_Y, 0));
  XCopyArea(display.display, sptr->image, display.back, GCN(GC_OR),
      0, 0, CELL_WIDTH, CELL_HEIGHT, PIXELX(DEN_X, 0), PIXELY(DEN_Y, 0));
  return;
}
/*}}}*/
/*{{{  void draw_extra()*/
extern void draw_extra FUNCARGVOID
{
  int       x;
  SPRITE    *sptr;
  
  x = XTRA_X + extra.select * XTRA_SPACING;
  sptr = &sprites[SPRITE_XTRA + (random() & 1)];
  XCopyArea(display.display, sptr->mask, display.back, GCN(GC_MASK),
      0, 0, CELL_WIDTH, CELL_HEIGHT, x, XTRA_Y);
  XCopyArea(display.display, sptr->image, display.back, GCN(GC_OR),
      0, 0, CELL_WIDTH, CELL_HEIGHT, x, XTRA_Y);
  add_background(x, XTRA_Y, CELL_WIDTH, CELL_HEIGHT);
  return;
}
/*}}}*/
/*{{{  void new_board()*/
extern void new_board FUNCARGVOID
/*
 * sets up a new board
 */
{
  BOARD const *map;

  player.screen++;
  map = &boards[player.screen % BOARDS];
  /*{{{  clear board array*/
  {
    unsigned  i;
    CELL      *ptr;
	
    for(i = sizeof(board) / sizeof(board[0]), ptr = board; i--; ptr++)
      {
	ptr->depths[0] = 0;
	ptr->depths[1] = 0;
	ptr->depths[2] = 0;
	ptr->depths[3] = 0;
	ptr->visit = 0;
	ptr->distance = 0;
	ptr->sprite = 0;
      }
  }
  /*}}}*/
  /*{{{  draw blank background*/
  {
    /*{{{  set context*/
    {
      XGCValues gcv;
	  
      gcv.fill_style = FillTiled;
      gcv.tile = sprites[SPRITE_FILL_BASE + map->fill].image;
      XChangeGC(display.display, GCN(GC_BOARD), GCTile | GCFillStyle, &gcv);
    }
    /*}}}*/
    XFillRectangle(display.display, display.back, GCN(GC_CLEAR),
	0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    XFillRectangle(display.display, display.back, GCN(GC_BOARD),
	BORDER_LEFT, BORDER_TOP, BOARD_WIDTH, BOARD_HEIGHT);
    XFillRectangle(display.display, display.back, GCN(GC_CLEAR),
	PIXELX(DEN_X, -GAP_WIDTH / 2), PIXELY(DEN_Y, -GAP_HEIGHT / 2),
	CELL_WIDTH + GAP_WIDTH, CELL_HEIGHT + GAP_HEIGHT);
    XFillRectangle(display.display, display.back, GCN(GC_CLEAR),
	BORDER_LEFT + (CELL_WIDTH + GAP_WIDTH) * 4 + GAP_WIDTH, BORDER_TOP,
	XTRA_SPACING * 4 + CELL_WIDTH, GAP_HEIGHT * 2);
    XDrawRectangle(display.display, display.back, GCN(GC_SET),
	XTRA_X - 1, XTRA_Y - 1,
	XTRA_SPACING * 4 + CELL_WIDTH + 1, CELL_HEIGHT + 1);
    XDrawRectangle(display.display, display.back, GCN(GC_SET),
	BORDER_LEFT, BORDER_TOP,
	BOARD_WIDTH - 1, BOARD_HEIGHT - 1);
  }
  /*}}}*/
  /*{{{  add the xtra*/
  {
    unsigned  i;
      
    extra.escape = 0;
    for(i = 5; i--;)
      draw_extra_letter(i);
    draw_extra();
  }
  /*}}}*/
  /*{{{  add the screen number*/
  {
    char      text[10];
    int       length;
    int       ascent, descent;
    int       direction;
    XCharStruct chars;
      
    strcpy(text, "Screen ");
    length = 7 + itoa(text + 7, player.screen, 0);
    XQueryTextExtents(display.display, font.font, text, length,
	  &direction, &ascent, &descent, &chars);
    XDrawImageString(display.display, display.back, GCN(GC_TEXT),
	BORDER_LEFT + (CELL_WIDTH + GAP_WIDTH) * 8 +
	CELL_WIDTH / 2 + GAP_WIDTH, (CELL_HEIGHT - ascent - descent) / 2 +
	ascent + BORDER_TOP - CELL_HEIGHT, text, length);
  }
  /*}}}*/
  add_score(0, 0, 0);
  /*{{{  add lives*/
  if(player.lives)
    {
      unsigned  lives;
      SPRITE    *sptr;
      
      sptr = &sprites[SPRITE_PLAYER + 6];
      for(lives = player.lives - 1; lives--;)
	{
	  unsigned  x, y;
	    
	  x = PIXELX(lives, 0);
	  y = PIXELY(CELLS_DOWN, 0);
	  XCopyArea(display.display, sptr->mask, display.back, GCN(GC_MASK),
	      0, 0, CELL_WIDTH, CELL_HEIGHT, x, y);
	  XCopyArea(display.display, sptr->image, display.back, GCN(GC_OR),
	      0, 0, CELL_WIDTH, CELL_HEIGHT, x, y);
	}
    }
  /*}}}*/
  /*{{{  copy the map*/
  {
    char const *mptr;
    CELL      *cptr;
    unsigned  y, x;
	
    mptr = (char const *)map->map;
    cptr = BOARDCELL(0, 0);
    for(y = CELLS_DOWN; y--; cptr += CELL_STRIDE - CELLS_ACROSS)
      for(x = CELLS_ACROSS; x--; mptr++, cptr++)
	{
	  switch(*mptr)
	  {
	    /*{{{  case '@': (cherry)*/
	    case '@':
	      cptr->sprite = SPRITE_CHERRY;
	      break;
	    /*}}}*/
	    /*{{{  case 'X': (path)*/
	    case 'X':
	      cptr[0].visit = 1;
	      if(cptr[-1].visit)
		{
		  cptr[-1].depths[3] = CELL_WIDTH + GAP_WIDTH;
		  cptr[0].depths[2] = -(CELL_WIDTH + GAP_WIDTH);
		}
	      if(cptr[-CELL_STRIDE].visit)
		{
		  cptr[-CELL_STRIDE].depths[1] = CELL_HEIGHT + GAP_HEIGHT;
		  cptr[0].depths[0] = -(CELL_HEIGHT + GAP_HEIGHT);
		}
	      break;
	    /*}}}*/
	  }
	}
  }
  /*}}}*/
  /*{{{  add the apples*/
  {
    unsigned  i;
	
    apple.apples = 0;
    for(i = INITIAL_APPLES; i--;)
      {
	unsigned  y, x;
	unsigned  j;
	CELL      *cptr;
	APPLE     *aptr;
	  
	do
	  {
	    do
	      j = random();
	    while(j >= CELLS_ACROSS * (CELLS_DOWN - 1));
	    x = j % CELLS_ACROSS;
	    y = j / CELLS_ACROSS;
	    cptr = BOARDCELL(x, y);
	    for(aptr = apple.list, j = apple.apples; j--; aptr++)
	      if(aptr->cell.x == x && aptr->cell.y == y)
		{
		  aptr = NULL;
		  break;
		}
	  }
	while(cptr->visit || cptr->sprite || cptr[CELL_STRIDE].visit || !aptr);
	spawn_apple(x, y, 0, 0);
      }
  }
  /*}}}*/
  /*{{{  cut the background*/
  {
    unsigned  y, x;
    COORD     cell;
    CELL      *cptr;
	
    cptr = BOARDCELL(0, 0);
    cell.x = GAP_WIDTH + BORDER_LEFT;
    cell.y = GAP_HEIGHT + BORDER_TOP;
    for(y = CELLS_DOWN; y--; cptr += CELL_STRIDE - CELLS_ACROSS,
	cell.x -= (CELL_WIDTH + GAP_WIDTH) * CELLS_ACROSS,
	cell.y += CELL_HEIGHT + GAP_HEIGHT)
      for(x = CELLS_ACROSS; x--; cptr++, cell.x += CELL_WIDTH + GAP_WIDTH)
	if(cptr->visit)
	  munch_hole(cptr, cell.x, cell.y);
	else if(cptr->sprite)
	  {
	    SPRITE    *sptr;
      
	    sptr = &sprites[SPRITE_CHERRY];
	    XCopyArea(display.display, sptr->mask, display.back, GCN(GC_MASK),
		0, 0, CELL_WIDTH, CELL_HEIGHT, cell.x, cell.y);
	    XCopyArea(display.display, sptr->image, display.back, GCN(GC_OR),
		0, 0, CELL_WIDTH, CELL_HEIGHT, cell.x, cell.y);
	  }
  }
  /*}}}*/
  /*{{{  initialize stuff*/
  {
    global.cherries = 5 * 8;
    global.difficulty = player.screen + DIFFICULTY_PEDESTAL;
    monster.normals = player.screen >= 3 ? 8 : 6;
    update.back.backs = 0;
    update.score.scores = 0;
  }
  /*}}}*/
  XCopyArea(display.display, display.back, display.copy, GCN(GC_COPY),
      0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0);
  return;
}
/*}}}*/
/*{{{  void refresh_window()*/
extern void refresh_window FUNCARGVOID
/*
 * refreshes the display window
 */
{
  XCopyArea(display.display, display.copy, display.window, GCN(GC_COPY), 
      0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0);
  xor_ball();
  XDrawLine(display.display, display.window, GCN(GC_BALL),
      WINDOW_WIDTH - global.missed, WINDOW_HEIGHT - 1,
      WINDOW_WIDTH, WINDOW_HEIGHT - 1);
  return;
}
/*}}}*/
/*{{{  void show_updates()*/
extern void show_updates FUNCARGVOID
/*
 * shows all the updates on the update list
 */
{
  xor_ball();
  /*{{{  backgrounds to copy*/
  {
    unsigned  i;
    BACKGROUND *bptr;
	
    for(bptr = update.back.list, i = update.back.backs; i--; bptr++)
      XCopyArea(display.display, display.back, display.copy, GCN(GC_COPY),
	  bptr->place.x, bptr->place.y, bptr->size.x, bptr->size.y,
	  bptr->place.x, bptr->place.y);
  }
  /*}}}*/
  /*{{{  do the monster backgrounds*/
  {
    int       i;
    MONSTER   *mptr;
      
    for(mptr = monster.list, i = monster.monsters; i--; mptr++)
      {
	int   new;
      
	update.set = 0;
	if(mptr->type == 5)
	  new = 0;
	else if(mptr->type > 5)
	  new = mptr->type;
	else if(mptr->chew)
	  new = SPRITE_CHOMP + mptr->image;
	else if(mptr->face >= 8)
	  new = SPRITE_SQUISHED - 8 + mptr->type * 2 + mptr->face;
	else
	  new = SPRITE_MONSTERS + mptr->type * (6 * MONSTER_IMAGES) +
	      mptr->face * MONSTER_IMAGES + mptr->image;
	if(mptr->old_sprite && (new != mptr->old_sprite ||
	    mptr->pixel.x != mptr->old_pixel.x ||
	    mptr->pixel.y != mptr->old_pixel.y ||
	    (mptr == &monster.list[0] &&
	      !player.old_ball.state && player.ball.state)))
	  {
	    XCopyArea(display.display, display.back, display.copy, GCN(GC_COPY),
		mptr->old_pixel.x, mptr->old_pixel.y, CELL_WIDTH, CELL_HEIGHT,
		mptr->old_pixel.x, mptr->old_pixel.y);
	    if(mptr->pixel.x != mptr->old_pixel.x ||
		mptr->pixel.y != mptr->old_pixel.y || !new)
	      bounding_box(mptr->old_pixel.x, mptr->old_pixel.y,
		  CELL_WIDTH, CELL_HEIGHT);
	  }
	if(new)
	  {
	    mptr->old_pixel.x = mptr->pixel.x;
	    mptr->old_pixel.y = mptr->pixel.y;
	    mptr->old_sprite = new;
	    bounding_box(mptr->old_pixel.x, mptr->old_pixel.y,
		CELL_WIDTH, CELL_HEIGHT);
	  }
	else
	  {
	    memmove(mptr, mptr + 1, i * sizeof(MONSTER));
	    mptr--;
	    monster.monsters--;
	  }
	if(update.set)
	  add_background(update.tl.x, update.tl.y,
	      update.br.x - update.tl.x, update.br.y - update.tl.y);
      }
  }
  /*}}}*/
  /*{{{  do the apple backgrounds*/
  {
    int       i;
    APPLE     *aptr;
      
    for(aptr = apple.list, i = apple.apples; i--; aptr++)
      {
	int   new;
      
	update.set = 0;
	new = aptr->state;
	if(new != aptr->old_state || aptr->pixel.x != aptr->old_pixel.x ||
	    aptr->pixel.y != aptr->old_pixel.y)
	  {
	    APPLE_SIZE const *asp;
	    int       x, y;
	    int       width, height;
      
	    asp = &apple_sizes[aptr->old_state];
	    x = aptr->old_pixel.x + asp->offset.x;
	    y = aptr->old_pixel.y + asp->offset.y;
	    width = asp->size.x;
	    height = asp->size.y;
	    XCopyArea(display.display, display.back, display.copy, GCN(GC_COPY),
		x, y, width, height, x, y);
	    bounding_box(x, y, width, height);
	  }
	if(new != 6)
	  {
	    APPLE_SIZE const *asp;
	      
	    aptr->old_pixel.x = aptr->pixel.x;
	    aptr->old_pixel.y = aptr->pixel.y;
	    aptr->old_state = new;
	    asp = &apple_sizes[new];
	    bounding_box(aptr->old_pixel.x + asp->offset.x,
		aptr->old_pixel.y + asp->offset.y, asp->size.x, asp->size.y);
	  }
	else
	  {
	    unsigned  j;
	    MONSTER   *mptr;
      
	    for(mptr = monster.list, j =  monster.monsters; j--; mptr++)
	      if(!mptr->apple)
		/*EMPTY*/;
	      else if(mptr->apple == aptr)
		mptr->apple = NULL;
	      else if(mptr->apple > aptr)
		mptr->apple--;
	    memmove(aptr, aptr + 1, i * sizeof(APPLE));
	    aptr--;
	    apple.apples--;
	  }
	if(update.set)
	  add_background(update.tl.x, update.tl.y,
	      update.br.x - update.tl.x, update.br.y - update.tl.y);
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
	APPLE_SIZE const *asp;
	int       x, y;
	int       width, height;
      
	asp = &apple_sizes[aptr->old_state];
	sptr = &sprites[SPRITE_APPLE + aptr->old_state];
	x = aptr->old_pixel.x + asp->offset.x;
	y = aptr->old_pixel.y + asp->offset.y;
	width = asp->size.x;
	height = asp->size.y;
	XCopyArea(display.display, sptr->mask, display.copy, GCN(GC_MASK),
	    0, 0, width, height, x, y);
	XCopyArea(display.display, sptr->image, display.copy, GCN(GC_OR),
	    0, 0, width, height, x, y);
      }
  }
  /*}}}*/
  /*{{{  do the monster sprites*/
  {
    int       i;
    MONSTER   *mptr;
      
    for(mptr = &monster.list[monster.monsters - 1], i = monster.monsters; i--; mptr--)
      {
	SPRITE    *sptr;
      
	sptr = &sprites[mptr->old_sprite];
	XCopyArea(display.display, sptr->mask, display.copy, GCN(GC_MASK),
	    0, 0, CELL_WIDTH, CELL_HEIGHT,
	    mptr->old_pixel.x, mptr->old_pixel.y);
	XCopyArea(display.display, sptr->image, display.copy, GCN(GC_OR),
	    0, 0, CELL_WIDTH, CELL_HEIGHT,
	    mptr->old_pixel.x, mptr->old_pixel.y);
      }
  }
  /*}}}*/
  /*{{{  holding the ball?*/
  if(!player.ball.state && player.ball.count < 8)
    {
      COORD const *hold;
      
      hold = &ball_hold[player.ball.count * MONSTER_IMAGES + player.ball.image];
      if(display.foreground == COLOUR_WEIRD)
	XCopyArea(display.display, sprites[SPRITE_BALL].mask,
	    display.copy, GCN(GC_MASK), 0, 0, BALL_WIDTH, BALL_HEIGHT,
	    player.ball.pixel.x + hold->x, player.ball.pixel.y + hold->y);
      XCopyArea(display.display, sprites[SPRITE_BALL].image,
	  display.copy, GCN(GC_OR), 0, 0, BALL_WIDTH, BALL_HEIGHT,
	  player.ball.pixel.x + hold->x, player.ball.pixel.y + hold->y);
    }
  /*}}}*/
  /*{{{  scores to copy*/
  {
    unsigned  i;
    SCORE     *sptr;
      
    for(sptr = update.score.list, i = update.score.scores; i--; sptr++)
      {
	XCopyArea(display.display, sptr->mask, display.copy, GCN(GC_MASK),
	    0, 0, DIGIT_WIDTH * 4, DIGIT_HEIGHT,
	    sptr->place.x, sptr->place.y);
	XCopyArea(display.display, sptr->image, display.copy, GCN(GC_OR),
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
      XCopyArea(display.display, display.copy, display.window, GCN(GC_COPY),
	  bptr->place.x, bptr->place.y, bptr->size.x, bptr->size.y,
	  bptr->place.x, bptr->place.y);
  }
  /*}}}*/
  /*{{{  scores to window*/
  {
    unsigned  i;
    SCORE     *sptr;
      
    for(sptr = update.score.list, i = update.score.scores; i--; sptr++)
      XCopyArea(display.display, display.copy, display.window, GCN(GC_COPY),
	  sptr->place.x, sptr->place.y, DIGIT_WIDTH * 4, DIGIT_HEIGHT,
	  sptr->place.x, sptr->place.y);
  }
  /*}}}*/
  update.back.backs = 0;
  memcpy(&player.old_ball, &player.ball, sizeof(BALL));
  xor_ball();
  XSync(display.display, False);
  return;
}
/*}}}*/
/*{{{  void text_size(text, length, tptr)*/
extern void text_size FUNCARGLIST((text, length, tptr))
char const *text  FUNCARGSEP
unsigned  length  FUNCARGSEP
TEXT      *tptr   FUNCARGTERM
/*
 * wraps up the XQueryTextExtents for us
 */
{
  int       direction;
  XCharStruct chars;
  
  XQueryTextExtents(display.display, font.font, text, length,
	  &direction, &tptr->ascent, &tptr->descent, &chars);
  tptr->width = chars.width;
  return;
}
/*}}}*/
/*{{{  void xor_ball()*/
static void xor_ball FUNCARGVOID
/*
 * draws the old ball on the window directly
 * using the ball's gc
 * if the ball is being held, then we don't draw it,
 * as that's done differently to stop it flickering
 */
{
  switch(player.old_ball.state)
  {
    /*{{{  case 0:*/
    case 0:
      break;
    /*}}}*/
    /*{{{  case 1:*/
    case 1:
    {
      XCopyArea(display.display, ball_xor, display.window, GCN(GC_BALL),
	  0, 0, BALL_WIDTH, BALL_HEIGHT,
	  player.old_ball.pixel.x - BALL_WIDTH / 2,
	  player.old_ball.pixel.y - BALL_HEIGHT / 2);
      break;
    }
    /*}}}*/
    /*{{{  case 2: case 4:*/
    case 2: case 4:
    {
      COORD     offset;
      COORD     pixel;
      unsigned  bits;
	
      pixel.x = player.old_ball.pixel.x - BALL_WIDTH / 2;
      pixel.y = player.old_ball.pixel.y - BALL_HEIGHT / 2;
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
	    if(pixel.x + offset.x > BOARD_WIDTH)
	      bits &= 0x8F;
	    if(pixel.y + offset.y > BOARD_HEIGHT)
	      bits &= 0xE3;
	  }
	  /*}}}*/
	  /*{{{  do inner bits*/
	  {
	    if(bits & 0x01)
	      XCopyArea(display.display, ball_xor, display.window, GCN(GC_BALL),
		  0, 0, BALL_WIDTH, BALL_HEIGHT,
		  pixel.x - offset.x, pixel.y - offset.y);
	    if(bits & 0x02)
	      XCopyArea(display.display, ball_xor, display.window, GCN(GC_BALL),
		  0, 0, BALL_WIDTH, BALL_HEIGHT,
		  pixel.x - offset.x, pixel.y);
	    if(bits & 0x04)
	      XCopyArea(display.display, ball_xor, display.window, GCN(GC_BALL),
		  0, 0, BALL_WIDTH, BALL_HEIGHT,
		  pixel.x - offset.x, pixel.y + offset.y);
	    if(bits & 0x08)
	      XCopyArea(display.display, ball_xor, display.window, GCN(GC_BALL),
		  0, 0, BALL_WIDTH, BALL_HEIGHT,
		  pixel.x, pixel.y + offset.y);
	    if(bits & 0x10)
	      XCopyArea(display.display, ball_xor, display.window, GCN(GC_BALL),
		  0, 0, BALL_WIDTH, BALL_HEIGHT,
		  pixel.x + offset.x, pixel.y + offset.y);
	    if(bits & 0x20)
	      XCopyArea(display.display, ball_xor, display.window, GCN(GC_BALL),
		  0, 0, BALL_WIDTH, BALL_HEIGHT,
		  pixel.x + offset.x, pixel.y);
	    if(bits & 0x40)
	      XCopyArea(display.display, ball_xor, display.window, GCN(GC_BALL),
		  0, 0, BALL_WIDTH, BALL_HEIGHT,
		  pixel.x + offset.x, pixel.y - offset.y);
	    if(bits & 0x80)
	      XCopyArea(display.display, ball_xor, display.window, GCN(GC_BALL),
		  0, 0, BALL_WIDTH, BALL_HEIGHT,
		  pixel.x, pixel.y - offset.y);
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
	    if(pixel.x + offset.x > BOARD_WIDTH)
	      bits &= 0x8F;
	    if(pixel.y + offset.y > BOARD_HEIGHT)
	      bits &= 0xE3;
	  }
	  /*}}}*/
	  /*{{{  do outer bits*/
	  {
	    if(bits & 0x01)
	      XCopyArea(display.display, ball_xor, display.window, GCN(GC_BALL),
		  0, 0, BALL_WIDTH, BALL_HEIGHT,
		  pixel.x - offset.x, pixel.y - offset.y);
	    if(bits & 0x02)
	      XCopyArea(display.display, ball_xor, display.window, GCN(GC_BALL),
		  0, 0, BALL_WIDTH, BALL_HEIGHT,
		  pixel.x - offset.x, pixel.y);
	    if(bits & 0x04)
	      XCopyArea(display.display, ball_xor, display.window, GCN(GC_BALL),
		  0, 0, BALL_WIDTH, BALL_HEIGHT,
		  pixel.x - offset.x, pixel.y + offset.y);
	    if(bits & 0x08)
	      XCopyArea(display.display, ball_xor, display.window, GCN(GC_BALL),
		  0, 0, BALL_WIDTH, BALL_HEIGHT,
		  pixel.x, pixel.y + offset.y);
	    if(bits & 0x10)
	      XCopyArea(display.display, ball_xor, display.window, GCN(GC_BALL),
		  0, 0, BALL_WIDTH, BALL_HEIGHT,
		  pixel.x + offset.x, pixel.y + offset.y);
	    if(bits & 0x20)
	      XCopyArea(display.display, ball_xor, display.window, GCN(GC_BALL),
		  0, 0, BALL_WIDTH, BALL_HEIGHT,
		  pixel.x + offset.x, pixel.y);
	    if(bits & 0x40)
	      XCopyArea(display.display, ball_xor, display.window, GCN(GC_BALL),
		  0, 0, BALL_WIDTH, BALL_HEIGHT,
		  pixel.x + offset.x, pixel.y - offset.y);
	    if(bits & 0x80)
	      XCopyArea(display.display, ball_xor, display.window, GCN(GC_BALL),
		  0, 0, BALL_WIDTH, BALL_HEIGHT,
		  pixel.x, pixel.y - offset.y);
	  }
	  /*}}}*/
	}
      else
	XCopyArea(display.display, ball_xor, display.window, GCN(GC_BALL),
	    0, 0, BALL_WIDTH, BALL_HEIGHT, pixel.x, pixel.y);
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
/*{{{  void zoom_board()*/
extern void zoom_board FUNCARGVOID
/* zooms out on the initial board */
{
  unsigned  count;
    
  XFillRectangle(display.display, display.copy, GCN(GC_CLEAR),
      BORDER_LEFT + 1, BORDER_TOP + 1, BOARD_WIDTH - 2, BOARD_HEIGHT - 2);
  XCopyArea(display.display, display.copy, display.window, GCN(GC_COPY),
      0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0);
  XCopyArea(display.display, display.back, display.copy, GCN(GC_COPY),
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
	XCopyArea(display.display, sptr->mask, display.copy, GCN(GC_MASK),
	    0, 0, CELL_WIDTH, CELL_HEIGHT, aptr->pixel.x, aptr->pixel.y);
	XCopyArea(display.display, sptr->image, display.copy, GCN(GC_OR),
	    0, 0, CELL_WIDTH, CELL_HEIGHT, aptr->pixel.x, aptr->pixel.y);
      }
  }
  /*}}}*/
  timer_start(ZOOM_RATE);
  for(count = 0; count < BOARD_HEIGHT / (2 * ZOOM_Y); count += 1)
    {
      XCopyArea(display.display, display.copy, display.window, GCN(GC_COPY),
	  BORDER_LEFT + BOARD_WIDTH / 2 - ZOOM_X - count * ZOOM_X,
	  BORDER_TOP + BOARD_HEIGHT / 2 - ZOOM_Y- count * ZOOM_Y,
	  ZOOM_X, count * 2 * ZOOM_Y + 2 * ZOOM_Y,
	  BORDER_LEFT + BOARD_WIDTH / 2 - ZOOM_X - count * ZOOM_X,
	  BORDER_TOP + BOARD_HEIGHT / 2 - ZOOM_Y - count * ZOOM_Y);
      XCopyArea(display.display, display.copy, display.window, GCN(GC_COPY),
	  BORDER_LEFT + BOARD_WIDTH / 2 + count * ZOOM_X,
	  BORDER_TOP + BOARD_HEIGHT / 2 - ZOOM_Y - count * ZOOM_Y,
	  ZOOM_X, count * 2 * ZOOM_Y + 2 * ZOOM_Y,
	  BORDER_LEFT + BOARD_WIDTH / 2 + count * ZOOM_X,
	  BORDER_TOP + BOARD_HEIGHT / 2 - ZOOM_Y - count * ZOOM_Y);
      XCopyArea(display.display, display.copy, display.window, GCN(GC_COPY),
	  BORDER_LEFT + BOARD_WIDTH / 2 - count * ZOOM_X,
	  BORDER_TOP + BOARD_HEIGHT / 2 - ZOOM_Y - count * ZOOM_Y,
	  count * 2 * ZOOM_X, ZOOM_Y,
	  BORDER_LEFT + BOARD_WIDTH / 2 - count * ZOOM_X,
	  BORDER_TOP + BOARD_HEIGHT / 2 - ZOOM_Y - count * ZOOM_Y);
      XCopyArea(display.display, display.copy, display.window, GCN(GC_COPY),
	  BORDER_LEFT + BOARD_WIDTH / 2 - count * ZOOM_X,
	  BORDER_TOP + BOARD_HEIGHT / 2 + count * ZOOM_Y,
	  count * 2 * ZOOM_X, ZOOM_Y,
	  BORDER_LEFT + BOARD_WIDTH / 2 - count * ZOOM_X,
	  BORDER_TOP + BOARD_HEIGHT / 2 + count * ZOOM_Y);
      XSync(display.display, False);
      timer_wait();
    }
  timer_stop();
  return;
}
/*}}}*/
