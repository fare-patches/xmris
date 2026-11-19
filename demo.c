/* Copyright (C) 1993, 1992 Nathan Sidwell */
/* RCS $Id: demo.c 1.2 Tue, 16 Mar 1999 11:28:16 +0000 nathan $ */
#include "xmris.h"
#include "sound.h"
#define SCORING_LEN 22
/*{{{  static*/
static char CONST *table_names[2] = {"Roll of honour", "Today's contenders"};
/*}}}*/
/*{{{  prototypes*/
static PROTOANIMATE(animate_board);
static PROTOANIMATE(animate_def_keys);
static PROTOANIMATE(animate_options);
static PROTOANIMATE(animate_keys);
static PROTOANIMATE(animate_name);
static PROTOANIMATE(animate_score);
static PROTOANIMATE(animate_sprites);
static VOIDFUNC back_mris PROTOARG((unsigned));
static VOIDFUNC back_score PROTOARG((HIGH_SCORE CONST *, unsigned, unsigned));
static int back_title PROTOARG((TITLE CONST *, unsigned));
static int move_demo PROTOARG((unsigned));
static VOIDFUNC move_mris PROTOARG((VOIDARG));
static VOIDFUNC print_scores PROTOARG((HIGH_SCORE CONST *));
/*}}}*/
/*{{{  ANIMATE animate_board(next)*/
static FUNCANIMATE(animate_board, next)
/*
 * animates a demonstration board. Puts on the M R I & S letters
 * and moves them about a bit, before returning from whence I came.
 */
{
  static PROTOANIMATE((*rts));
  static int state;
  static int cell_x;
  static int cell_y;
  PROTOVOID(*then);
  
  then = (PROTOVOID(*))animate_board;
  if(next)
    /*{{{  start*/
    {
      rts = (PROTOANIMATE((*)))next;
      global.screen = chaotic() % board.boards + 1;
      extra.select = chaotic() % 5;
      extra.got = chaotic() & 0x1F;
      create_xtra_monster(extra.select);
      new_board();
      state = 0;
      then = animate_zoom((void (*) PROTOARG((VOIDARG)))animate_board);
    }
    /*}}}*/
  else if(!global.count || global.pause || global.throw == 1 ||
      global.pressed & (1 << KEY_QUIT | 1 << KEY_KEYBOARD))
    /*{{{  end*/
    {
      if(global.throw == 1 && state > 1)
    	global.pedestal = global.screen - 1;
      timer_set((unsigned long)0, TIMING_OFF);
      assert(rts != (PROTOANIMATE((*)))NULL);
      then = (*rts)((PROTOVOID(*))NULL);
    }
    /*}}}*/
  else if(state == 0 || global.pressed & 0xF)
    /*{{{  start again*/
    {
      if(global.pressed & 0xF)
	/*{{{  change screen*/
	{
	  unsigned  shift;
	  
	  timer_set((unsigned long)0, TIMING_OFF);
	  if(global.pressed & 0x1)
	    shift = 10;
	  else if(global.pressed & 0x2)
	    shift = board.boards * 10 - 10;
	  else if(global.pressed & 0x4)
	    shift = board.boards - 1;
	  else
	    shift = 1;
	  global.screen = (global.screen + shift) % board.boards;
	  if(!global.screen)
	    global.screen = board.boards;
	  global.pressed &= ~0xF;
	  new_board();
	  present_area(BORDER_LEFT, BORDER_TOP,
	      BOARD_WIDTH, BOARD_HEIGHT);
	  add_background(PIXELX(8, -GAP_WIDTH), PIXELY(-1, 0),
	      (CELL_WIDTH + GAP_WIDTH) * 4 + GAP_WIDTH, CELL_HEIGHT);
	}
	/*}}}*/
      cell_y = 0;
      cell_x = -1;
      global.state = MODE_DEMO;
      spawn_monster(0, 4, 3, 3, global.start.x, global.start.y, 0, 0);
      extra.count = FRAMES_PER_SECOND - 1;
      monster.list[0].stop = 1;
      monster.den = 0xF;
      monster.drones = 5;
      monster.normals = 4;
      monster.delay = 0;
      player.old_ball.count = 16;
      global.diamond = 0;
      bounce_ball();
      /*{{{  plonk on M R I S*/
      {
      	unsigned  ix;
      
      	calc_extra_home(0);
      	for(ix = 4; ix--;)
      	  {
      	    unsigned  x, y;
      	    unsigned  j;
      	    CELL      *cptr;
      	    
      	    do
      	      {
      		do
      		  j = chaotic();
      		while(j >= CELLS_ACROSS * CELLS_DOWN);
      		x = j % CELLS_ACROSS;
      		y = j / CELLS_ACROSS;
      		cptr = BOARDCELL(x, y);
      	      }
      	    while(!cptr->xtra || cptr->xtra == 255);
      	    spawn_monster(0, SPRITE_MRIS + ix + (chaotic() & 4), 0, 0,
      		(int)x, (int)y, 0, 0);
      	  }
      }
      /*}}}*/
      global.count = DISPLAY_HOLD;
      timer_set(FRAME_RATE, TIMING_ON);
      state++;
    }
    /*}}}*/
  else
    /*{{{  animate*/
    {
      /*{{{  set den?*/
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
      	      set_back_sprite(SPRITE_DEN, cell_x, cell_y);
      	      break;
      	    }
      	}
      /*}}}*/
      /*{{{  calc distances?*/
      if(monster.den && monster.normals != monster.drones)
      	{
      	  unsigned  ix;
      	  
      	  monster.drones = monster.normals;
      	  ix = choose_direction(monster.den);
      	  monster.den ^= 1 << ix;
      	  calc_extra_home(4 + ix);
      	}
      /*}}}*/
      if(!extra.count--)
    	new_xtra();
      if(player.ball.state)
    	bounce_ball();
      else if(!monster.delay && chaotic() < 4)
    	{
    	  unsigned  throw;
    	  
    	  throw = global.throw;
    	  global.throw = 1;
    	  bounce_ball();
    	  global.throw = throw;
    	}
      /*{{{  show a diamond?*/
      if(!global.diamond && !monster.delay && !chaotic())
      	{
      	  unsigned  j;
      	  int       x, y;
      	  CELL      *cptr;
      		
      	  do
      	    {
      	      do
      		{
      		  j = chaotic();
      		  x = j % CELLS_ACROSS;
      		  y = j / CELLS_ACROSS;
      		  cptr = BOARDCELL(x, y);
      		}
      	      while(j >= CELLS_DOWN * CELLS_ACROSS ||
      		  (x == global.start.x && y == global.start.y) ||
      		  cptr->sprite);
      	    }
      	  while(!cptr->visit || cptr[CELL_STRIDE].visit);
      	  global.diamond = 1;
      	  spawn_monster(0, 6, 0, 0, x, y, 0, 0)->count = DIAMOND_DELAY;
      	}
      /*}}}*/
      if(!monster.delay--)
    	monster.delay = 31;
      move_mris();
      if(monster.monsters == 1)
    	global.count--;
    }
    /*}}}*/
  return then;
}
/*}}}*/
static int menu_save_ok = 1;

static void controls_line(int y, const char *line)
{
  video_text(display.back, GCN(GC_TEXT), 20, y, line, strlen(line));
}

