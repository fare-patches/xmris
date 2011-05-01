/* Copyright (C) 1993, 1992 Nathan Sidwell */
/* RCS $Id: xmris.h 1.2 Tue, 16 Mar 1999 11:28:16 +0000 nathan $ */
#include "common.h"
#define COPYRIGHT "Copyright (C) 1995-1992 Nathan Sidwell"
/*{{{  function pointers*/
/* see xmris.c for explanation */
#define PROTOVOID(name) VOIDFUNC (name) NESTEDPROTOARG((VOIDARG))
#define PROTOANIMATE(name) PROTOVOID(*name NESTEDPROTOARG((PROTOVOID(*))))
#if __STDC__ || FUNCPROTO & 1
#define FUNCANIMATE(name, arg) \
    PROTOVOID(*name (PROTOVOID(*arg)))
#else
#define FUNCANIMATE(name, arg) PROTOVOID(*name(arg)) PROTOVOID(*arg);
#endif
/*}}}*/
/*{{{  defines*/
/*{{{  board sizes*/
#define KNOCK_THROUGH /* how far we go to knock through a new cell */\
    ((CELL_WIDTH + CELL_HEIGHT + GAP_WIDTH + GAP_HEIGHT) / 3)
#define FLOOD_FILL    32  /* how many branches in the distance flood fill */
#define DEFAULT_CHERRY_X  ((CELLS_ACROSS - 1) >> 1)
#define DEFAULT_CHERRY_Y  0
/*}}}*/
/*{{{  frame rate*/
/* these are in uS, if it is too fast, the load line
 * at the bottom will start to grow */
#define FRAMES_PER_SECOND 27
#define FRAME_RATE    (unsigned long)(1000000 / FRAMES_PER_SECOND)
#define ZOOM_RATE (FRAME_RATE / 4)
#define SCORE_RATE (FRAME_RATE / 4)
#define NAME_RATE (FRAME_RATE / 2)
#define FRAME_SCALE 1024    /* precision of frame delay scaling */
#define FRAME_RATIO_UPDATE (5 * FRAMES_PER_SECOND) /* frame ratio calc */
#define FRAME_MISS_START  15  /* start dilation calc */
#define FRAME_MISS_STOP   5   /* stop dilation calc */
#define SCORE_SCALE 1024    /* precision of score scaling */
/*}}}*/
/*{{{  score file*/
#define SCORE_THRESHOLD 10000 /* minimum score for fame */
#define SCORE_ROUND     5     /* scores are rounded */
#define HIGH_SCORES (CELLS_DOWN - 2)
#define NAME_LEN  32
/*}}}*/
#define START_LIVES     3
/*{{{  velocities*/
#define VEL_X         GAP_WIDTH             /* how far we move per step */
#define VEL_Y         GAP_HEIGHT
#define VEL_X_FAST    (VEL_X * 5 / 4)       /* how far a fast stride is */
#define VEL_Y_FAST    (VEL_Y * 5 / 4)
#define FAST_STEPS    (CELL_WIDTH / VEL_X / 4) /* 1/2 no of fast strides */
#define APPLE_VEL_Y   (VEL_Y * 3 / 2)       /* how fast the apple falls */
#define APPLE_VEL_X   (VEL_X / 2)           /* how fast it goes sideways */
#define APPLE_ACC     (VEL_Y / 2)
#define APPLE_SINK_Y  1                     /* sinks to matrix */
#define BALL_STEPS    2                     /* relative ball speed */
#define BALL_EX       (CELL_WIDTH / 2)      /* how fast we explode */
#define BALL_EY       (CELL_HEIGHT / 2)
#define ZOOM_X        VEL_X                 /* how fast the zoom is */
#define ZOOM_Y        VEL_Y
#define STEP_OFF      1                     /* steps taken for granted */
/*}}}*/
/*{{{  ball stuff*/
#define BALL_EXPLODE    (BOARD_WIDTH / BALL_EX) /* how much we explode */
#define BALL_DISTANCE   6    /* how close to runaway from the ball */
/*}}}*/
#define DIE_DELAY       4  /* ticks for death spiral */
#define SCORE_SHOW      (2 * FRAMES_PER_SECOND) /* frames for board scores */
#define DISPLAY_HOLD    (SCORE_SHOW * 2)
#define SCORE_HOLD      (SCORE_SHOW * 12)
#define HISTORY_SHOW    3   /* how often we show the history */
/*{{{  apple stuff*/
#define APPLE_ROCK_DELAY  8    /* ticks for it to rock */
#define APPLE_SPLIT_DELAY 8    /* ticks for split */
#define APPLE_DECAY_DELAY 8    /* ticks for decay */
#define APPLE_ROT_DELAY   8    /* ticks for rot */
#define APPLE_FALL_SPLIT  (CELL_HEIGHT + GAP_HEIGHT + APPLE_VEL_Y)
    /* safe fall distance */
