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
static void munch_back PROTOARGLIST((int, int, int, int, int, int, SPRITE *));
/*}}}*/
/*{{{  unsigned choose_direction(valid)*/
extern unsigned choose_direction FUNCARGLIST((valid))
unsigned  valid FUNCARGTERM
/* picks a direction at random from the valid ones */
{
  unsigned  choices;
  unsigned  temp;
  unsigned  choice;
  
  assert(valid && !(valid & ~0xF));
  for(choices = 0, temp = valid; temp; choices++)
    temp ^= temp & -temp;
  if(choices == 1)
    choice = 0;
  else if(choices == 3)
    choice = random() % 3;
  else
    choice = random() & (choices - 1);
  do
    {
      temp = valid & -valid;
      valid ^= temp;
    }
  while(choice--);
  assert(temp);
  for(valid = 0; !(temp & 1); valid++)
    temp >>= 1;
  return valid;
}
/*}}}*/
/*{{{  CELL *drop_apple(aptr, cptr)*/
extern CELL *drop_apple FUNCARGLIST((aptr, cptr))
APPLE   *aptr FUNCARGSEP
CELL    *cptr FUNCARGTERM
/*
 * deals with apples which break through to the cell below
 * the apple has already been moved to the new coordinate
 * returns NULL if we stay in the same cell, or a pointer
 * to the new cell
 */
{
  CELL      *new;
  COORD     pixel;

  update.set = 0;
  pixel.x = aptr->pixel.x - aptr->offset.x;
  pixel.y = aptr->pixel.y - aptr->offset.y;
  if(aptr->offset.y <= cptr[0].depths[1])
    new = NULL;
  else if(cptr[CELL_STRIDE].visit)
    /*{{{  break through below*/
    {
      /*{{{  munch*/
      {
	SPRITE      *sptr;
		      
	sptr = &sprites[SPRITE_EDGE_BASE + 1];
	/*{{{  munch the left half of the edge below*/
	{
	  unsigned  type;
			
	  type = 0;
	  if(cptr[0].depths[2] < -VEL_X)
	    type = GAP_HEIGHT;
	  if(cptr[CELL_STRIDE].depths[2] < -VEL_X)
	    type += 2 * GAP_HEIGHT;
	  if(type == 3 * GAP_HEIGHT &&
	      (cptr[-1].depths[1] > VEL_Y ||
	      cptr[CELL_STRIDE-1].depths[0] < -VEL_Y))
	    type = 4 * GAP_HEIGHT;
	  munch_back(0, type,
	      EDGE_WIDTH >> 1, GAP_HEIGHT,
	      pixel.x + (CELL_WIDTH >> 1) - (EDGE_WIDTH >> 1),
	      pixel.y + CELL_HEIGHT, sptr);
	  if(!cptr[0].depths[2])
	    munch_back(0, 0, EDGE_WIDTH >> 1, GAP_HEIGHT,
		pixel.x + (CELL_WIDTH >> 1) - (EDGE_WIDTH >> 1),
		pixel.y + CELL_HEIGHT - GAP_WIDTH, sptr);
	  if(!cptr[CELL_STRIDE].depths[2])
	    munch_back(0, 0, EDGE_WIDTH >> 1, GAP_HEIGHT,
		pixel.x + (CELL_WIDTH >> 1) - (EDGE_WIDTH >> 1),
		pixel.y + CELL_HEIGHT + GAP_HEIGHT, sptr);
	}
	/*}}}*/
	/*{{{  munch the right half of the edge below*/
	{
	  unsigned  type;
			
	  type = 0;
	  if(cptr[0].depths[3] > VEL_X)
	    type = GAP_HEIGHT;
	  if(cptr[CELL_STRIDE].depths[3] > VEL_X)
	    type += 2 * GAP_HEIGHT;
	  if(type == 3 * GAP_HEIGHT &&
	      (cptr[1].depths[1] > VEL_Y ||
	      cptr[CELL_STRIDE+1].depths[0] < -VEL_Y))
	    type = 4 * GAP_HEIGHT;
	  munch_back(EDGE_WIDTH >> 1, type,
	      EDGE_WIDTH >> 1, GAP_HEIGHT,
	      pixel.x + (CELL_WIDTH >> 1),
	      pixel.y + CELL_HEIGHT, sptr);
	  if(!cptr[0].depths[3])
	    munch_back(EDGE_WIDTH >> 1, 0,
		EDGE_WIDTH >> 1, GAP_HEIGHT,
		pixel.x + (CELL_WIDTH >> 1),
		pixel.y + CELL_HEIGHT - GAP_WIDTH, sptr);
	  if(!cptr[CELL_STRIDE].depths[3])
	    munch_back(EDGE_WIDTH >> 1, 0,
		EDGE_WIDTH >> 1, GAP_HEIGHT,
		pixel.x + (CELL_WIDTH >> 1),
		pixel.y + CELL_HEIGHT + GAP_HEIGHT, sptr);
	}
	/*}}}*/
      }
      /*}}}*/
      aptr->offset.y -= CELL_HEIGHT + GAP_HEIGHT;
      aptr->cell.y += 1;
      pixel.y += CELL_HEIGHT + GAP_HEIGHT;
      global.broken = 1;
      cptr[0].depths[1] = CELL_HEIGHT + GAP_HEIGHT;
      new = cptr + CELL_STRIDE;
      cptr[CELL_STRIDE].depths[0] = -(CELL_HEIGHT + GAP_HEIGHT);
    }
    /*}}}*/
  else if(aptr->offset.y - cptr[CELL_STRIDE * 2].depths[0] >=
      CELL_HEIGHT + GAP_HEIGHT)
    /*{{{  breakthrough 2 below*/
    {
      aptr->offset.y -= CELL_HEIGHT + GAP_HEIGHT;
      aptr->cell.y += 1;
      pixel.y += CELL_HEIGHT + GAP_HEIGHT;
      global.broken = 1;
      cptr[CELL_STRIDE].visit = 1;
      cptr[0].depths[1] = CELL_HEIGHT + GAP_HEIGHT;
      cptr[CELL_STRIDE].depths[0] = -(CELL_HEIGHT + GAP_HEIGHT);
      cptr[CELL_STRIDE].depths[1] = CELL_HEIGHT + GAP_HEIGHT;
      cptr[CELL_STRIDE * 2].depths[0] = -(CELL_HEIGHT + GAP_HEIGHT);
      if(cptr[CELL_STRIDE - 1].depths[3])
	{
	  cptr[CELL_STRIDE].depths[2] = -(CELL_WIDTH + GAP_WIDTH);
	  cptr[CELL_STRIDE-1].depths[3] = CELL_WIDTH + GAP_WIDTH;
	}
      if(cptr[CELL_STRIDE+1].depths[2])
	{
	  cptr[CELL_STRIDE].depths[3] = CELL_WIDTH + GAP_WIDTH;
	  cptr[CELL_STRIDE+1].depths[2] = -(CELL_WIDTH + GAP_WIDTH);
	}
      munch_hole(cptr + CELL_STRIDE, pixel.x, pixel.y);
      new = cptr + CELL_STRIDE;
    }
    /*}}}*/
  else
    new = NULL;
  /*{{{  redraw prize?*/
  if(new && new->sprite)
    {
      SPRITE    *sptr;
	
      sptr = &sprites[new->sprite];
      if(display.background != COLOUR_ZERO)
	XCopyArea(display.display, sptr->mask, display.back, GCN(GC_MASK),
	    0, 0, CELL_WIDTH, CELL_HEIGHT,
	    pixel.x, pixel.y);
      XCopyArea(display.display, sptr->image, display.back, GCN(GC_OR),
	  0, 0, CELL_WIDTH, CELL_HEIGHT,
	  pixel.x, pixel.y);
    }
  /*}}}*/
  if(update.set)
    add_background(update.tl.x, update.tl.y,
	update.br.x - update.tl.x, update.br.y - update.tl.y);
  return new;
}
/*}}}*/
/*{{{  CELL *move_movable(mptr, cptr)*/
extern CELL *move_movable FUNCARGLIST((mptr, cptr))
MONSTER   *mptr FUNCARGSEP
CELL      *cptr FUNCARGTERM
{
  unsigned  delta;
  
  switch(mptr->dir)
  {
    /*{{{  case 0: (up)*/
    case 0:
      if(!mptr->fast)
	delta = VEL_Y;
      else if(mptr->offset.y > (CELL_HEIGHT + GAP_HEIGHT) - VEL_Y_FAST * FAST_STEPS)
	delta = VEL_Y_FAST;
      else if(mptr->offset.y > GAP_HEIGHT + VEL_Y_FAST * FAST_STEPS)
	delta = VEL_Y;
      else if(mptr->offset.y > GAP_HEIGHT)
	delta = VEL_Y_FAST;
      else if(mptr->offset.y > 0)
	delta = VEL_Y;
      else if(mptr->offset.y > - VEL_Y_FAST * FAST_STEPS)
	delta = VEL_Y_FAST;
      else if(mptr->offset.y > - CELL_HEIGHT + VEL_Y_FAST * FAST_STEPS)
	delta = VEL_Y;
      else if(mptr->offset.y > - CELL_HEIGHT)
	delta = VEL_Y_FAST;
      else
	delta = VEL_Y;
      mptr->pixel.y -= delta;
      mptr->offset.y -= delta;
      if(mptr->offset.y == -(CELL_HEIGHT + GAP_HEIGHT))
	{
	  mptr->offset.y = 0;
	  mptr->cell.y -= 1;
	  cptr -= CELL_STRIDE;
	}
      assert(cptr->visit && !mptr->offset.x && mptr->cell.y >= 0);
      break;
    /*}}}*/
    /*{{{  case 1: (down)*/
    case 1:
      if(!mptr->fast)
	delta = VEL_Y;
      else if(mptr->offset.y < -(CELL_HEIGHT + GAP_HEIGHT) + VEL_Y_FAST * FAST_STEPS)
	delta = VEL_Y_FAST;
      else if(mptr->offset.y < - GAP_HEIGHT - VEL_Y_FAST * FAST_STEPS)
	delta = VEL_Y;
      else if(mptr->offset.y < -GAP_HEIGHT)
	delta = VEL_Y_FAST;
      else if(mptr->offset.y < 0)
	delta = VEL_Y;
      else if(mptr->offset.y < VEL_Y_FAST * FAST_STEPS)
	delta = VEL_Y_FAST;
      else if(mptr->offset.y < CELL_HEIGHT - VEL_Y_FAST * FAST_STEPS)
	delta = VEL_Y;
      else if(mptr->offset.y < CELL_HEIGHT)
	delta = VEL_Y_FAST;
      else
	delta = VEL_Y;
      mptr->pixel.y += delta;
      mptr->offset.y += delta;
      if(mptr->offset.y == CELL_HEIGHT + GAP_HEIGHT)
	{
	  mptr->offset.y = 0;
	  mptr->cell.y += 1;
	  cptr += CELL_STRIDE;
	}
      assert(cptr->visit && mptr->cell.y < CELLS_DOWN &&
	  (!mptr->offset.x || (!mptr->cell.y && mptr->offset.y <= 0)));
      break;
    /*}}}*/
    /*{{{  case 2: (left)*/
    case 2:
      if(!mptr->fast)
	delta = VEL_X;
      else if(mptr->offset.x > (CELL_WIDTH + GAP_WIDTH) - VEL_X_FAST * FAST_STEPS)
	delta = VEL_X_FAST;
      else if(mptr->offset.x > GAP_WIDTH + VEL_X_FAST * FAST_STEPS)
	delta = VEL_X;
      else if(mptr->offset.x > GAP_WIDTH)
	delta = VEL_X_FAST;
      else if(mptr->offset.x > 0)
	delta = VEL_X;
      else if(mptr->offset.x > - VEL_X_FAST * FAST_STEPS)
	delta = VEL_X_FAST;
      else if(mptr->offset.x > - CELL_WIDTH + VEL_X_FAST * FAST_STEPS)
	delta = VEL_X;
      else if(mptr->offset.x > -CELL_WIDTH)
	delta = VEL_X_FAST;
      else
	delta = VEL_X;
      mptr->pixel.x -= delta;
      mptr->offset.x -= delta;
      if(mptr->offset.x == -(CELL_WIDTH + GAP_WIDTH))
	{
	  mptr->offset.x = 0;
	  mptr->cell.x -= 1;
	  cptr -= 1;
	}
      assert(cptr->visit && !mptr->offset.y && mptr->cell.x >= 0);
      break;
    /*}}}*/
    /*{{{  case 3: (right)*/
    case 3:
      if(!mptr->fast)
	delta = VEL_X;
      else if(mptr->offset.x < -(CELL_WIDTH + GAP_WIDTH) + VEL_X_FAST * FAST_STEPS)
	delta = VEL_X_FAST;
      else if(mptr->offset.x < - GAP_WIDTH - VEL_X_FAST * FAST_STEPS)
	delta = VEL_X;
      else if(mptr->offset.x < -GAP_WIDTH)
	delta = VEL_X_FAST;
      else if(mptr->offset.x < 0)
	delta = VEL_X;
      else if(mptr->offset.x < VEL_X_FAST * FAST_STEPS)
	delta = VEL_X_FAST;
      else if(mptr->offset.x < CELL_WIDTH - VEL_X_FAST * FAST_STEPS)
	delta = VEL_X;
      else if(mptr->offset.x < CELL_WIDTH)
	delta = VEL_X_FAST;
      else
	delta = VEL_X;
      mptr->pixel.x += delta;
      mptr->offset.x += delta;
      if(mptr->offset.x == CELL_WIDTH + GAP_WIDTH)
	{
	  mptr->offset.x = 0;
	  mptr->cell.x += 1;
	  cptr += 1;
	}
      assert(cptr->visit && !mptr->offset.y && mptr->cell.x < CELLS_ACROSS);
      break;
    /*}}}*/
  }
  return cptr;
}
/*}}}*/
/*{{{  CELL *move_muncher(mptr)*/
extern CELL *move_muncher FUNCARGLIST((mptr))
MONSTER   *mptr FUNCARGTERM /* the object to move */
/*
 * moves and munches the board for an object which can munch
 * apple checking is performed here too
 * (ie the man, or a munch monster)
 * the board array is updated as required
 * returns a pointer to the new cell, if we have arrived elsewhere
 * or NULL if we stayed on the same cell
 */
{
  unsigned  broke;
  CELL      *nptr;
  CELL      *cherry;
  CELL      *cptr;
  COORD     pixel;
  COORD     cell;
  SPRITE    *sptr;
  int       knocked;

  assert(!mptr->stop && !mptr->pause);
  broke = 0;
  nptr = NULL;
  cherry = NULL;
  update.set = 0;
  cell.x = mptr->cell.x;
  cell.y = mptr->cell.y;
  cptr = BOARDCELL(cell.x, cell.y);
  pixel.x = PIXELX(cell.x, 0);
  pixel.y = PIXELY(cell.y, 0);
  knocked = 0;
  if(!apple_stop(mptr, cptr))
    {
      switch(mptr->dir)
      {
	/*{{{  case 0: (up)*/
	case 0:
	  /*
	   * if the depth upwards is less than the future depth,
	   * then we have to do some munching
	   */
	  mptr->offset.y -= VEL_Y;
	  mptr->pixel.y = pixel.y + mptr->offset.y;
	  if(cptr[0].depths[0] > mptr->offset.y)
	    /*{{{  munch*/
	    {
	      cptr[0].depths[0] = mptr->offset.y;
	      sptr = &sprites[SPRITE_MUNCH_BASE + 0];
	      munch_back(0, 0, CELL_WIDTH, MUNCH_HEIGHT >> 1,
		  pixel.x, pixel.y + cptr->depths[0], sptr);
	      if(mptr->offset.y == -VEL_Y && cptr[-CELL_STRIDE].visit)
		{
		  sptr = &sprites[SPRITE_EDGE_BASE + 1];
		  /*{{{  munch the left half of the edge above*/
		  {
		    unsigned  type;
						    
		    type = 0;
		    if(cptr[0].depths[2] < -VEL_X)
		      type = 2 * GAP_HEIGHT;
		    if(cptr[-CELL_STRIDE].depths[2] < -VEL_X)
		      type += GAP_HEIGHT;
		    if(type == 3 * GAP_HEIGHT &&
			(cptr[-1].depths[0] < -VEL_Y ||
			cptr[-CELL_STRIDE-1].depths[1] > VEL_Y))
		      type = 4 * GAP_HEIGHT;
		    munch_back(0, type,
			EDGE_WIDTH >> 1, GAP_HEIGHT,
			pixel.x + (CELL_WIDTH >> 1) - (EDGE_WIDTH >> 1),
			pixel.y - GAP_HEIGHT, sptr);
		    if(!cptr[-CELL_STRIDE].depths[2])
		      munch_back(0, 0, EDGE_WIDTH >> 1, GAP_HEIGHT,
			  pixel.x + (CELL_WIDTH >> 1) - (EDGE_WIDTH >> 1),
			  pixel.y - GAP_HEIGHT * 2, sptr);
		  }
		  /*}}}*/
		  /*{{{  munch the right half of the edge above*/
		  {
		    unsigned  type;
						    
		    type = 0;
		    if(cptr[0].depths[3] > VEL_X)
		      type = 2 * GAP_HEIGHT;
		    if(cptr[-CELL_STRIDE].depths[3] > VEL_X)
		      type += GAP_HEIGHT;
		    if(type == 3 * GAP_HEIGHT &&
			(cptr[1].depths[0] < -VEL_Y ||
			cptr[-CELL_STRIDE+1].depths[1] > VEL_Y))
		      type = 4 * GAP_HEIGHT;
		    munch_back(EDGE_WIDTH >> 1, type,
			EDGE_WIDTH >> 1, GAP_HEIGHT,
			pixel.x + (CELL_WIDTH >> 1),
			pixel.y - GAP_HEIGHT, sptr);
		    if(!cptr[-CELL_STRIDE].depths[3])
		      munch_back(EDGE_WIDTH >> 1, 0,
			  EDGE_WIDTH >> 1, GAP_HEIGHT,
			  pixel.x + (CELL_WIDTH >> 1),
			  pixel.y - GAP_HEIGHT * 2, sptr);
		  }
		  /*}}}*/
		  cptr[0].depths[0] = -(CELL_HEIGHT + GAP_HEIGHT);
		  cptr[-CELL_STRIDE].depths[1] = CELL_HEIGHT + GAP_HEIGHT;
		  broke = 1;
		}
	      else
		{
		  if(mptr->offset.y == -(VEL_Y * 2))
		    {
		      sptr = &sprites[SPRITE_EDGE_BASE + 1];
		      /*{{{  round top left corner?*/
		      if(cptr[0].depths[2] < -VEL_X)
			munch_back(0, 2 * GAP_HEIGHT,
			    EDGE_WIDTH >> 1, GAP_HEIGHT,
			    pixel.x + (CELL_WIDTH >> 1) - (EDGE_WIDTH >> 1),
			    pixel.y - GAP_HEIGHT, sptr);
		      /*}}}*/
		      /*{{{  round top right corner?*/
		      if(cptr[0].depths[3] > VEL_X)
			munch_back(EDGE_WIDTH >> 1, 2 * GAP_HEIGHT,
			    EDGE_WIDTH >> 1, GAP_HEIGHT,
			    pixel.x + (CELL_WIDTH >> 1),
			    pixel.y - GAP_HEIGHT, sptr);
		      /*}}}*/
		    }
		  /*{{{  knocked through?*/
		  /*
		   * have we bumped into any of the following ?
		   * path from 2 above me
		   * path from above left
		   * path from above right
		   */
		  if((cptr[-CELL_STRIDE*2].depths[1] - cptr[0].depths[0] >=
		      CELL_HEIGHT + GAP_HEIGHT * 2) ||
		      (cptr[-CELL_STRIDE-1].depths[3] &&
			cptr[-CELL_STRIDE-1].depths[3] - cptr[0].depths[0] >=
			KNOCK_THROUGH) ||
		      (cptr[-CELL_STRIDE+1].depths[2] &&
			cptr[-CELL_STRIDE+1].depths[2] + cptr[0].depths[0] <=
			-KNOCK_THROUGH))
		    {
		      knocked = -CELL_STRIDE;
		      pixel.y -= CELL_HEIGHT + GAP_HEIGHT;
		      cell.y -= 1;
		    }
		  else
		    {
		      if(cptr->depths[0] == -(CELL_HEIGHT + GAP_HEIGHT))
			{
			  cptr[-CELL_STRIDE].visit = 1;
			  cptr[-CELL_STRIDE].depths[1] =
			      CELL_HEIGHT + GAP_HEIGHT;
			}
		      cherry = cptr - CELL_STRIDE;
		      pixel.y -= CELL_HEIGHT + GAP_HEIGHT;
		    }
		  /*}}}*/
		}
	    }
	    /*}}}*/
	  else if(mptr->offset.y < -VEL_Y)
	    {
	      cherry = cptr - CELL_STRIDE;
	      pixel.y -= CELL_HEIGHT + GAP_HEIGHT;
	    }
	  if(mptr->offset.y == -(CELL_HEIGHT + GAP_HEIGHT))
	    {
	      mptr->offset.y = 0;
	      mptr->cell.y--;
	      nptr = cptr - CELL_STRIDE;
	    }
	  break;
	/*}}}*/
	/*{{{  case 1: (down)*/
	case 1:
	{
	  mptr->offset.y += VEL_Y;
	  mptr->pixel.y = pixel.y + mptr->offset.y;
	  if(cptr->depths[1] < mptr->offset.y)
	    /*{{{  munch*/
	    {
	      cptr->depths[1] = mptr->offset.y;
	      sptr = &sprites[SPRITE_MUNCH_BASE + 0];
	      munch_back(0, MUNCH_HEIGHT >> 1,
		  CELL_WIDTH, MUNCH_HEIGHT >> 1,
		  pixel.x, pixel.y + cptr[0].depths[1] +
		  CELL_HEIGHT - (MUNCH_HEIGHT >> 1), sptr);
	      if(mptr->offset.y == VEL_Y && cptr[CELL_STRIDE].visit)
		{
		  sptr = &sprites[SPRITE_EDGE_BASE + 1];
		  /*{{{  munch the left half of the edge below*/
		  {
		    unsigned  type;
						    
		    type = 0;
		    if(cptr[0].depths[2] < -VEL_X)
		      type = GAP_HEIGHT;
		    if(cptr[CELL_STRIDE].depths[2] < -VEL_X)
		      type += 2 * GAP_HEIGHT;
		    if(type == 3 * GAP_HEIGHT &&
			(cptr[-1].depths[1] > VEL_Y ||
			cptr[CELL_STRIDE-1].depths[0] < -VEL_Y))
		      type = 4 * GAP_HEIGHT;
		    munch_back(0, type,
			EDGE_WIDTH >> 1, GAP_HEIGHT,
			pixel.x + (CELL_WIDTH >> 1) - (EDGE_WIDTH >> 1),
			pixel.y + CELL_HEIGHT, sptr);
		    if(!cptr[CELL_STRIDE].depths[2])
		      munch_back(0, 0, EDGE_WIDTH >> 1, GAP_HEIGHT,
			  pixel.x + (CELL_WIDTH >> 1) - (EDGE_WIDTH >> 1),
			  pixel.y + CELL_HEIGHT + GAP_HEIGHT, sptr);
		  }
		  /*}}}*/
		  /*{{{  munch the right half of the edge below*/
		  {
		    unsigned  type;
						    
		    type = 0;
		    if(cptr[0].depths[3] > VEL_X)
		      type = GAP_HEIGHT;
		    if(cptr[CELL_STRIDE].depths[3] > VEL_X)
		      type += 2 * GAP_HEIGHT;
		    if(type == 3 * GAP_HEIGHT &&
			(cptr[1].depths[1] > VEL_Y ||
			cptr[CELL_STRIDE+1].depths[0] < -VEL_Y))
		      type = 4 * GAP_HEIGHT;
		    munch_back(EDGE_WIDTH >> 1, type,
			EDGE_WIDTH >> 1, GAP_HEIGHT,
			pixel.x + (CELL_WIDTH >> 1),
			pixel.y + CELL_HEIGHT, sptr);
		    if(!cptr[CELL_STRIDE].depths[3])
		      munch_back(EDGE_WIDTH >> 1, 0,
			  EDGE_WIDTH >> 1, GAP_HEIGHT,
			  pixel.x + (CELL_WIDTH >> 1),
			  pixel.y + CELL_HEIGHT + GAP_HEIGHT, sptr);
		  }
		  /*}}}*/
		  cptr[0].depths[1] = CELL_HEIGHT + GAP_HEIGHT;
		  cptr[CELL_STRIDE].depths[0] = -(CELL_HEIGHT + GAP_HEIGHT);
		  broke = 1;
		}
	      else
		{
		  if(mptr->offset.y == (VEL_Y * 2))
		    {
		      sptr = &sprites[SPRITE_EDGE_BASE + 1];
		      /*{{{  round bottom left corner?*/
		      if(cptr[0].depths[2] < -VEL_X)
			munch_back(0, GAP_HEIGHT,
			    EDGE_WIDTH >> 1, GAP_HEIGHT,
			    pixel.x + (CELL_WIDTH >> 1) - (EDGE_WIDTH >> 1),
			    pixel.y + CELL_HEIGHT, sptr);
		      /*}}}*/
		      /*{{{  round bottom right corner?*/
		      if(cptr[0].depths[3] > VEL_X)
			munch_back(EDGE_WIDTH >> 1, GAP_HEIGHT,
			    EDGE_WIDTH >> 1, GAP_HEIGHT,
			    pixel.x + (CELL_WIDTH >> 1),
			    pixel.y + CELL_HEIGHT, sptr);
		      /*}}}*/
		    }
		  /*{{{  knocked through?*/
		  /*
		   * have we bumped into any of the following ?
		   * path from 2 below me
		   * path from below left
		   * path from below right
		   */
		  if((cptr[0].depths[1] - cptr[CELL_STRIDE*2].depths[0] >=
		      CELL_HEIGHT + GAP_HEIGHT * 2) ||
		      (cptr[CELL_STRIDE-1].depths[3] &&
			cptr[CELL_STRIDE-1].depths[3] + cptr[0].depths[1] >=
			KNOCK_THROUGH) ||
		      (cptr[CELL_STRIDE+1].depths[2] &&
			cptr[0].depths[1] - cptr[CELL_STRIDE+1].depths[2] >=
			KNOCK_THROUGH))
		    {
		      knocked = CELL_STRIDE;
		      pixel.y += CELL_HEIGHT + GAP_HEIGHT;
		      cell.y += 1;
		    }
		  else
		    {
		      if(cptr->depths[1] == (CELL_HEIGHT + GAP_HEIGHT))
			{
			  cptr[CELL_STRIDE].visit = 1;
			  cptr[CELL_STRIDE].depths[0] =
			      -(CELL_HEIGHT + GAP_HEIGHT);
			}
		      cherry = cptr + CELL_STRIDE;
		      pixel.y += CELL_HEIGHT + GAP_HEIGHT;
		    }
		  /*}}}*/
		}
	    }
	    /*}}}*/
	  else if(mptr->offset.y > VEL_Y)
	    {
	      cherry = cptr + CELL_STRIDE;
	      pixel.y += CELL_HEIGHT + GAP_HEIGHT;
	    }
	  if(mptr->offset.y == (CELL_HEIGHT + GAP_HEIGHT))
	    {
	      mptr->offset.y = 0;
	      mptr->cell.y++;
	      nptr = cptr + CELL_STRIDE;
	    }
	  break;
	}
	/*}}}*/
	/*{{{  case 2: (left)*/
	case 2:
	{
	  mptr->offset.x -= VEL_X;
	  mptr->pixel.x = pixel.x + mptr->offset.x;
	  if(cptr[0].depths[2] > mptr->offset.x)
	    /*{{{  munch*/
	    {
	      cptr[0].depths[2] = mptr->offset.x;
	      sptr = &sprites[SPRITE_MUNCH_BASE + 1];
	      munch_back(0, 0, MUNCH_WIDTH >> 1, CELL_HEIGHT,
		  pixel.x + cptr[0].depths[2], pixel.y, sptr);
	      if(mptr->offset.x == -VEL_X && cptr[-1].visit)
		{
		  sptr = &sprites[SPRITE_EDGE_BASE + 0];
		  /*{{{  munch the top half of the edge left*/
		  {
		    unsigned  type;
						    
		    type = 0;
		    if(cptr[0].depths[0] < -VEL_Y)
		      type = 2 * GAP_WIDTH;
		    if(cptr[-1].depths[0] < -VEL_Y)
		      type += GAP_WIDTH;
		    if(type == 3 * GAP_WIDTH &&
			(cptr[-CELL_STRIDE].depths[2] < -VEL_X ||
			cptr[-CELL_STRIDE-1].depths[3] > VEL_X))
		      type = 4 * GAP_WIDTH;
		    munch_back(type, 0,
			GAP_HEIGHT, EDGE_HEIGHT >> 1,
			pixel.x - GAP_WIDTH,
			pixel.y + (CELL_HEIGHT >> 1) - (EDGE_HEIGHT >> 1),
			sptr);
		    if(!cptr[-1].depths[0])
		      munch_back(0, 0, GAP_WIDTH, EDGE_HEIGHT >> 1,
			  pixel.x - GAP_WIDTH * 2,
			  pixel.y + (CELL_HEIGHT >> 1) - (EDGE_HEIGHT >> 1),
			  sptr);
		  }
		  /*}}}*/
		  /*{{{  munch the bottom half of the edge left*/
		  {
		    unsigned  type;
						    
		    type = 0;
		    if(cptr[0].depths[1] > VEL_Y)
		      type = 2 * GAP_WIDTH;
		    if(cptr[-1].depths[1] > VEL_Y)
		      type += GAP_WIDTH;
		    if(type == 3 * GAP_WIDTH &&
			(cptr[CELL_STRIDE].depths[2] < -VEL_X ||
			cptr[CELL_STRIDE-1].depths[3] > VEL_X))
		      type = 4 * GAP_WIDTH;
		    munch_back(type, EDGE_HEIGHT >> 1,
			GAP_WIDTH, EDGE_HEIGHT >> 1,
			pixel.x - GAP_WIDTH,
			pixel.y + (CELL_HEIGHT >> 1), sptr);
		    if(!cptr[-1].depths[1])
		      munch_back(0, EDGE_HEIGHT >> 1,
			  GAP_WIDTH, EDGE_HEIGHT >> 1,
			  pixel.x - GAP_WIDTH * 2,
			  pixel.y + (CELL_HEIGHT >> 1), sptr);
		  }
		  /*}}}*/
		  cptr[0].depths[2] = -(CELL_WIDTH + GAP_WIDTH);
		  cptr[-1].depths[3] = CELL_WIDTH + GAP_WIDTH;
		  broke = 1;
		}
	      else
		{
		  if(mptr->offset.x == -(VEL_X * 2))
		    {
		      sptr = &sprites[SPRITE_EDGE_BASE + 0];
		      /*{{{  round left top corner?*/
		      if(cptr[0].depths[0] < -VEL_Y)
			munch_back(2 * GAP_WIDTH, 0,
			    GAP_WIDTH, EDGE_HEIGHT >> 1,
			    pixel.x - GAP_WIDTH,
			    pixel.y + (CELL_HEIGHT >> 1) - (EDGE_HEIGHT >> 1),
			    sptr);
		      /*}}}*/
		      /*{{{  round left bottom corner?*/
		      if(cptr[0].depths[1] > VEL_Y)
			munch_back(2 * GAP_HEIGHT, EDGE_HEIGHT >> 1,
			    GAP_WIDTH, EDGE_HEIGHT >> 1,
			    pixel.x - GAP_WIDTH,
			    pixel.y + (CELL_HEIGHT >> 1), sptr);
		      /*}}}*/
		    }
		  /*{{{  knocked through?*/
		  /*
		   * have we bumped into any of the following ?
		   * path from 2 left me
		   * path from left above
		   * path from left below
		   */
		  if((cptr[-2].depths[3] - cptr[0].depths[2] >=
		      CELL_WIDTH + GAP_WIDTH * 2) ||
		      (cptr[-CELL_STRIDE-1].depths[1] &&
			cptr[-CELL_STRIDE-1].depths[1] - cptr[0].depths[2] >=
			KNOCK_THROUGH) ||
		      (cptr[CELL_STRIDE-1].depths[0] &&
			cptr[CELL_STRIDE-1].depths[0] + cptr[0].depths[2] <=
			-KNOCK_THROUGH))
		    {
		      knocked = -1;
		      pixel.x -= CELL_WIDTH + GAP_WIDTH;
		      cell.x -= 1;
		    }
		  else
		    {
		      if(cptr->depths[2] == -(CELL_WIDTH + GAP_WIDTH))
			{
			  cptr[-1].visit = 1;
			  cptr[-1].depths[3] = CELL_WIDTH + GAP_WIDTH;
			}
		      cherry = cptr - 1;
		      pixel.x -= CELL_WIDTH + GAP_WIDTH;
		    }
		  /*}}}*/
		}
	    }
	    /*}}}*/
	  else if(mptr->offset.x < -VEL_X)
	    {
	      cherry = cptr - 1;
	      pixel.x -= CELL_WIDTH + GAP_WIDTH;
	    }
	  if(mptr->offset.x == -(CELL_WIDTH + GAP_WIDTH))
	    {
	      mptr->offset.x = 0;
	      mptr->cell.x--;
	      nptr = cptr - 1;
	    }
	  break;
	}
	/*}}}*/
	/*{{{  case 3: (right)*/
	case 3:
	{
	  mptr->offset.x += VEL_X;
	  mptr->pixel.x = pixel.x + mptr->offset.x;
	  if(cptr->depths[3] < mptr->offset.x)
	    /*{{{  munch*/
	    {
	      cptr->depths[3] = mptr->offset.x;
	      sptr = &sprites[SPRITE_MUNCH_BASE + 1];
	      munch_back(MUNCH_WIDTH >> 1, 0,
		  MUNCH_WIDTH >> 1, CELL_HEIGHT,
		  pixel.x + cptr->depths[3] +
		  CELL_HEIGHT - (MUNCH_WIDTH >> 1), pixel.y, sptr);
	      if(mptr->offset.x == VEL_X && cptr[1].visit)
		{
		  sptr = &sprites[SPRITE_EDGE_BASE + 0];
		  /*{{{  munch the top half of the edge right*/
		  {
		    unsigned  type;
						    
		    type = 0;
		    if(cptr[0].depths[0] < -VEL_Y)
		      type = GAP_WIDTH;
		    if(cptr[1].depths[0] < -VEL_Y)
		      type += 2 * GAP_WIDTH;
		    if(type == 3 * GAP_WIDTH &&
		      (cptr[-CELL_STRIDE].depths[3] > VEL_X ||
			cptr[-CELL_STRIDE+1].depths[2] < -VEL_X))
		      type = 4 * GAP_WIDTH;
		    munch_back(type, 0,
			GAP_WIDTH, EDGE_HEIGHT >> 1,
			pixel.x + CELL_WIDTH, pixel.y +
			(CELL_HEIGHT >> 1) - (EDGE_HEIGHT >> 1), sptr);
		    if(!cptr[1].depths[0])
		      munch_back(0, 0, GAP_WIDTH, EDGE_HEIGHT >> 1,
			  pixel.x + CELL_WIDTH + GAP_WIDTH, pixel.y +
			  (CELL_HEIGHT >> 1) - (EDGE_HEIGHT >> 1), sptr);
		  }
		  /*}}}*/
		  /*{{{  munch the bottom half of the edge right*/
		  {
		    unsigned  type;
						    
		    type = 0;
		    if(cptr[0].depths[1] > VEL_Y)
		      type = GAP_WIDTH;
		    if(cptr[1].depths[1] > VEL_Y)
		      type += 2 * GAP_WIDTH;
		    if(type == 3 * GAP_WIDTH &&
			(cptr[CELL_STRIDE].depths[3] > VEL_X ||
			cptr[CELL_STRIDE+1].depths[2] < -VEL_X))
		      type = 4 * GAP_WIDTH;
		    munch_back(type, EDGE_HEIGHT >> 1,
			GAP_WIDTH, EDGE_HEIGHT >> 1,
			pixel.x + CELL_WIDTH,
			pixel.y + (CELL_HEIGHT >> 1), sptr);
		    if(!cptr[1].depths[1])
		      munch_back(0, EDGE_HEIGHT >> 1,
			  GAP_WIDTH, EDGE_HEIGHT >> 1,
			  pixel.x + CELL_WIDTH + GAP_WIDTH,
			  pixel.y + (CELL_HEIGHT >> 1), sptr);
		  }
		  /*}}}*/
		  cptr[0].depths[3] = CELL_WIDTH + GAP_WIDTH;
		  cptr[1].depths[2] = -(CELL_WIDTH + GAP_WIDTH);
		  broke = 1;
		}
	      else
		{
		  if(mptr->offset.x == (VEL_X * 2))
		    {
		      sptr = &sprites[SPRITE_EDGE_BASE + 0];
		      /*{{{  round right top corner?*/
		      if(cptr[0].depths[0] < -VEL_Y)
			munch_back(GAP_HEIGHT, 0,
			    GAP_WIDTH, EDGE_HEIGHT >> 1,
			    pixel.x + CELL_WIDTH, pixel.y +
			    (CELL_HEIGHT >> 1) - (EDGE_HEIGHT >> 1), sptr);
		      /*}}}*/
		      /*{{{  round right bottom corner?*/
		      if(cptr[0].depths[1] > VEL_Y)
			munch_back(GAP_WIDTH, EDGE_HEIGHT >> 1,
			    GAP_WIDTH, EDGE_HEIGHT >> 1,
			    pixel.x + CELL_WIDTH, pixel.y +
			    (CELL_HEIGHT >> 1), sptr);
		      /*}}}*/
		    }
		  /*{{{  knocked through?*/
		  /*
		   * have we bumped into any of the following ?
		   * path from 2 right me
		   * path from right above
		   * path from right below
		   */
		  if((cptr[0].depths[3] - cptr[2].depths[2] >=
		      CELL_WIDTH + GAP_WIDTH * 2) ||
		      (cptr[-CELL_STRIDE+1].depths[1] &&
			cptr[-CELL_STRIDE+1].depths[1] + cptr[0].depths[3] >=
			KNOCK_THROUGH) ||
		      (cptr[CELL_STRIDE+1].depths[0] &&
			cptr[0].depths[3] - cptr[CELL_STRIDE+1].depths[0] >=
			KNOCK_THROUGH))
		    {
		      knocked = 1;
		      pixel.x += CELL_WIDTH + GAP_WIDTH;
		      cell.x += 1;
		    }
		  else
		    {
		      if(cptr->depths[3] == (CELL_WIDTH + GAP_WIDTH))
			{
			  cptr[1].visit = 1;
			  cptr[1].depths[2] = -(CELL_WIDTH + GAP_WIDTH);
			}
		      cherry = cptr + 1;
		      pixel.x += CELL_WIDTH + GAP_WIDTH;
		    }
		  /*}}}*/
		}
	    }
	    /*}}}*/
	  else if(mptr->offset.x > VEL_X)
	    {
	      cherry = cptr + 1;
	      pixel.x += CELL_WIDTH + GAP_WIDTH;
	    }
	  if(mptr->offset.x == (CELL_WIDTH + GAP_WIDTH))
	    {
	      mptr->offset.x = 0;
	      mptr->cell.x++;
	      nptr = cptr + 1;
	    }
	  break;
	}
	/*}}}*/
      }
      apple_under(mptr, nptr ? nptr : cptr);
    }
  /*{{{  knocked through?*/
  /*
   * if we knocked through to an adjoining cell
   * we clear out the specified cell and check all the corners
   * note, cell has already been altered correctly
   * we must also check to se if this launches an apple
   */
  if(knocked)
    {
      unsigned  i;
      APPLE     *aptr;
	
      broke = 1;
      cherry = cptr += knocked;
      cptr[0].visit = 1;
      /*{{{  alter the depths*/
      {
	if(cptr[-CELL_STRIDE].depths[1])
	  {
	    cptr[0].depths[0] = -(CELL_HEIGHT + GAP_HEIGHT);
	    cptr[-CELL_STRIDE].depths[1] = CELL_HEIGHT + GAP_HEIGHT;
	  }
	if(cptr[CELL_STRIDE].depths[0])
	  {
	    cptr[0].depths[1] = CELL_HEIGHT + GAP_HEIGHT;
	    cptr[CELL_STRIDE].depths[0] = -(CELL_HEIGHT + GAP_HEIGHT);
	  }
	if(cptr[-1].depths[3])
	  {
	    cptr[0].depths[2] = -(CELL_WIDTH + GAP_WIDTH);
	    cptr[-1].depths[3] = CELL_WIDTH + GAP_WIDTH;
	  }
	if(cptr[1].depths[2])
	  {
	    cptr[0].depths[3] = CELL_WIDTH + GAP_WIDTH;
	    cptr[1].depths[2] = -(CELL_WIDTH + GAP_WIDTH);
	  }
      }
      /*}}}*/
      munch_hole(cptr, pixel.x, pixel.y);
      /*{{{  now see if we launch any apples*/
      for(aptr = apple.list, i = apple.apples; i--; aptr++)
	{
	  if(aptr->state > 1 || pixel.x - aptr->pixel.x > CELL_WIDTH / 2 ||
	      pixel.x - aptr->pixel.x < -CELL_WIDTH / 2)
	    /*EMPTY*/;
	  else if(pixel.y - aptr->pixel.y > CELL_HEIGHT ||
	      pixel.y - aptr->pixel.y < 0)
	    /*EMPTY*/;
	  else
	    {
	      /*{{{  move apple cell.x?*/
	      if(aptr->cell.x < cell.x)
		{
		  aptr->cell.x += 1;
		  aptr->offset.x -= CELL_WIDTH + GAP_WIDTH;
		}
	      else if(aptr->cell.x > cell.x)
		{
		  aptr->cell.x -= 1;
		  aptr->offset.x += CELL_WIDTH + GAP_WIDTH;
		}
	      /*}}}*/
	      if(pixel.y - aptr->pixel.y == CELL_HEIGHT && !cptr[0].depths[1])
		{
		  if(!aptr->state)
		    {
		      aptr->state = 1;
		      aptr->count = APPLE_ROCK_DELAY;
		    }
		}
	      else
		{
		  aptr->state = 1;
		  aptr->count = 1;
		}
	    }
	}
      /*}}}*/
    }
  /*}}}*/
  /*{{{  redraw or blank prize?*/
  if(cherry && cherry->sprite)
    {
      SPRITE    *sptr;
      
      if(!nptr || cherry->sprite == SPRITE_DEN ||
	  cherry->sprite == SPRITE_NORMAL + 2 ||
	  (cherry->sprite != SPRITE_CHERRY && mptr->type != 4))
	{
	  sptr = &sprites[cherry->sprite];
	  if(display.background != COLOUR_ZERO)
	    XCopyArea(display.display, sptr->mask, display.back, GCN(GC_MASK),
		0, 0, CELL_WIDTH, CELL_HEIGHT, pixel.x, pixel.y);
	  XCopyArea(display.display, sptr->image, display.back, GCN(GC_OR),
	      0, 0, CELL_WIDTH, CELL_HEIGHT, pixel.x, pixel.y);
	}
      else if(!knocked || cherry->sprite != SPRITE_CHERRY)
	{
	  sptr = &sprites[SPRITE_CENTER_BASE +
	      (cherry->sprite != SPRITE_CHERRY)];
	  munch_back(0, 0, CELL_WIDTH, CELL_HEIGHT,
	    pixel.x, pixel.y, sptr);
	}
    }
  /*}}}*/
  if(update.set)
    add_background(update.tl.x, update.tl.y,
	update.br.x - update.tl.x, update.br.y - update.tl.y);
  if(broke || nptr)
    global.broken = 1;
  assert(!nptr || nptr->visit);
  assert(mptr->cell.y >= 0 && mptr->cell.y < CELLS_DOWN &&
      mptr->cell.x >= 0 && mptr->cell.x < CELLS_ACROSS &&
      (mptr->cell.x || mptr->offset.x >= 0) &&
      (mptr->cell.y || mptr->offset.y >= 0) &&
      (mptr->cell.x < CELLS_ACROSS - 1 || mptr->offset.x <= 0) &&
      (mptr->cell.y < CELLS_DOWN - 1 || mptr->offset.y <= 0) &&
      (!mptr->offset.x || !mptr->offset.y));
  return nptr;
}
/*}}}*/
/*{{{  void munch_back(sx, sy, width, height, dx, dy, sprite)*/
static void munch_back FUNCARGLIST((sx, sy, width, height, dx, dy, sprite))
int     sx      FUNCARGSEP
int     sy      FUNCARGSEP
int     width   FUNCARGSEP
int     height  FUNCARGSEP
int     dx      FUNCARGSEP
int     dy      FUNCARGSEP
SPRITE  *sprite FUNCARGTERM
/*
 * munches the background image with the specified sprite
 */
{
  if(display.background != COLOUR_ONE)
    XCopyArea(display.display, sprite->mask, display.back, GCN(GC_MASK),
	sx, sy, width, height, dx, dy);
  if(display.background != COLOUR_ZERO)
    XCopyArea(display.display, sprite->image, display.back, GCN(GC_BACK),
	sx, sy, width, height, dx, dy);
  bounding_box(dx, dy, width, height);
  return;
}
/*}}}*/
/*{{{  void munch_hole(cptr, x, y)*/
extern void munch_hole FUNCARGLIST((cptr, x, y))
CELL      *cptr FUNCARGSEP
int       x     FUNCARGSEP
int       y     FUNCARGTERM
/*
 * cut out the background for a whole cell, and
 * deal with connections to the adjoining cells
 */
{
  SPRITE    *sptr;


  /*{{{  cut out the center*/
  {
    sptr = &sprites[SPRITE_CENTER_BASE];
    munch_back(0, 0,
	CELL_WIDTH >> 1, CELL_HEIGHT >> 1,
	x, y,
	&sptr[!!(cptr[0].depths[0] || cptr[0].depths[2])]);
    munch_back(CELL_WIDTH >> 1, 0,
	CELL_WIDTH >> 1, CELL_HEIGHT >> 1,
	x + (CELL_WIDTH >> 1), y,
	&sptr[!!(cptr[0].depths[0] || cptr[0].depths[3])]);
    munch_back(CELL_WIDTH >> 1, CELL_HEIGHT >> 1,
	CELL_WIDTH >> 1, CELL_HEIGHT >> 1,
	x + (CELL_WIDTH >> 1), y + (CELL_HEIGHT >> 1),
	&sptr[!!(cptr[0].depths[1] || cptr[0].depths[3])]);
    munch_back(0, CELL_HEIGHT >> 1,
	CELL_WIDTH >> 1, CELL_HEIGHT >> 1,
	x, y + (CELL_HEIGHT >> 1),
	&sptr[!!(cptr[0].depths[1] || cptr[0].depths[2])]);
  }
  /*}}}*/
  sptr = &sprites[SPRITE_EDGE_BASE + 1];
  /*{{{  munch above?*/
  if(cptr[0].depths[0])
    {
      /*{{{  munch the left half of the edge above*/
      {
	unsigned  type;
	    
	type = 0;
	if(cptr[0].depths[2])
	  type = 2 * GAP_HEIGHT;
	if(cptr[-CELL_STRIDE].depths[2] < -VEL_X)
	  type += GAP_HEIGHT;
	if(type == 3 * GAP_HEIGHT && (cptr[-1].depths[0] < -VEL_Y ||
	    cptr[-CELL_STRIDE-1].depths[1] > VEL_Y))
	  type = 4 * GAP_HEIGHT;
	munch_back(0, type, EDGE_WIDTH >> 1, GAP_HEIGHT,
	    x + (CELL_WIDTH >> 1) - (EDGE_WIDTH >> 1), y - GAP_HEIGHT, sptr);
      }
      /*}}}*/
      /*{{{  munch the right half of the edge above*/
      {
	unsigned  type;
	    
	type = 0;
	if(cptr[0].depths[3])
	  type = 2 * GAP_HEIGHT;
	if(cptr[-CELL_STRIDE].depths[3] > VEL_X)
	  type += GAP_HEIGHT;
	if(type == 3 * GAP_HEIGHT && (cptr[1].depths[0] < -VEL_Y ||
	    cptr[-CELL_STRIDE+1].depths[1] > VEL_Y))
	  type = 4 * GAP_HEIGHT;
	munch_back(EDGE_WIDTH >> 1, type, EDGE_WIDTH >> 1, GAP_HEIGHT,
	    x + (CELL_WIDTH >> 1), y - GAP_HEIGHT, sptr);
      }
      /*}}}*/
    }
  /*}}}*/
  /*{{{  munch below?*/
  if(cptr[0].depths[1])
    {
      /*{{{  munch the left half of the edge below*/
      {
	unsigned  type;
	    
	type = 0;
	if(cptr[0].depths[2])
	  type = GAP_HEIGHT;
	if(cptr[CELL_STRIDE].depths[2] < -VEL_X)
	  type += 2 * GAP_HEIGHT;
	if(type == 3 * GAP_HEIGHT && (cptr[-1].depths[1] > VEL_Y ||
	    cptr[CELL_STRIDE-1].depths[0] < -VEL_Y))
	  type = 4 * GAP_HEIGHT;
	munch_back(0, type, EDGE_WIDTH >> 1, GAP_HEIGHT,
	    x + (CELL_WIDTH >> 1) - (EDGE_WIDTH >> 1),
	    y + CELL_HEIGHT, sptr);
      }
      /*}}}*/
      /*{{{  munch the right half of the edge below*/
      {
	unsigned  type;
	    
	type = 0;
	if(cptr[0].depths[3])
	  type = GAP_HEIGHT;
	if(cptr[CELL_STRIDE].depths[3] > VEL_X)
	  type += 2 * GAP_HEIGHT;
	if(type == 3 * GAP_HEIGHT &&
	    (cptr[1].depths[1] > VEL_Y ||
	    cptr[CELL_STRIDE+1].depths[0] < -VEL_Y))
	  type = 4 * GAP_HEIGHT;
	munch_back(EDGE_WIDTH >> 1, type, EDGE_WIDTH >> 1, GAP_HEIGHT,
	    x + (CELL_WIDTH >> 1), y + CELL_HEIGHT, sptr);
      }
      /*}}}*/
    }
  /*}}}*/
  sptr = &sprites[SPRITE_EDGE_BASE + 0];
  /*{{{  munch left?*/
  if(cptr[0].depths[2])
    {
      /*{{{  munch the top half of the edge left*/
      {
	unsigned  type;
	    
	type = 0;
	if(cptr[0].depths[0])
	  type = 2 * GAP_WIDTH;
	if(cptr[-1].depths[0] < -VEL_Y)
	  type += GAP_WIDTH;
	if(type == 3 * GAP_WIDTH &&
	    (cptr[-CELL_STRIDE].depths[2] < -VEL_X ||
	    cptr[-CELL_STRIDE-1].depths[3] > VEL_X))
	  type = 4 * GAP_WIDTH;
	munch_back(type, 0, GAP_HEIGHT, EDGE_HEIGHT >> 1,
	    x - GAP_WIDTH, y + (CELL_HEIGHT >> 1) - (EDGE_HEIGHT >> 1),
	    sptr);
      }
      /*}}}*/
      /*{{{  munch the bottom half of the edge left*/
      {
	unsigned  type;
	    
	type = 0;
	if(cptr[0].depths[1])
	  type = 2 * GAP_WIDTH;
	if(cptr[-1].depths[1] > VEL_Y)
	  type += GAP_WIDTH;
	if(type == 3 * GAP_WIDTH &&
	    (cptr[CELL_STRIDE].depths[2] < -VEL_X ||
	    cptr[CELL_STRIDE-1].depths[3] > VEL_X))
	  type = 4 * GAP_WIDTH;
	munch_back(type, EDGE_HEIGHT >> 1, GAP_WIDTH, EDGE_HEIGHT >> 1,
	    x - GAP_WIDTH, y + (CELL_HEIGHT >> 1), sptr);
      }
      /*}}}*/
    }
  /*}}}*/
  /*{{{  munch right?*/
  if(cptr[0].depths[3])
    {
      /*{{{  munch the top half of the edge right*/
      {
	unsigned  type;
	    
	type = 0;
	if(cptr[0].depths[0])
	  type = GAP_WIDTH;
	if(cptr[1].depths[0] < -VEL_Y)
	  type += 2 * GAP_WIDTH;
	if(type == 3 * GAP_WIDTH &&
	    (cptr[-CELL_STRIDE].depths[3] > VEL_X ||
	    cptr[-CELL_STRIDE+1].depths[2] < -VEL_X))
	  type = 4 * GAP_WIDTH;
	munch_back(type, 0, GAP_WIDTH, EDGE_HEIGHT >> 1,
	    x + CELL_WIDTH, y + (CELL_HEIGHT >> 1) - (EDGE_HEIGHT >> 1),
	    sptr);
      }
      /*}}}*/
      /*{{{  munch the bottom half of the edge right*/
      {
	unsigned  type;
	    
	type = 0;
	if(cptr[0].depths[1])
	  type = GAP_WIDTH;
	if(cptr[1].depths[1] > VEL_Y)
	  type += 2 * GAP_WIDTH;
	if(type == 3 * GAP_WIDTH &&
	    (cptr[CELL_STRIDE].depths[3] > VEL_X ||
	    cptr[CELL_STRIDE+1].depths[2] < -VEL_X))
	  type = 4 * GAP_WIDTH;
	munch_back(type, EDGE_HEIGHT >> 1, GAP_WIDTH, EDGE_HEIGHT >> 1,
	    x + CELL_WIDTH, y + (CELL_HEIGHT >> 1), sptr);
      }
      /*}}}*/
    }
  /*}}}*/
  return;
}
/*}}}*/
/*{{{  void new_face(mptr)*/
extern void new_face FUNCARGLIST((mptr))
MONSTER *mptr FUNCARGTERM
/*
 * calculates the required face for the direction,
 * given the old face
 */
{
  int       dir;
  int       face;

  if(mptr->push)
    dir = mptr->dir ^ 1;
  else
    dir = mptr->dir;
  face = mptr->face < 6 ? mptr->face : 2 + (mptr->face & 1);
  if(mptr->apple)
    mptr->face = 8 + (face > 2);
  else if(dir & 2)
    mptr->face = dir;
  else if(face == 2)
    mptr->face = dir;
  else if(face == 3)
    mptr->face = dir + 4;
  else if((dir ^ face) & 1)
    mptr->face ^= 5;
  if(mptr->face & 2 && mptr->pushing)
    mptr->face += 4;
  return;
}
/*}}}*/
/*{{{  int valid_directions(mptr, cptr)*/
extern int valid_directions FUNCARGLIST((mptr, cptr))
MONSTER   *mptr FUNCARGSEP
CELL      *cptr FUNCARGTERM
/*
 * sets the valid and nearer direction bits
 * these are nr, nl, nd, nu, r, l, d, u
 */
{
  int       answer;
  int       offset;

  answer = 0;
  if(offset = mptr->offset.y)
    /*{{{  up down only*/
    {
      if(offset > cptr->depths[0])
	answer |= 0x1;
      if(offset < cptr->depths[1])
	answer |= 0x2;
      if(offset < 0 ? cptr[0].distance < cptr[-CELL_STRIDE].distance :
	  cptr[0].distance >= cptr[CELL_STRIDE].distance)
	answer |= 0x20;
      else
	answer |= 0x10;
    }
    /*}}}*/
  else if(offset = mptr->offset.x)
    /*{{{  left right only*/
    {
      if(offset > cptr->depths[2])
	answer |= 0x4;
      if(offset < cptr->depths[3])
	answer |= 0x8;
      if(offset < 0 ? cptr[0].distance < cptr[-1].distance :
	  cptr[0].distance >= cptr[1].distance)
	answer |= 0x80;
      else
	answer |= 0x40;
    }
    /*}}}*/
  else
    /*{{{  at intersection*/
    {
      int       distance;
	  
      if(cptr->depths[0])
	answer |= 0x1;
      if(cptr->depths[1])
	answer |= 0x2;
      if(cptr->depths[2])
	answer |= 0x4;
      if(cptr->depths[3])
	answer |= 0x8;
      distance = cptr->distance;
      if(distance > cptr[-CELL_STRIDE].distance)
	answer |= 0x10;
      if(distance > cptr[CELL_STRIDE].distance)
	answer |= 0x20;
      if(distance > cptr[-1].distance)
	answer |= 0x40;
      if(distance > cptr[1].distance)
	answer |= 0x80;
    }
    /*}}}*/
  answer &= answer << 4 | 0xF;
  return answer;
}
/*}}}*/
