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
/*{{{  void bounce_ball()*/
extern void bounce_ball FUNCARGVOID
/*
 * throws, bounces, explodes & implodes the ball
 * note that show_updates, does the ball ploting
 */
{
  switch(player.ball.state)
  {
    /*{{{  case 0: (held)*/
    case 0:
      if(player.throw == 1)
	{
	  CELL      *cptr;
	  unsigned  face;
	
	  player.throw = 2;
	  player.ball.state = 1;
	  face = monster.list[0].face;
	  player.ball.cell.x = monster.list[0].cell.x;
	  player.ball.cell.y = monster.list[0].cell.y;
	  player.ball.offset.x = monster.list[0].offset.x + ball_throw[face].x;
	  player.ball.offset.y = monster.list[0].offset.y + ball_throw[face].y;
	  player.ball.count = ball_dir[face];
	  cptr = BOARDCELL(player.ball.cell.x, player.ball.cell.y);
	  switch(monster.list[0].dir)
	  {
	    /*{{{  case 0: (up)*/
	    case 0:
	      if(player.ball.offset.y > -(CELL_HEIGHT / 2))
		/*EMPTY*/;
	      else if(cptr[-CELL_STRIDE].visit)
		{
		  player.ball.offset.y += CELL_HEIGHT + GAP_HEIGHT;
		  player.ball.cell.y--;
		}
	      else if(cptr[-CELL_STRIDE * 2].depths[1] - player.ball.offset.y >
		  CELL_HEIGHT / 2 + CELL_HEIGHT + GAP_HEIGHT)
		{
		  player.ball.offset.y += (CELL_HEIGHT + GAP_HEIGHT) * 2;
		  player.ball.cell.y -= 2;
		}
	      else if(player.ball.offset.y + CELL_HEIGHT / 2 <
		  cptr[0].depths[0])
		player.ball.state = 0;
	      break;
	    /*}}}*/
	    /*{{{  case 1: (down)*/
	    case 1:
	      if(player.ball.offset.y < (CELL_HEIGHT / 2))
		/*EMPTY*/;
	      else if(cptr[CELL_STRIDE].visit)
		{
		  player.ball.offset.y -= CELL_HEIGHT + GAP_HEIGHT;
		  player.ball.cell.y++;
		}
	      else if(player.ball.offset.y - cptr[CELL_STRIDE * 2].depths[0] >
		  CELL_HEIGHT / 2 + CELL_HEIGHT + GAP_HEIGHT)
		{
		  player.ball.offset.y -= (CELL_HEIGHT + GAP_HEIGHT) * 2;
		  player.ball.cell.y += 2;
		}
	      else if(player.ball.offset.y - CELL_HEIGHT / 2 >
		  cptr[0].depths[1])
		player.ball.state = 0;
	      break;
	    /*}}}*/
	    /*{{{  case 2: (left)*/
	    case 2:
	      if(player.ball.offset.x > -(CELL_WIDTH / 2))
		/*EMPTY*/;
	      else if(cptr[-1].visit)
		{
		  player.ball.offset.x += CELL_WIDTH + GAP_WIDTH;
		  player.ball.cell.x--;
		}
	      else if(cptr[-2].depths[3] - player.ball.offset.x >
		  CELL_WIDTH / 2 + CELL_WIDTH + GAP_WIDTH)
		{
		  player.ball.offset.x += (CELL_WIDTH + GAP_WIDTH) * 2;
		  player.ball.cell.x -= 2;
		}
	      else if(player.ball.offset.x + CELL_WIDTH / 2 <
		  cptr[0].depths[2])
		player.ball.state = 0;
	      break;
	    /*}}}*/
	    /*{{{  case 3: (right)*/
	    case 3:
	      if(player.ball.offset.x < (CELL_HEIGHT / 2))
		/*EMPTY*/;
	      else if(cptr[1].visit)
		{
		  player.ball.offset.x -= CELL_WIDTH + GAP_WIDTH;
		  player.ball.cell.x++;
		}
	      else if(player.ball.offset.x - cptr[2].depths[2] >
		  CELL_WIDTH / 2 + CELL_WIDTH + GAP_WIDTH)
		{
		  player.ball.offset.x -= (CELL_WIDTH + GAP_WIDTH) * 2;
		  player.ball.cell.x += 2;
		}
	      else if(player.ball.offset.x - CELL_HEIGHT / 2 >
		  cptr[0].depths[3])
		player.ball.state = 0;
	      break;
	    /*}}}*/
	  }
	  player.ball.pixel.x =
	      PIXELX(player.ball.cell.x, player.ball.offset.x) + CELL_WIDTH / 2;
	  player.ball.pixel.y =
	      PIXELY(player.ball.cell.y, player.ball.offset.y) + CELL_HEIGHT / 2;
	}
      break;
    /*}}}*/
    /*{{{  case 1: (bounce)*/
    case 1:
    {
      int       i;
      CELL      *cptr;
	
      cptr = BOARDCELL(player.ball.cell.x, player.ball.cell.y);
      for(i = BALL_STEPS; i--;)
	{
	  int       walls;
	  /* bit vector tl, br, bl, tr, t, b, l, r */
	
	  /*{{{  moved to above or below cell?*/
	  if(player.ball.offset.y > CELL_HEIGHT / 2 &&
	      cptr[CELL_STRIDE].depths[0])
	    {
	      player.ball.offset.y -= CELL_HEIGHT + GAP_HEIGHT;
	      player.ball.cell.y++;
	      cptr += CELL_STRIDE;
	    }
	  else if(player.ball.offset.y < -CELL_HEIGHT / 2 &&
	      cptr[-CELL_STRIDE].depths[1])
	    {
	      player.ball.offset.y += CELL_HEIGHT + GAP_HEIGHT;
	      player.ball.cell.y--;
	      cptr -= CELL_STRIDE;
	    }
	  /*}}}*/
	  /*{{{  moved to left or right cell?*/
	  if(player.ball.offset.x > CELL_WIDTH / 2 && cptr[1].depths[2])
	    {
	      player.ball.offset.x -= CELL_WIDTH + GAP_WIDTH;
	      player.ball.cell.x++;
	      cptr++;
	    }
	  else if(player.ball.offset.x < -CELL_WIDTH / 2 && cptr[-1].depths[3])
	    {
	      player.ball.offset.x += CELL_WIDTH + GAP_WIDTH;
	      player.ball.cell.x--;
	      cptr--;
	    }
	  /*}}}*/
	  walls = 0x00;
	  /*{{{  set the maze walls*/
	  {
	    unsigned  quadrant;
	    /* quadrants to, bo, lo, ro, ti, bi, li, ri, t, b, l, r */
		    
	    /* work out where we are in the cell */
	    quadrant = 0x000;
	    /*{{{  y place*/
	    if(player.ball.offset.y < -CELL_HEIGHT / 2 - VEL_Y)
	      quadrant |= 0x008;
	    else if(player.ball.offset.y < -CELL_HEIGHT / 2)
	      quadrant |= 0x800;
	    else if(player.ball.offset.y < -CELL_HEIGHT / 2 + VEL_Y)
	      quadrant |= 0x080;
	    else if(player.ball.offset.y >= CELL_HEIGHT / 2 + VEL_Y)
	      quadrant |= 0x004;
	    else if(player.ball.offset.y >= CELL_HEIGHT / 2)
	      quadrant |= 0x400;
	    else if(player.ball.offset.y >= CELL_HEIGHT / 2 - VEL_Y)
	      quadrant |= 0x040;
	    /*}}}*/
	    /*{{{  x place*/
	    if(player.ball.offset.x < -CELL_WIDTH / 2 - VEL_X)
	      quadrant |= 0x002;
	    else if(player.ball.offset.x < -CELL_WIDTH / 2)
	      quadrant |= 0x200;
	    else if(player.ball.offset.x < -CELL_WIDTH / 2 + VEL_X)
	      quadrant |= 0x020;
	    else if(player.ball.offset.x >= CELL_WIDTH / 2 + VEL_X)
	      quadrant |= 0x001;
	    else if(player.ball.offset.x >= CELL_WIDTH / 2)
	      quadrant |= 0x100;
	    else if(player.ball.offset.x >= CELL_WIDTH / 2 - VEL_X)
	      quadrant |= 0x010;
	    /*}}}*/
	    if(quadrant & 0x888)
	      /*{{{  above cell*/
	      {
		if(player.ball.offset.y <=
		    cptr[0].depths[0] - CELL_HEIGHT / 2 + VEL_Y)
		  walls |= 0x98;
		if(quadrant & 0x808)
		  {
		    unsigned  mask;
				  
		    if(quadrant & 0x800)
		      mask = 0x93;
		    else
		      mask = 0xF3;
		    if(quadrant & 0x020)
		      walls |= mask & 0xA2;
		    if(quadrant & 0x010)
		      walls |= mask & 0x51;
		  }
		else
		  {
		    if(quadrant & 0x020)
		      {
			if(cptr[0].depths[0] >= -VEL_Y ||
			    cptr[0].depths[2] >= -VEL_X ||
			    cptr[-1].depths[0] >= -VEL_Y ||
			    cptr[-CELL_STRIDE].depths[2] >= -VEL_X)
			  walls |= 0x80;
		      }
		    else if(quadrant & 0x010)
		      {
			if(cptr[0].depths[0] >= -VEL_Y ||
			    cptr[0].depths[3] <= VEL_X ||
			    cptr[1].depths[0] >= -VEL_Y ||
			    cptr[-CELL_STRIDE].depths[3] <= VEL_X)
			  walls |= 0x10;
		      }
		  }
	      }
	      /*}}}*/
	    else if(quadrant & 0x444)
	      /*{{{  below cell*/
	      {
		if(player.ball.offset.y >=
		    cptr[0].depths[1] + CELL_HEIGHT / 2 - VEL_Y)
		  walls |= 0x64;
		if(quadrant & 0x404)
		  {
		    unsigned  mask;
				  
		    if(quadrant & 0x400)
		      mask = 0x63;
		    else
		      mask = 0xF3;
		    if(quadrant & 0x020)
		      walls |= mask & 0xA2;
		    if(quadrant & 0x010)
		      walls |= mask & 0x51;
		  }
		else
		  {
		    if(quadrant & 0x020)
		      {
			if(cptr[0].depths[1] <= VEL_Y ||
			    cptr[0].depths[2] >= -VEL_X ||
			    cptr[-1].depths[1] <= VEL_Y ||
			    cptr[CELL_STRIDE].depths[2] >= -VEL_X)
			  walls |= 0x20;
		      }
		    else if(quadrant & 0x010)
		      {
			if(cptr[0].depths[1] <= VEL_Y ||
			    cptr[0].depths[3] <= VEL_X ||
			    cptr[1].depths[1] <= VEL_Y ||
			    cptr[CELL_STRIDE].depths[3] <= VEL_X)
			  walls |= 0x40;
		      }
		  }
	      }
	      /*}}}*/
	    if(quadrant & 0x222)
	      /*{{{  left cell*/
	      {
		if(player.ball.offset.x <=
		    cptr[0].depths[2] - CELL_WIDTH / 2 + VEL_X)
		  walls |= 0xA2;
		if(quadrant & 0x202)
		  {
		    unsigned  mask;
				  
		    if(quadrant & 0x200)
		      mask = 0xAC;
		    else
		      mask = 0xFC;
		    if(quadrant & 0x080)
		      walls |= mask & 0x98;
		    if(quadrant & 0x040)
		      walls |= mask & 0x64;
		  }
	      }
	      /*}}}*/
	    else if(quadrant & 0x111)
	      /*{{{  right cell*/
	      {
		if(player.ball.offset.x >=
		    cptr[0].depths[3] + CELL_WIDTH / 2 - VEL_X)
		  walls |= 0x51;
		if(quadrant & 0x101)
		  {
		    unsigned  mask;
				  
		    if(quadrant & 0x100)
		      mask = 0x5C;
		    else
		      mask = 0xFC;
		    if(quadrant & 0x080)
		      walls |= mask & 0x98;
		    if(quadrant & 0x040)
		      walls |= mask & 0x64;
		  }
	      }
	      /*}}}*/
	  }
	  /*}}}*/
	  /*{{{  set apple walls*/
	  {
	    unsigned  i;
	    APPLE     *aptr;
		    
	    for(aptr = apple.list, i = apple.apples; i--; aptr++)
	      {
		int     x, y;
		int     edge;
		/* edge ot, ob, ol, or, t, b, l, r */
			  
		if(aptr->state < 4)
		  {
		    x = aptr->pixel.x;
		    y = aptr->pixel.y;
		    edge = 0;
		    /*{{{  Y edge*/
		    if(player.ball.pixel.y - y < -VEL_Y ||
			player.ball.pixel.y - y >= CELL_HEIGHT + VEL_Y)
		      /*EMPTY*/;
		    else if(player.ball.pixel.y - y < 0)
		      edge |= 0X80;
		    else if(player.ball.pixel.y - y < CELL_HEIGHT / 2)
		      edge |= 0X08;
		    else if(player.ball.pixel.y - y < CELL_HEIGHT)
		      edge |= 0x04;
		    else
		      edge |= 0x40;
		    /*}}}*/
		    /*{{{  X edge*/
		    if(player.ball.pixel.x - x < -VEL_X ||
			player.ball.pixel.x - x >= CELL_WIDTH + VEL_X)
		      /*EMPTY*/;
		    else if(player.ball.pixel.x - x < 0)
		      edge |= 0X20;
		    else if(player.ball.pixel.x - x < CELL_WIDTH / 2)
		      edge |= 0X02;
		    else if(player.ball.pixel.x - x < CELL_WIDTH)
		      edge |= 0x01;
		    else
		      edge |= 0x10;
		    /*}}}*/
		    if(edge)
		      {
			/*{{{  top*/
			if(edge & 0X88 && edge & 0X33)
			  {
			    if(edge & 0X03)
			      walls |= 0X04;
			    if(edge & 0X23)
			      walls |= 0X40;
			    if(walls & 0X13)
			      walls |= 0X20;
			  }
			/*}}}*/
			/*{{{  bottom*/
			if(edge & 0X44 && edge & 0X33)
			  {
			    if(edge & 0X03)
			      walls |= 0X08;
			    if(edge & 0X23)
			      walls |= 0X10;
			    if(walls & 0X13)
			      walls |= 0X80;
			  }
			/*}}}*/
			/*{{{  left*/
			if(edge & 0X22 && edge & 0X0C)
			  {
			    walls |= 0X01;
			    if(edge & 0X8C)
			      walls |= 0X40;
			    if(walls & 0X4C)
			      walls |= 0X10;
			  }
			/*}}}*/
			/*{{{  right*/
			if(edge & 0X11 && edge & 0X0C)
			  {
			    walls |= 0X02;
			    if(edge & 0X8C)
			      walls |= 0X20;
			    if(walls & 0X4C)
			      walls |= 0X80;
			  }
			/*}}}*/
		      }
		  }
	      }
	  }
	  /*}}}*/
	  if((walls & 0xC) == 0xC || (walls & 0x03) == 0x03)
	    {
	      player.ball.state = 2;
	      player.ball.count = 0;
	      break;
	    }
	  else
	    {
	      /*{{{  bounce?*/
	      switch(player.ball.count)
	      {
		/*{{{  case 0: (up left)*/
		case 0:
		  if(!(walls & 0X80))
		    /*EMPTY*/;
		  else if((walls & 0X0A) == 0X08)
		    player.ball.count = 2;
		  else if((walls & 0X0A) == 0X02)
		    player.ball.count = 3;
		  else
		    player.ball.count = 1;
		  break;
		/*}}}*/
		/*{{{  case 1: (down right)*/
		case 1:
		  if(!(walls & 0X40))
		    /*EMPTY*/;
		  else if((walls & 0X05) == 0X04)
		    player.ball.count = 3;
		  else if((walls & 0X05) == 0X01)
		    player.ball.count = 2;
		  else
		    player.ball.count = 0;
		  break;
		/*}}}*/
		/*{{{  case 2: (down left)*/
		case 2:
		  if(!(walls & 0X20))
		    /*EMPTY*/;
		  else if((walls & 0X06) == 0X04)
		    player.ball.count = 0;
		  else if((walls & 0X06) == 0X02)
		    player.ball.count = 1;
		  else
		    player.ball.count = 3;
		  break;
		/*}}}*/
		/*{{{  case 3: (up right)*/
		case 3:
		  if(!(walls & 0X10))
		    /*EMPTY*/;
		  else if((walls & 0X09) == 0X08)
		    player.ball.count = 1;
		  else if((walls & 0X09) == 0X01)
		    player.ball.count = 0;
		  else
		    player.ball.count = 2;
		  break;
		/*}}}*/
	      }
	      /*}}}*/
	      /*{{{  move ball*/
	      switch(player.ball.count)
	      {
		/*{{{  case 0: (up left)*/
		case 0:
		  player.ball.offset.x -= VEL_X;
		  player.ball.offset.y -= VEL_Y;
		  player.ball.pixel.x -= VEL_X;
		  player.ball.pixel.y -= VEL_Y;
		  break;
		/*}}}*/
		/*{{{  case 1: (down right)*/
		case 1:
		  player.ball.offset.x += VEL_X;
		  player.ball.offset.y += VEL_Y;
		  player.ball.pixel.x += VEL_X;
		  player.ball.pixel.y += VEL_Y;
		  break;
		/*}}}*/
		/*{{{  case 2: (down left)*/
		case 2:
		  player.ball.offset.x -= VEL_X;
		  player.ball.offset.y += VEL_Y;
		  player.ball.pixel.x -= VEL_X;
		  player.ball.pixel.y += VEL_Y;
		  break;
		/*}}}*/
		/*{{{  case 3: (up right)*/
		case 3:
		  player.ball.offset.x += VEL_X;
		  player.ball.offset.y -= VEL_Y;
		  player.ball.pixel.x += VEL_X;
		  player.ball.pixel.y -= VEL_Y;
		  break;
		/*}}}*/
	      }
	      /*}}}*/
	    }
	}
      if(player.ball.state == 1)
	/*{{{  hit a monster?*/
	{
	  unsigned  i;
	  MONSTER   *mptr;
		
	  for(mptr = monster.list, i = monster.monsters; i--; mptr++)
	    if(!mptr->shot && !mptr->apple &&
		player.ball.pixel.x - mptr->pixel.x >= 0 &&
		player.ball.pixel.x - mptr->pixel.x < CELL_WIDTH &&
		player.ball.pixel.y - mptr->pixel.y >= 0 &&
		player.ball.pixel.y - mptr->pixel.y < CELL_HEIGHT)
	      {
		if(mptr == monster.list)
		  {
		    player.ball.state = 0;
		    player.ball.count = monster.list[0].face;
		    player.ball.pixel.x = monster.list[0].pixel.x +
			CELL_WIDTH / 2;
		    player.ball.pixel.y = monster.list[0].pixel.y +
			CELL_HEIGHT / 2;
		  }
		else
		  {
		    mptr->shot = 1;
		    player.ball.state = 2;
		    player.ball.count = 0;
		  }
		break;
	      }
	}
	/*}}}*/
      break;
    }
    /*}}}*/
    /*{{{  case 2: (explode)*/
    case 2:
      player.ball.count++;
      if(player.ball.count == BALL_EXPLODE)
	player.ball.state = 3;
      break;
    /*}}}*/
    /*{{{  case 3: (exploded)*/
    case 3:
      if(random() < 4)
	{
	  player.ball.state = 4;
	  player.ball.count--;
	}
      break;
    /*}}}*/
    /*{{{  case 4: (implode)*/
    case 4:
    {
      int       face;
	  
      player.ball.count--;
      face = monster.list[0].face;
      player.ball.pixel.x = monster.list[0].pixel.x + CELL_WIDTH / 2;
      player.ball.pixel.y = monster.list[0].pixel.y + CELL_HEIGHT / 2;
      if(!player.ball.count)
	player.ball.state = 0;
      break;
    }
    /*}}}*/
  }
  if(!player.ball.state)
    {
      player.ball.count = monster.list[0].face;
      player.ball.image = monster.list[0].image;
      player.ball.pixel.x = monster.list[0].pixel.x + CELL_WIDTH / 2;
      player.ball.pixel.y = monster.list[0].pixel.y + CELL_HEIGHT / 2;
    }
  return;
}
/*}}}*/
/*{{{  void move_player()*/
extern void move_player FUNCARGVOID
/*
 * moves the player
 * and deals with eating cherries etc
 */
{
  CELL      *cptr;
  unsigned  dir;

  dir = monster.list[0].dir;
  /*{{{  motionevent?*/
  if(player.motionevent && !player.keyboard &&
      (!monster.list[0].stop ||
       player.raw_mouse.x - player.mouse.x * (CELL_WIDTH + GAP_WIDTH) >
	  BORDER_LEFT + GAP_WIDTH / 2 + (CELL_WIDTH + GAP_WIDTH) / 4 +
	  CELL_WIDTH + GAP_WIDTH ||
       player.raw_mouse.y - (int)player.mouse.y * (CELL_HEIGHT + GAP_HEIGHT) >
	  BORDER_TOP + GAP_HEIGHT / 2 + (CELL_HEIGHT + GAP_HEIGHT) / 4 +
	  CELL_HEIGHT + GAP_HEIGHT ||
       player.raw_mouse.x - (int)player.mouse.x * (CELL_WIDTH + GAP_WIDTH) <
	  BORDER_LEFT + GAP_WIDTH / 2 - (GAP_WIDTH + CELL_WIDTH) / 4||
       player.raw_mouse.y - (int)player.mouse.y * (CELL_HEIGHT + GAP_HEIGHT) <
	  BORDER_TOP + GAP_HEIGHT / 2 - (GAP_HEIGHT + CELL_HEIGHT) / 4))
    /*
     * for mouse motion, we get the cell number for the mouse, and
     * set dir & mouse dir to point towards it
     */
    {
      int       x, y;
	
      x = (player.raw_mouse.x - BORDER_LEFT - GAP_WIDTH / 2) /
	  (GAP_WIDTH + CELL_WIDTH);
      y = (player.raw_mouse.y - BORDER_TOP - GAP_HEIGHT / 2) /
	  (GAP_HEIGHT + CELL_HEIGHT);
      /*{{{  clip x*/
      if(x < 0)
	x = 0;
      else if(x > CELLS_ACROSS - 1)
	x = CELLS_ACROSS - 1;
      /*}}}*/
      /*{{{  clip y*/
      if(y < 0)
	y = 0;
      else if(y > CELLS_DOWN - 1)
	y = CELLS_DOWN - 1;
      /*}}}*/
      if(y != player.mouse.y || x != player.mouse.x)
	{
	  unsigned  dir;
	  unsigned  second;
	
	  player.mouse.x = x;
	  player.mouse.y = y;
	  /*{{{  calc x direction*/
	  if(x == monster.list[0].cell.x && !monster.list[0].offset.x)
	    x = 0;
	  else if(x < monster.list[0].cell.x ||
	      (x == monster.list[0].cell.x && monster.list[0].offset.x > 0))
	    x = 6;
	  else
	    x = 7;
	  /*}}}*/
	  /*{{{  calc y direction*/
	  if(y == monster.list[0].cell.y && !monster.list[0].offset.y)
	    y = 0;
	  else if(y < monster.list[0].cell.y ||
	      (y == monster.list[0].cell.y && monster.list[0].offset.y > 0))
	    y = 4;
	  else
	    y = 5;
	  /*}}}*/
	  /*{{{  select which way round to do them*/
	  if(monster.list[0].dir & 2)
	    {
	      if(monster.list[0].dir != x)
		{
		  dir = y;
		  second = x;
		}
	      else
		{
		  dir = x;
		  second = y;
		}
	    }
	  else
	    {
	      if(monster.list[0].dir != y)
		{
		  dir = x;
		  second = y;
		}
	      else
		{
		  dir = y;
		  second = x;
		}
	    }
	  /*}}}*/
	  player.next_dir = 0;
	  /*{{{  do u turn fiddling*/
	  if(!dir)
	    dir = second;
	  else if(!second)
	    second = dir;
	  else if(monster.list[0].offset.x && !(dir & 2))
	    {
	      player.next_dir = dir;
	      if(monster.list[0].offset.x < -(CELL_WIDTH + GAP_WIDTH) * 3 / 4)
		dir = 6;
	      else if(monster.list[0].offset.x > (CELL_WIDTH + GAP_WIDTH) * 3 / 4)
		dir = 7;
	      else if(monster.list[0].offset.x > 0 &&
		  monster.list[0].offset.x <= (CELL_WIDTH + GAP_WIDTH) / 4)
		dir = 6;
	      else if(monster.list[0].offset.x < 0 &&
		  monster.list[0].offset.x >= -(CELL_WIDTH + GAP_WIDTH) / 4)
		dir = 7;
	      else
		dir = monster.list[0].dir | 4;
	    }
	  else if(monster.list[0].offset.y && (dir & 2))
	    {
	      player.next_dir = dir;
	      if(monster.list[0].offset.y < -(CELL_HEIGHT + GAP_HEIGHT) * 3 / 4)
		dir = 4;
	      else if(monster.list[0].offset.y > (CELL_HEIGHT + GAP_HEIGHT) * 3 / 4)
		dir = 5;
	      else if(monster.list[0].offset.y > 0 &&
		  monster.list[0].offset.y <= (CELL_HEIGHT + GAP_HEIGHT) / 4)
		dir = 4;
	      else if(monster.list[0].offset.y < 0 &&
		  monster.list[0].offset.y >= -(CELL_HEIGHT + GAP_HEIGHT) / 4)
		dir = 5;
	      else
		dir = monster.list[0].dir | 4;
	    }
	  /*}}}*/
	  monster.list[0].dir = dir & 3;
	  monster.list[0].stop = !(dir & 4);
	  player.mouse_dir = second;
	}
    }
  /*}}}*/
  /*{{{  key change?*/
  if(player.bashed || (player.old_pressed ^ player.pressed) & 0xF)
  /*
   * The keys have changed, so we need to select a new direction
   * if we're not an interchange to change dir, we keep going
   * in the current direction, if that is non-zero,
   * else we go to the nearest interchange
   */
  {
    unsigned  valid;
    unsigned  pressed;
    
    player.mouse_dir = 0;
    pressed = player.pressed & ~player.bashed;
    if(!pressed)
      pressed = player.pressed;
    valid = 0xF;
    /*{{{  work out valid direction and the edges*/
    if(monster.list[0].offset.x)
      {
	valid &= 0xC;
	if(!monster.list[0].cell.y)
	  pressed &= 0xE;
	else if(monster.list[0].cell.y == CELLS_DOWN - 1)
	  pressed &= 0xD;
      }
    else if(monster.list[0].offset.y)
      {
	valid &= 0x3;
	if(!monster.list[0].cell.x)
	  pressed &= 0xB;
	else if(monster.list[0].cell.x == CELLS_ACROSS - 1)
	  pressed &= 0x7;
      }
    else
      {
	if(!monster.list[0].cell.x)
	  valid &= 0xB;
	else if(monster.list[0].cell.x == CELLS_ACROSS - 1)
	  valid &= 0x7;
	if(!monster.list[0].cell.y)
	  valid &= 0xE;
	else if(monster.list[0].cell.y == CELLS_DOWN - 1)
	  valid &= 0xD;
	pressed &= valid;
      }
    /*}}}*/
    if((pressed & 0x3) == 0x3)
      pressed &= 0xC;
    if((pressed & 0xC) == 0xC)
      pressed &= 0x3;
    if(!pressed)
      {
	monster.list[0].stop = 1;
	player.next_dir = 0;
      }
    else if((valid & pressed) == pressed)
      {
	unsigned  dir;
    
	if((pressed & 0x3) && (pressed & 0xC))
	  pressed &= monster.list[0].dir & 2 ? 0x3 : 0xC;
	for(dir = 0; !(pressed & 1); dir++, pressed >>= 1)
	  /*EMPTY*/;
	player.next_dir = 0;
	monster.list[0].dir = dir;
	monster.list[0].stop = 0;
      }
    else
      {
	unsigned  dir;
    
	pressed &= ~valid;
	for(dir = 0; !(pressed & 1); pressed >>= 1)
	  dir++;
	player.next_dir = dir | 4;
	if(monster.list[0].stop)
	  {
	    /*{{{  select nearest cell*/
	    if(dir & 2)
	      {
		if(monster.list[0].offset.y <
		    -(CELL_HEIGHT + GAP_HEIGHT) * 3 / 4)
		  dir = 0;
		else if(monster.list[0].offset.y >
		    (CELL_HEIGHT + GAP_HEIGHT) * 3 / 4)
		  dir = 1;
		else if(monster.list[0].offset.y > 0 &&
		    monster.list[0].offset.y <=
		    (CELL_HEIGHT + GAP_HEIGHT) / 4)
		  dir = 0;
		else if(monster.list[0].offset.y < 0 &&
		    monster.list[0].offset.y >=
		    -(CELL_HEIGHT + GAP_HEIGHT) / 4)
		  dir = 1;
		else
		  dir = monster.list[0].dir;
	      }
	    else
	      {
		if(monster.list[0].offset.x <
		    -(CELL_WIDTH + GAP_WIDTH) * 3 / 4)
		  dir = 2;
		else if(monster.list[0].offset.x >
		    (CELL_WIDTH + GAP_WIDTH) * 3 / 4)
		  dir = 3;
		else if(monster.list[0].offset.x > 0 &&
		    monster.list[0].offset.x <=
		    (CELL_WIDTH + GAP_WIDTH) / 4)
		  dir = 2;
		else if(monster.list[0].offset.x < 0 &&
		    monster.list[0].offset.x >=
		    -(CELL_WIDTH + GAP_WIDTH) / 4)
		  dir = 3;
		else
		  dir = monster.list[0].dir;
	      }
	    /*}}}*/
	    monster.list[0].dir = dir;
	    monster.list[0].stop = 0;
	  }
      }
    if(monster.list[0].stop)
      player.bashed = 0;
    else if(player.bashed == 0x10)
      player.bashed = 0;
    else if(player.bashed & (1 << monster.list[0].dir))
      player.bashed = 0;
    else if(!(player.bashed & (3 << (monster.list[0].dir & 2))))
      player.bashed = 0x10;
  }
  /*}}}*/
  player.old_pressed = player.pressed;
  if(player.throw == 2)
    {
      if(!player.button)
	player.throw = 0;
    }
  else if(player.throw == 0)
    player.throw = player.button;
  if(dir != monster.list[0].dir)
    new_face(&monster.list[0]);
  if(monster.list[0].pause)
    {
      player.cherry = player.distance = 0;
      monster.list[0].pause = 0;
    }
  else if(!monster.list[0].stop)
    {
      if(!monster.list[0].cycle)
	{
	  monster.list[0].cycle = MONSTER_CYCLES;
	  monster.list[0].image++;
	  if(monster.list[0].image == MONSTER_IMAGES)
	    monster.list[0].image = 0;
	  
	}
      monster.list[0].cycle--;
      cptr = move_muncher(&monster.list[0]);
      /*{{{  pushing?*/
      {
	unsigned  push;
	    
	push = monster.list[0].pause && (monster.list[0].dir & 2);
	if(push != monster.list[0].pushing)
	  {
	    monster.list[0].pushing = push;
	    new_face(&monster.list[0]);
	  }
      }
      /*}}}*/
      if(monster.list[0].stop)
	{
	  monster.list[0].pause = 0;
	  player.bashed = 1 << monster.list[0].dir;
	}
      if(cptr)
	{
	  if(cptr->sprite == SPRITE_CHERRY)
	    /*{{{  eat cherry*/
	    {
	      cptr->sprite = 0;
	      global.cherries--;
	      add_score(50, 0, 0);
	      player.cherry++;
	      if(player.cherry == 8)
		{
		  add_score(500,
		      monster.list[0].pixel.x + CELL_WIDTH / 2,
		      monster.list[0].pixel.y + CELL_HEIGHT / 2);
		  player.cherry = 0;
		}
	      else
		player.distance =
		    (CELL_WIDTH + GAP_WIDTH) / VEL_X;
	    }
	    /*}}}*/
	  else if(cptr->sprite >= SPRITE_PRIZE_BASE &&
	      cptr->sprite < SPRITE_PRIZE_BASE + SPRITE_PRIZES)
	    /*{{{  eat prize*/
	    {
	      int     score;
			
	      history.prize |= 1;
	      cptr->sprite = 0;
	      score = player.screen * 500 + 500;
	      add_score(score > 9500 ? 9500 : score,
		  monster.list[0].pixel.x + CELL_WIDTH / 2,
		  monster.list[0].pixel.y + CELL_HEIGHT / 2);
	      global.state = 2;
	      if(!extra.escape)
		extra_escape();
	      monster.den = 3;
	      monster.delay = 1;
	    }
	    /*}}}*/
	}
    }
  if(player.distance)
    player.distance--;
  else
    player.cherry = 0;
  /*{{{  player looping*/
  if(monster.list[0].stop ||
      monster.list[0].offset.x || monster.list[0].offset.y)
    /*EMPTY*/;
  else if(player.mouse_dir &&
      monster.list[0].cell.x == player.mouse.x &&
      monster.list[0].cell.y == player.mouse.y)
    {
      player.next_dir = player.mouse_dir = 0;
      monster.list[0].stop = 1;
    }
  else if(player.mouse_dir &&
      monster.list[0].dir != (player.mouse_dir & 3) &&
      (monster.list[0].cell.x == player.mouse.x ||
       monster.list[0].cell.y == player.mouse.y))
    {
      monster.list[0].dir = player.mouse_dir & 3;
      player.next_dir = 0;
      new_face(&monster.list[0]);
    }
  else if((!monster.list[0].cell.x && monster.list[0].dir == 2) ||
      (!monster.list[0].cell.y && monster.list[0].dir == 0) ||
      (monster.list[0].cell.x == CELLS_ACROSS - 1 &&
	  monster.list[0].dir == 3) ||
      (monster.list[0].cell.y == CELLS_DOWN - 1 &&
	  monster.list[0].dir == 1))
    {
      player.next_dir = 0;
      monster.list[0].stop = 1;
      player.bashed = 1 << monster.list[0].dir;
    }
  else if(player.next_dir)
    {
      monster.list[0].dir = player.next_dir & 3;
      player.next_dir = 0;
      new_face(&monster.list[0]);
    }
  /*}}}*/
  if(!monster.list[0].shot)
    /*{{{  hit monster?*/
    {
      unsigned  i;
      MONSTER   *mptr;
      int       x, y;
	
      x = monster.list[0].pixel.x - CELL_WIDTH + GAP_WIDTH;
      y = monster.list[0].pixel.y - CELL_HEIGHT + GAP_HEIGHT;
      for(mptr = &monster.list[1], i = monster.monsters - 1; i--; mptr++)
	if(!mptr->chew && !mptr->shot && !mptr->apple &&
	    mptr->pixel.x - x > 0 &&
	    mptr->pixel.x - x < 2 * (CELL_WIDTH - GAP_WIDTH) &&
	    mptr->pixel.y - y > 0 &&
	    mptr->pixel.y - y < 2 * (CELL_HEIGHT - GAP_HEIGHT))
	  {
	    monster.list[0].shot = 1;
	    break;
	  }
    }
    /*}}}*/
  return;
}
/*}}}*/