#define APPLE_GHOST_DELAY (((CELL_HEIGHT + GAP_HEIGHT) / VEL_Y + \
    (CELL_WIDTH + GAP_WIDTH) / VEL_X) * 2)
/*}}}*/
/*{{{  den escape*/
#define DEN_ESCAPE_PROB   8     /* normal monster gets out of the den */
#define DEN_ESCAPE_DELAY  6     /* how many den flashes before escape */
#define DEN_ESCAPE_FLASH  0x8   /* time per den flash */
/*}}}*/
/*{{{  normal & muncher*/
#define GO_MUNCH_PROB      1    /* difficulty scale that normal goes munchy */
#define TRAPPED_MUNCH_PROB  8   /* go munch if trapped */
#define DIFFICULTY_PEDESTAL 2   /* offset for difficulty */
#define STOP_TOGGLE_CONT_PEDESTAL  2
#define STOP_TOGGLE_CONT_SCREEN_SCALE  5
#define STOP_TOGGLE_CONT_STOP   10
#define STOP_TOGGLE_CONT_NEAR   2
#define SET_CONT_PEDESTAL      18    /* continue flag set */
#define CLEAR_CONT_PEDESTAL    24    /* continue flag clear */
#define CLEAR_CONT_VISIT_SCALE (CELLS_ACROSS * CELLS_DOWN / 8)
#define CLEAR_CONT_MONSTER_SCALE 3
#define PUSH_TURN_PROB    64    /* muncher turns around when pushing */
#define MUNCH_CONT_PEDESTAL     18  /* continue munching */
#define MUNCH_CONT_SCREEN_SCALE 4   /* continue munching scale */
#define GO_MUNCH_DELAY    16    /* pause when we start munching */
#define STOP_MUNCH_DELAY  16    /* pause when we stop munching */
#define PANIC_COUNT       (CELLS_DOWN * (CELL_HEIGHT + GAP_HEIGHT) / VEL_Y)
/*}}}*/
/*{{{  xtra & drone*/
#define XTRA_NEW_DELAY    (FRAMES_PER_SECOND * 4)
#define XTRA_GOT_DELAY    (FRAMES_PER_SECOND * 2)
#define XTRA_BIRTH_DELAY  ((CELL_WIDTH + GAP_WIDTH) / VEL_X + 4)
    /* pause while giving birth to drones */
#define XTRA_CONT_OFF_PROB 16   /* that xtra stops continuing */
#define CHOMP_DELAY     (CELL_WIDTH / VEL_X * 6)
    /* how long to eat an apple */
#define XTRA_HOME_DELAY   (FRAMES_PER_SECOND * 30)
/*}}}*/
#define MONSTER_CYCLES    6     /* cycle counter */
#define SCORE_CYCLES (MONSTER_CYCLES / 2)
#define MONSTER_IMAGES    2     /* how many different images */
#define DIAMOND_IMAGES    3
#define DIAMOND_CYCLES    2
/*{{{  diamond stuff*/
#define DIAMOND_PROB      1     /* chance of apple turning to diamond */
#define DIAMOND_VEL_Y     ((CELL_HEIGHT + GAP_HEIGHT) / 4)
#define DIAMOND_DELAY     (20 * FRAMES_PER_SECOND)
#define DIAMOND_GHOSTING  (5 * FRAMES_PER_SECOND)
/*}}}*/
#define GHOSTING (0x100 | ((GHOSTING_HEIGHT - CELL_HEIGHT) / 2 << 4) | \
    (GHOSTING_WIDTH - CELL_WIDTH) / 2)
