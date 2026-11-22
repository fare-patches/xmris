/* Copyright (C) 1993, 1992 Nathan Sidwell */
/* RCS $Id: xmris.c 1.2 Tue, 16 Mar 1999 11:28:16 +0000 nathan $ */
/*{{{  the problem with recursive function definitions*/
/*
 * In order to keep a single control loop, I use animate functions,
 * which are called through a pointer. To chain these, the animate
 * functions return a pointer to the animate function to call next time
 * round. Uh oh, a function which returns a pointer to a function which
 * returns a pointer to ..., you get the idea. To make it worse, in order
 * to implement callback, the animate functions take a pointer to the
 * animate function to call back, when they're done. This gives the two
 * main loops (demo and game play), the ability to both call the same
 * animate functions, and have them return correctly.
 * We could wrap the function definition up in a structure, because we
 * can have self referential structures, but that introduces another
 * level of indirection 'cos passing structures is not K&R compatible.
 * So what I do is define the animate function as taking a pointer to
 * a void function returning void and returning a pointer to a void
 * function returning a void. Then all I have to do is _carefully_ cast
 * these things before using them.
 * And remember kiddies, ANSI says that sizeof(void *) maybe _smaller_
 * than sizeof(void (*)()), which is why I don't cast function pointers
 * into data pointers, 'cos they might not fit.
 * Some (supposidly ANSI) compilers are confused about the declaration
 * and definition of such functions. They only accept K&R style
 * declarations and definitions for them. (It is possible that I've got
 * them slightly wrong, with a superflous set of () some where, but gcc
 * takes it, and my depreciated cdecl, hacked out of K&R 2 agrees with
 * what I've got.) For these platforms, you can give the define
 * BROKEN_FPTRS, which will force K&R mode on just those cases where
 * it all goes sadly wrong. A notable case is codecenter (alias saberc),
 * which just doesn't want to know. (It also performs an addition level
 * of string stripping on the make file, so that the high score file name
 * has its quotes stripped off, but that's another story.)
 * May be I should just have a big switch statement, which will
 * be more robust, and easier on optimizers, but break the
 * object orientedness of the whole thing.
 */
/*}}}*/
#include "xmris.h"
#include "sound.h"
#ifdef NDEBUG
#define game_assert()
#endif /* NDEBUG */
static VOIDFUNC age_scores PROTOARG((VOIDARG));
static PROTOANIMATE(animate_death);
static PROTOANIMATE(animate_pause);
#ifndef NDEBUG
static VOIDFUNC game_assert PROTOARG((VOIDARG));
#endif /* NDEBUG */
void game_keypress(KeySym keysym)
{
  unsigned ix;
  global.key = keysym;
  if (in_attract_mode && global.state != MODE_KEY_DEF) {
    const char *name = portable_key_name(keysym);
    if (name && !strcmp(name,"F1")) {
      global.pressed |= 1 << KEY_KEYBOARD;
      return;
    }
  }
  if(global.state != MODE_KEY_DEF)
    for(ix = KEYS; ix--;)
      if(keysym == data.keysyms[ix])
	{
	  global.pressed |= 1 << ix;
	  if(ix == KEY_PAUSE)
	    {
	      timer_set((unsigned long)0, TIMING_PAUSE);
	      sound_music(MUSIC_OFF);
	      global.pause = 1;
	    }
	  break;
	}
}

void game_keyrelease(KeySym keysym)
{
  unsigned ix;
  if(global.state != MODE_KEY_DEF)
    for(ix = KEYS; ix--;)
      if(keysym == data.keysyms[ix])
	{
	  global.pressed &= ~(1 << ix);
	  break;
	}
}

