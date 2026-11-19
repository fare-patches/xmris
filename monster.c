/* Copyright (C) 1993, 1992 Nathan Sidwell */
/* RCS $Id: monster.c 1.2 Tue, 16 Mar 1999 11:28:16 +0000 nathan $ */
#include "xmris.h"
/*{{{  MONSTER *extra_escape()*/
extern MONSTER *extra_escape FUNCARGVOID
/*
 * remove the extra monster from the top, so it
 * can run around
 */
{
  int       x;
  
  extra.escape = 1;
  x = PIXELX(4, extra.select * XTRA_SPACING);
  video_fill(display.back, GCN(GC_CLEAR),
      x, PIXELY(-1, 0), CELL_WIDTH, CELL_HEIGHT);
  add_background(x, PIXELY(-1, 0), CELL_WIDTH, CELL_HEIGHT);
  x -= BORDER_LEFT + GAP_WIDTH;
  return spawn_monster(1, 2, 1, 1, x / (CELL_WIDTH + GAP_WIDTH), 0,
      x % (CELL_WIDTH + GAP_WIDTH), -CELL_HEIGHT);
}
/*}}}*/
/*{{{  void extra_dies()*/
extern VOIDFUNC extra_dies FUNCARGVOID
/*
 * the extra monster has died,
 * put it back at the top
 * and maybe alter the state
 */
{
  unsigned  got;
  
  if(global.state == 2)
    {
      color_set(BACKGROUND_NORMAL);
      global.state = 3;
      monster.den = 0;
      monster.delay = 0;
    }
  got = extra.got & (1 << extra.select);
  extra.got |= 1 << extra.select;
  extra.escape = 0;
  extra.count = FRAMES_PER_SECOND - 1;
  if(!got)
    create_xtra_monster(extra.select);
  draw_extra();
  return;
}
/*}}}*/
/*{{{  void move_monsters()*/
extern VOIDFUNC move_monsters FUNCARGVOID
/*
 * move all the monsters
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
      if(mptr->type == 5 || mptr->squished)
	/* EMPTY */;
      else if(mptr->shot || (mptr->type == 3 && global.state == 3))
	/*{{{  shot*/
	{
	  if(BOARDCELL(mptr->cell.x, mptr->cell.y)->distance ==
	      monster.nearest)
	    global.difficulty++;
	  if(mptr->type < 2)
	    monster.normals--;
	  else if(mptr->type < 4)
	    /*{{{  convert to apple*/
	    {
	      APPLE     *aptr;
	    
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
	      aptr = spawn_apple(mptr->cell.x, mptr->cell.y,
		  mptr->offset.x, mptr->offset.y);
	      if(mptr->type == 3 && global.state == 3)
		aptr->ghost = APPLE_GHOST_DELAY;
	    }
	    /*}}}*/
	  else
	    global.diamond = 2;
	  if(mptr->shot)
	    add_score(mptr->type == 6 ?
		(unsigned)(8000 / SCORE_ROUND) : (unsigned)(500 / SCORE_ROUND),
		mptr->pixel.x + CELL_WIDTH / 2,
		mptr->pixel.y + CELL_HEIGHT / 2);
	  mptr->type = 5;
	}
	/*}}}*/
      else if(mptr->type == 6)
	/*{{{  diamond*/
	{
	  /*{{{  new image?*/
	  if(!mptr->cycle)
	    {
	      mptr->cycle = DIAMOND_CYCLES;
	      mptr->image++;
	      if(mptr->image == DIAMOND_IMAGES)
		mptr->image = 0;
	    }
	  /*}}}*/
	  mptr->cycle--;
	  if(!mptr->count--)
	    {
	      global.diamond = 0;
	      mptr->type = 5;
	    }
	  if(mptr->count == DIAMOND_GHOSTING)
	    mptr->ghosting = GHOSTING;
	  if(!mptr->stop)
	    move_diamond(mptr, BOARDCELL(mptr->cell.x, mptr->cell.y));
	}
	/*}}}*/
      else if(global.state != 4)
	{
	  CELL      *cptr;

	  cptr = BOARDCELL(mptr->cell.x, mptr->cell.y);
	  assert(cptr->visit);
	  /*{{{  new image?*/
	  if(!mptr->cycle)
	    {
	      mptr->cycle = MONSTER_CYCLES;
	      mptr->image++;
	      if(mptr->image == MONSTER_IMAGES)
		mptr->image = 0;
	    }
	  /*}}}*/
	  if((!mptr->count || mptr->type & 2) && !mptr->pause)
	    mptr->cycle--;
	  /*{{{  set nearest and farthest*/
	  if(mptr->type & 2 || global.state != 2)
	    {
	      if(nearest > cptr->distance)
		nearest = cptr->distance;
	      if(farthest < cptr->distance)
		farthest = cptr->distance;
	    }
	  /*}}}*/
	  if(mptr->type & 2 && !mptr->chew)
	    /*{{{  xtra or drone*/
	    {
	      if(monster.delay && mptr->type == 2 && global.state == 2 &&
		  (mptr->cell.y || mptr->offset.y >= 0) &&
		  !(mptr->offset.x % VEL_X))
		/*{{{  give birth*/
		{
		  monster.delay--;
		  if(!monster.delay && monster.den)
		    {
		      monster.delay = XTRA_BIRTH_DELAY;
		      monster.den--;
		      spawn_monster(2, 3, mptr->dir, mptr->face,
			  mptr->cell.x, mptr->cell.y,
			  mptr->offset.x, mptr->offset.y);
		      monster.drones++;
		      i++;
		    }
		}
		/*}}}*/
	      else
		{
		  unsigned  valid;
		  
		  if(extra.escape == 2 && mptr->pixel.y < PIXELY(0, 0))
		    valid = mptr->offset.y == -(CELL_HEIGHT + GAP_HEIGHT) ?
			0x0 : 0x1;
		  else
		    {
		      valid = valid_directions(mptr, cptr);
		      /*{{{  near an apple?*/
		      {
			unsigned  i;
			APPLE     *aptr;
			int       x, y;
				      
			x = mptr->pixel.x;
			y = mptr->pixel.y;
			for(aptr = apple.list, i = apple.apples; i--; aptr++)
			  {
			    if(!aptr->monsters && aptr->state < 3 &&
				!aptr->chewed &&
				INRANGE(aptr->pixel.x - x, VEL_X - CELL_WIDTH,
				    CELL_WIDTH - VEL_X + 1) &&
				INRANGE(aptr->pixel.y - y, VEL_Y - CELL_HEIGHT,
				    CELL_HEIGHT - VEL_Y + 1))
			      {
				if((mptr->dir & 2 ?
				    INRANGE(aptr->pixel.x - x,
				      -VEL_X, VEL_X + 1) :
				    INRANGE(aptr->pixel.x - x,
				      -VEL_X * 3, VEL_X * 3 + 1)) &&
				    INRANGE(aptr->pixel.y - y, -VEL_Y,
				      aptr->state == 2 ?
					VEL_Y + APPLE_VEL_Y + 1 : VEL_Y + 1) &&
				    (aptr->state < 2 || !mptr->dir ||
				    aptr->pixel.y >= y))
				  {
				    mptr->chew = 1;
				    aptr->chewed = 1;
				  }
				else
				  {
				    if(mptr->dir != 0 && valid & 0x2 &&
					aptr->pixel.y > y)
				      valid = 0x2;
				    else if(mptr->dir != 1 && valid & 0x1 &&
					aptr->pixel.y < y)
				      valid = 0x1;
				    else if(mptr->dir != 2 && valid & 0x8 &&
					aptr->pixel.x > x)
				      valid = 0x8;
				    else if(mptr->dir != 3 && valid & 0x4 &&
					aptr->pixel.x < x)
				      valid = 0x4;
				  }
				break;
			      }
			  }
		      }
		      /*}}}*/
		    }
		  if(mptr->offset.x || mptr->offset.y)
		    /* EMPTY */;
		  else if(extra.escape == 2)
		    mptr->fast = 0;
		  else if(global.state == 3)
		    mptr->fast = 1;
		  if(!valid)
		    {
		      mptr->type = 5;
		      extra.escape = 0;
		      extra.count = FRAMES_PER_SECOND - 1;
		      draw_extra();
		    }
		  else
		    /*{{{  pick a direction*/
		    {
		      unsigned  temp;
		    
		      if(player.ball.state == 1 && cptr->ball)
			/*{{{  run away*/
			{
			  temp = 0;
			  if(mptr->offset.y <= 0 &&
			      cptr[-CELL_STRIDE].ball < cptr->ball)
			    temp |= 0x1;
			  if(mptr->offset.y >= 0 &&
			      cptr[CELL_STRIDE].ball < cptr->ball)
			    temp |= 0x2;
			  if(mptr->offset.x <= 0 &&
			      cptr[-1].ball < cptr->ball)
			    temp |= 0x4;
			  if(mptr->offset.x >= 0 &&
			      cptr[1].ball < cptr->ball)
			    temp |= 0x8;
			  if(temp & valid)
			    valid &= temp | (temp << 4);
			}
			/*}}}*/
		      temp = valid & (~(0x11 << (mptr->dir ^ 1)));
		      if(mptr->type == 2)
			/*{{{  xtra*/
			{
			  if((player.ball.state != 0) != mptr->gomunch)
			    temp = valid;
			  if(player.ball.state ||
			      (global.state == 2 ?
				mptr->count == monster.farthest :
				mptr->count != monster.nearest))
			    /*{{{  go towards*/
			    {
			      if(extra.escape == 2)
				temp &= 0xF;
			      if(temp & 0xF0)
				valid = temp >> 4;
			      else if(temp)
				valid = temp;
			      else
				valid &= 0xF;
			    }
			    /*}}}*/
			  else
			    /*{{{  run away*/
			    {
			      unsigned  suess;
					
			      suess = (temp ^ (temp >> 4)) & 0xF;
			      if(suess)
				valid = suess;
			      else if(temp & 0xF)
				valid = temp & 0xF;
			      else
				valid &= 0xF;
			    }
			    /*}}}*/
			  if(extra.escape == 2)
			    {
			      if(mptr->pixel.y <= PIXELY(0, 0) &&
				  mptr->pixel.x ==
				  PIXELX(4, extra.select * XTRA_SPACING))
				valid = 0x1;
			      else
				/*{{{  run home*/
				{
				  unsigned  preferred;
				  
				  preferred = run_home(mptr, cptr);
				  if(preferred & valid)
				    valid &= preferred;
				}
				/*}}}*/
			    }
			}
			/*}}}*/
		      else
			/*{{{  drone*/
			{
			  if(temp & 0xF0)
			    valid = temp >> 4;
			  else if(temp)
			    valid = temp;
			  else if(valid & 0xF0)
			    valid >>= 4;
			  else
			    valid &= 0xF;
			}
			/*}}}*/
		      valid = choose_direction(valid);
		      if(mptr->pixel.y == PIXELY(0, 0) && mptr->dir == 1)
			{
			  int       x;
			  
			  x = (mptr->offset.x + (valid & 1 ? 0 : 3)) / 
			      VEL_X * VEL_X;
			  mptr->pixel.x += x - mptr->offset.x;
			  mptr->offset.x = x;
			}
		      if(valid != mptr->dir)
			{
			  mptr->dir = valid;
			  new_face(mptr);
			}
		      mptr->count = cptr->distance;
		      cptr = move_movable(mptr, cptr);
		      if(mptr->pixel.y >= PIXELY(0, 0))
			/*{{{  walked into apple?*/
			{
			  unsigned  i;
			  APPLE     *aptr;
			  int       x, y;
			  int       width, height;
					
			  x = mptr->pixel.x;
			  y = mptr->pixel.y;
			  /*{{{  set offset*/
			  switch(mptr->dir)
			  {
			    /*{{{  case 0:*/
			    case 0:
			    {
			      x -= VEL_X - 1;
			      y -= APPLE_VEL_Y - 1;
			      width = VEL_X * 2 - 1;
			      height = VEL_Y + APPLE_VEL_Y * 2 - 1;
			      break;
			    }
			    /*}}}*/
			    /*{{{  case 1:*/
			    case 1:
			    {
			      x -= VEL_X - 1;
			      width = VEL_X * 2 - 1;
			      height = VEL_Y + APPLE_VEL_Y;
			      break;
			    }
			    /*}}}*/
			    /*{{{  case 2:*/
			    case 2:
			    {
			      x -= VEL_X + APPLE_VEL_X - 1;
			      y -= VEL_Y - 1;
			      width = (VEL_X + APPLE_VEL_X) * 2 - 1;
			      height = APPLE_VEL_Y + VEL_Y;
			      break;
			    }
			    /*}}}*/
			    /*{{{  case 3:*/
			    case 3:
			    {
			      x -= VEL_X + APPLE_VEL_X - 1;
			      y -= VEL_Y - 1;
			      width = (VEL_X + APPLE_VEL_X) * 2 - 1;
			      height = APPLE_VEL_Y + VEL_Y;
			      break;
			    }
			    /*}}}*/
			    /*{{{  default: inhibit optimizer warning*/
			    default:
			    {
			      width = height = 0;
			      break;
			    }
			    /*}}}*/
			  }
			  /*}}}*/
			  for(aptr = apple.list, i = apple.apples; i--; aptr++)
			    {
			      if(aptr->state < 3 && !aptr->chewed &&
				  INRANGE(aptr->pixel.x - x, 0, width) &&
				  INRANGE(aptr->pixel.y - y, 0, height))
				{
				  if(aptr->monsters)
				    {
				      assert(aptr->list && aptr->state == 2);
				      mptr->list = aptr->list;
				      aptr->list = mptr;
				      squish_monster(mptr);
				      mptr->pixel.y +=
					CELL_HEIGHT - CELL_HEIGHT / 4;
				      aptr->monsters++;
				    }
				  else
				    {
				      mptr->chew = 1;
				      aptr->chewed = 1;
				    }
				  break;
				}
			    }
			}
			/*}}}*/
		      /*{{{  go home adjust*/
		      if(extra.escape == 2 && mptr->type == 2 &&
			  mptr->pixel.y == PIXELY(0, 0) && mptr->dir & 2)
			{
			  int       x;
			  
			  x = PIXELX(4, extra.select * XTRA_SPACING) -
			      mptr->pixel.x;
			  if(mptr->dir & 1 ? x < 0 && x > -VEL_X :
			      x > 0 && x < VEL_X)
			    {
			      mptr->pixel.x += x;
			      mptr->offset.x += x;
			    }
			}
		      /*}}}*/
		      mptr->gomunch = player.ball.state != 0;
		    }
		    /*}}}*/
		}
	    }
	    /*}}}*/
	  else
	    /*{{{  normal or muncher or chewing*/
	    {
	      if(global.state == 2 && !mptr->push && !mptr->chew)
		{
		  if(mptr->count)
		    mptr->count--;
		  else
		    mptr->count = 1;
		}
	      else
		{
		  unsigned  valid;
		  unsigned  pause;
		  
		  pause = 0;
		  if(mptr->chew == 1)
		    /*{{{  chewing*/
		    {
		      mptr->chew = 2;
		      mptr->count = CHOMP_DELAY;
		      mptr->image = 0;
		      mptr->cycle = MONSTER_CYCLES - 1;
		    }
		    /*}}}*/
		  valid = valid_directions(mptr, cptr);
		  if(mptr->type || mptr->cont)
		    /* EMPTY */;
		  else if((mptr->pause || mptr->stop) &&
		      chaotic() < (mptr->dir & 2 ? 0 : (0x10 << mptr->dir) &
		      valid ? global.screen / STOP_TOGGLE_CONT_NEAR : 
		      STOP_TOGGLE_CONT_STOP) + STOP_TOGGLE_CONT_PEDESTAL +
		      global.screen / STOP_TOGGLE_CONT_SCREEN_SCALE)
		    /*{{{  set cont & turn round*/
		    {
		      mptr->cont = 1;
		      mptr->dir ^= 1;
		      new_face(mptr);
		    }
		    /*}}}*/
		  else if(!cptr->distance)
		    mptr->cont = 1;
		  if(mptr->push)
		    /*{{{  disable left or right*/
		    {
		      if(mptr->push < 0)
			valid &= 0x77;
		      else
			valid &= 0xBB;
		      if(mptr->offset.y >= 0)
			{
			  if(valid & 0x2)
			    {
			      valid &= ~0x11;
			      mptr->cont = 1;
			    }
			}
		      else
			{
			  if(valid & 0x1)
			    {
			      valid &= ~0x22;
			      mptr->cont = 1;
			    }
			}
		    }
		    /*}}}*/
		  else if(mptr->count)
		    pause = 1;
		  else if(mptr->pause || mptr->stop)
		    /*{{{  test go munch or turn round*/
		    {
		      if(!mptr->type)
			{
			  if(chaotic() < GO_MUNCH_PROB * global.difficulty)
			    mptr->gomunch = 1;
			}
		      else if(mptr->stop || chaotic() < PUSH_TURN_PROB)
			{
			  mptr->dir ^= 1;
			  new_face(mptr);
			  mptr->count = GO_MUNCH_DELAY;
			}
		      valid = 0;
		      mptr->stop = 0;
		      mptr->pause = 0;
		    }
		    /*}}}*/
		  else if(!mptr->offset.x && !mptr->offset.y)
		    /*{{{  intersection stuff*/
		    {
		      mptr->fast = !mptr->type && global.state == 3;
		      if(mptr->gomunch || (!cptr->distance && 
			  !(valid & 1 << mptr->dir) &&
			  chaotic() < TRAPPED_MUNCH_PROB))
			/*{{{  start munching?*/
			{
			  unsigned  temp;
					      
			  mptr->panic = 0;
			  temp = valid & 0xF;
			  if(!mptr->cell.y)
			    temp |= 0x1;
			  else if(mptr->cell.y == CELLS_DOWN - 1)
			    temp |= 0x2;
			  if(!mptr->cell.x)
			    temp |= 0x4;
			  else if(mptr->cell.x == CELLS_ACROSS - 1)
			    temp |= 0x8;
			  if(temp != 0xF)
			    {
			      mptr->type = 1;
			      mptr->count = GO_MUNCH_DELAY;
			      mptr->gomunch = 0;
			    }
			}
			/*}}}*/
		      else if(mptr->type)
			/*{{{  stop munching?*/
			{
			  int       temp;
					
			  temp = valid & 0xF;
			  if((temp & -temp) != temp && (temp == 0xF ||
			      chaotic() >= MUNCH_CONT_PEDESTAL +
			      global.screen / MUNCH_CONT_SCREEN_SCALE))
			    {
			      mptr->type = 0;
			      mptr->count = STOP_MUNCH_DELAY;
			      valid = 0;
			    }
			}
			/*}}}*/
		      else if(!mptr->type)
			/*{{{  cont & gomunch stuff*/
			{
			  if(chaotic() < GO_MUNCH_PROB * global.difficulty)
			    mptr->gomunch = 1;
			  if(mptr->cont)
			    {
			      if(chaotic() < CLEAR_CONT_PEDESTAL +
				  global.visited / CLEAR_CONT_VISIT_SCALE +
				  (monster.den ? 0 : global.screen /
				  (monster.normals * monster.normals) *
				  CLEAR_CONT_MONSTER_SCALE))
				mptr->cont = 0;
			    }
			  else
			    {
			      if(chaotic() < SET_CONT_PEDESTAL)
				mptr->cont = 1;
			    }
			}
			/*}}}*/
		    }
		    /*}}}*/
		  if(mptr->count)
		    mptr->count--;
		  if(pause)
		    /* EMPTY */;
		  else if(mptr->type == 1)
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
			  if(mptr->cell.x < monster.list[0].cell.x)
			    valid = 0x8;
			  else if(mptr->cell.x > monster.list[0].cell.x)
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
		  else if(valid)
		    /*{{{  pick a direction*/
		    {
		      unsigned  temp;
				    
		      if(mptr->chew)
			valid &= 0xF;
		      else if(mptr->panic)
			/*{{{  panic mode*/
			{
			  temp = valid & ((1 << (mptr->dir ^ 1)) ^ 0xF);
			  if(mptr->offset.x < VEL_X * 4 - CELL_WIDTH &&
			      valid & 0x8)
			    valid = 0x8;
			  else if(mptr->offset.x > CELL_WIDTH - VEL_X * 4 &&
			      valid & 0x4)
			    valid = 0x4;
			  else if(temp & 0xC)
			    valid = temp & 0xC;
			  else if(valid & 0xC)
			    valid &= 0xC;
			  else if(valid & 0x2 && (mptr->dir == 1 ||
			      mptr->offset.y < VEL_Y - CELL_HEIGHT / 2))
			    valid = 0x2;
			  else if(valid & 1)
			    valid = 0x1;
			  else
			    valid &= 0xF;
			  mptr->panic++;
			  if(mptr->panic > PANIC_COUNT)
			    mptr->panic = 0;
			}
			/*}}}*/
		      else
			{
			  if(player.ball.state == 1 && cptr->ball)
			    /*{{{  run away*/
			    {
			      temp = 0;
			      if(mptr->offset.y <= 0 &&
				  cptr[-CELL_STRIDE].ball < cptr->ball)
				temp |= 0x1;
			      if(mptr->offset.y >= 0 &&
				  cptr[CELL_STRIDE].ball < cptr->ball)
				temp |= 0x2;
			      if(mptr->offset.x <= 0 &&
				  cptr[-1].ball < cptr->ball)
				temp |= 0x4;
			      if(mptr->offset.x >= 0 &&
				  cptr[1].ball < cptr->ball)
				temp |= 0x8;
			      if(temp & valid)
				valid &= temp | (temp << 4);
			    }
			    /*}}}*/
			  if(mptr->cont || mptr->gomunch)
			    /*{{{  continue mode*/
			    {
			      valid &= 0xF;
			      temp = valid & ~(1 << (mptr->dir ^ 1));
			      if(temp)
				valid = temp;
			    }
			    /*}}}*/
			  else if(valid & 0xF0)
			    valid = valid & valid >> 4;
			}
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
		  if(!mptr->count)
		    mptr->chew = 0;
		}
	    }
	    /*}}}*/
	}
    }
  monster.nearest = nearest;
  monster.farthest = farthest;
  return;
}
/*}}}*/
/*{{{  void new_xtra()*/
extern VOIDFUNC new_xtra FUNCARGVOID
/*
 * increment the extra monster, and draw it up
 */
{
  static int direction = 1;
  
  draw_extra_letter(extra.select);
  if(direction < 0 ? extra.select == 0 : extra.select == 4)
    direction = -direction;
  extra.select += direction;
  create_xtra_monster(extra.select);
  extra.count = extra.got & (1 << extra.select) ?
      XTRA_GOT_DELAY - 1 : XTRA_NEW_DELAY - 1;
  draw_extra();
  return;
}
/*}}}*/
/*{{{  MONSTER *spawn_monster(insert, type, dir, face, cx, cy, ox, oy)*/
extern MONSTER *spawn_monster
FUNCARG((insert, type, dir, face, cx, cy, ox, oy),
	unsigned  insert /* where to insert in list (0 for end) */
ARGSEP  unsigned  type  /* type of monster 0-4 */
ARGSEP  unsigned  dir   /* direction 0-3 */
ARGSEP  unsigned  face  /* face 0-5 */
ARGSEP  int       cx    /* cell x */
ARGSEP  int       cy    /* cell y */
ARGSEP  int       ox    /* offset x */
ARGSEP  int       oy    /* offset y */
)
/*
 * create a new monster onto the monster list
 * if insert != 0 the insert the monster at that point on the
 * list, adjusting the monster chains appropriately
 * returns a pointer to the new monster
 */
{
  MONSTER   *mptr;

  assert(monster.monsters != MONSTERS);
  assert((!(ox % VEL_X) || (oy < 0 && !cy)) && !(oy % VEL_Y));
  if(insert)
    {
      MONSTER   *sptr;
      APPLE     *aptr;
      unsigned  ix;
      
      assert(insert <= monster.monsters);
      mptr = &monster.list[insert];
      for(sptr = &monster.list[monster.monsters]; sptr-- != mptr;)
	{
	  if(sptr->list && sptr->list >= mptr)
	    sptr->list++;
	  memcpy(sptr + 1, sptr, sizeof(MONSTER));
	}
      for(aptr = apple.list, ix = apple.apples; ix--; aptr++)
	if(aptr->list && aptr->list >= mptr)
	  aptr->list++;
      monster.monsters++;
    }
  else
    mptr = &monster.list[monster.monsters++];
  mptr->dir = dir;
  mptr->type = type;
  mptr->face = face;
  mptr->push = 0;
  mptr->gomunch = mptr->cont = mptr->chew = mptr->pause = mptr->stop = 0;
  mptr->panic = mptr->shot = mptr->squished = 0;
  mptr->fast = mptr->pushing = 0;
  mptr->ghosting = 0;
  mptr->count = 0;
  mptr->cell.x = cx;
  mptr->cell.y = cy;
  mptr->offset.x = ox;
  mptr->offset.y = oy;
  mptr->old_pixel.x = mptr->pixel.x = PIXELX(cx, ox);
  mptr->old_pixel.y = mptr->pixel.y = PIXELY(cy, oy);
  mptr->list = NULL;
  mptr->image = chaotic() % MONSTER_IMAGES;
  mptr->cycle = chaotic() % MONSTER_CYCLES;
  mptr->old_sprite = 0;
  mptr->back = 0;
  mptr->on = 1;
  return mptr;
}
/*}}}*/