/*{{{  limits*/
#define APPLES          (APPLE_LIMIT + 8)    /* max number of apples */
#define MONSTERS        ((CELLS_DOWN - 2) * 2)  /* max number of monsters */
#define BOARD_SCORES    4   /* number of on board displayed scores */
#define BACK_UPDATES    64  /* background updates we can cope with */
/*}}}*/
/*{{{  whizz*/
#define WHIZZ_STEPS     32
#define WHIZZ_CYCLES    2
#define WHIZZ_SIZE      (int)(0xFFFF / (WHIZZ_STEPS - 1))
#define WHIZZ_PROB      64
/*}}}*/
/*{{{  timing states*/
#define TIMING_OFF    0
#define TIMING_ON     1
#define TIMING_PAUSE  2
/*}}}*/
/*}}}*/
/*{{{  macros*/
#define LETTERDIR(l)      ((l) & 3)
#define LETTERDIST(l)     ((l) >> 2)
#define LETTERPATH(d, l)  ((d) | ((l) << 2))
#define LETTEREND         0
#define LETTERPOS(x, y)   (((x) << 4) | (y))
#define LETTERX(l)        ((l) >> 4)
#define LETTERY(l)        ((l) & 0xF)
#define BOARDCELL(CX, CY) \
    (&garden[(CY) * CELL_STRIDE + (CX) + (CELL_LEFT + CELL_TOP * CELL_STRIDE)])