static FUNCANIMATE(animate_def_keys, next)
{
  static KeySym pending[KEYS];
  static int selected, capturing;
  static const char *message;
  static const char *const labels[] = {
    "Up","Down","Left","Right","Throw","Pause","Quit","Iconize","Options",
    "Restore defaults","Save and return"
  };
  const char *name = next ? NULL : portable_key_name(global.key);
  if (next) {
    memcpy(pending,data.keysyms,sizeof(pending));
    selected = capturing = 0;
    message = NULL;
    global.state = MODE_KEY_DEF;
  } else if (name) {
    if (!strcmp(name,"Escape")) {
      if (!capturing) return animate_options((PROTOVOID(*))animate_demo);
      capturing = 0;
      message = NULL;
    } else if (capturing) {
      message = NULL;
      if (!strcmp(name,"F1") || !strcmp(name,"F11"))
        message = "Reserved key. Choose another.";
      for (int i=0;i<KEYS;++i)
        if (i!=selected && pending[i]==global.key)
          message = "Already assigned. Choose another.";
      if (!message) {
        pending[selected] = global.key;
        capturing = 0;
      }
    } else if (!strcmp(name,"Up")) selected=(selected+KEYS+1)%(KEYS+2);
    else if (!strcmp(name,"Down")) selected=(selected+1)%(KEYS+2);
    else if (!strcmp(name,"Return")) {
      message = NULL;
      if (selected<KEYS) capturing=1;
      else if (selected==KEYS) {
        default_bindings(pending);
        message="Defaults ready. Select Save to apply.";
      } else {
        memcpy(data.keysyms,pending,sizeof(pending));
        menu_save_ok=options_save(3);
        return animate_options((PROTOVOID(*))animate_demo);
      }
    }
  }
  global.key=NoSymbol;
  video_fill(display.back,GCN(GC_CLEAR),0,0,WINDOW_WIDTH,WINDOW_HEIGHT);
  controls_line(46,"Controls");
  for (int i=0;i<KEYS+2;++i) {
    char line[96];
    const char *binding=i<KEYS ? key_name(pending[i]) : "";
    snprintf(line,sizeof(line),"%c %-18s %.15s",i==selected ? '>' : ' ',
        labels[i],binding ? binding : "?");
    controls_line(90+i*29,line);
  }
  controls_line(445,capturing ? "Press a new key. Esc cancels." :
      "Up/Down: select   Enter: choose");
  controls_line(480,message ? message : "Esc: discard edits and return");
  video_copy(display.back,display.copy,GCN(GC_COPY),0,0,WINDOW_WIDTH,WINDOW_HEIGHT,0,0);
  refresh_window();
  return (PROTOVOID(*))animate_def_keys;
}

static FUNCANIMATE(animate_options, next)
{
  static int selected;
  const char *labels[] = {"Sound effects", "Background music",
                          "Diamond difficulty", "Controls", "Done"};
  const char *values[] = {data.effects ? "On" : "Off", data.music ? "On" : "Off",
      data.diamond_mask == 0 ? "Classic" : data.diamond_mask == 7 ? "Hard" : "Easy"};
  KeySym key = global.key;
  if (next) {
    selected = 0;
    global.state = MODE_KEY_DEF;
    global.pressed = global.throw = 0;
    monster.monsters = apple.apples = 0;
    player.ball.state = player.old_ball.state = 0;
    player.ball.count = player.old_ball.count = 16;
    update.back.backs = update.score.scores = 0;
  } else {
    const char *name = portable_key_name(key);
    int activate = name && !strcmp(name,"Return");
    if (name && !strcmp(name,"Escape")) {
      selected = 4;
      activate = 1;
    } else if (name && !strcmp(name,"Up")) selected = (selected+4)%5;
    else if (name && !strcmp(name,"Down")) selected = (selected+1)%5;
    if (activate) {
      if (selected == 4) {
        global.key = NoSymbol;
        return animate_demo((PROTOVOID(*))NULL);
      }
      if (selected == 3)
        return animate_def_keys((PROTOVOID(*))animate_demo);
      if (selected == 0) {
        data.effects = !data.effects;
        values[0] = data.effects ? "On" : "Off";
      } else if (selected == 1) {
        data.music = !data.music;
        values[1] = data.music ? "On" : "Off";
        if (!data.music) sound_music(MUSIC_OFF);
      } else {
        data.diamond_mask = data.diamond_mask == 1 ? 7 : data.diamond_mask == 7 ? 0 : 1;
        values[2] = data.diamond_mask == 0 ? "Classic" : data.diamond_mask == 7 ? "Hard" : "Easy";
      }
      menu_save_ok = options_save(selected);
    }
  }
  global.key = NoSymbol;
  video_fill(display.back,GCN(GC_CLEAR),0,0,WINDOW_WIDTH,WINDOW_HEIGHT);
  video_text(display.back,GCN(GC_TEXT),CENTER_X-3*font.width,100,"Options",7);
  for (int i=0;i<5;++i) {
    char line[64];
    int len=snprintf(line,sizeof(line),"%c %-19s %s",selected==i ? '>' : ' ',
        labels[i],i<3 ? values[i] : "");
    video_text(display.back,GCN(GC_TEXT),4*font.width,160+i*40,line,len);
  }
  const char *hint = menu_save_ok ? "Up/down, Enter to choose, Esc to exit" : "Could not save preferences";
  video_text(display.back,GCN(GC_TEXT),CENTER_X-(int)(strlen(hint)*font.width/2),
      WINDOW_HEIGHT-60,hint,strlen(hint));
  video_copy(display.back,display.copy,GCN(GC_COPY),0,0,WINDOW_WIDTH,WINDOW_HEIGHT,0,0);
  refresh_window();
  return (PROTOVOID(*))animate_options;
}

/*{{{  ANIMATE animate_demo(next)*/
int in_attract_mode = 1;