/*{{{  void add_score(increment, x, y)*/
extern VOIDFUNC add_score
FUNCARG((points, x, y),
	unsigned  points
ARGSEP  int       x
ARGSEP  int       y
)
/*
 * adds the given score (which may be zero)
 * and displays it at the top of the screen
 * if the coordinate > 0 then add the score into the onboard list
 */
{
  points = (unsigned)(((unsigned long)points * global.scale / SCORE_SCALE) *
	SCORE_ROUND);
  if(!global.pedestal)
    global.score += points;
  /*{{{  text score*/
  {
    size_t    length;
    char      text[10];
    int       x, y;
	
    length = format_number(text, global.score, 0);
    x = PIXELX(4, -GAP_WIDTH) - (length + 2) * font.width;
    y = PIXELY(-1, CELL_HEIGHT / 2) + font.center;
    video_opaque_text(display.back, GCN(GC_TEXT),
	x, y, text, (int)length);
    add_background(x, y - font.ascent,
	length * font.width, (unsigned)(font.ascent + font.descent));
  }
  /*}}}*/
  /*{{{  board score?*/
  if(y)
    {
      size_t    length;
      char      text[10];
      int       i;
      SCORE     *sptr;
      SPRITE    *dptr;
	
      dptr = &sprites[SPRITE_DIGITS];
      length = format_number(text, (unsigned long)points, 0);
      /*{{{  remove oldest score?*/
      if(update.score.scores == BOARD_SCORES)
	{
	  add_background(update.score.list[0].place.x,
	      update.score.list[0].place.y,
	      DIGIT_WIDTH * 4, DIGIT_HEIGHT);
	  update.score.scores--;
	  for(sptr = update.score.list, i = update.score.scores; i--; sptr++)
	    memcpy(sptr, sptr + 1, sizeof(SCORE));
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
	  video_copy(dptr->mask, sptr->mask, GCN(GC_COPY),
	      10 * DIGIT_WIDTH, 0, (unsigned)x, DIGIT_HEIGHT, 0, 0);
	  video_copy(dptr->mask, sptr->mask, GCN(GC_COPY),
	      10 * DIGIT_WIDTH, 0, (unsigned)x,
	      DIGIT_HEIGHT, 4 * DIGIT_WIDTH - x, 0);
	}
      else
	x = 0;
      /*}}}*/
      for(i = 0; i < length; i++, x += DIGIT_WIDTH)
	{
	  video_copy(dptr->image, sptr->image, GCN(GC_COPY),
	      (text[i] - '0') * DIGIT_WIDTH, 0, DIGIT_WIDTH, DIGIT_HEIGHT,
	      x, 0);
	  video_copy(dptr->mask, sptr->mask, GCN(GC_COPY),
	      (text[i] - '0') * DIGIT_WIDTH, 0, DIGIT_WIDTH, DIGIT_HEIGHT,
	      x, 0);
	}
      video_fill(sptr->mask, GCN(GC_COPY),
	  (int)((4 - length) * (DIGIT_WIDTH / 2) + DIGIT_WIDTH / 2), 1,
	  (length - 1) * DIGIT_WIDTH, DIGIT_HEIGHT - 2);
      video_copy(sptr->mask, sptr->image, GCN(GC_AND),
	  0, 0, DIGIT_WIDTH * 4, DIGIT_HEIGHT, 0, 0);
    }
  /*}}}*/
  return;
}
/*}}}*/
/*{{{  void age_scores()*/
static VOIDFUNC age_scores FUNCARGVOID
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
	unsigned  ix;
	SCORE     *optr;
	
	add_background(sptr->place.x, sptr->place.y,
	    DIGIT_WIDTH * 4, DIGIT_HEIGHT);
	mask = sptr->mask;
	image = sptr->image;
	for(ix = i, optr = sptr; ix--; optr++)
	  memcpy(optr, optr + 1, sizeof(SCORE));
	optr->mask = mask;
	optr->image = image;
	update.score.scores--;
	sptr--;
     }
  return;
}
/*}}}*/
/*{{{  ANIMATE animate_death(next)*/
static FUNCANIMATE(animate_death, next)
/*
 * spin the player around a bit, and then remove all the monsters and
 * return back to whoever called me
 */
{
  static PROTOANIMATE((*rts));
  PROTOVOID(*then);
  
  then = (PROTOVOID(*))animate_death;
  if(next)
    /*{{{  start*/
    {
      unsigned  i;
      unsigned  base;
    
      rts = (PROTOANIMATE((*)))next;
      player.ball.count = 8;
      i = monster.list[0].face;
      if(i >= 6)
	i = 2 + (i & 1);
      for(base = 8; base--;)
	if(player_dies[base] == i)
	  break;
      global.count = 0;
      monster.list[0].shot = 0;
      monster.list[0].count = 0;
      monster.list[0].cycle = (base & 3) << 1;
      monster.list[0].image = (base & 4) << 1;
      monster.list[0].type = SPRITE_PLAYER_DEAD + (base << 1);
      timer_set((unsigned long)0, TIMING_PAUSE);
    }
    /*}}}*/
  if(global.count && global.count < 16)
    /*{{{  skip?*/
    if(global.throw == 1)
      {
	global.throw = 2;
	global.count = 16;
	monster.list[0].count = DIE_DELAY;
      }
    /*}}}*/
  if(monster.list[0].count++ < DIE_DELAY)
    age_scores();
  else if(global.count == 16)
    /*{{{  finish*/
    {
      MONSTER   *mptr;
      unsigned  ix;
      
      for(mptr = monster.list, ix = monster.monsters; ix--; mptr++)
	add_background(mptr->pixel.x, mptr->pixel.y,
	  CELL_WIDTH, CELL_HEIGHT);
      monster.monsters = 0;
      global.lives--;
      if(global.lives)
	{
	  video_fill(display.back, GCN(GC_CLEAR),
	      PIXELX((int)global.lives - 1, 0), PIXELY(CELLS_DOWN, 0),
	      CELL_WIDTH, CELL_HEIGHT);
	  add_background(PIXELX((int)global.lives - 1, 0),
	      PIXELY(CELLS_DOWN, 0), CELL_WIDTH, CELL_HEIGHT);
	}
      if(extra.escape)
	{
	  extra.escape = 0;
	  draw_extra();
	}
      global.count = 17;
    }
    /*}}}*/
  else if(global.count == 17)
    /*{{{  end*/
    {
      assert(rts != (PROTOANIMATE((*)))NULL);
      global.count = 0;
      then = (*rts)((PROTOVOID(*))NULL);
    }
    /*}}}*/
  else
    /*{{{  animate*/
    {
      age_scores();
      monster.list[0].count = 0;
      global.count++;
      monster.list[0].type =
	  SPRITE_PLAYER_DEAD + monster.list[0].image +
	  ((monster.list[0].cycle + global.count) & 7);
    }
    /*}}}*/
  return then;
}
/*}}}*/
/*{{{  ANIMATE animate_pause(next)*/
static FUNCANIMATE(animate_pause, next)
/*
 * let everybody go to sleep until they're woken up.
 * Don't forget to put a flying carpet under the player, so
 * it doesn't have to float in mid air. (No one has yet observed
 * yogic flying, and they're not going to start now.
 */
{
  static PROTOANIMATE((*rts));
  static int face;
  static MONSTER *seat;
  PROTOVOID(*then);
  
  then = (PROTOVOID(*))animate_pause;
  global.pause = 0;
  if(next)
    /*{{{  start*/
    {
      char      buffer[65];
      size_t    length;
      CELL      *cptr;
      int       depth;
      
      timer_set((unsigned long)0, TIMING_PAUSE);
      set_key_default();
      rts = (PROTOANIMATE((*)))next;
      sound_music(MUSIC_OFF);
      snprintf(buffer, sizeof(buffer), "%.21s to continue, %.21s to quit",
	  key_name(data.keysyms[KEY_THROW]),
	  key_name(data.keysyms[KEY_QUIT]));
      length = strlen(buffer);
      video_fill(display.copy, GCN(GC_CLEAR),
	  BORDER_LEFT + 1, PIXELY(CELLS_DOWN, 0),
	  BOARD_WIDTH - 2, CELL_HEIGHT);
      video_opaque_text(display.copy, GCN(GC_TEXT),
	  CENTER_X - (int)(length * font.width / 2),
	  PIXELY(CELLS_DOWN, CELL_HEIGHT / 2) + font.center,
	  buffer, (int)length);
      present_area(BORDER_LEFT + 1, PIXELY(CELLS_DOWN, 0),
	  BOARD_WIDTH - 2, CELL_HEIGHT);
      face = monster.list[0].face;
      monster.list[0].cycle = MONSTER_CYCLES * 3;
      cptr = BOARDCELL(monster.list[0].cell.x, monster.list[0].cell.y);
      depth = cptr->depths[1];
      if(monster.list[0].offset.x < 0)
	{
	  if(depth < cptr[-1].depths[1])
	    depth = cptr[-1].depths[1];
	}
      else if(monster.list[0].offset.x > 0)
	{
	  if(depth < cptr[1].depths[1])
	    depth = cptr[1].depths[1];
	}
      if(!monster.list[0].shot && monster.list[0].offset.y < depth)
	seat = spawn_monster(0, SPRITE_SEAT, 0, 0,
	    monster.list[0].cell.x, monster.list[0].cell.y,
	    monster.list[0].offset.x, monster.list[0].offset.y + CELL_HEIGHT);
      else
	seat = NULL;
    }
    /*}}}*/
  if(global.pressed & (1 << KEY_QUIT) || global.throw == 1)
    /*{{{  end*/
    {
      MONSTER   *mptr;
      unsigned  count;
      
      global.throw = 2;
      if(global.pressed & (1 << KEY_QUIT))
	{
	  global.lives = 1;
	  global.count = 1;
	  monster.list[0].shot = 1;
	}
      global.pressed &= ~(1 << KEY_QUIT);
      for(mptr = monster.list, count = monster.monsters; count--; mptr++)
	mptr->cycle %= MONSTER_CYCLES;
      if(seat)
	seat->type = 5;
      monster.list[0].face = face;
      add_background(BORDER_LEFT + 1, PIXELY(CELLS_DOWN, 0),
	  BOARD_WIDTH - 2, CELL_HEIGHT);
      set_key_single();
      timer_set((unsigned long)0, TIMING_ON);
      assert(rts != (PROTOANIMATE((*)))NULL);
      then = (*rts)((PROTOVOID(*))NULL);
    }
    /*}}}*/
  else
    /*{{{  animate*/
    {
      MONSTER   *mptr;
      unsigned  count;
      
      age_scores();
      if(player.ball.state == 2 || player.ball.state == 4)
	{
	  bounce_ball();
	  if(!player.ball.state)
	    player.ball.count = 8;
	}
      for(mptr = monster.list, count = monster.monsters; count--; mptr++)
	if(mptr->face < 16)
	  {
	    if(!mptr->cycle)
	      {
		if(mptr == monster.list && mptr->face < 8)
		  {
		    mptr->face = 8 | ((mptr->face & 1) ^
		      (mptr->face == 1 || mptr->face == 4));
		    if(!player.ball.state)
		      player.ball.count = 8;
		  }
		mptr->cycle = MONSTER_CYCLES * 4;
		mptr->image++;
		if(mptr->image == (mptr->type != 6 ?
		    MONSTER_IMAGES : DIAMOND_IMAGES))
		  mptr->image = 0;
	      }
	    mptr->cycle--;
	  }
    }
    /*}}}*/
  return then;
}
/*}}}*/
/* Distance in arcade pixels (16 per cell). Normal monsters follow the
 * tunnel distance field; digging monsters can approach through earth. */