/*}}}*/
/*{{{  structs*/
/*{{{  typedef struct Title*/
typedef struct Title
/* title text */
{
  char CONST *text;       /* the text */
  unsigned  ix;           /*  key index */
} TITLE;
/*}}}*/
/*{{{  typedef struct Background*/
typedef struct Background
/* area to update from back to copy to window */
{
  COORD   place;  /* top left area */
  SIZE    size;   /* size of area */
} BACKGROUND;
/*}}}*/
/*{{{  typedef struct Score*/
typedef struct Score
/* on board score display */
{
  Pixmap    mask;   /* the mask to use */
  Pixmap    image;  /* the image to display */
  COORD     place;  /* where to bung it */
  unsigned  count;  /* removal countdown */
} SCORE;
/*}}}*/
/*{{{  typedef struct Ball*/
typedef struct Ball
/* the ball state */
{
  COORD     cell;
  COORD     offset;
  COORD     pixel;
  unsigned  state;  /* state of the ball
		     * 0 held by player
		     * 1 bouncing
		     * 2 exploding
		     * 3 exploded
		     * 4 imploding
		     */
  unsigned  count;  /* count or direction or player type */
  unsigned  image;  /* player image */
} BALL;
/*}}}*/
/*{{{  typedef struct Apple*/
typedef struct Apple
/* non-background apple information */
{
  COORD     cell;       /* apple's cell */
  COORD     offset;     /* offset from center */
  COORD     pixel;      /* pixel for sprite */
  int       push;       /* horizontal push */
  unsigned  count;      /* general counter */
  unsigned  state;      /* state
			 * 0 stationary
			 * 1 rock
			 * 2 falling
			 * 3 split
			 * 4 decay
			 * 5 rot
			 * 6 delete
			 */
  unsigned  ghost;      /* ghost count */
  unsigned  distance;   /* distance we've fallen */
  unsigned  monsters;   /* monsters we've squashed */
  unsigned  chewed;     /* chewed */
  int       maypush;    /* temp push vector */
  struct Monster *list; /* list of monsters for initial fall */
  COORD     old_pixel;  /* where i was */
  unsigned  old_state;  /* what I looked like */
  unsigned  back;       /* background set */
} APPLE;
/*}}}*/
/*{{{  typedef struct Apple_Size*/
typedef struct Apple_Size
{
  SIZE    size;
  COORD   offset;
} APPLE_SIZE;
/*}}}*/
/*{{{  typedef struct Monster*/
typedef struct Monster
/* monster information */
{
  COORD     cell;     /* board cell we're related to */
  COORD     offset;   /* offset from this cell */
  COORD     pixel;    /* pixel coordinate */
  unsigned  dir;      /* direction we're moving in */
  unsigned  pause;    /* we're paused for one */
  unsigned  stop;     /* we're stopped (player only) */
  unsigned  type;     /* type
		       * 0 - normal
		       * 1 - munch
		       * 2 - xtra
		       * 3 - drone
		       * 4 - player
		       * 5 - delete
		       *>5 - demo sprite
		       */
  unsigned  face;     /* direction we're facing
		       * 0 up left,
		       * 1 down right
		       * 2 left
		       * 3 right
		       * 4 up right
		       * 5 down left
		       * 6 push left
		       * 7 push right
		       * 8 squish left
		       * 9 squish right
		       */
  int       push;     /* being pushed in this direction */
  unsigned  gomunch;  /* change munch state */
  unsigned  ghosting; /* ghosting is occuring */
  unsigned  cont;     /* continue */
  unsigned  chew;     /* chewing */
  unsigned  count;    /* counter */
  unsigned  panic;    /* help I'm about to be squashed, ghosting for player */
  unsigned  shot;     /* has been shot */
  unsigned  cycle;    /* image cycler */
  unsigned  image;    /* which image to display */
  unsigned  fast;     /* fast speed */
  unsigned  pushing;  /* pushing apple */
  unsigned  squished; /* under an apple */
  struct Monster *list; /* list of monsters for initial apple fall */
  struct Monster *tptr; /* temporary pointer */
  COORD     old_pixel;  /* where we were */
  int       old_sprite; /* what we looked like */
  unsigned  back;       /* background set */
  unsigned  on;         /* visible */
} MONSTER;
/*}}}*/
/*{{{  typedef struct High_Score*/
typedef struct High_Score
{
  time_t    stamp;          /* integrity stamp */
  unsigned  marker;         /* flag */
  unsigned long score;      /* score */
  unsigned  screen;         /* final screen */
  unsigned  elapsed;        /* playing time seconds */
  char name[NAME_LEN + 1];  /* name of player */
} HIGH_SCORE;
/*}}}*/
/*{{{  typedef struct Cell*/
typedef struct Cell
/* information about 1 cell on the board */
{
  int       depths[4];    /* depths moved to from cell */
  int       holes[4];     /* apple holes above and below (tl, tr, bl, br) */
  unsigned  distance;     /* distance from player */
  unsigned  xtra;         /* distance from xtra base */
  unsigned  ball;         /* distance from ball */
  unsigned  visit;        /* visited */
  unsigned  sprite;       /* has cherry */
  unsigned  apple;        /* apple possibility */
  unsigned  den;          /* is a den */
  unsigned  spare;        /* pad to 16 words */
} CELL;
/*}}}*/
/*}}}*/
/*{{{  board*/
EXTERN struct
/* board information */
{
  unsigned  boards;           /* number of boards loaded */
  BOARD CONST *list[BOARD_LIMIT];   /* pointers to them */
} board;
/*}}}*/
/*{{{  player*/
EXTERN struct
/* player specific information
 * note, the player place info is stored as monster 0
 */
{
  unsigned  next;       /* direction at next intersection */
  unsigned  bashed;     /* we bashed into a wall */
  BALL      old_ball;   /* what was the ball */
  BALL      ball;       /* ball information */
  unsigned  count;      /* ball return dec count */
  unsigned  thrown;     /* how many times its been thrown */
  unsigned  cherry;     /* consecutive cherry count */
  unsigned  distance;   /* distance to next cherry */
  unsigned  pressed;    /* keys we had pressed */
} player;
/*}}}*/
EXTERN CELL garden[(CELLS_DOWN + CELL_TOP * 2) * CELL_STRIDE];
/*{{{  global*/
EXTERN struct
{
  unsigned  pedestal;       /* starting screen number */
  unsigned  screen;         /* current screen number */
  unsigned long score;      /* score */
  unsigned  lives;          /* lives left (including on screen) */
  unsigned long msec;       /* number of milliseconds this screen */
  unsigned  dilation;       /* frame delay dilation factor */
  unsigned  scale;          /* score scale factor */
  unsigned  difficulty;     /* increments in to make it harder */
  unsigned  broken;         /* broken through a new path */
  unsigned  stepped;        /* player moved to new cell */
  unsigned  cherries;       /* number of cherries left */
  COORD     start;          /* player starting position */
  unsigned  dens;           /* number of dens */
  unsigned  state;          /* den state
			     * 0 - den on screen
			     * 1 - cake on screen
			     * 2 - xtras & drone running around
			     * 3 - done xtras
			     * 4 - end game
			     * 5 - mid game demo
			     * 6 - demo
			     * 7 - defining keys
			     * 8 - zooming
			     */
#define MODE_GAME_DEMO 5
#define MODE_DEMO      6
#define MODE_KEY_DEF   7
#define MODE_ZOOM      8
  unsigned  diamond;        /* diamond state */
  unsigned  missed;         /* missed interrupt count */
  unsigned  count;          /* general counter */
  unsigned  visited;        /* amount of cells visited */
  unsigned  quit;           /* quit selected */
  unsigned  pause;          /* game paused */
  unsigned  iconic;         /* game iconized */
  unsigned  pressed;        /* keys pressed */
  unsigned  throw;          /* throw key state */
  KeySym    key;            /* last key pressed */
} global;
/*}}}*/
/*{{{  extra*/
EXTERN struct
{
  unsigned  got;      /* one we've got */
  unsigned  select;   /* the one which is selected */
  unsigned  escape;   /* its out/homesick */
  unsigned  score;    /* last checked score */
  unsigned  count;    /* change countdown */
} extra;
/*}}}*/
/*{{{  apple*/
EXTERN struct
{
  APPLE     list[APPLES]; /* apple list */
  unsigned  apples;       /* number of apples out */
  unsigned  moving;       /* an apple is moving */
} apple;
/*}}}*/
/*{{{  history*/
EXTERN struct
{
  unsigned  prize;        /* did we get the prize? */
  unsigned  ending;       /* how did the screen end? */
  unsigned long msec;     /* how many milliseconds have been played? */
  unsigned long times[CELLS_DOWN];  /* how many milliseconds per screen? */
} history;
/*}}}*/
/*{{{  monster*/
EXTERN struct
{
  MONSTER   list[MONSTERS]; /* monsters [0] is player */
  CELL      *player;        /* where the player is */
  unsigned  monsters;       /* number of monsters out (inc player) */
  unsigned  delay;          /* escape delay */
  unsigned  den;            /* monster spawn count */
  unsigned  normals;        /* normal monsters alive */
  unsigned  drones;         /* drones out */
  unsigned  nearest;        /* what was the nearest distance to player */
  unsigned  farthest;       /* what was the farthest distance to player */
} monster;
/*}}}*/
/*{{{  update*/
EXTERN struct
{
  COORD     tl;       /* top left */
  COORD     br;       /* bottom right */
  unsigned  set;      /* tl & br set */
  struct
  {
    unsigned  backs; /* number of areas to update to window */
    BACKGROUND list[BACK_UPDATES]; /* the area information */
  } back;
  struct
  {
    unsigned  scores;  /* number of displayed scores */
    SCORE     list[BOARD_SCORES];   /* the displayed scores */
  } score;
} update;
/*}}}*/
/*{{{  scoring*/
EXTERN struct
{
  HIGH_SCORE high[HIGH_SCORES];
  HIGH_SCORE today[HIGH_SCORES];
  HIGH_SCORE personal[HIGH_SCORES];
  HIGH_SCORE mine;
  HIGH_SCORE *display;
  char *alternate;
} scoring;
/*}}}*/
/*{{{  tables*/
extern APPLE_SIZE CONST apple_sizes[6];
extern COORD ball_hold[16];
extern COORD CONST ball_throw[8];
#define BALL_RETURNS 3
extern unsigned CONST ball_returns[BALL_RETURNS];
extern unsigned CONST ball_dir[8];
extern unsigned CONST player_dies[8];
#define SQUISH_SCORES 7
extern unsigned CONST squish_scores[SQUISH_SCORES];
extern TITLE title_text[];
/*}}}*/
/*{{{  prototypes*/
/*{{{  apple*/
extern int apple_stop PROTOARG((MONSTER *, CELL *));
extern VOIDFUNC move_apples PROTOARG((VOIDARG));
extern VOIDFUNC panic_monsters PROTOARG((int, int, CELL *));
extern APPLE *spawn_apple PROTOARG((int, int, int, int));
extern VOIDFUNC squish_monster PROTOARG((MONSTER *));
/*}}}*/
/*{{{  demo*/
extern PROTOANIMATE(animate_demo);
extern PROTOANIMATE(animate_diamond);
extern PROTOANIMATE(animate_extra_life);
extern PROTOANIMATE(animate_history);
extern VOIDFUNC list_scores PROTOARG((VOIDARG));
/*}}}*/
/*{{{  draw*/
extern VOIDFUNC add_background PROTOARG((int, int, unsigned, unsigned));
extern PROTOANIMATE(animate_zoom);
extern VOIDFUNC blank_board PROTOARG((BOARD CONST *, unsigned));
extern VOIDFUNC bounding_box PROTOARG((int, int, unsigned, unsigned));
extern VOIDFUNC create_boards PROTOARG((VOIDARG));
extern VOIDFUNC create_xtra_monster PROTOARG((unsigned));
extern VOIDFUNC draw_extra PROTOARG((VOIDARG));
extern VOIDFUNC draw_extra_letter PROTOARG((unsigned));
extern VOIDFUNC new_board PROTOARG((VOIDARG));
extern VOIDFUNC refresh_window PROTOARG((VOIDARG));
extern VOIDFUNC set_back_sprite PROTOARG((unsigned, unsigned, unsigned));
extern VOIDFUNC show_updates PROTOARG((VOIDARG));
/*}}}*/
/*{{{  makemris*/
extern VOIDFUNC color_cycle PROTOARG((VOIDARG));
extern VOIDFUNC color_set PROTOARG((int));
extern VOIDFUNC create_widget PROTOARG((VOIDARG));
#ifndef NDEBUG
extern int error_handler PROTOARG((Display *, XErrorEvent *));
#endif /* NDEBUG */
extern VOIDFUNC fatal_error PROTOVARARG((char CONST *, VARARG));
extern VOIDFUNC list_help PROTOARG((char CONST *));
extern VOIDFUNC nadger_widget_colors PROTOARG((Widget, WidgetClass));
extern VOIDFUNC open_toolkit PROTOARG((int, String *));
/*}}}*/
/*{{{  monster*/
extern VOIDFUNC extra_dies PROTOARG((VOIDARG));
extern MONSTER *extra_escape PROTOARG((VOIDARG));
extern VOIDFUNC move_monsters PROTOARG((VOIDARG));
extern VOIDFUNC new_xtra PROTOARG((VOIDARG));
extern MONSTER *spawn_monster
    PROTOARG((unsigned, unsigned, unsigned, unsigned, int, int, int, int));