extern FUNCANIMATE(animate_demo, next)
/*
 * The main demonstration control loop. Start or quit the game
 * as necessary, or just cycle round the key display, demo board, and
 * high score tables.
 */
{
  static int state = 1;
  PROTOVOID(*then);

  then = NULL;
  assert(!next);
  in_attract_mode = 1;
  global.pause = 0;
  if(global.pressed & 1 << KEY_QUIT)
    global.quit = 1;
  else if(global.throw == 1)
    {
      state = 0;
      global.throw = 2;
      in_attract_mode = 0;
      then = animate_game((PROTOVOID(*))NULL);
    }
  else if(global.pressed & 1 << KEY_KEYBOARD)
    {
      state = 2;
      then = animate_options((PROTOVOID(*))animate_demo);
    }
  else
    {
      static PROTOANIMATE((*modes[])) =
	{
	  (PROTOANIMATE((*)))animate_score,
	  (PROTOANIMATE((*)))animate_name,
	  (PROTOANIMATE((*)))animate_keys,
	  (PROTOANIMATE((*)))animate_sprites,
	  (PROTOANIMATE((*)))animate_board,
	};
      
      then = modes[state++]((PROTOVOID(*))animate_demo);
      if(state == 3 && data.sprites == False)
	state = 4;
      else if(state == 5)
	state = 0; 
    }
  return then;
}
/*}}}*/
/*{{{  ANIMATE animate_diamond(next)*/
extern FUNCANIMATE(animate_diamond, next)
/*
 * A gem of a screen.
 */
{
  static PROTOANIMATE((*rts));
  PROTOVOID(*then);
  
  then = (PROTOVOID(*))animate_diamond;
  if(next)
    /*{{{  start*/
    {
      rts = (PROTOANIMATE((*)))next;
      monster.monsters = 0;
      apple.apples = 0;
      update.back.backs = 0;
      update.score.scores = 0;
      player.ball.state = 0;
      player.ball.count = 8;
      add_background(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT - 1);
      video_fill(display.back, GCN(GC_CLEAR),
    	  BORDER_LEFT + 1, BORDER_TOP + 1, BOARD_WIDTH - 2, BOARD_HEIGHT - 2);
      spawn_monster(0, 4, 3, 3, CELLS_ACROSS + 1, CELLS_DOWN / 2,
    	  -(CELL_WIDTH + GAP_WIDTH) * CELLS_ACROSS, 0);
      spawn_monster(0, 6, 0, 0, CELLS_ACROSS / 2, CELLS_DOWN / 2, 0, 0);
      if(!global.lives)
    	monster.list[0].ghosting = GHOSTING;
      draw_extra_letter(extra.select);
      global.state = MODE_GAME_DEMO;
      global.count = 3;
      timer_set(FRAME_RATE, TIMING_PAUSE);
    }
    /*}}}*/
  /*{{{  skip?*/
  if(global.throw == 1)
    {
      global.throw = 2;
      global.count = 0;
    }
  /*}}}*/
  if(!global.count)
    /*{{{  end*/
    {
      assert(rts != (PROTOANIMATE((*)))NULL);
      then = (*rts)((PROTOVOID(*))NULL);
    }
    /*}}}*/
  else
    /*{{{  animate*/
    {
      if(monster.list[0].offset.x == VEL_X - (CELL_WIDTH + GAP_WIDTH) *
    	  (CELLS_ACROSS + 2 - CELLS_ACROSS / 2))
    	{
    	  monster.list[0].pushing = 1;
    	  new_face(&monster.list[0]);
	  sound_play(FX_CREDIT);
	  monster.list[1].cell.x = CELLS_ACROSS + 1;
    	  monster.list[1].offset.x = -(CELL_WIDTH + GAP_WIDTH) *
    	    (CELLS_ACROSS + 1 - CELLS_ACROSS / 2);
    	}
      if(!move_demo(MONSTER_CYCLES))
    	global.count--;
    }
    /*}}}*/
  return then;
}
/*}}}*/
/*{{{  ANIMATE animate_extra_life(next)*/
extern FUNCANIMATE(animate_extra_life, next)
/*
 * animate an extra life
 */
{
  static PROTOANIMATE((*rts));
  static char CONST *text[] = {"Congratulations", "You win extra", NULL};
  PROTOVOID(*then);
  
  then = (PROTOVOID(*))animate_extra_life;
  if(next)
    /*{{{  start*/
    {
      rts = (PROTOANIMATE((*)))next;
      monster.monsters = 0;
      apple.apples = 0;
      update.back.backs = 0;
      update.score.scores = 0;
      player.ball.state = 0;
      player.ball.count = 3;
      player.old_ball.state = 0;
      player.old_ball.count = 16;
      add_background(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT - 1);
      video_fill(display.back, GCN(GC_CLEAR),
    	  BORDER_LEFT + 1, BORDER_TOP + 1, BOARD_WIDTH - 2, BOARD_HEIGHT - 2);
      /*{{{  display some text*/
      {
      	unsigned  line;
      	char CONST **tptr;
      	    
      	line = BORDER_TOP + CELL_HEIGHT;
      	for(tptr = text; *tptr; tptr++)
      	  {
      	    size_t    length;
      	    
      	    length = strlen(*tptr);
            video_opaque_text(display.back, GCN(GC_TEXT),
      		CENTER_X - (int)(length * font.width / 2),
      		(int)line + font.ascent, *tptr, (int)length);
      	    line += font.ascent + font.descent;
      	  }
      }
      /*}}}*/
      spawn_monster(0, 4, 3, 3, 3, CELLS_DOWN / 2,
    	  -(CELL_WIDTH + GAP_WIDTH) * 3, 0);
      spawn_monster(0, 0, 2, 2, 0, CELLS_DOWN / 2,
    	  (CELLS_ACROSS - 1) * (CELL_WIDTH + GAP_WIDTH), 0);
      if(!global.lives)
    	monster.list[0].ghosting = GHOSTING;
      draw_extra_letter(extra.select);
      /*{{{  add m r i s*/
      {
      	unsigned  i;
      	    
      	for(i = 4; i--;)
      	  {
      	    back_sprite(SPRITE_MRIS + 4 + i, 0,
      		PIXELX(CELLS_ACROSS / 2 - 2 + (int)i, 0), PIXELY(2, 0));
      	    spawn_monster(0, SPRITE_MRIS + i, 1, 1,
      		CELLS_ACROSS / 2 - 2 + (int)i, 2,
      		0, -(CELL_HEIGHT + GAP_HEIGHT) * 3 -
      		  (int)CELL_HEIGHT / 2 * (int)i);
      	  }
      }
      /*}}}*/
      /*{{{  create a path*/
      {
      	unsigned  i;
      	CELL      *cptr;
      	    
      	apple.apples = 0;
      	for(cptr = BOARDCELL(0, CELLS_DOWN / 2), i = CELLS_ACROSS; i--; cptr++)
      	  {
      	    cptr->visit = 1;
      	    cptr->sprite = 0;
      	    cptr->distance = CELLS_ACROSS - i;
      	    cptr->depths[0] = 0;
      	    cptr->depths[1] = 0;
      	    cptr->depths[2] = -(CELL_WIDTH + GAP_WIDTH);
      	    cptr->depths[3] = CELL_WIDTH + GAP_WIDTH;
      	    cptr->holes[0] = 0;
      	    cptr->holes[1] = 0;
      	    cptr->holes[2] = 0;
      	    cptr->holes[3] = 0;
      	  }
      	BOARDCELL(0, CELLS_DOWN / 2)->depths[2] = 0;
      	BOARDCELL(CELLS_ACROSS - 1, CELLS_DOWN / 2)->depths[3] = 0;
      }
      /*}}}*/
      global.state = MODE_GAME_DEMO;
      global.count = 3;
      timer_set(FRAME_RATE, TIMING_PAUSE);
    }
    /*}}}*/
  if(global.throw == 1)
    {
      global.count = 0;
      global.throw = 2;
    }
  if(!global.count)
    /*{{{  end*/
    {
      unsigned  i;
      int       x;
      
      extra.got = 0;
      create_xtra_monster(extra.select);
      for(i = 5; i--;)
    	draw_extra_letter(i);
      x = PIXELX((int)global.lives - 1, 0);
      video_copy(sprites[SPRITE_PLAYER + 6].mask,
    	  display.back, GCN(GC_MASK), 0, 0,
    	  CELL_WIDTH, CELL_HEIGHT, x, PIXELY(CELLS_DOWN, 0));
      video_copy(sprites[SPRITE_PLAYER + 6].image,
    	  display.back, GCN(GC_OR), 0, 0,
    	  CELL_WIDTH, CELL_HEIGHT, x, PIXELY(CELLS_DOWN, 0));
      add_background(x, PIXELY(CELLS_DOWN, 0), CELL_WIDTH, CELL_HEIGHT);
      global.lives++;
      assert(rts != (PROTOANIMATE((*)))NULL);
      then = (*rts)((PROTOVOID(*))NULL);
    }
    /*}}}*/
  else
    /*{{{  animate*/
    {
      unsigned  throw;
      
      throw = global.throw;
      if(!monster.list[0].offset.x && global.count == 3)
    	{
    	  global.count--;
    	  global.throw = 1;
    	}
      bounce_ball();
      global.throw = throw;
      if(player.ball.state == 3)
    	player.ball.state = 4;
      else if(player.ball.state == 4)
    	{
    	  player.ball.pixel.x = PIXELX(global.lives - 1, 0) + CELL_WIDTH / 2;
    	  player.ball.pixel.y = BORDER_TOP + BOARD_HEIGHT + 1 +
    	      CELL_HEIGHT / 2 - (CELL_HEIGHT + GAP_HEIGHT) * 3;
    	}
      else if(!player.ball.state && global.count == 2)
    	{
    	  monster.list[0].face = 10;
    	  spawn_monster(0, SPRITE_PLAYER + 6, 0, 1, (int)global.lives - 1,
    	      CELLS_DOWN, 0, -(CELL_HEIGHT + GAP_HEIGHT) * 3);
    	  global.count--;
    	}
      if(global.count == 1)
    	player.ball.count = 8;
      if(!move_demo(MONSTER_CYCLES) && global.count == 1)
    	global.count--;
    }
    /*}}}*/
  return then;
}
/*}}}*/
/*{{{  ANIMATE animate_history(next)*/
extern FUNCANIMATE(animate_history, next)
/*
 * shows the history list
 */
{
  static PROTOANIMATE((*rts));
  PROTOVOID(*then);
  
  then = (PROTOVOID(*))animate_history;
  if(next)
    /*{{{  start*/
    {
      rts = (PROTOANIMATE((*)))next;
      monster.monsters = 0;
      apple.apples = 0;
      update.back.backs = 0;
      update.score.scores = 0;
      player.ball.state = 0;
      player.ball.count = 16;
      player.old_ball.state = 0;
      player.old_ball.count = 16;
      add_background(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT - 1);
      draw_extra_letter(extra.select);
      video_fill(display.back, GCN(GC_CLEAR),
    	  BORDER_LEFT + 1, BORDER_TOP + 1, BOARD_WIDTH - 2, BOARD_HEIGHT - 2);
      /*{{{  add in the parts*/
      {
      	unsigned  base;
      	unsigned  screens;
      	unsigned  ix;
      	unsigned  sprite;
      	MONSTER   *mptr;
        char      text[64];
      	size_t    length;
      	static unsigned sprites[4] =
      	    {SPRITE_CHERRY, SPRITE_NORMAL + 4, 4, 6};
      	    
      	screens = global.screen < CELLS_DOWN - 2 ?
      	    global.screen : CELLS_DOWN - 2;
      	base = global.screen - screens;
      	for(ix = screens; ix--;)
      	  {
      	    unsigned long time;
      	    
            snprintf(text, sizeof(text), "Garden %u", base + ix + 1);
      	    length = strlen(text);
            video_opaque_text(display.back, GCN(GC_TEXT),
      		PIXELX(CELLS_ACROSS / 2 - 1,
      		  -GAP_WIDTH) - (int)(font.width * length),
      		PIXELY((int)(screens - ix),
      		  (CELL_HEIGHT + GAP_HEIGHT) / 2) + font.center,
      		  text, (int)length);
      	    time = history.times[screens - 1 - ix] / 1000;
            snprintf(text, sizeof(text), "%3lu\'%02lu\"", time / 60, time % 60);
      	    length = strlen(text);
            video_opaque_text(display.back, GCN(GC_TEXT),
      		PIXELX(CELLS_ACROSS / 2 + 2, 0),
      		PIXELY((int)(screens - ix),
      		  (CELL_HEIGHT + GAP_HEIGHT) / 2) + font.center,
      		  text, (int)length);
      	    if(history.prize & 1 << (screens - 1 - ix))
      	      spawn_monster(0, SPRITE_PRIZE_BASE +
      		  (base + ix) % SPRITE_PRIZES, 0, 0,
      		  CELLS_ACROSS / 2 + 1, (int)(screens - ix),
      		  0, (int)ix * (2 * CELL_HEIGHT + GAP_HEIGHT) +
      		  CELLS_DOWN * GAP_HEIGHT + CELL_HEIGHT / 2 + GAP_HEIGHT +
      		  (CELL_HEIGHT + GAP_HEIGHT) *
      		    (2 + CELLS_DOWN - (int)screens));
      	    sprite = sprites[(history.ending >> (screens - 1 - ix) * 2) & 3];
      	    mptr = spawn_monster(0, sprite,
      		0, 0, CELLS_ACROSS / 2 - 1, (int)(screens - ix),
      		0, (int)ix * (2 * CELL_HEIGHT + GAP_HEIGHT) +
      		CELLS_DOWN * GAP_HEIGHT + (CELL_HEIGHT + GAP_HEIGHT) *
      		(2 + CELLS_DOWN - (int)screens));
      	    if(sprite == 4)
      	      mptr->face = 10;
      	  }
      }
      /*}}}*/
      global.state = MODE_GAME_DEMO;
      global.count = DISPLAY_HOLD;
      timer_set(FRAME_RATE, TIMING_PAUSE);
    }
    /*}}}*/
  /*{{{  skip?*/
  if(global.throw == 1)
    {
      global.throw = 2;
      global.count = 0;
    }
  /*}}}*/
  if(!global.count)
    /*{{{  end*/
    {
      assert(rts != (PROTOANIMATE((*)))NULL);
      then = (*rts)((PROTOVOID(*))NULL);
    }
    /*}}}*/
  else
    /*{{{  animate*/
    {
      if(!move_demo(MONSTER_CYCLES))
    	global.count--;
    }
    /*}}}*/
  return then;
}
/*}}}*/
/*{{{  ANIMATE animate_keys(next)*/
static FUNCANIMATE(animate_keys, next)
/*
 * show the current key bindings and stuff
 */
{
  static PROTOANIMATE((*rts));
  PROTOVOID(*then);
  
  then = (PROTOVOID(*))animate_keys;
  if(next)
    /*{{{  start*/
    {
      rts = (PROTOANIMATE((*)))next;
      global.state = MODE_DEMO;
      monster.monsters = 0;
      apple.apples = 0;
      update.back.backs = 0;
      update.score.scores = 0;
      player.ball.state = 0;
      player.ball.count = 16;
      player.old_ball.state = 0;
      player.old_ball.count = 16;
      video_fill(display.back, GCN(GC_CLEAR),
    	  0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
      /*{{{  put on M R I S*/
      {
      	unsigned  missing;
      	int       x, y;
      	    
      	missing = chaotic() & 3;
      	back_mris((unsigned)(1 << missing));
      	y = chaotic() % (CELLS_DOWN + 2) * (CELL_HEIGHT + GAP_HEIGHT);
      	x = chaotic() & 1 ? CELLS_ACROSS * (CELL_WIDTH + GAP_WIDTH) :
      	    -CELLS_ACROSS * (CELL_WIDTH + GAP_WIDTH);
      	spawn_monster(0, SPRITE_MRIS + missing, 0, 0,
      	    4 + (int)missing, -1, x, y);
      }
      /*}}}*/
      video_copy(display.back, display.copy, GCN(GC_COPY),
      0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0);
      /*{{{  put on the title text*/
      {
      	TITLE CONST *tptr;
      	unsigned  ix;
      	unsigned  gnome;
      	    
      	gnome = 0;
      	for(tptr = title_text, ix = 0; tptr->text; tptr++, ix++)
      	  if(back_title(tptr, ix))
	    /*{{{  spawn monster*/
	    {
	      unsigned  type;
	      int       cellx;
	      int       offsetx;
	    
	      do
	    	{
	    	  type = chaotic() & 3;
	    	  if(type & 2)
	    	    type++;
	    	}
	      while(type == 4 && gnome);
	      if(type == 4)
	    	gnome = 1;
	      if(chaotic() & 1)
	    	{
	    	  cellx = -2;
	    	  offsetx = (CELLS_ACROSS + 2) * (CELL_WIDTH + GAP_WIDTH);
	    	}
	      else
	    	{
	    	  cellx = CELLS_ACROSS + 1;
	    	  offsetx = -(CELLS_ACROSS + 5) * (CELL_WIDTH + GAP_WIDTH);
	    	}
	      offsetx += CELL_WIDTH * (chaotic() & 3);
	      spawn_monster(0, type, 0, 0, cellx, (int)ix, offsetx, 0);
	    }
	    /*}}}*/
      }
      /*}}}*/
      global.count = DISPLAY_HOLD;
      global.missed = 0;
      refresh_window();
      timer_set(FRAME_RATE, TIMING_ON);
    }
    /*}}}*/
  if(!global.count || global.pause || global.throw == 1 ||
      global.pressed & (1 << KEY_QUIT | 1 << KEY_KEYBOARD))
    /*{{{  end*/
    {
      timer_set((unsigned long)0, TIMING_OFF);
      assert(rts != (PROTOANIMATE((*)))NULL);
      then = (*rts)((PROTOVOID(*))NULL);
    }
    /*}}}*/
  else
    /*{{{  animate*/
    {
      if(!move_demo(MONSTER_CYCLES))
    	global.count--;
    }
    /*}}}*/
  return then;
}
/*}}}*/
/*{{{  ANIMATE animate_name(next)*/
static FUNCANIMATE(animate_name, next)
/*
 * Animates the game name in an exciting way.
 */
{
  static PROTOANIMATE((*rts));
  static int state;
  PROTOVOID(*then);
  
  then = (PROTOVOID(*))animate_name;
  if(next)
    /*{{{  start*/
    {
      rts = (PROTOANIMATE((*)))next;
      state = -1;
      extra.got = 0x1F;
      blank_board(board.list[chaotic() % board.boards], 0);
      video_copy(display.back, display.copy, GCN(GC_COPY),
    	  0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0);
      player.old_ball.count = 16;
      player.ball.count = 16;
      then = animate_zoom((void (*) PROTOARG((VOIDARG)))animate_name);
    }
    /*}}}*/
  else if(!global.count || global.pause || global.throw == 1 ||
      global.pressed & (1 << KEY_QUIT | 1 << KEY_KEYBOARD))
    /*{{{  end*/
    {
      assert(rts != (PROTOANIMATE((*)))NULL);
      then = (*rts)((PROTOVOID(*))NULL);
    }
    /*}}}*/
  else if(state == -1)
    /*{{{  start again*/
    {
      global.state = MODE_DEMO;
      player.old_ball.state = 0;
      player.old_ball.count = 16;
      player.ball.state = 3;
      player.ball.count = BALL_EXPLODE;
      refresh_window();
      timer_set(NAME_RATE, TIMING_OFF);
      state = 0;
    }
    /*}}}*/
  else
    /*{{{  animate*/
    {
      unsigned  throw;
      
      throw = global.throw;
      if(player.ball.state == 3)
	/*{{{  exploded?*/
	{
	  if(lettering[state])
	    {
	      player.ball.state = 4;
	      player.ball.count--;
	      if(lettering[state])
		{
		  monster.list[0].pixel.x =
		      PIXELX(LETTERX(lettering[state][0]), 0);
		  monster.list[0].pixel.y =
		      PIXELY(LETTERY(lettering[state][0]), 0);
		}
	    }
	  else
	    global.count--;
	}
	/*}}}*/
      else if(player.ball.state)
    	{
    	  bounce_ball();
	  /*{{{  imploded?*/
	  if(!player.ball.state)
	    {
	      unsigned char letter;
	      CELL      *cptr;
	      
	      letter = lettering[state][0];
	      spawn_monster(0, 4, 0, 0,
	  	  LETTERX(letter), LETTERY(letter), 0, 0);
	      cptr = BOARDCELL(LETTERX(letter), LETTERY(letter));
	      cptr->visit = 1;
	      update.set = 0;
	      munch_hole(cptr, PIXELX(LETTERX(letter), 0),
	  	  PIXELY(LETTERY(letter), 0));
	      add_background(update.tl.x, update.tl.y,
	  	  (unsigned)(update.br.x - update.tl.x),
	  	  (unsigned)(update.br.y - update.tl.y));
	      global.count = 1;
	      player.ball.count = 16;
	    }
	  /*}}}*/
    	}
      else
    	{
	  /*{{{  new direction?*/
	  if(!monster.list[0].count)
	    {
	      unsigned char letter;
	      
	      letter = lettering[state][global.count++];
	      if(letter == LETTEREND)
	  	{
	  	  monster.list[0].type = 5;
	  	  player.ball.pixel.x = monster.list[0].pixel.x +
	  	      CELL_WIDTH / 2;
	  	  player.ball.pixel.y = monster.list[0].pixel.y +
	  	      CELL_HEIGHT / 2;
	  	  player.ball.state = 2;
	  	  player.ball.count = 0;
	  	  global.count = DISPLAY_HOLD *
	  	      (unsigned)(FRAME_RATE / NAME_RATE);
	  	  state++;
	  	}
	      else
	  	{
	  	  monster.list[0].dir = LETTERDIR(letter);
	  	  monster.list[0].count = LETTERDIST(letter);
	  	  new_face(&monster.list[0]);
	  	}
	    }
	  /*}}}*/
    	  if(!monster.list[0].cycle)
    	    {
    	      monster.list[0].cycle = MONSTER_CYCLES;
    	      monster.list[0].image++;
    	      if(monster.list[0].image == MONSTER_IMAGES)
    		monster.list[0].image = 0;
    	    }
    	  monster.list[0].cycle--;
    	  if(monster.list[0].type != 5 && move_muncher(&monster.list[0]))
    	    monster.list[0].count--;
    	}
      global.throw = throw;
    }
    /*}}}*/
  return then;
}
/*}}}*/
/*{{{  ANIMATE animate_score(next)*/
static FUNCANIMATE(animate_score, next)
/*
 * pick a high score table and show it
 */
{
  static PROTOANIMATE((*rts));
  PROTOVOID(*then);
  unsigned  cycle;
  unsigned  start;
  unsigned  term;
  
  then = (PROTOVOID(*))animate_score;
  cycle = start = term = 0;
  if(next)
    /*{{{  new*/
    {
      rts = (PROTOANIMATE((*)))next;
      check_scores();
      start = 1;
      cycle = !scoring.display || !scoring.display->score;
    }
    /*}}}*/
  else if(!global.count || global.pause || global.throw == 1 ||
      global.pressed & (1 << KEY_QUIT | 1 << KEY_KEYBOARD))
    /*{{{  end*/
    {
      cycle = 1;
      term = 1;
    }
    /*}}}*/
  else if(global.pressed & 0xF)
    /*{{{  cycle*/
    {
      start = 1;
      cycle = global.pressed & 0x5 ? 1 : 2;
      global.pressed &= ~0xF;
    }
    /*}}}*/
  /*{{{  cycle scoring.display?*/
  if(cycle)
    {
      static HIGH_SCORE *list[3] =
  	{scoring.today, scoring.high, scoring.personal};
      unsigned  ix;
      unsigned  count;
      
      if(!scoring.display)
  	ix = 0;
      else
  	for(ix = 3; ix--;)
  	  if(list[ix] == scoring.display)
  	    break;
      for(count = 3; (ix = (ix + cycle) % 3), count--;)
  	if(list[ix]->score)
  	  {
  	    scoring.display = list[ix];
  	    break;
  	  }
      if(!scoring.display)
  	{
  	  term = 1;
  	  start = 0;
  	}
    }
  /*}}}*/
  if(start)
    /*{{{  initialize*/
    {
      global.state = MODE_DEMO;
      monster.monsters = 0;
      apple.apples = 0;
      update.back.backs = 0;
      update.score.scores = 0;
      player.ball.state = 0;
      player.ball.count = 16;
      player.old_ball.state = 0;
      player.old_ball.count = 16;
      video_fill(display.back, GCN(GC_CLEAR),
    	  0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
      video_line(display.back, GCN(GC_BORDER),
    	BORDER_LEFT, PIXELY(CELLS_DOWN, 0) - 1,
    	BORDER_LEFT + BOARD_WIDTH, PIXELY(CELLS_DOWN, 0) - 1);
      back_mris(0);
      /*{{{  check if splitting lines*/
      {
      	size_t    length;
      	HIGH_SCORE CONST *sptr;
      	unsigned  ix;
      	    
      	length = 0;
      	for(sptr = scoring.display, ix = 0;
      	    sptr->score && ix != HIGH_SCORES; sptr++, ix++)
      	  {
      	    size_t    temp;
      	    
      	    temp = strlen(sptr->name);
      	    if(temp > length)
      	      length = temp;
      	  }
      	monster.den = (length + SCORING_LEN) * font.width >
      	    PIXELX(CELLS_ACROSS, -GAP_WIDTH) - PIXELX(0, 0);
      }
      /*}}}*/
      /*{{{  heading*/
      {
      	char CONST *string;
      	size_t    length;
      	
      	if(scoring.display == scoring.personal)
      	  string = scoring.mine.name;
      	else
      	  string = table_names[scoring.display != scoring.high];
      	length = strlen(string);
        video_opaque_text(display.back, GCN(GC_TEXT),
      	    PIXELX(CELLS_ACROSS / 2, 0) - (int)(length * font.width / 2),
      	    PIXELY(0, CELL_HEIGHT / 2) + font.center,
      	    string, (int)length);
      	if(scoring.display == scoring.personal)
      	  string = "   Score Garden Time  Date";
      	else
      	  string = "   Score Garden Time  Name";
      	length = strlen(string);
        video_opaque_text(display.back, GCN(GC_TEXT),
      	    PIXELX(0, 0), PIXELY(1, CELL_HEIGHT / 2) + font.center,
      	    string, (int)length);
      	if(scoring.mine.score)
      	  back_score(&scoring.mine, CELLS_DOWN, 0);
      }
      /*}}}*/
      video_copy(display.back, display.copy, GCN(GC_COPY),
    	  0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0);
      /*{{{  put on the scores*/
      {
      	HIGH_SCORE CONST *sptr;
      	unsigned  ix;
      	    
      	for(sptr = scoring.display, ix = 0;
      	    sptr->score && ix != HIGH_SCORES; sptr++, ix++)
      	  back_score(sptr, 2 + ix, monster.den);
      	spawn_monster(0, 4, 3, 3, 0, (int)ix + 2,
      	    -2 * (CELL_WIDTH + GAP_WIDTH), 0);
      }
      /*}}}*/
      global.count = SCORE_HOLD;
      global.missed = 0;
      refresh_window();
      timer_set(SCORE_RATE, TIMING_OFF);
    }
    /*}}}*/
  else if(term)
    /*{{{  term*/
    {
      assert(rts != (PROTOANIMATE((*)))NULL);
      then = (*rts)((PROTOVOID(*))NULL);
    }
    /*}}}*/
  else
    /*{{{  animate*/
    {
      add_background(PIXELX(monster.list[0].cell.x, monster.list[0].offset.x +
    	  (monster.list[0].offset.x > 0 ? CELL_WIDTH - VEL_X : 0)),
    	  PIXELY(monster.list[0].cell.y, monster.list[0].offset.y -
    	  (monster.list[0].offset.y < 0 ? 0 : GAP_HEIGHT / 2)),
    	  monster.list[0].offset.x ? VEL_X : CELL_WIDTH,
    	  CELL_HEIGHT + GAP_HEIGHT);
      if(!move_demo(SCORE_CYCLES))
    	global.count--;
      /*{{{  turn player?*/
      if(monster.list[0].offset.x == 0 &&
      	  monster.list[0].offset.y == 0 && monster.list[0].cell.y > 0)
      	{
      	  if(monster.list[0].dir == 0)
      	    {
      	      if(monster.list[0].cell.x)
		/*{{{  go left*/
		{
		  monster.list[0].offset.x =
		      monster.list[0].cell.x * (CELL_WIDTH + GAP_WIDTH);
		  monster.list[0].cell.x = 0;
		}
		/*}}}*/
      	      else
		/*{{{  go right*/
		{
		  size_t    length;
		  
		  if(monster.den)
		    length = CELLS_ACROSS - 1;
		  else
		    {
		      size_t    temp;
		      unsigned  ix;
		      
		      ix = monster.list[0].cell.y - 2;
		      length = strlen(scoring.display[ix].name);
		      if(ix != 0)
			{
			  temp = strlen(scoring.display[ix - 1].name);
			  if(temp > length)
			    length = temp;
			}
		      length = ((length + SCORING_LEN) * font.width +
			  GAP_WIDTH) / (CELL_WIDTH + GAP_WIDTH) ;
		    }
		  monster.list[0].cell.x = length;
		  monster.list[0].offset.x =
		      -(length * (CELL_WIDTH + GAP_WIDTH));
		}
		/*}}}*/
      	    }
      	  else if(monster.list[0].cell.y != 2)
	    /*{{{  go up 1*/
	    {
	      monster.list[0].cell.y--;
	      monster.list[0].offset.y = CELL_HEIGHT + GAP_HEIGHT;
	    }
	    /*}}}*/
      	  else
	    /*{{{  go off screen*/
	    {
	      monster.list[0].cell.y = -2;
	      monster.list[0].offset.y = 4 * (CELL_HEIGHT + GAP_HEIGHT);
	    }
	    /*}}}*/
      	}
      /*}}}*/
    }
    /*}}}*/
  return then;
}
/*}}}*/
/*{{{  ANIMATE animate_sprites(next)*/
static FUNCANIMATE(animate_sprites, next)
/*
 * show off the sprites
 */
{
  static PROTOANIMATE((*rts));
  static unsigned pressed = 0;
  static MONSTER  *happy;
  PROTOVOID(*then);
  
  then = (PROTOVOID(*))animate_sprites;
  if(next)
    /*{{{  initialize*/
    {
      rts = (PROTOANIMATE((*)))next;
      global.state = MODE_DEMO;
      monster.monsters = 0;
      apple.apples = 0;
      update.back.backs = 0;
      update.score.scores = 0;
      player.ball.state = 0;
      player.ball.count = 16;
      player.old_ball.state = 0;
      player.old_ball.count = 16;
      extra.select = 0;
      extra.got = 0;
      video_fill(display.back, GCN(GC_CLEAR),
    	  0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
      /*{{{  M R I S*/
      {
      	unsigned  ix;
      	
      	for(ix = 4; ix--;)
      	  {
      	    back_sprite(SPRITE_MRIS + ix, 0, PIXELX(4 + (int)ix, 0),
      		PIXELY(-1, 0));
      	    back_sprite(SPRITE_MRIS + 4 + ix, 0, PIXELX((int)ix, 0),
      		PIXELY(-1, 0));
      	    back_sprite(SPRITE_MRIS + 4 + ix, 0, PIXELX(8 + (int)ix, 0),
      		PIXELY(-1, 0));
      	  }
      }
      /*}}}*/
      /*{{{  normal, munch, drone, player*/
      {
      	unsigned  ix;
      	
      	for(ix = 12; ix--;)
      	  {
      	    back_sprite(SPRITE_NORMAL + ix, 0,
      		PIXELX(0, 0), PIXELY((int)ix, 0));
      	    back_sprite(SPRITE_MUNCHER + ix, 0,
      		PIXELX(1, 0), PIXELY((int)ix, 0));
      	    back_sprite(SPRITE_DRONE + ix, 0,
      		PIXELX(3, 0), PIXELY((int)ix, 0));
      	    back_sprite(SPRITE_PLAYER + ix, 0,
      		PIXELX(4, 0), PIXELY((int)ix, 0));
      	  }
      }
      /*}}}*/
      /*{{{  xtra*/
      {
      	unsigned  ix;
      	
      	for(ix = 10; ix--;)
      	  {
      	    if(ix & 1)
      	      create_xtra_monster(ix >> 1);
      	    back_sprite(SPRITE_XTRA + ix, 0,
      		PIXELX(2, 0), PIXELY((int)ix, 0));
      	  }
      }
      /*}}}*/
      back_sprite(SPRITE_CHOMP, 0, PIXELX(2, 0), PIXELY(10, 0));
      back_sprite(SPRITE_CHOMP + 1, 0, PIXELX(2, 0), PIXELY(11, 0));
      /*{{{  squished*/
      {
      	unsigned  ix;
      	
      	for(ix = 5; ix--;)
      	  {
      	    back_sprite(SPRITE_SQUISHED + (ix << 1), 0,
      		PIXELX((int)ix, 0), PIXELY(12, 0));
      	    back_sprite(SPRITE_SQUISHED + 1 + (ix << 1), 0,
      		PIXELX((int)ix, 0), PIXELY(12, CELL_HEIGHT / 2));
      	  }
      }
      /*}}}*/
      /*{{{  push, pause, happy, dead*/
      {
      	unsigned  ix;
      	
      	for(ix = 4; ix--;)
      	  back_sprite(SPRITE_PLAYER_PUSH + ix, 0,
      	      PIXELX(5, 0), PIXELY((int)ix, 0));
      	for(ix = 4; ix--;)
      	  back_sprite(SPRITE_PLAYER_REST + ix, 0,
      	      PIXELX(5, 0), PIXELY((int)ix + 4, 0));
      	for(ix = 2; ix--;)
      	  back_sprite(SPRITE_PLAYER_HAPPY + ix, 0,
      	      PIXELX(5, 0), PIXELY((int)ix + 8, 0));
      	if(data.gender != False)
      	  for(ix = 2; ix--;)
      	    back_sprite(SPRITE_PLAYER_DEAD + ix, 0,
      	      PIXELX(5, 0), PIXELY((int)ix + 10, 0));
      }
      /*}}}*/
      back_sprite(SPRITE_BALL, 0, PIXELX(6, CELL_WIDTH / 2 - BALL_WIDTH / 2),
    	  PIXELY(0, CELL_HEIGHT / 4 - BALL_HEIGHT / 2));
      back_sprite(SPRITE_SEAT, 0, PIXELX(6, 0), PIXELY(0, CELL_HEIGHT / 2));
      back_sprite(SPRITE_DEN, 0, PIXELX(6, 0), PIXELY(1, 0));
      back_sprite(SPRITE_CHERRY, 0, PIXELX(6, 0), PIXELY(2, 0));
      /*{{{  prizes*/
      {
      	unsigned  ix;
      	
      	for(ix = SPRITE_PRIZES; ix--;)
      	  back_sprite(SPRITE_PRIZE_BASE + ix, 0,
      	      PIXELX(6, 0), PIXELY((int)ix + 3, 0));
      	for(ix = 3; ix--;)
      	  back_sprite(SPRITE_DIAMOND + ix, 0, 
      	      PIXELX(10, CELL_WIDTH / 2), PIXELY((int)ix + 8, 0));
      }
      /*}}}*/
      /*{{{  lettering*/
      {
      	back_sprite(SPRITE_DIGITS, 0, PIXELX(5, GAP_WIDTH +
      	    (int)sprites[SPRITE_EXTRA].size.x),
      	    PIXELY(CELLS_DOWN - 1, CELL_HEIGHT / 2 - DIGIT_HEIGHT / 2));
      	back_sprite(SPRITE_EXTRA, 0, PIXELX(5, 0), PIXELY(CELLS_DOWN - 1,
      	    CELL_HEIGHT / 2 - (int)sprites[SPRITE_EXTRA].size.y / 2));
      	back_sprite(SPRITE_EXTRA + 1, 0,
      	    PIXELX(CELLS_ACROSS - 1, CELL_WIDTH) -
      	      (int)sprites[SPRITE_EXTRA + 1].size.x,
      	    PIXELY(CELLS_DOWN - 1,
      	      CELL_HEIGHT / 2 - (int)sprites[SPRITE_EXTRA + 1].size.y / 2));
      }
      /*}}}*/
      /*{{{  apples*/
      {
      	unsigned  ix;
      	
      	for(ix = 2; ix--;)
      	  back_sprite(SPRITE_GHOST + ix, 0,
      	      PIXELX(10, CELL_WIDTH / 2), PIXELY((int)ix, 0));
      	for(ix = 6; ix--;)
      	  back_sprite(SPRITE_APPLE + ix, 0,
      	      PIXELX(10, apple_sizes[ix].offset.x + CELL_WIDTH / 2),
      	      PIXELY((int)ix + 2, apple_sizes[ix].offset.y));
      }
      /*}}}*/
      /*{{{  backgrounds*/
      {
      	unsigned  ix;
      	
      	color_set((int)0);
      	for(ix = BACKGROUNDS * FILLS; ix--;)
      	  {
            unsigned long foreground, background;
      Pixmap stipple;
            background = data.mono != False ? display.white :
      		colors[display.dynamic ? COLOR_DYNAMIC :
      		backgrounds[ix % BACKGROUNDS][0]].pixel;
            stipple = fills[ix / BACKGROUNDS].mask;
            foreground = data.mono != False ?
      		display.black : colors[display.dynamic ?
      		COLOR_DYNAMIC + 1 : backgrounds[ix % BACKGROUNDS][1]].pixel;
            video_pattern(foreground, background, stipple);
            video_fill(display.back, GCN(GC_BOARD),
      		PIXELX(7, -(GAP_WIDTH / 2)),
      		PIXELY((int)ix, -(GAP_HEIGHT / 2)),
      		(CELL_WIDTH + GAP_WIDTH) * 3, CELL_HEIGHT + GAP_HEIGHT);
            video_rect(display.back, GCN(GC_SET),
      		PIXELX(7, -(GAP_WIDTH / 2)),
      		PIXELY((int)ix, -(GAP_HEIGHT / 2)),
      		(CELL_WIDTH + GAP_WIDTH) * 3 - 1,
      		CELL_HEIGHT + GAP_HEIGHT - 1);
      	  }
      	for(ix = BACKGROUNDS * FILLS; ix--;)
      	  {
      	    back_sprite(SPRITE_APPLE, 1, PIXELX(8,
      		(CELL_WIDTH + GAP_WIDTH) / 2), PIXELY((int)ix, 0));
      	    back_sprite(SPRITE_CHERRY, 1, PIXELX(8,
      		-(CELL_WIDTH + GAP_WIDTH) / 2), PIXELY((int)ix, 0));
      	  }
      	global.count = 1;
      }
      /*}}}*/
      /*{{{  spawn things*/
      {
      	unsigned  ix;
      	
      	for(ix = 5; ix--;)
      	  spawn_monster(0, ix, 3, 3, (int)ix, CELLS_DOWN, 0, 0);
      	spawn_monster(0, 4, 3, 7, 5, CELLS_DOWN, 0, 0)->pushing = 1;
      	spawn_monster(0, 4, 3, 9, 6, CELLS_DOWN, 0, 0);
      	happy = spawn_monster(0, 4, 3, 10, 7, CELLS_DOWN, 0, 0);
      	spawn_monster(0, 3, 3, 6, 8, CELLS_DOWN, 0, 0)->chew = 1;
      	spawn_monster(0, 6, 0, 0, 9, CELLS_DOWN, 0, 0);
      	spawn_apple(10, CELLS_DOWN, CELL_WIDTH / 2, 0)->count = 0;
      }
      /*}}}*/
      video_copy(display.back, display.copy, GCN(GC_COPY),
    	  0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0);
      refresh_window();
      timer_set(FRAME_RATE * 2, TIMING_OFF);
    }
    /*}}}*/
  if(global.pause || global.pressed & (1 << KEY_QUIT | 1 << KEY_KEYBOARD))
    /*{{{  end*/
    {
      assert(rts != (PROTOANIMATE((*)))NULL);
      then = (*rts)((PROTOVOID(*))NULL);
    }
    /*}}}*/
  else
    /*{{{  animate*/
    {
      MONSTER   *mptr;
      unsigned  count;
      unsigned  dir;
      
      dir = global.pressed & ~pressed & 0xF;
      if(dir)
    	dir = choose_direction(dir) | 4;
      /*{{{  throw?*/
      if(global.throw == 1)
      	{
      	  global.throw = 2;
	  /*{{{  new backgrounds*/
	  {
	    if(global.count == BACKGROUNDS)
	      {
	  	color_set(BACKGROUND_DRONES);
	  	global.count = 0;
	      }
	    else
	      color_set((int)global.count++ % BACKGROUNDS);
	  }
	  /*}}}*/
      	  if(happy->type == 4)
      	    {
      	      unsigned ix;
      	      
      	      for(ix = 8; ix--;)
      		if(monster.list[0].face == player_dies[ix])
      		  break;
      	      happy->type = SPRITE_PLAYER_DEAD + ix * 2;
      	    }
      	  else
      	    happy->type = 4;
	  /*{{{  xtra*/
	  {
	    unsigned  ix;
	    unsigned  got;
	    
	    got = extra.got;
	    extra.got = happy->type != 4 ? 0x1F : 0;
	    for(ix = 10; ix--;)
	      {
	  	if(ix & 1)
	  	  create_xtra_monster(ix >> 1);
	  	back_sprite(SPRITE_XTRA + ix, 1,
	  	    PIXELX(2, 0), PIXELY((int)ix, 0));
	      }
	    add_background(PIXELX(2, 0), PIXELY(0, 0),
	  	CELL_WIDTH, CELL_HEIGHT * 10 + GAP_HEIGHT * 9);
	    extra.got = got;
	    create_xtra_monster(extra.select);
	  }
	  /*}}}*/
      	}
      /*}}}*/
      for(mptr = monster.list, count = monster.monsters; count--; mptr++)
    	{
	  /*{{{  new face?*/
	  if(dir)
	    {
	      if(mptr->type < 5 && mptr->face < 10)
	  	{
	  	  if(mptr->face < 6 || dir >= 6)
	  	    {
	  	      unsigned  shift;
	  	      
	  	      shift = mptr->face > 7;
	  	      mptr->dir = dir & 0x3;
	  	      new_face(mptr);
	  	      if(shift)
	  		mptr->face += 6;
	  	      if(mptr->type == 2)
	  		{
	  		  extra.select++;
	  		  if(extra.select == 5)
	  		    {
	  		      extra.select = 0;
	  		      extra.got ^= 0x1F;
	  		    }
	  		  create_xtra_monster(extra.select);
	  		}
	  	    }
	  	}
	      else if(happy->type != 4)
	  	{
	  	  unsigned ix;
	  	  
	  	  for(ix = 8; ix--;)
	  	    if(monster.list[0].face == player_dies[ix])
	  	      break;
	  	  happy->type = SPRITE_PLAYER_DEAD + ix * 2;
	  	}
	    }
	  /*}}}*/
    	  if(!mptr->cycle)
	    /*{{{  next image*/
	    {
	      mptr->cycle = mptr->type == 6 ? DIAMOND_CYCLES : MONSTER_CYCLES;
	      mptr->image++;
	      if(mptr->image == (mptr->type != 6 ?
	    	  MONSTER_IMAGES : DIAMOND_IMAGES))
	    	mptr->image = 0;
	      if(mptr == happy && mptr->type != 4)
	    	{
	    	  mptr->type++;
	    	  if(mptr->type == SPRITE_PLAYER_DEAD + 8)
	    	    mptr->type = SPRITE_PLAYER_DEAD;
	    	  else if(mptr->type == SPRITE_PLAYER_DEAD + 16)
	    	    mptr->type = SPRITE_PLAYER_DEAD + 8;
	    	}
	    }
	    /*}}}*/
    	  mptr->cycle--;
    	}
      if(!apple.list[0].count)
    	{
    	  apple.list[0].count = MONSTER_CYCLES;
    	  apple.list[0].state++;
    	  if(apple.list[0].state == 6)
    	    apple.list[0].state = 0;
    	}
      apple.list[0].count--;
      pressed = global.pressed;
    }
    /*}}}*/
  return then;
}
/*}}}*/
/*{{{  void back_mris(shell)*/
static VOIDFUNC back_mris
FUNCARG((shell),
	unsigned  shell
)
/*
 * Stuff a M R I or S sprite onto the background
 */
{
  unsigned  ix;
  
  for(ix = 4; ix--;)
    back_sprite(SPRITE_MRIS + ix + 4 * !!((1 << ix) & shell), 0,
	PIXELX(4 + (int)ix, 0), PIXELY(-1, 0));
  return;
}
/*}}}*/
/*{{{  void back_score(sptr, ix. split)*/
static VOIDFUNC back_score
FUNCARG((sptr, ix, split),
	HIGH_SCORE CONST *sptr
ARGSEP  unsigned  ix
ARGSEP  unsigned  split
)
/*
 * stuff a score onto the background
 */
{
  size_t    length;
  char      string[SCORING_LEN];
  char      *name;
  int       y;
  int       height;
  
  /* Keep each displayed field within its column. */
  unsigned minutes = sptr->elapsed / 60;
  snprintf(string, sizeof(string), "%8lu  %3u %3u'%02u\"",
      sptr->score > 99999999UL ? 99999999UL : sptr->score,
      sptr->screen > 999U ? 999U : sptr->screen,
      minutes > 999U ? 999U : minutes, sptr->elapsed % 60);
  assert(strlen(string) == SCORING_LEN - 1);
  height = font.digitup + font.digitdown + font.ascent + font.descent;
  y = PIXELY((int)ix, CELL_HEIGHT / 2);
  video_text(display.back, GCN(GC_TEXT),
      PIXELX(0, 0), y + (split ? height > CELL_HEIGHT + GAP_HEIGHT - 2 ?
      font.digitup - (CELL_HEIGHT + GAP_HEIGHT) / 2 + 1 :
      font.digitup - height / 2 :
      font.center), string, SCORING_LEN - 1);
  name = (char *)sptr->name;
  length = strlen(name);
  /*{{{  compress name for this user?*/
  if(ix == CELLS_DOWN && (length + SCORING_LEN) * font.width >
      PIXELX(CELLS_ACROSS, -GAP_WIDTH) - PIXELX(0, 0) && scoring.alternate)
    {
      char      *ptr;
      
      ptr = name;
      while(*ptr)
      {
  	for(name = ptr; *name == ' '; name++)
  	  /* EMPTY */;
  	ptr = strchr(name, ' ');
  	if(!ptr)
  	  break;
  	if(ptr[-1] != '.' && ptr > name + 1)
  	  break;
      }
      length = ptr ? ptr - name : strlen(name);
      if(!length)
  	{
  	  name = scoring.alternate;
  	  length = strlen(name);
  	}
    }
  /*}}}*/
  video_text(display.back, GCN(GC_TEXT),
      split ? PIXELX(CELLS_ACROSS, -GAP_WIDTH) - (length + 2) * font.width :
      PIXELX(0, 0) + SCORING_LEN * font.width,
      y + (split ? height > CELL_HEIGHT + GAP_HEIGHT - 2 ?
	  (CELL_HEIGHT + GAP_HEIGHT) / 2 - 1 - font.descent :
	  height / 2 - font.descent : font.center),
      name, (int)length);
  return;
}
/*}}}*/
/*{{{  int back_title(tptr, ix)*/
static int back_title
FUNCARG((tptr, ix),
	TITLE CONST *tptr
ARGSEP  unsigned  ix
)
/*
 * stuff a title line onto the background
 */
{
  char      buffer[65];
  size_t    length;
  
  /*{{{  insert key string?*/
  {
    char CONST *ptr;
  	
    ptr = strchr(tptr->text, '%');
    if(!ptr)
      snprintf(buffer, sizeof(buffer), "%s", tptr->text);
    else
      snprintf(buffer, sizeof(buffer), tptr->text, key_name(data.keysyms[tptr->ix]));
  }
  /*}}}*/
  length = strlen(buffer);
  if(length)
    {
      char CONST *ptr;
      unsigned  shift;
   
      ptr = strchr(buffer, '-');
      if(ptr)
	shift = (ptr - buffer + 1) * font.width;
      else
	shift = 0;
      video_opaque_text(display.back, GCN(GC_TEXT),
	  CENTER_X - (int)(shift ? shift : length * font.width / 2),
	  PIXELY((int)ix, CELL_HEIGHT / 2) + font.center,
	  buffer, (int)length);
    }
  return length;
}
/*}}}*/
/*{{{  void list_scores()*/
extern VOIDFUNC list_scores FUNCARGVOID
/*
 * lists the high score tables to stdout
 */
{
  unsigned  flag;
  
  flag = 1;
  if(scoring.high[0].score)
    {
      printf("%s\n", table_names[0]);
      print_scores(scoring.high);
      flag = 0;
    }
  if(scoring.today[0].score)
    {
      printf("%s\n", table_names[1]);
      print_scores(scoring.today);
      flag = 0;
    }
  else if(!flag)
    printf("Everyone's busy today.\n");
  if(scoring.personal[0].score)
    {
      printf("%s'%s personal best\n", scoring.mine.name,
	  scoring.mine.name[strlen(scoring.mine.name) - 1] == 's' ? "" : "s");
      print_scores(scoring.personal);
      flag = 0;
    }
  if(flag)
    printf("Nobody ever plays with me.\n");
  return;
}
/*}}}*/
/*{{{  int move_demo(cycle)*/
static int move_demo
	FUNCARG((cycle),
unsigned  cycle
)
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
	  if(!mptr->stop || mptr->type == 6 || mptr->face == 10)
	    {
	      if(!mptr->cycle)
		{
		  mptr->cycle = mptr->type == 6 ? DIAMOND_CYCLES : cycle;
		  mptr->image++;
		  if(mptr->image == (mptr->type == 6 ?
		      DIAMOND_IMAGES : MONSTER_IMAGES))
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
static VOIDFUNC move_mris FUNCARGVOID
/*
 * moves M R I S sprites around the board
 * towards the top
 */
{
  unsigned  i;
  MONSTER   *mptr;

  for(mptr = &monster.list[1], i = monster.monsters - 1; i--; mptr++)
    if(mptr->type == 6)
      /*{{{  diamond*/
      {
      	if(!mptr->count--)
      	  {
      	    global.diamond = 2;
      	    mptr->type = 5;
      	  }
      	if(mptr->count == DIAMOND_GHOSTING)
      	  mptr->ghosting = GHOSTING;
      	if(!mptr->cycle)
      	  {
      	    mptr->cycle = mptr->type == 6 ? DIAMOND_CYCLES : MONSTER_CYCLES;
      	    mptr->image++;
      	    if(mptr->image == (mptr->type == 6 ?
      		DIAMOND_IMAGES : MONSTER_IMAGES))
      	      mptr->image = 0;
      	  }
      	mptr->cycle--;
      }
      /*}}}*/
    else
      /*{{{  letter*/
      {
      	CELL      *cptr;
      	unsigned  chosen;
      	
      	assert(mptr->type >= SPRITE_MRIS && mptr->type < SPRITE_MRIS + 8);
      	cptr = BOARDCELL(mptr->cell.x, mptr->cell.y);
      	chosen = !(monster.den & 1 << ((mptr->type - SPRITE_MRIS) & 3));
      	if(mptr->offset.x || mptr->offset.y)
      	  move_movable(mptr, cptr);
      	else if(!mptr->cell.y && chosen &&
      	    mptr->cell.x == ((mptr->type - SPRITE_MRIS) & 3) + 4)
      	  {
      	    back_sprite(mptr->type, 0, mptr->old_pixel.x, mptr->old_pixel.y);
      	    mptr->type = 5;
      	    monster.normals--;
      	  }
      	else
      	  {
      	    unsigned  valid;
      	    unsigned  temp;
      	  
      	    valid = valid_directions(mptr, cptr);
      	    temp = valid & (0xF ^ (1 << (mptr->dir ^ 1)));
      	    if(temp)
      	      valid &= temp;
      	    if(chosen)
      	      {
      		temp = run_home(mptr, cptr);
      		if(temp & valid)
      		  valid &= temp;
      	      }
      	    else
      	      valid &= 0xF;
      	    mptr->dir = choose_direction(valid);
      	    move_movable(mptr, cptr);
      	  }
      }
      /*}}}*/
  return;
}
/*}}}*/
/*{{{  void print_scores(sptr)*/
static VOIDFUNC print_scores
FUNCARG((sptr),
	HIGH_SCORE CONST *sptr
)
{  
  unsigned  count;
  
  for(count = HIGH_SCORES; sptr->score && count--; sptr++)
    printf("%9lu %3u %3u\'%02u\" %s\n", sptr->score, sptr->screen,
	sptr->elapsed / 60, sptr->elapsed % 60, sptr->name);
  return;
}
/*}}}*/