static unsigned danger_distance(void)
{
  unsigned i, nearest = 255;
  for (i = 1; i < monster.monsters; ++i) {
    const MONSTER *enemy = &monster.list[i];
    unsigned distance;
    if (enemy->type > 1 || enemy->shot || enemy->squished)
      continue;
    if (enemy->type == 1) {
      unsigned dx = abs(enemy->pixel.x - monster.list[0].pixel.x) * 16 / (CELL_WIDTH + GAP_WIDTH);
      unsigned dy = abs(enemy->pixel.y - monster.list[0].pixel.y) * 16 / (CELL_HEIGHT + GAP_HEIGHT);
      distance = dx > dy ? dx : dy;
    } else {
      unsigned cells = BOARDCELL(enemy->cell.x, enemy->cell.y)->distance;
      if (!cells || cells > 8) continue;
      distance = (cells - 1) * 16;
    }
    if (distance < nearest) nearest = distance;
  }
  return nearest;
}

/*{{{  ANIMATE animate_game(next)*/
extern FUNCANIMATE(animate_game, next)
/*
 * this is one of the main control loops. It
 * plays a game until the player dies.
 */
{
  static int cell_x = 0;
  static int cell_y = 0;
  static int state = 0;
  static int den_x = 0;
  static int den_y = 0;
  static unsigned fast_music;
  PROTOVOID(*then);
  
  assert(!next);
  if(state == 1 && global.pause)
    {
      global.pause = 0;
      then = animate_pause((PROTOVOID(*))animate_game);
    }
  else
    {
      for(then = (PROTOVOID(*))NULL; then == (PROTOVOID(*))NULL;)
	switch(state)
	{
	  /*{{{  case 0:*/
	  case 0:
	  {
	    extra.got = 0;
	    extra.select = 0;
	    extra.escape = 0;
	    extra.score = 0;
	    extra.count = FRAMES_PER_SECOND - 1;
	    global.diamond = 0;
	    create_xtra_monster(0);
	    global.lives = START_LIVES;
	    global.score = 0;
	    global.screen = global.pedestal;
	    history.msec = 0;
	    state = 4;
	    set_key_single();
	    break;
	  }
	  /*}}}*/
	  /*{{{  case 1:*/
	  case 1:
	  {
	    if(global.count)
	      /*{{{  game frame*/
	      {
		then = (PROTOVOID(*))animate_game;
                if(data.music && global.state < 4) {
                  unsigned distance = danger_distance();
                  if(distance <= 80) fast_music = 1;
                  else if(distance >= 113) fast_music = 0;
                  sound_music((fast_music ? MUSIC_CHASE : MUSIC_GARDEN) |
                      (extra.escape ? MUSIC_EXTRA : 0) |
                      (monster.drones ? MUSIC_DRONES : 0) |
                      (global.diamond == 1 ? MUSIC_DIAMOND : 0));
                }
		age_scores();
		/*{{{  den?*/
		while(cell_y != CELLS_DOWN)
		  {
		    cell_x++;
		    if(cell_x == CELLS_ACROSS)
		      {
			cell_x = 0;
			cell_y++;
		      }
		    if(BOARDCELL(cell_x, cell_y)->den)
		      {
			set_back_sprite(global.state ? global.state != 1 ?
			    0 : (global.screen - 1) % SPRITE_PRIZES +
			    SPRITE_PRIZE_BASE : SPRITE_DEN, cell_x, cell_y);
			break;
		      }
		  }
		/*}}}*/
		if(global.state != 4)
		  {
		    if(move_player())
		      {
			cell_x = -1;
			cell_y = 0;
		      }
		  }
		else if(!monster.list[0].shot)
		  {
		    if(!monster.list[0].cycle)
		      {
			monster.list[0].cycle = MONSTER_CYCLES;
			monster.list[0].image++;
			if(monster.list[0].image == MONSTER_IMAGES)
			  monster.list[0].image = 0;
		      }
		    monster.list[0].cycle--;
		  }
		if(global.broken || global.stepped)
		  calc_distances();
		if(global.broken && extra.escape == 2)
		  calc_extra_home(0);
		global.broken = global.stepped = 0;
		game_assert();
		move_monsters();
		game_assert();
		bounce_ball();
		game_assert();
		move_apples();
		game_assert();
		if(!global.state)
		  /*{{{  monster escape?*/
		  {
		    if(!monster.delay && chaotic() < DEN_ESCAPE_PROB)
		      {
			unsigned  ix;
			
			monster.delay =
			    DEN_ESCAPE_DELAY * DEN_ESCAPE_FLASH + 1;
			ix = chaotic() % global.dens;
			den_x = -1;
			den_y = 0;
			do
			  {
			    do
			      {
				den_x++;
				if(den_x == CELLS_ACROSS)
				  {
				    den_x = 0;
				    den_y++;
				  }
			      }
			    while(!BOARDCELL(den_x, den_y)->den);
			  }
			while(ix--);
		      }
		    if(monster.delay)
		      {
			monster.delay--;
			if(!(monster.delay % DEN_ESCAPE_FLASH))
			  set_back_sprite((unsigned)(monster.delay /
			      DEN_ESCAPE_FLASH & 1 ?
			      SPRITE_NORMAL + 6 : SPRITE_DEN), den_x, den_y);
			if(!monster.delay)
			  {
			    spawn_monster(0, 0, 3, 3, den_x, den_y, 0, 0);
			    monster.den--;
			    if(!monster.den)
			      {
				global.state = 1;
				cell_x = -1;
				cell_y = 0;
			      }
			  }
		      }
		  }
		  /*}}}*/
		else if(global.state == 2 &&
		    !monster.den && !monster.drones)
		  {
		    color_set(BACKGROUND_NORMAL);
		    global.state = 3;
		  }
		if(global.state != 4)
		  {
		    killed_player();
		    if(!global.cherries || !monster.normals ||
			monster.list[0].shot || extra.got == 0x1F ||
			global.diamond == 2)
		      {
			timer_set((unsigned long)0, TIMING_PAUSE);
			/*{{{  happy?*/
			if(!monster.list[0].shot)
			  {
			    int       depth;
			    CELL      *cptr;
			    
			    monster.list[0].face = 10;
			    cptr = BOARDCELL(monster.list[0].cell.x,
				monster.list[0].cell.y);
			    depth = cptr->depths[1];
			    if(monster.list[0].offset.x < 0)
			      {
				if(depth < cptr[-1].depths[1])
				  depth = cptr[-1].depths[1];
			      }
			    else if(monster.list[0].offset.x > 0)
			      {
				if(depth < cptr[1].depths[1])
				  depth = cptr[1].depths[1];
			      }
			    if(monster.list[0].offset.y < depth)
			      spawn_monster(0, SPRITE_SEAT, 0, 0,
				  monster.list[0].cell.x,
				  monster.list[0].cell.y,
				  monster.list[0].offset.x,
				  monster.list[0].offset.y + CELL_HEIGHT);
			  }
			/*}}}*/
			sound_music(MUSIC_OFF);
			sound_play(monster.list[0].shot ? FX_DEATH : FX_CLEAR);
			global.state = 4;
			color_set(BACKGROUND_NORMAL);
		      }
		  }
		else
		  /*{{{  ending*/
		  {
		    global.count--;
		    if(update.score.scores || apple.moving ||
			player.ball.state)
		      {
			if(!global.count)
			  global.count = 1;
		      }
		    else if(global.throw == 1)
		      global.count = 0;
		    if(player.ball.state == 1 &&
			(global.count < SCORE_SHOW / 2 || global.throw == 1))
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
		
		    if(!extra.count--)
		      new_xtra();
		    temp = (unsigned)(global.score / 5000);
		    if(global.state != 4 && temp != extra.score)
		      {
			extra.score = temp;
			extra_escape();
			extra.count = XTRA_HOME_DELAY - 1;
		      }
		  }
		else
		  {
		    extra.score = (unsigned)(global.score / 5000);
		    if(global.state != 2 && extra.escape != 2 &&
			!extra.count--)
		      {
			extra.escape = 2;
			calc_extra_home(0);
		      }
		  }
		/*}}}*/
		game_assert();
	      }
	      /*}}}*/
	    else if(monster.list[0].shot)
	      {
		if(global.lives > 1 ||
		    (extra.got != 0x1F && global.diamond != 2))
		  state = 2;
		then = animate_death((PROTOVOID(*))animate_game);
	      }
	    else
	      {
		if(global.throw == 1)
		  global.throw = 2;
		timer_set((unsigned long)0, TIMING_OFF);
		history.msec += global.msec;
		history.times[0] = global.msec;
		global.msec = 0;
		if(global.diamond == 2)
		  {
		    global.diamond = 3;
		    history.ending |= 3;
		    state = global.lives ? 3 : extra.got == 0x1F ? 1 : 2;
		    sound_play(FX_DIAMOND);
                    then = animate_diamond((PROTOVOID(*))animate_game);
		  }
		else if(extra.got == 0x1F)
		  /*{{{  extra life*/
		  {
		    history.ending |= 2;
		    state = 3;
                    sound_play(FX_LIFE);
		    then = animate_extra_life((PROTOVOID(*))animate_game);
		  }
		  /*}}}*/
		else if(!monster.normals)
		  {
		    history.ending |= 1;
		    state = 3;
		  }
		else
		  state = 3;
	      }
	    break;
	  }
	  /*}}}*/
	  /*{{{  case 2:*/
	  case 2:
	  {
	    global.state = 0;
	    if(!global.lives)
	      {
		global.pedestal = 0;
		state = 0;
		timer_set((unsigned long)0, TIMING_OFF);
		history.msec += global.msec;
		sound_play(global.score >= SCORE_THRESHOLD &&
                    global.score >= scoring.high[HIGH_SCORES - 1].score ?
                    FX_HIGH_SCORE : FX_GAME_OVER);
                high_score(global.score, global.screen, history.msec);
		set_key_default();
		then = animate_demo((PROTOVOID(*))NULL);
	      }
	    else
	      {
		state = 1;
		timer_set(FRAME_RATE, TIMING_ON);
		global.count = SCORE_SHOW;
		if(data.diamond_mask || global.diamond < 3)
		  global.diamond = 0;
		/*{{{  initialize stuff*/
		{
		  cell_x = -1;
		  cell_y = 0;
		  monster.monsters = 0;
		  monster.delay = 0;
		  monster.den = monster.normals;
		  monster.drones = 0;
                  fast_music = 0;
		  spawn_monster(0, 4, 3, 3, global.start.x, global.start.y,
		      0, 0);
		  monster.list[0].stop = 1;
		  monster.player = BOARDCELL(global.start.x, global.start.y);
		  player.old_ball.state = 0;
		  player.old_ball.count = 8;
		  player.ball.state = 0;
		  player.thrown = 0;
		  player.cherry = player.cherry_ticks = 0;
		  player.bashed = 0;
		  player.pressed = 0;
		  player.next = 0;
		  extra.count = FRAMES_PER_SECOND - 1;
		  bounce_ball();
		}
		/*}}}*/
		calc_distances();
	      }
	    break;
	  }
	  /*}}}*/
	  /*{{{  case 3:*/
	  case 3:
	  {
	    state = 4;
	    if(!(global.screen % HISTORY_SHOW))
              {
                sound_play(FX_HISTORY);
                then = animate_history((PROTOVOID(*))animate_game);
              }
	    break;
	  }
	  /*}}}*/
	  /*{{{  case 4:*/
	  case 4:
	  {
	    unsigned  ix;
	    
	    global.state = 0;
	    state = 2;
	    global.screen++;
            sound_play(FX_START);
	    new_board();
	    global.difficulty = global.screen + DIFFICULTY_PEDESTAL;
	    monster.normals = global.screen >= 3 ?
		8 + global.screen / 10 * 2 : 6;
	    if(monster.normals > MONSTERS - 6)
	      monster.normals = MONSTERS - 6;
	    history.prize <<= 1;
	    history.ending <<= 2;
	    for(ix = CELLS_DOWN - 2; ix--;)
	      history.times[ix + 1] = history.times[ix];
	    then = animate_zoom((PROTOVOID(*))animate_game);
	    break;
	  }
	  /*}}}*/
	}
    }
  return then;
}
/*}}}*/
/*{{{  void calc_distances()*/
extern VOIDFUNC calc_distances FUNCARGVOID
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
  int     visited;

  for(y = CELLS_DOWN; y--;)
    for(cptr = BOARDCELL(0, y), x = CELLS_ACROSS; x--; cptr++)
      cptr->distance = cptr->visit ? 0 : 255;
  toggle = 0;
  cptr = BOARDCELL(monster.list[0].cell.x, monster.list[0].cell.y);
  cptr->distance = count = 1;
  visited = 0;
  list[0][0] = cptr;
  list[0][1] = NULL;
  while(list[toggle][0])
  {
    sptr = list[toggle];
    toggle = !toggle;
    aptr = list[toggle];
    count++;
    visited++;
    while((cptr = *sptr++) != NULL)
      {
	CELL      *tptr;
	
	/*{{{  go up?*/
	if(cptr->depths[0])
	  {
	    tptr = cptr - CELL_STRIDE;
	    if(!tptr->distance)
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
	    if(!tptr->distance)
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
	    if(!tptr->distance)
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
	    if(!tptr->distance)
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
  global.visited = visited;
  return;
}
/*}}}*/
/*{{{  void calc_extra_home(start)*/
extern VOIDFUNC calc_extra_home
FUNCARG((start),
	unsigned  start
)
/*
 * sets the distances from each cell to the extra home row
 * this is so the monsters have non-local knowlegde
 * increment the non-zero cells
 * this proceeds as a sort of flood fill operation, starting
 * from the home row and moving outwards
 */
{
  CELL    **aptr, **sptr;
  CELL    *list[2][FLOOD_FILL];
  CELL    *cptr;
  int     toggle;
  int     x, y;
  int     count;

  for(y = CELLS_DOWN; y--;)
    for(cptr = BOARDCELL(0, y), x = CELLS_ACROSS; x--; cptr++)
      cptr->xtra = cptr->visit ? 0 : 255;
  toggle = 0;
  /*{{{  start positions*/
  if(start)
    {
      cptr = BOARDCELL(start, 0);
      cptr->xtra = 1;
      list[0][0] = cptr;
      list[0][1] = NULL;
    }
  else
    {
      cptr = BOARDCELL(4, 0);
      cptr[0].xtra = cptr[1].xtra = cptr[2].xtra = cptr[3].xtra = 1;
      list[0][0] = cptr;
      list[0][1] = cptr + 1;
      list[0][2] = cptr + 2;
      list[0][3] = cptr + 3;
      list[0][4] = NULL;
    }
  /*}}}*/
  count = 1;
  while(list[toggle][0])
  {
    sptr = list[toggle];
    toggle = !toggle;
    aptr = list[toggle];
    count++;
    while((cptr = *sptr++) != NULL)
      {
	CELL      *tptr;
	
	/*{{{  go up?*/
	if(cptr->depths[0])
	  {
	    tptr = cptr - CELL_STRIDE;
	    if(!tptr->xtra)
	      {
		tptr->xtra = count;
		*aptr++ = tptr;
	      }
	  }
	/*}}}*/
	/*{{{  go down?*/
	if(cptr->depths[1])
	  {
	    tptr = cptr + CELL_STRIDE;
	    if(!tptr->xtra)
	      {
		tptr->xtra = count;
		*aptr++ = tptr;
	      }
	  }
	/*}}}*/
	/*{{{  go left?*/
	if(cptr->depths[2])
	  {
	    tptr = cptr - 1;
	    if(!tptr->xtra)
	      {
		tptr->xtra = count;
		*aptr++ = tptr;
	      }
	  }
	/*}}}*/
	/*{{{  go right?*/
	if(cptr->depths[3])
	  {
	    tptr = cptr + 1;
	    if(!tptr->xtra)
	      {
		tptr->xtra = count;
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
/*{{{  unsigned chaotic()*/
extern unsigned chaotic FUNCARGVOID
/*
 * a simple pseudo random number generator
 * it generates 8 new bits of number at each call
 * using a 31 bit maximal length linear feedback shift register
 * the taps are bits 0 and 3
 */
{
static unsigned long seed;
  unsigned  bits;

  if(!seed)
    seed = time((time_t *)NULL);
  bits = (unsigned)(((seed >> 3) ^ seed) & 0xFF);
  seed = (seed >> 8) | ((unsigned long)bits << 23);
  return bits;
}
/*}}}*/
#ifndef NDEBUG
/*{{{  void game_assert()*/
static VOIDFUNC game_assert FUNCARGVOID
{
  unsigned  count;
  unsigned  ix;
  
  count = 0;
  for(ix = monster.monsters; --ix;)
    if(monster.list[ix].type < 2 && !monster.list[ix].squished)
      count++;
  if(global.state != 4)
    assert(count == monster.normals - (global.state ? 0 : monster.den));
  return;
}
/*}}}*/
#endif /* NDEBUG */
/*{{{  size_t format_number(text, n, width)*/
extern size_t format_number
FUNCARG((text, number, digits),
	char      *text    /* output text (include 0) */
ARGSEP  unsigned long number   /* number to convert */
ARGSEP  unsigned  digits   /* field width to convert into */
)
/*
 * formats an integer to a string
 * in the specified number of digits
 * pads leading zeros to ' '
 * returns the number of characters used
 */
{
  char      reverse[10];
  size_t    l, length;

  l = 0;
  do
    {
      reverse[l++] = (char)(number % 10 + '0');
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
void game_tick(void)
{
  static PROTOANIMATE((*animate)) = animate_demo;
  if(global.pressed & (1 << KEY_THROW))
	{
	  if(global.throw == 0)
	    global.throw = 1;
	}
  else if(global.throw == 2)
	global.throw = 0;
  assert(animate);
  animate = (PROTOANIMATE((*)))(*animate)((PROTOVOID(*))NULL);
  show_updates();
  assert(monster.normals < 20);
}