/*}}}*/
/*{{{  move*/
extern VOIDFUNC back_sprite PROTOARG((unsigned, unsigned, int, int));
extern unsigned choose_direction PROTOARG((unsigned));
extern CELL *drop_apple PROTOARG((APPLE *, CELL *));
extern CELL *move_diamond PROTOARG((MONSTER *, CELL *));
extern CELL *move_movable PROTOARG((MONSTER *, CELL *));
extern CELL *move_muncher PROTOARG((MONSTER *));
extern VOIDFUNC munch_hole PROTOARG((CELL *, int, int));
extern VOIDFUNC new_face PROTOARG((MONSTER *));
extern unsigned run_home PROTOARG((MONSTER *, CELL *));
extern unsigned valid_directions PROTOARG((MONSTER *, CELL *));
/*}}}*/
/*{{{  player*/
extern VOIDFUNC bounce_ball PROTOARG((VOIDARG));
extern VOIDFUNC killed_player PROTOARG((VOIDARG));
extern unsigned move_player PROTOARG((VOIDARG));
/*}}}*/
/*{{{  scoring*/
extern VOIDFUNC check_scores PROTOARG((VOIDARG));
extern VOIDFUNC high_score PROTOARG((unsigned long, unsigned, unsigned long));
extern VOIDFUNC init_scores_early PROTOARG((VOIDARG));
extern VOIDFUNC init_scores PROTOARG((VOIDARG));
extern VOIDFUNC cleanup_scores PROTOARG((VOIDARG));
/*}}}*/
/*{{{  timer*/
extern VOIDFUNC timer_close PROTOARG((VOIDARG));
extern VOIDFUNC timer_open PROTOARG((VOIDARG));
extern unsigned timer_set PROTOARG((unsigned long, unsigned));
extern VOIDFUNC timer_wait PROTOARG((VOIDARG));
/*}}}*/
/*{{{  xmris*/
extern PROTOANIMATE(animate_game);
extern VOIDFUNC add_score PROTOARG((unsigned, int, int));
extern VOIDFUNC calc_distances PROTOARG((VOIDARG));
extern VOIDFUNC calc_extra_home PROTOARG((unsigned));
extern unsigned chaotic PROTOARG((VOIDARG));
extern size_t itoa PROTOARG((char *, unsigned long, unsigned));
extern int main PROTOARG((int, char CONST **));
/*}}}*/
/*}}}*/
