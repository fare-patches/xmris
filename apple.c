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
/*{{{  APPLE *apple_search(x, y, width, height, found)*/
extern APPLE *apple_search FUNCARGLIST((x, y, width, height, found))
int       x       FUNCARGSEP  /* x coord start */
int       y       FUNCARGSEP  /* y coord start */
unsigned  width   FUNCARGSEP  /* width of box */
unsigned  height  FUNCARGSEP  /* hieght of box */
unsigned  found   FUNCARGTERM /* already found ones */
/*
 * looks for an apple in the specified box
 * and returns a ptr to it, if found
 */
{
  unsigned  i;
  APPLE     *aptr;
  
  for(aptr = apple.list, i = 0; i != apple.apples; i++, aptr++, found >>= 1)
    if(!(found & 1) && aptr->pixel.x + aptr->push - x >= 0 &&
	aptr->pixel.x + aptr->push - x < width &&
	aptr->pixel.y - y >= 0 && aptr->pixel.y - y < height)
      return aptr;
  return NULL;
}
/*}}}*/
/*{{{  int apple_stop(mptr, cptr)*/
extern int apple_stop FUNCARGLIST((mptr, cptr))
MONSTER   *mptr FUNCARGSEP
CELL      *cptr FUNCARGTERM
/*
 * sees if the monster is about to walk into an apple
 * returns 0 if clear, 1 if stopped
 * sets mptr->pause & mptr->stop as required
 * the apples and other monsters are altered
 * if they're pushed about
 * called before we've moved the monster
 */
{
  unsigned  stop;
  unsigned  pause;
  pause = stop = 0;
  switch(mptr->dir)
  {
    /*{{{  case 0: (up)*/
    case 0:
    {
      if(mptr->offset.y == -VEL_Y)
	/*{{{  walked into apple?*/
	{
	  unsigned  i;
	  APPLE     *aptr;
	  int       px, py;
			
	  px = mptr->pixel.x;
	  py = mptr->pixel.y;
	  for(aptr = apple.list, i = apple.apples; i--; aptr++)
	    {
	      if(aptr->pixel.x + aptr->push - px < CELL_WIDTH &&
		  aptr->pixel.x + aptr->push - px > -CELL_WIDTH &&
		  py - aptr->pixel.y > 0 &&
		  py - aptr->pixel.y <= CELL_HEIGHT)
		{
		  stop = 1;
		  break;
		}
	    }
	}
	/*}}}*/
      break;
    }
    /*}}}*/
    /*{{{  case 1: (down)*/
    case 1:
    {
      unsigned  i;
      APPLE     *aptr;
      int       px, py;
	
      px = mptr->pixel.x;
      py = mptr->pixel.y;
      for(aptr = apple.list, i = apple.apples; i--; aptr++)
	{
	  if(aptr->pixel.x + aptr->push - px < CELL_WIDTH &&
	      aptr->pixel.x + aptr->push - px > -CELL_WIDTH &&
	      aptr->pixel.y + apple_sizes[aptr->state].offset.y - py <=
		  CELL_HEIGHT &&
	      aptr->pixel.y + apple_sizes[aptr->state].offset.y - py > 0)
	    {
	      stop = 1;
	      break;
	    }
	}
      break;
    }
    /*}}}*/
    /*{{{  case 2: (left)*/
    case 2:
    {
      unsigned  found;
      int       x, y;
      int       width;
      APPLE     *final;
	  
      found = 0;
      final = NULL;
      width = VEL_X;
      x = mptr->pixel.x - CELL_WIDTH + 1 - width;
      y = mptr->pixel.y - CELL_HEIGHT + VEL_Y;
      /*{{{  look for apple*/
      for(;;)
      {
	APPLE     *aptr;
	CELL      *cptr;
	      
	aptr = apple_search(x, y,
	    width + CELL_WIDTH / 2, 2 * (CELL_HEIGHT - VEL_Y), found);
	if(!aptr)
	  break;
	else if(aptr->state > 3 || aptr->push ||
	    aptr->pixel.x - width < PIXELX(0, 0))
	  {
	    stop = 1;
	    break;
	  }
	else if(aptr->state == 2 && ((mptr->type != 1 && mptr->type != 4) ||
	    (aptr->offset.x < 0 &&
	    (cptr = BOARDCELL(aptr->cell.x, aptr->cell.y +
		(aptr->offset.y <= 0)))->depths[2] > aptr->offset.x &&
		  !cptr[-1].visit)))
	  break;
	else
	  {
	    final = aptr;
	    found |= 1 << (aptr - apple.list);
	    width = aptr->pixel.x - x + 1;
	    aptr->maypush = width;
	    x = aptr->pixel.x - CELL_WIDTH + 1 - width;
	    if(aptr->state != 2 && aptr->offset.y > 0 &&
		apple_search(x, aptr->pixel.y - aptr->offset.y +
		CELL_HEIGHT, width, aptr->offset.y, found))
	      {
		stop = 1;
		break;
	      }
	    y = aptr->pixel.y - CELL_HEIGHT + VEL_Y;
	    if(!width || aptr->state == 2)
	      break;
	  }
      }
      /*}}}*/
      if(found && !stop)
	{
	  /*{{{  check if against monster*/
	  if(width)
	    {
	      unsigned  i;
	      MONSTER   *mptr;
	      MONSTER   *list;
			    
	      list = NULL;
	      for(mptr = monster.list, i = monster.monsters; i--; mptr++)
		{
		  if(mptr->pixel.x - x >= 0 &&
		      mptr->pixel.x - x < width + CELL_WIDTH &&
		      mptr->pixel.y - y >= 0 &&
		      mptr->pixel.y - y < 2 * (CELL_HEIGHT - VEL_Y))
		    {
		      if(mptr->type == 1)
			{
			  if(mptr->dir & 2 ||
			      mptr->pixel.x - x >= width + CELL_WIDTH / 2)
			    stop = 1;
			}
		      else if(mptr->pixel.x - x >= width + CELL_WIDTH / 2)
			/*(EMPTY*/;
		      else if(mptr->type == 4)
			stop = 1;
		      else if(mptr->type == 0)
			{
			  CELL      *cptr;
			    
			  cptr = BOARDCELL(mptr->cell.x, mptr->cell.y);
			  if(cptr->depths[2] <= mptr->offset.x - width ||
			      ((cptr->depths[1] || cptr->depths[0]) &&
			      mptr->offset.x >= 0))
			    {
			      mptr->list = list;
			      list = mptr;
			      mptr->push = -1;
			    }
			  else
			    stop = 1;
			}
		    }
		}
	      if(final->state < 2)
		final->list = list;
	    }
	  /*}}}*/
	  if(!stop)
	  {
	    pause = 1;
	    /*{{{  push the apples*/
	    {
	      unsigned  i;
	      APPLE     *aptr;
				  
	      for(aptr = apple.list, i = apple.apples; i--; aptr++, found >>= 1)
		if(found & 1)
		  aptr->push = -aptr->maypush;
	    }
	    /*}}}*/
	  }
	}
      break;
    }
    /*}}}*/
    /*{{{  case 3: (right)*/
    case 3:
    {
      unsigned  found;
      int       x, y;
      int       width;
      APPLE     *final;
	  
      found = 0;
      final = NULL;
      width = VEL_X;
      x = mptr->pixel.x + CELL_WIDTH;
      y = mptr->pixel.y - CELL_HEIGHT + VEL_Y;
      /*{{{  look for apple*/
      for(;;)
      {
	APPLE       *aptr;
	      
	aptr = apple_search(x - CELL_WIDTH / 2, y,
	    width + CELL_WIDTH / 2, 2 * (CELL_HEIGHT - VEL_Y), found);
	if(!aptr)
	  break;
	else if(aptr->state > 3 || aptr->push ||
	    aptr->pixel.x + width > PIXELX(CELLS_ACROSS - 1, 0))
	  {
	    stop = 1;
	    break;
	  }
	else if(aptr->state == 2 && ((mptr->type != 1 && mptr->type != 4) ||
	    (aptr->offset.x > 0 &&
	    (cptr = BOARDCELL(aptr->cell.x, aptr->cell.y +
		(aptr->offset.y <= 0)))->depths[3] < aptr->offset.x &&
		  !cptr[1].visit)))
	  break;
	else
	  {
	    final = aptr;
	    found |= 1 << (aptr - apple.list);
	    width -= aptr->pixel.x - x;
	    aptr->maypush = width;
	    x = aptr->pixel.x + CELL_WIDTH;
	    if(aptr->state != 2 && aptr->offset.y > 0 &&
		apple_search(x, aptr->pixel.y - aptr->offset.y +
		CELL_HEIGHT, width, aptr->offset.y, found))
	      {
		stop = 1;
		break;
	      }
	    y = aptr->pixel.y - CELL_HEIGHT + VEL_Y;
	    if(!width || aptr->state == 2)
	      break;
	  }
      }
      /*}}}*/
      /*{{{  check if against monster*/
      if(found && width)
	{
	  unsigned  i;
	  MONSTER   *mptr;
	  MONSTER   *list;
		
	  list = NULL;
	  for(mptr = monster.list, i = monster.monsters; i--; mptr++)
	    {
	      if(mptr->pixel.x - x >= -CELL_WIDTH &&
		  mptr->pixel.x - x < width &&
		  mptr->pixel.y - y >= 0 &&
		  mptr->pixel.y - y < 2 * (CELL_HEIGHT - VEL_Y))
		{
		  if(mptr->type == 1)
		    {
		      if(mptr->dir & 2 ||
			  mptr->pixel.x - x < -CELL_WIDTH / 2)
			stop = 1;
		    }
		  else if(mptr->pixel.x - x < -CELL_WIDTH / 2)
		    /*EMPTY*/;
		  else if(mptr->type == 4)
		    stop = 1;
		  else if(mptr->type == 0)
		    {
		      CELL      *cptr;
	    
		      cptr = BOARDCELL(mptr->cell.x, mptr->cell.y);
		      if(cptr->depths[3] >= mptr->offset.x + width ||
			  ((cptr->depths[1] || cptr->depths[0]) &&
			  mptr->offset.x <= 0))
			{
			  mptr->list = list;
			  list = mptr;
			  mptr->push = 1;
			}
		      else
			stop = 1;
		    }
		}
	    }
	  if(final->state < 2)
	    final->list = list;
	}
      /*}}}*/
      if(found && !stop)
	{
	  pause = 1;
	  /*{{{  push the apples*/
	  {
	    unsigned  i;
	    APPLE     *aptr;
			      
	    for(aptr = apple.list, i = apple.apples; i--; aptr++, found >>= 1)
	      if(found & 1)
		aptr->push = aptr->maypush;
	  }
	  /*}}}*/
	}
      break;
    }
    /*}}}*/
  }
  mptr->stop = stop;
  mptr->pause = pause | stop;
  return stop;
}
/*}}}*/
/*{{{  void apple_under(mptr, cptr)*/
extern void apple_under FUNCARGLIST((mptr, cptr))
MONSTER   *mptr FUNCARGSEP
CELL      *cptr FUNCARGTERM
/*
 * deals with walking under apples
 * so that they start or stop rocking
 * called after we've done the move
 * only for player and muncher
 */
{
  assert(mptr->type == 4 || mptr->type == 1);
  switch(mptr->dir)
  {
    /*{{{  case 0: (up)*/
    case 0:
      if(!mptr->offset.y)
	/*{{{  stop apple rocking?*/
	{
	  unsigned  i;
	  APPLE     *aptr;
			  
	  for(aptr = apple.list, i = apple.apples; i--; aptr++)
	    {
	      if(aptr->pixel.x - mptr->pixel.x <= CELL_WIDTH / 2 &&
		 aptr->pixel.x - mptr->pixel.x >= -(CELL_WIDTH / 2) &&
		  aptr->state == 1 && mptr->cell.y == aptr->cell.y + 1)
	      {
		aptr->state = 0;
		aptr->count = 1;
	      }
	  }
	}
	/*}}}*/
      break;
    /*}}}*/
    /*{{{  case 1: (down)*/
    case 1:
      if(mptr->offset.y == VEL_Y)
	/*{{{  start apple rocking?*/
	{
	  unsigned  i;
	  APPLE     *aptr;
			
	  for(aptr = apple.list, i = apple.apples; i--; aptr++)
	    {
	      if(aptr->pixel.x - mptr->pixel.x <= CELL_WIDTH / 2 &&
		  aptr->pixel.x - mptr->pixel.x >= -(CELL_WIDTH / 2) &&
		  !aptr->state && mptr->cell.y == aptr->cell.y + 1)
		{
		  aptr->state = 1;
		  aptr->count = APPLE_ROCK_DELAY;
		}
	    }
	}
	/*}}}*/
      break;
    /*}}}*/
    /*{{{  case 2: (left)*/
    case 2:
    {
      unsigned  i;
      APPLE     *aptr;
      int       px, py;
      int       cy;
	
      px = mptr->pixel.x;
      py = mptr->pixel.y;
      cy = mptr->cell.y;
      /*{{{  walking under apple?*/
      for(aptr = apple.list, i = apple.apples; i--; aptr++)
	{
	  if(aptr->cell.y + 1 != cy)
	    /*EMPTY*/;
	  else if(aptr->pixel.x - px == CELL_WIDTH / 2 && !aptr->state)
	    {
	      aptr->state = 1;
	      aptr->count = APPLE_ROCK_DELAY;
	    }
	  else if(mptr->type == 4 && aptr->state == 1 &&
	      aptr->pixel.x - px == -CELL_WIDTH / 2)
	    {
	      aptr->state = 0;
	      aptr->count = 1;
	    }
	}
      /*}}}*/
      break;
    }
    /*}}}*/
    /*{{{  case 3: (right)*/
    case 3:
    {
      unsigned  i;
      APPLE     *aptr;
      int       px, py;
      int       cy;
	
      px = mptr->pixel.x;
      py = mptr->pixel.y;
      cy = mptr->cell.y;
      /*{{{  walking under apple?*/
      for(aptr = apple.list, i = apple.apples; i--; aptr++)
	{
	  if(aptr->cell.y + 1 != cy)
	    /*EMPTY*/;
	  else if(px - aptr->pixel.x == CELL_WIDTH / 2 && !aptr->state)
	    {
	      aptr->state = 1;
	      aptr->count = APPLE_ROCK_DELAY;
	    }
	  else if(mptr->type == 4  && aptr->state == 1 &&
	      px - aptr->pixel.x == -CELL_WIDTH / 2)
	    {
	      aptr->state = 0;
	      aptr->count = 1;
	    }
	}
      /*}}}*/
      break;
    }
    /*}}}*/
  }
  return;
}
/*}}}*/
/*{{{  void move_apples()*/
extern void move_apples FUNCARGVOID
/*
 * moves all the apples
 */
{
  APPLE     *aptr;
  unsigned  i;

  for(aptr = apple.list, i = apple.apples; i--; aptr++)
    {
      CELL      *cptr;
  
      cptr = BOARDCELL(aptr->cell.x, aptr->cell.y);
      if(aptr->chewed)
	{
	  if(aptr->monsters)
	    add_score(squish_scores[aptr->monsters >= SQUISH_SCORES ?
		SQUISH_SCORES - 1 : aptr->monsters],
		aptr->pixel.x + CELL_WIDTH / 2,
		aptr->pixel.y + apple_sizes[4].size.y / 2);
	  aptr->state = 6;
	}
      else
	switch(aptr->state)
	{
	  /*{{{  case 0: (static) case 1: (rock)*/
	  case 0: case 1:
	    if(aptr->push)
	      {
		aptr->pixel.x += aptr->push;
		aptr->offset.x += aptr->push;
		if(aptr->offset.x < -(CELL_WIDTH / 2))
		  {
		    aptr->offset.x += CELL_WIDTH + GAP_WIDTH;
		    aptr->cell.x -= 1;
		    cptr -= 1;
		  }
		else if(aptr->offset.x > (CELL_WIDTH / 2))
		  {
		    aptr->offset.x -= CELL_WIDTH + GAP_WIDTH;
		    aptr->cell.x += 1;
		    cptr += 1;
		  }
		if(cptr[0].depths[1] > aptr->offset.y)
		  aptr->state = 2;
		else if(aptr->state == 0 && cptr[CELL_STRIDE].visit)
		  {
		    aptr->state = 1;
		    aptr->count = APPLE_ROCK_DELAY;
		  }
		else if(aptr->state == 1 && !cptr[CELL_STRIDE].visit)
		  {
		    aptr->state = 0;
		    aptr->count = 1;
		  }
	      }
	    if(aptr->state == 1 && !aptr->count--)
	      aptr->state = 2;
	    if(aptr->state == 2)
	      {
		MONSTER   *mptr;
				  
		aptr->count = APPLE_ACC;
		aptr->distance = 0;
		if(!cptr[0].visit)
		  {
		    aptr->cell.y += 1;
		    aptr->offset.y -= CELL_HEIGHT + GAP_HEIGHT;
		  }
		if(cptr[CELL_STRIDE].visit || cptr[0].depths[1])
		  /*EMPTY*/;
		else if(cptr[CELL_STRIDE-1].depths[3] +
		    cptr[CELL_STRIDE+1].depths[2] > 0)
		  {
		    aptr->cell.x--;
		    aptr->offset.x += CELL_WIDTH + GAP_WIDTH;
		  }
		else if(cptr[CELL_STRIDE-1].depths[3] +
		    cptr[CELL_STRIDE+1].depths[2] < 0)
		  {
		    aptr->cell.x++;
		    aptr->offset.x -= CELL_WIDTH + GAP_WIDTH;
		  }
		else
		  {
		    aptr->state = 0;
		    aptr->list = NULL;
		  }
		for(mptr = aptr->list; mptr; mptr = mptr->list)
		  {
		    mptr->apple = aptr;
		    aptr->monsters++;
		    aptr->distance = APPLE_FALL_SPLIT;
		  }
	      }
	    else
	      aptr->list = NULL;
	    break;
	  /*}}}*/
	  /*{{{  case 2: (fall)*/
	  case 2:
	  {
	    unsigned  j;
	    APPLE     *optr;
	    COORD     pixel;
			    
	    /*{{{  horizontal movement*/
	    if(aptr->push)
	      {
		aptr->pixel.x += aptr->push;
		aptr->offset.x += aptr->push;
		if(aptr->offset.x < -(CELL_WIDTH / 2))
		  {
		    aptr->offset.x += CELL_WIDTH + GAP_WIDTH;
		    aptr->cell.x -= 1;
		    cptr -= 1;
		  }
		else if(aptr->offset.x > (CELL_WIDTH / 2))
		  {
		    aptr->offset.x -= CELL_WIDTH + GAP_WIDTH;
		    aptr->cell.x += 1;
		    cptr += 1;
		  }
	      }
	    else if(aptr->waspushed)
	      /*EMPTY*/;
	    else if(aptr->offset.x < 0)
	      {
		aptr->offset.x += APPLE_VEL_X;
		aptr->pixel.x += APPLE_VEL_X;
		assert(aptr->offset.x <= 0);
	      }
	    else if(aptr->offset.x > 0)
	      {
		aptr->offset.x -= APPLE_VEL_X;
		aptr->pixel.x -= APPLE_VEL_X;
		assert(aptr->offset.x >= 0);
	      }
	    /*}}}*/
	    pixel.x = aptr->pixel.x - aptr->offset.x;
	    pixel.y = aptr->pixel.y - aptr->offset.y;
	    aptr->offset.y += aptr->count;
	    aptr->pixel.y += aptr->count;
	    aptr->distance += aptr->count;
	    aptr->count += APPLE_ACC;
	    if(aptr->count > APPLE_VEL_Y)
	      aptr->count = APPLE_VEL_Y;
	    /*{{{  bashes into another?*/
	    {
	      int       x, y;
				
	      x = aptr->pixel.x - CELL_WIDTH;
	      y = pixel.y + aptr->offset.y;
	      for(optr = apple.list, j = apple.apples; j--; optr++)
		if(optr != aptr &&
		    optr->pixel.x - x > 0 &&
		    optr->pixel.x - x < CELL_WIDTH * 2 &&
		    optr->pixel.y + apple_sizes[optr->state].offset.y - y > 0 &&
		    optr->pixel.y + apple_sizes[optr->state].offset.y - y <
		    CELL_HEIGHT - APPLE_VEL_Y && (optr->state ||
		    BOARDCELL(optr->cell.x, optr->cell.y)->visit))
		  {
		    if(optr->state < 3)
		      {
			optr->state = 3;
			optr->count = APPLE_SPLIT_DELAY;
		      }
		    optr = NULL;
		    break;
		  }
	    }
	    /*}}}*/
	    if(!optr)
	      {
		aptr->state = 3;
		aptr->count = APPLE_SPLIT_DELAY;
	      }
	    else if(aptr->offset.y >= cptr[0].depths[1])
	      {
		if(drop_apple(aptr, cptr))
		  /*EMPTY*/;
		else if(aptr->distance >= APPLE_FALL_SPLIT ||
		    aptr->cell.y == CELLS_DOWN - 1)
		  /*{{{  fell too far*/
		  {
		    aptr->state = 3;
		    aptr->count = APPLE_SPLIT_DELAY;
		    aptr->offset.y = cptr[0].depths[1];
		  }
		  /*}}}*/
		else
		  /*{{{  only fell a short way*/
		  {
		    aptr->state = 0;
		    aptr->offset.y = cptr[0].depths[1];
		  }
		  /*}}}*/
	      }
	    else if(aptr->offset.y > 0 && cptr[CELL_STRIDE].depths[0])
	      /*{{{  fall into next cell*/
	      {
		aptr->cell.y += 1;
		aptr->offset.y -= CELL_HEIGHT + GAP_HEIGHT;
		pixel.y += CELL_HEIGHT + GAP_HEIGHT;
	      }
	      /*}}}*/
	    /*{{{  crushed someone?*/
	    {
	      unsigned  i;
	      MONSTER   *mptr;
	      int       x, y;
				  
	      x = pixel.x + aptr->offset.x - CELL_WIDTH + VEL_X;
	      y = pixel.y + aptr->offset.y - APPLE_VEL_Y;
	      for(mptr = monster.list, i = monster.monsters; i--; mptr++)
		if(!mptr->apple && !mptr->shot && mptr->type != 5 &&
		    mptr->pixel.x - x > 0 &&
		    mptr->pixel.x - x < 2 * (CELL_WIDTH - VEL_X) &&
		    mptr->pixel.y - y > 0 &&
		    mptr->pixel.y - y < CELL_WIDTH / 2 + APPLE_VEL_Y &&
		    (!(mptr->type & 2) || global.state == 4 ||
		      aptr->monsters || mptr->chew || mptr->dir == 1 ||
		      (mptr->dir == 2 && mptr->pixel.x - x <
			CELL_WIDTH - VEL_X + CELL_WIDTH / 2) ||
		      (mptr->dir == 3 && mptr->pixel.x - x >
			CELL_WIDTH - VEL_X -  CELL_WIDTH / 2)))
		  {
		    if(mptr->type & 2 && !mptr->dir && !mptr->chew &&
			!aptr->monsters && global.state != 4)
		      {
			aptr->chewed = 1;
			mptr->chew = 1;
		      }
		    else
		      {
			if(mptr != &monster.list[0])
			  aptr->monsters++;
			mptr->apple = aptr;
			aptr->distance = APPLE_FALL_SPLIT;
			if(!aptr->state)
			  {
			    aptr->state = 3;
			    aptr->count = APPLE_SPLIT_DELAY;
			  }
		      }
		  }
	    }
	    /*}}}*/
	    aptr->pixel.y = pixel.y + aptr->offset.y;
	    break;
	  }
	  /*}}}*/
	  /*{{{  case 3: (split)*/
	  case 3:
	    if(!aptr->count--)
	      {
		aptr->state = 4;
		aptr->count = APPLE_DECAY_DELAY;
		if(aptr->monsters)
		  add_score(squish_scores[aptr->monsters >= SQUISH_SCORES ?
		      SQUISH_SCORES - 1 : aptr->monsters],
		      aptr->pixel.x + CELL_WIDTH / 2,
		      aptr->pixel.y + apple_sizes[4].size.y / 2);
	      }
	    break;
	  /*}}}*/
	  /*{{{  case 4: (decay)*/
	  case 4:
	    if(!aptr->count--)
	      {
		aptr->state = 5;
		aptr->count = APPLE_ROT_DELAY;
	      }
	    break;
	  /*}}}*/
	  /*{{{  case 5: (rot)*/
	  case 5:
	    if(!aptr->count--)
	      aptr->state = 6;
	    break;
	  /*}}}*/
	  /*{{{  default:*/
	  default:
	    assert(0);
	  /*}}}*/
	}
      aptr->waspushed = aptr->push;
      aptr->push = 0;
    }
  return;
}
/*}}}*/
/*{{{  APPLE *spawn_apple(cx, cy, ox, oy)*/
extern APPLE *spawn_apple FUNCARGLIST((cx, cy, ox, oy))
int       cx FUNCARGSEP
int       cy FUNCARGSEP
int       ox FUNCARGSEP
int       oy FUNCARGTERM
/*
 * spawns a new apple in state 0
 */
{
  APPLE     *aptr;

  assert(apple.apples != APPLES);
  aptr = &apple.list[apple.apples++];
  aptr->state = 0;
  aptr->cell.x = cx;
  aptr->cell.y = cy;
  aptr->offset.x = ox = ox / APPLE_VEL_X * APPLE_VEL_X;
  aptr->offset.y = oy;
  aptr->count = 1;
  aptr->old_pixel.x = aptr->pixel.x = PIXELX(cx, ox);
  aptr->old_pixel.y = aptr->pixel.y = PIXELY(cy, oy);
  aptr->distance = 0;
  aptr->monsters = 0;
  aptr->push = 0;
  aptr->chewed = 0;
  aptr->old_state = 7;
  return aptr;
}
/*}}}*/
