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
static void extra_dies PROTOARGLIST((void));
/*}}}*/
/*{{{  MONSTER *extra_escape()*/
extern MONSTER *extra_escape FUNCARGVOID
/*
 * remove the extra monster from the top, so it
 * can run around
 */
{
  int       x;
  
  extra.escape = 1;
  x = XTRA_X + extra.select * XTRA_SPACING;
  XFillRectangle(display.display, display.back, GCN(GC_CLEAR),
      x, XTRA_Y, CELL_WIDTH, CELL_HEIGHT);
  add_background(x, XTRA_Y, CELL_WIDTH, CELL_HEIGHT);
  x -= BORDER_LEFT + GAP_WIDTH;
  assert(x % (CELL_WIDTH + GAP_WIDTH) % VEL_X == 0);
  return spawn_monster(2, 1, 1, x / (CELL_WIDTH + GAP_WIDTH), 0,
      x % (CELL_WIDTH + GAP_WIDTH), XTRA_Y - BORDER_TOP);
}
/*}}}*/
/*{{{  void extra_dies()*/
static void extra_dies FUNCARGVOID
/*
 * the extra monster has died,
 * put it back at the top
 * and maybe alter the state
 */
{
  unsigned  got;
  
  if(global.state == 2)
    {
      global.state = 3;
      monster.den = 0;
      monster.delay = 0;
    }
  got = extra.got & (1 << extra.select);
  extra.got |= 1 << extra.select;
  extra.escape = 0;
  if(!got)
    create_xtra_monster(extra.select);
  draw_extra();
  return;
}
/*}}}*/
/*{{{  void fall_monsters()*/
extern void fall_monsters FUNCARGVOID
/*
 * makes all the monsters fall
 */
{
  unsigned  i;
  MONSTER   *mptr;

  for(mptr = monster.list, i = monster.monsters; i--; mptr++)
    if(mptr->apple)
      {
	/*{{{  just hit?*/
	if(mptr->face < 8)
	  {
	    if(BOARDCELL(mptr->cell.x, mptr->cell.y)->distance ==
	      monster.nearest)
	    global.difficulty++;
	    new_face(mptr);
	    if(mptr == &monster.list[0])
	      mptr->shot = 1;
	    else
	      {
		mptr->chew = 0;
		if(mptr->type < 2)
		  monster.normals--;
		else if(mptr->type == 2)
		  extra_dies();
		else if(mptr->type == 3)
		  monster.drones--;
	      }
	  }
	/*}}}*/
	mptr->pixel.y = mptr->apple->pixel.y + CELL_HEIGHT;
	if(mptr->apple && mptr->apple->state > 3)
	  {
	    if(mptr->type != 4)
	      mptr->type = 5;
	    else
	      mptr->apple = NULL;
	  }
      }
  return;
}
/*}}}*/
/*{{{  void move_monsters()*/
extern void move_monsters FUNCARGVOID
/*
 * moves all the monsters
 */
{
  MONSTER   *mptr;
  unsigned  i;
  unsigned  nearest;
  unsigned  farthest;

  nearest = 255;
  farthest = 0;
  for(mptr = &monster.list[1], i = monster.monsters - 1; i--; mptr++)
    {
      if(mptr->shot || (mptr->type == 3 && global.state == 3))
	/*{{{  shot*/
	{
	  if(BOARDCELL(mptr->cell.x, mptr->cell.y)->distance ==
	      monster.nearest)
	    global.difficulty++;
	  if(mptr->type < 2)
	    monster.normals--;
	  else
	    /*{{{  convert to apple*/
	    {
	      APPLE     *aptr;
	      CELL      *cptr;
			  
	      if(mptr->type == 2)
		extra_dies();
	      else if(mptr->type == 3)
		monster.drones--;
	      if(mptr->offset.x > (CELL_WIDTH + GAP_WIDTH) / 2)
		{
		  mptr->offset.x -= CELL_WIDTH + GAP_WIDTH;
		  mptr->cell.x++;
		}
	      else if(mptr->offset.x < -(CELL_WIDTH + GAP_WIDTH) / 2)
		{
		  mptr->offset.x += CELL_WIDTH + GAP_WIDTH;
		  mptr->cell.x--;
		}
	      cptr = BOARDCELL(mptr->cell.x, mptr->cell.y);
	      aptr = spawn_apple(mptr->cell.x, mptr->cell.y,
		  mptr->offset.x, mptr->offset.y);
	      if(mptr->cell.y == CELLS_DOWN - 1)
		aptr->state = 2;
	      else if(mptr->offset.y < cptr->depths[1])
		aptr->state = 2;
	      else if(!mptr->offset.y && cptr[CELL_STRIDE].visit)
		{
		  aptr->state = 1;
		  aptr->count = APPLE_ROCK_DELAY;
		}
	    }
	    /*}}}*/
	  if(mptr->shot)
	    add_score(500, mptr->pixel.x + CELL_WIDTH / 2,
		mptr->pixel.y + CELL_HEIGHT / 2);
	  mptr->type = 5;
	}
	/*}}}*/
      else if(mptr->apple)
	/*EMPTY*/;
      else if(mptr->chew)
	/*{{{  chewing*/
	{
	  if(mptr->chew == 1)
	    {
	      mptr->chew = 2;
	      mptr->count = CHOMP_DELAY;
	      mptr->image = 0;
	      mptr->cycle = MONSTER_CYCLES - 1;
	    }
	  mptr->count--;
	  if(!mptr->count)
	    mptr->chew = 0;
	  if(!mptr->cycle)
	    {
	      mptr->image = (mptr->image + 1) % MONSTER_IMAGES;
	      mptr->cycle = MONSTER_CYCLES;
	    }
	  mptr->cycle--;
	}
	/*}}}*/
      else if(global.state != 4)
	{
	  CELL      *cptr;

	  cptr = BOARDCELL(mptr->cell.x, mptr->cell.y);
	  assert((cptr->distance || global.broken) && cptr->visit);
	  if(!mptr->cycle)
	    {
	      mptr->cycle = MONSTER_CYCLES;
	      mptr->image++;
	      if(mptr->image == MONSTER_IMAGES)
		mptr->image = 0;
	    }
	  if((!mptr->count || mptr->type & 2) && !mptr->pause)
	    mptr->cycle--;
	  if(mptr->type & 2 || global.state != 2)
	    {
	      if(nearest > cptr->distance)
		nearest = cptr->distance;
	      if(farthest < cptr->distance)
		farthest = cptr->distance;
	    }
	  switch(mptr->type)
	  {
	    /*{{{  case 0: case 1: (normal or muncher)*/
	    case 0: case 1:
	    {
	      int     valid;
			  
	      valid = valid_directions(mptr, cptr);
	      if(mptr->count)
		mptr->count--;
	      if(!mptr->type && random() < CONT_TOGGLE_PROB)
		/*{{{  toggle the cont & maybe turn round*/
		{
		  mptr->cont = !mptr->cont;
		  if(mptr->pause || mptr->stop)
		    {
		      mptr->dir ^= 1;
		      new_face(mptr);
		    }
		}
		/*}}}*/
	      if(mptr->push)
		/*{{{  disable left or right*/
		{
		  if(mptr->push < 0)
		    valid &= 0x77;
		  else
		    valid &= 0xBB;
		}
		/*}}}*/
	      else if(mptr->count)
		valid = 0;
	      else if(mptr->pause)
		{
		  if(!mptr->type)
		    {
		      if(random() < GO_MUNCH_PROB * global.difficulty)
			mptr->gomunch = 1;
		      valid = 0;
		    }
		  else if(mptr->stop || random() < PUSH_TURN_PROB)
		    {
		      mptr->dir ^= 1;
		      new_face(mptr);
		      valid = 0;
		    }
		  mptr->stop = 0;
		  mptr->pause = 0;
		}
	      else if(global.state == 2)
		valid = 0;
	      else if(!mptr->offset.x && !mptr->offset.y)
		{
		  mptr->fast = 0;
		  if(!mptr->type && global.state == 3)
		    mptr->fast = 1;
		  if(mptr->gomunch)
		    {
		      mptr->gomunch = 0;
		      mptr->type = 1;
		      mptr->count = GO_MUNCH_DELAY;
		    }
		  else if(mptr->type)
		    {
		      int       temp;
			
		      temp = valid & 0xF;
		      if((temp & -temp) != temp)
			{
			  APPLE     *aptr;
			  unsigned  i;
			  int       x;
				      
			  mptr->type = 0;
			  mptr->count = STOP_MUNCH_DELAY;
			  valid = 0;
			  x = mptr->pixel.x - CELL_WIDTH + GAP_WIDTH * 2;
			  for(aptr = apple.list, i = apple.apples; i--; aptr++)
			    {
			      if(!aptr->state && aptr->pixel.x - x >= 0 &&
				  aptr->pixel.x - x < CELL_WIDTH * 2 - GAP_WIDTH * 4 &&
				  aptr->cell.y == mptr->cell.y - 1)
				{
				  aptr->state = 1;
				  aptr->count = APPLE_ROCK_DELAY;
				}
			    }
			}
		    }
		  else if(!mptr->type && random() <
		      GO_MUNCH_PROB * global.difficulty)
		    mptr->gomunch = 1;
		}
	      if(!valid)
		/*EMPTY*/;
	      else if(mptr->type)
		/*{{{  move the muncher*/
		{
		  if(mptr->offset.x || mptr->offset.y)
		    /*{{{  carry on*/
		    {
		      CELL      *nptr;
					  
		      nptr = move_muncher(mptr);
		      if(nptr)
			{
			  cptr = nptr;
			  if(nptr->sprite == SPRITE_CHERRY)
			    {
			      global.cherries--;
			      nptr->sprite = 0;
			    }
			}
		    }
		    /*}}}*/
		  else
		    /*{{{  pick new direction*/
		    {
		      int     temp;
					  
		      temp = ~valid & 0xF;
		      if(!mptr->cell.y)
			temp &= 0xE;
		      else if(mptr->cell.y == CELLS_DOWN - 1)
			temp &= 0xD;
		      if(!mptr->cell.x)
			temp &= 0xB;
		      else if(mptr->cell.x == CELLS_ACROSS - 1)
			temp &= 0x7;
		      if(!temp)
			temp = valid & 0xF;
		      if(mptr->pixel.x < monster.list[0].pixel.x)
			valid = 0x8;
		      else if(mptr->pixel.x > monster.list[0].pixel.x)
			valid = 0x4;
		      else
			valid = 0;
		      if(!(valid & temp))
			{
			  if(mptr->pixel.y < monster.list[0].pixel.y)
			    valid = 0x2;
			  else if(mptr->pixel.y > monster.list[0].pixel.y)
			    valid = 0x1;
			  if(!(valid & temp))
			    valid = temp;
			}
		      assert(valid);
		      for(temp = 0; !(valid & 1); temp++)
			valid >>= 1;
		      if(temp != mptr->dir)
			{
			  mptr->dir = temp;
			  new_face(mptr);
			}
		      move_muncher(mptr);
		    }
		    /*}}}*/
		}
		/*}}}*/
	      else
		/*{{{  pick a direction*/
		{
		  unsigned  temp;
				  
		  if(mptr->cont)
		    {
		      valid &= 0xF;
		      temp = valid & ~(1 << (mptr->dir ^ 1));
		      if(temp)
			valid = temp;
		    }
		  else if(valid & 0xF0)
		    valid = valid & valid >> 4;
		  valid = choose_direction(valid);
		  temp = mptr->dir;
		  if(valid != mptr->dir)
		    {
		      mptr->dir = valid;
		      if(mptr->push && (valid ^ temp) != 1)
			{
			  mptr->push = 0;
			  mptr->cont = 1;
			}
		      new_face(mptr);
		    }
		  if(!apple_stop(mptr, cptr))
		    cptr = move_movable(mptr, cptr);
		  if(mptr->push)
		    {
		      mptr->dir = temp;
		      mptr->push = 0;
		    }
		}
		/*}}}*/
	      break;
	    }
	    /*}}}*/
	    /*{{{  case 2: case 3: (xtra or drone)*/
	    case 2: case 3:
	    {
	      int     valid;
	      int     temp;
			  
	      /*{{{  giving birth?*/
	      if(monster.delay && mptr->type == 2 && global.state == 2)
		{
		  monster.delay--;
		  if(monster.delay)
		    break;
		  if(monster.den)
		    {
		      monster.delay = XTRA_BIRTH_DELAY;
		      monster.den--;
		      spawn_monster(3, mptr->dir, mptr->face,
			  mptr->cell.x, mptr->cell.y,
			  mptr->offset.x, mptr->offset.y);
		      monster.drones++;
		      i++;
		    }
		}
	      /*}}}*/
	      valid = valid_directions(mptr, cptr);
	      if(global.state == 3 && !mptr->offset.x && ! mptr->offset.y)
		mptr->fast = 1;
	      /*{{{  pick a direction*/
	      {
		if(valid & 0xF0 &&
		    (mptr->type != 2 || player.ball.state ||
		    (global.state == 2 ? mptr->count == monster.farthest :
		     mptr->count != monster.nearest)))
		  {
		    mptr->cont = 0;
		    valid >>= 4;
		  }
		else
		  {
		    temp = valid & ((valid >> 4) ^ 0xF);
		    if(temp)
		      valid = temp;
		    else
		      valid &= 0xF;
		  }
		valid = choose_direction(valid);
		if(valid != mptr->dir)
		  {
		    mptr->dir = valid;
		    new_face(mptr);
		  }
		mptr->count = cptr->distance;
		cptr = move_movable(mptr, cptr);
		/*{{{  walked into apple?*/
		{
		  unsigned  i;
		  APPLE     *aptr;
		  int       x, y;
		  int       width, height;
				  
		  x = mptr->pixel.x;
		  y = mptr->pixel.y;
		  /*{{{  set offset*/
		  if(mptr->dir & 2)
		    {
		      x -= CELL_WIDTH / 2;
		      y -= CELL_HEIGHT / 4;
		      width = CELL_WIDTH;
		      height = CELL_HEIGHT / 2;
		    }
		  else
		    {
		      x -= CELL_WIDTH / 4;
		      y -= CELL_HEIGHT / 2;
		      width = CELL_WIDTH / 2;
		      height = CELL_HEIGHT;
		    }
		  /*}}}*/
		  for(aptr = apple.list, i = apple.apples; i--; aptr++)
		    {
		      if(aptr->state < 3 &&
			  aptr->pixel.x - x >= 0 &&
			  aptr->pixel.x - x < width &&
			  aptr->pixel.y - y >= 0 &&
			  aptr->pixel.y - y < height)
			{
			  mptr->chew = 1;
			  aptr->chewed = 1;
			  break;
			}
		    }
		}
		/*}}}*/
	      }
	      /*}}}*/
	      break;
	    }
	    /*}}}*/
	    /*{{{  default:*/
	    default:
	      assert(0);
	      break;
	    /*}}}*/
	  }
	}
    }
  monster.nearest = nearest;
  monster.farthest = farthest;
  return;
}
/*}}}*/
/*{{{  void new_xtra()*/
extern void new_xtra FUNCARGVOID
/*
 * increments the extra monster, and draws it up
 */
{
  draw_extra_letter(extra.select);
  if(extra.select == 4)
    extra.select = 0;
  else
    extra.select++;
  create_xtra_monster(extra.select);
  draw_extra();
  return;
}
/*}}}*/
/*{{{  MONSTER *spawn_monster(type, dir, face, cx, cy, ox, oy)*/
extern MONSTER *spawn_monster FUNCARGLIST((type, dir, face, cx, cy, ox, oy))
int       type  FUNCARGSEP  /* type of monster 0-4 */
int       dir   FUNCARGSEP  /* direction 0-3 */
int       face  FUNCARGSEP  /* face 0-5 */
int       cx    FUNCARGSEP  /* cell x */
int       cy    FUNCARGSEP  /* cell y */
int       ox    FUNCARGSEP  /* offset x */
int       oy    FUNCARGTERM /* offset y */
/*
 * creates a new monster onto the monster list
 * returns a pointer to the new monster
 */
{
  MONSTER *mptr;

  assert(monster.monsters != MONSTERS);
  assert(!(ox % VEL_X) && !(oy % VEL_Y));
  mptr = &monster.list[monster.monsters++];
  mptr->dir = dir;
  mptr->type = type;
  mptr->face = face;
  mptr->apple = NULL;
  mptr->push = 0;
  mptr->gomunch = mptr->cont = mptr->chew = mptr->pause = mptr->stop = 0;
  mptr->fast = mptr->pushing = 0;
  mptr->count = 0;
  mptr->cell.x = cx;
  mptr->cell.y = cy;
  mptr->offset.x = ox;
  mptr->offset.y = oy;
  mptr->pixel.x = PIXELX(cx, ox);
  mptr->pixel.y = PIXELY(cy, oy);
  mptr->image = random() % MONSTER_IMAGES;
  mptr->cycle = random() % MONSTER_CYCLES;
  mptr->shot = 0;
  mptr->old_sprite = 0;
  return mptr;
}
/*}}}*/
