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
#include "patchlevel.h"
#ifndef EXTERN
#define EXTERN extern
#endif
/*{{{  ANSI or K&R?*/
#ifdef __STDC__
#include <stdarg.h>
#define PROTOARGLIST(list) list
#define VARARG
#define FUNCVARARG ... )
#define FUNCARGLIST(list) (
#define FUNCARGSEP ,
#define FUNCARGTERM )
#define FUNCARGVOID ()
#define VARARGSET(args, last) va_start(args, last)
#else /* !__STDC__ */
#include <varargs.h>
#define PROTOARGLIST(list) ()
#define const
#define volatile
#define VARARG , va_alist
#define FUNCVARARG va_dcl
#define FUNCARGLIST(list) list
#define FUNCARGSEP  ;
#define FUNCARGTERM ;
#define FUNCARGVOID ()
#define VARARGSET(args, last) va_start(args)
#ifndef DATE
#define DATE "July 1992"
#endif /* DATE */
#define memmove memcpy /* K&R doesn't have memmove, but memcpy is a superset */
#endif /* __STDC__ */
/*}}}*/
/*{{{  include*/
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
/*}}}*/
/*{{{  defines*/
/*{{{  board sizes*/
#define CELLS_ACROSS  12  /* size of the board */
#define CELLS_DOWN    13
#define CELL_WIDTH    32  /* size of the internal cell */
#define CELL_HEIGHT   32  /* monster sprites must be this size */
#define GAP_WIDTH     4   /* spacing of the cells */
#define GAP_HEIGHT    4
#define PLAYER_START_X  ((CELLS_ACROSS - 1) >> 1)
#define PLAYER_START_Y  (CELLS_DOWN - 1)
#define DEN_X           ((CELLS_ACROSS - 1) >> 1)
#define DEN_Y           (CELLS_DOWN >> 1)
#define CELL_STRIDE   16  /* board array is bigger than the board */
#define CELL_TOP      2   /* and the board is offset by this much */
#define CELL_LEFT     2
#define KNOCK_THROUGH /* how far we go to knock through a new cell */\
    ((CELL_WIDTH + CELL_HEIGHT + GAP_WIDTH + GAP_HEIGHT) / 3)
#define BORDER_TOP    (CELL_HEIGHT + 2) /* placing of the board */
#define BORDER_BOTTOM (CELL_HEIGHT + 2) /* on the window */
#define BORDER_LEFT   2
#define BORDER_RIGHT  2
#define FLOOD_FILL    32  /* how many branches in the distance flood fill */
#define BOARD_WIDTH   ((CELL_WIDTH + GAP_WIDTH) * CELLS_ACROSS + GAP_WIDTH)
#define BOARD_HEIGHT  ((CELL_HEIGHT + GAP_HEIGHT) * CELLS_DOWN + GAP_WIDTH)
#define WINDOW_WIDTH  (BOARD_WIDTH + BORDER_LEFT + BORDER_RIGHT)
#define WINDOW_HEIGHT (BOARD_HEIGHT + BORDER_TOP + BORDER_BOTTOM)
#define XTRA_SPACING  ((CELL_WIDTH * 4 + GAP_WIDTH * 3) / 5)
#define XTRA_X        (BORDER_LEFT + CELL_WIDTH * 4 + GAP_WIDTH * 5)
#define XTRA_Y        (BORDER_TOP - CELL_HEIGHT)
/*}}}*/
/*{{{  frame rate*/
/* these are in uS, if it is too fast, the load line
 * at the bottom will start to grow */
#ifndef FRAME_RATE
#define FRAME_RATE    37000
#endif
#define ZOOM_RATE (FRAME_RATE / 4)
/*}}}*/
/*{{{  font name*/
#ifndef FONT_NAME
#define FONT_NAME "-*-courier-*-r-*-*-18-*-*-*-*-*-*-*"
#endif
/*}}}*/
/*{{{  game gender*/
#ifndef GAME_GENDER
#define GAME_GENDER (random() & 1)
#endif
/*}}}*/
#define START_LIVES     3
/*{{{  velocities*/
#define VEL_X           GAP_WIDTH             /* how far we move per step */
#define VEL_Y           GAP_HEIGHT
#define VEL_X_FAST      (VEL_X * 5 / 4)       /* how far a fast stride is */
#define VEL_Y_FAST      (VEL_Y * 5 / 4)
#define FAST_STEPS      (CELL_WIDTH / VEL_X / 4) /* how many fast strides */
#define APPLE_VEL_Y     (VEL_Y * 3 / 2)       /* how fast the apple falls */
#define APPLE_VEL_X     (VEL_X / 2)           /* how fast it goes sideways */
#define APPLE_ACC       (VEL_Y / 2)
#define BALL_STEPS      2                     /* relative ball speed */
#define BALL_EX         (CELL_WIDTH / 2)      /* how fast we explode */
#define BALL_EY         (CELL_HEIGHT / 2)
#define ZOOM_X          VEL_X                 /* how fast the zoom is */
#define ZOOM_Y          VEL_Y
/*}}}*/
/*{{{  ball stuff*/
#define BALL_EXPLODE    (BOARD_WIDTH / BALL_EX) /* how much we explode */
#define BALL_IMPLODE_PROB 4     /* that the ball returns */
/*}}}*/
#define DIE_DELAY       10  /* ticks for death spiral */
#define SCORE_SHOW      45  /* how long on board scores show */
#define DISPLAY_HOLD    (SCORE_SHOW * 2)
#define HISTORY_SHOW    3   /* how often we show the history */
/*{{{  apple stuff*/
#define APPLE_ROCK_DELAY  8    /* ticks for it to rock */
#define APPLE_SPLIT_DELAY 8    /* ticks for split */
#define APPLE_DECAY_DELAY 8    /* ticks for decay */
#define APPLE_ROT_DELAY   8    /* ticks for rot */
#define APPLE_FALL_SPLIT  (CELL_HEIGHT + GAP_HEIGHT + APPLE_VEL_Y) /* safe fall distance */
/*}}}*/
/*{{{  den escape*/
#define DEN_ESCAPE_PROB   8     /* normal monster gets out of the den */
#define DEN_ESCAPE_DELAY  6     /* how many den flashes before escape */
#define DEN_ESCAPE_FLASH  0x8   /* time per den flash */
/*}}}*/
/*{{{  normal & muncher*/
#define GO_MUNCH_PROB     1     /* difficulty scale that normal goes munchy */
#define DIFFICULTY_PEDESTAL 2   /* offset for difficulty */
#define CONT_TOGGLE_PROB  2     /* continue flag toggle */
#define PUSH_TURN_PROB    64    /* muncher turns around when pushing */
#define GO_MUNCH_DELAY    16    /* pause when we start munching */
#define STOP_MUNCH_DELAY  16    /* pause when we stop munching */
/*}}}*/
/*{{{  xtra & drone*/
#define XTRA_INC_PROB     2     /* next xtra monster is selected */
#define XTRA_BIRTH_DELAY  ((CELL_WIDTH + GAP_WIDTH) / VEL_X + 4)    /* pause while giving birth to drones */
#define XTRA_CONT_OFF_PROB 16   /* that xtra stops continuing */
#define NEXT_XTRA_PROB    1     /* probability that the extra changes */
#define CHOMP_DELAY     (CELL_WIDTH / VEL_X * 6) /* how long to eat an apple */
/*}}}*/
#define MONSTER_CYCLES    6     /* cycle counter */
#define MONSTER_IMAGES    2     /* how many different images */
/*{{{  limits*/
#define INITIAL_APPLES  6
#define APPLES          (INITIAL_APPLES + 4)    /* max number of apples */
#define MONSTERS        ((CELLS_DOWN - 2) * 2)  /* maximum number of monsters */
#define BOARD_SCORES    4   /* number of on board displayed scores */
#define BACK_UPDATES    32  /* background updates we can cope with */
/*}}}*/
/*{{{  colour flags*/
#define COLOUR_ZERO     0   /* colour is all zeros */
#define COLOUR_ONE      1   /* colour is all ones */
#define COLOUR_WEIRD    2   /* colour is neither zeros or ones */
/*}}}*/
/*}}}*/
/*{{{  macros*/
#define PIXELX(CX, OX) \
    ((CX) * (CELL_WIDTH + GAP_WIDTH) + (OX) + GAP_WIDTH + BORDER_LEFT)
#define PIXELY(CY, OY) \
    ((CY) * (CELL_HEIGHT + GAP_HEIGHT) + (OY) + GAP_HEIGHT + BORDER_TOP)
#define BOARDCELL(CX, CY) \
    (&board[(CY) * CELL_STRIDE + (CX) + CELL_LEFT + CELL_TOP * CELL_STRIDE])
/*}}}*/
/*{{{  structs*/
/*{{{  typedef struct Cell*/
typedef struct Cell
/* information about 1 cell on the board */
{
  int     depths[4];      /* depths moved to from cell */
  unsigned  distance;     /* distance from player */
  unsigned  visit    : 1; /* has been visited */
  unsigned  sprite;       /* has a sprite in it (cherry, or center stuff) */
} CELL;
/*}}}*/
/*{{{  typedef struct Coord*/
typedef struct Coord
/* general coordinate store */
{
  int       x;
  int       y;
} COORD;
/*}}}*/
/*{{{  typedef struct Sprite*/
typedef struct Sprite
/* sprite definition */
{
  char *image_bits;           /* image bitmap */
  char *mask_bits;            /* mask bitmap */
  unsigned  width;            /* width of sprite */
  unsigned  height;           /* height of sprite */
  COORD     expected;         /* expected size */
  unsigned  copy;             /* generated from this sprite */
  unsigned  reflect;          /* reflection 1 = v axis, 2 = h axis */
  Pixmap    image;            /* image pixmap */
  Pixmap    mask;             /* mask pixmap */
} SPRITE;
/*}}}*/
/*{{{  typedef struct Arg*/
typedef struct Arg
/* command line argument specifier */
{
  char const *arg;      /* arg text */
  int       flag;       /* switch or value */
  void      *ptr;       /* value pointer */
  char const *help;     /* help message */
} ARG;
/*}}}*/
/*{{{  typedef struct Board*/
typedef struct Board
/* initial board information */
{
  unsigned  fill;   /* fill pattern */
  char      map[CELLS_DOWN][CELLS_ACROSS];
} BOARD;
/*}}}*/
/*{{{  typedef struct Background*/
typedef struct Background
/* area to update from back to copy to window */
{
  COORD   place;  /* top left area */
  COORD   size;   /* size of area */
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
  unsigned  distance;     /* distance we've fallen */
  unsigned  monsters;     /* monsters we've squashed */
  unsigned  chewed;       /* chewed */
  int       maypush;      /* temp push vector */
  unsigned  waspushed;    /* was pushed the previous go */
  struct Monster *list;   /* list of monsters for initial fall */
  COORD     old_pixel;    /* where i was */
  unsigned  old_state;    /* what I looked like */
} APPLE;
/*}}}*/
/*{{{  typedef struct Apple_Size*/
typedef struct Apple_Size
{
  COORD size;
  COORD offset;
} APPLE_SIZE;
/*}}}*/
/*{{{  typedef struct Text*/
typedef struct Text
/* how we say text sizes */
{
  int       ascent;
  int       descent;
  int       width;
} TEXT;
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
  unsigned  cont;     /* continue */
  unsigned  chew;     /* chewing */
  unsigned  count;    /* counter */
  unsigned  shot;     /* has been shot */
  unsigned  cycle;    /* image cycler */
  unsigned  image;    /* which image to display */
  unsigned  fast;     /* fast speed */
  unsigned  pushing;  /* pushing apple */
  struct Monster *list; /* list of monsters for initial apple fall */
  APPLE     *apple;   /* which apple we're stuck to */
  COORD     old_pixel;  /* where we were */
  int       old_sprite; /* what we looked like */
} MONSTER;
/*}}}*/
/*}}}*/
/*{{{  display*/
EXTERN struct
{
  char const *name;   /* display name */
  Display   *display; /* server name stuff */
  int       screen;   /* it's default screen */
  Colormap  colormap; /* color map for display */
  Window    root;     /* its root window */
  Window    window;   /* game window */
  int       depth;    /* bitplanes */
  unsigned  long black; /* black pixel index */
  unsigned  long white; /* white pixel index */
  unsigned  long xor;   /* black xor white */
  Atom      DEC_icon_atom; /* DEC iconizing hack */
  unsigned  long event_mask; /* default event mask */
  Pixmap    back;     /* background store */
  Pixmap    copy;     /* backing store */
  Cursor    cursor;   /* cursor to use */
  Pixmap    icon;     /* icon to use */
  unsigned  background; /* type of background colour */
  unsigned  foreground; /* type of foreground colour */
} display;
/*}}}*/
/*{{{  font*/
EXTERN struct
{
  char const *name;   /* name of font */
  Font      font;     /* font handle */
} font;
/*}}}*/
EXTERN CELL board[(CELLS_DOWN + CELL_TOP * 2) * CELL_STRIDE];
EXTERN Pixmap ball_xor;
/*{{{  gc*/
#define GCN(n)    gcs[n]
#define GC_COPY   0   /* src */
#define GC_CLEAR  1   /* background */
#define GC_SET    2   /* foreground */
#define GC_MASK   3   /* NOT src AND dst */
#define GC_OR     4   /* src OR dst */
#define GC_BACK   5   /* background cutter */
#define GC_BALL   6   /* XOR for the ball */
#define GC_TEXT   7   /* text drawer */
#define GC_AND    8   /* and */
#define GC_BOARD  9   /* board background */
#define GCS       10
EXTERN GC gcs[GCS];
/*}}}*/
/*{{{  flags*/
EXTERN struct
{
  unsigned  reverse;  /* reverse black & white */
  unsigned  bw;       /* force black & white */
  unsigned  iconic;   /* start iconic */
  unsigned  help;     /* gimme some help */
  unsigned  gender;   /* he or she? */
} flags;
/*}}}*/
/*{{{  player*/
EXTERN struct
/* player specific information
 * note, the playe place info is stored as monster 0
 */
{
  unsigned  screen;     /* current screen number */
  unsigned  score;      /* our score */
  unsigned  lives;      /* lives we have left (including on screen) */
  COORD     mouse;      /* mouse destination square */
  unsigned  throw;      /* throw the ball */
  unsigned  button;     /* throw button state */
  unsigned  mouse_dir;  /* mouse direction */
  unsigned  next_dir;   /* direction at next intersection */
  unsigned  pressed;    /* keys we have pressed */
  unsigned  bashed;     /* we bashed into a wall */
  BALL      old_ball;   /* what was the ball */
  BALL      ball;       /* ball information */
  unsigned  keyboard;   /* use keyboard */
  unsigned  cherry;     /* consecutive cherry count */
  unsigned  distance;   /* distance to next cherry */
  COORD     raw_mouse;  /* the raw mouse input */
  unsigned  old_pressed; /* old keys pressed */
  unsigned  motionevent; /* the mouse moved */
} player;
/*}}}*/
EXTERN unsigned long seed; /* random number seed */
EXTERN char const *game_name; /* name of the game */
/*{{{  global*/
EXTERN struct
{
  unsigned  difficulty;     /* increments in to make it harder */
  unsigned  broken;         /* broken through a new path */
  unsigned  cherries;       /* number of cherries left */
  unsigned  state;          /* den state
			     * 0 - den on screen
			     * 1 - cake on screen
			     * 2 - xtras & drone running around
			     * 3 - done xtras
			     * 4 - end game
			     * 5 - extra life
			     * 6 - demo
			     * 7 - high scores
			     * 8 - history
			     */
  unsigned  missed;         /* missed interrupt count */
} global;
/*}}}*/
/*{{{  extra*/
EXTERN struct
{
  unsigned  got;      /* one we've got */
  unsigned  select;   /* the one which is selected */
  unsigned  escape;   /* its out */
  unsigned  score;    /* last checked score */
} extra;
/*}}}*/
/*{{{  apple*/
EXTERN struct
{
  APPLE     list[APPLES]; /* apple list */
  unsigned  apples;       /* number of apples out */
} apple;
/*}}}*/
/*{{{  history*/
EXTERN struct
{
  unsigned  prize;
  unsigned  ending;
} history;
/*}}}*/
/*{{{  monster*/
EXTERN struct
{
  MONSTER   list[MONSTERS]; /* monsters [0] is player */
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
/*{{{  tables*/
extern ARG const args[];
/*{{{  sprite numbers*/
#define SPRITE_CENTER_BASE     0
#define SPRITE_MUNCH_BASE      2
#define SPRITE_EDGE_BASE       4
#define SPRITE_FILL_BASE       6
#define SPRITE_DIGITS         10
#define SPRITE_CHERRY         11
#define SPRITE_DEN            12
#define SPRITE_BALL           13
#define SPRITE_APPLE          14
#define SPRITE_EXTRA          20
#define SPRITE_XTRA_SOURCE    22
#define SPRITE_MONSTERS       24
#define SPRITE_NORMAL         24
#define SPRITE_MUNCHER        36
#define SPRITE_XTRA           48
#define SPRITE_DRONE          60
#define SPRITE_PLAYER         72
#define SPRITE_PLAYER_PUSH    84
#define SPRITE_PLAYER_DEAD    88
#define SPRITE_SQUISHED       90
#define SPRITE_CHOMP          100
#define SPRITE_MRIS           102
#define SPRITE_PRIZE_BASE     110
#define SPRITES               115
/*}}}*/
#define SPRITE_FILLS           4
#define SPRITE_PRIZES          5
/*{{{  random sizes*/
#define MUNCH_WIDTH     (VEL_X * 4) 
#define MUNCH_HEIGHT    (VEL_Y * 4)
#define EDGE_WIDTH      (CELL_WIDTH + GAP_WIDTH * 2)
#define EDGE_HEIGHT     (CELL_HEIGHT + GAP_HEIGHT * 2)
#define DIGIT_HEIGHT    (CELL_HEIGHT / 2)
#define DIGIT_WIDTH     (CELL_WIDTH / 4)
#define DECAY_WIDTH     (CELL_WIDTH / 2 * 3)
#define DECAY_HEIGHT    (CELL_HEIGHT / 4 * 3)
#define ROT_WIDTH       (CELL_WIDTH / 2 * 3)
#define ROT_HEIGHT      (CELL_HEIGHT / 2)
/*}}}*/
#define BALL_WIDTH    6
#define BALL_HEIGHT   6
extern SPRITE sprites[SPRITES];
extern APPLE_SIZE const apple_sizes[6];
#define BOARDS 10
extern BOARD const boards[BOARDS];
extern char keystrokes[5];
extern COORD const ball_hold[16];
extern COORD const ball_throw[8];
extern int const ball_dir[8];
extern int const player_dies[8];
extern char const *title_text[];
#define SQUISH_SCORES 7
extern int const squish_scores[SQUISH_SCORES];
/*}}}*/
/*{{{  prototypes*/
/*{{{  apple*/
extern APPLE *apple_search PROTOARGLIST((int, int, unsigned, unsigned, unsigned));
extern int apple_stop PROTOARGLIST((MONSTER *, CELL *));
extern void apple_under PROTOARGLIST((MONSTER *, CELL *));
extern void move_apples PROTOARGLIST((void));
extern APPLE *spawn_apple PROTOARGLIST((int, int, int, int));
/*}}}*/
/*{{{  create*/
extern void create_resources PROTOARGLIST((int, char **));
extern void create_xtra_monster PROTOARGLIST((int));
extern void draw_extra_letter PROTOARGLIST((int));
extern void release_resources PROTOARGLIST((void));
/*}}}*/
/*{{{  demo*/
extern int demo_mode PROTOARGLIST((void));
extern void extra_life PROTOARGLIST((void));
extern void show_history PROTOARGLIST((void));
/*}}}*/
/*{{{  draw*/
extern void add_background PROTOARGLIST((int, int, int, int));
extern void bounding_box PROTOARGLIST((int, int, unsigned, unsigned));
extern void draw_center PROTOARGLIST((int));
extern void draw_extra PROTOARGLIST((void));
extern void new_board PROTOARGLIST((void));
extern void refresh_window PROTOARGLIST((void));
extern void show_updates PROTOARGLIST((void));
extern void text_size PROTOARGLIST((char const *, unsigned, TEXT *));
extern void zoom_board PROTOARGLIST((void));
/*}}}*/
/*{{{  monster*/
extern MONSTER *extra_escape PROTOARGLIST((void));
extern void fall_monsters PROTOARGLIST((void));
extern void move_monsters PROTOARGLIST((void));
extern void new_xtra PROTOARGLIST((void));
extern MONSTER *spawn_monster PROTOARGLIST((int, int, int, int, int, int, int));
/*}}}*/
/*{{{  move*/
extern unsigned choose_direction PROTOARGLIST((unsigned));
extern CELL *drop_apple PROTOARGLIST((APPLE *, CELL *));
extern CELL *move_movable PROTOARGLIST((MONSTER *, CELL *));
extern CELL *move_muncher PROTOARGLIST((MONSTER *));
extern void munch_hole PROTOARGLIST((CELL *, int, int));
extern void new_face PROTOARGLIST((MONSTER *));
extern int valid_directions PROTOARGLIST((MONSTER *, CELL *));
/*}}}*/
/*{{{  player*/
extern void bounce_ball PROTOARGLIST((void));
extern void move_player PROTOARGLIST((void));
/*}}}*/
/*{{{  timer*/
extern void timer_close PROTOARGLIST((void));
extern void timer_open PROTOARGLIST((void));
extern void timer_start PROTOARGLIST((unsigned long));
extern void timer_stop PROTOARGLIST((void));
extern void timer_wait PROTOARGLIST((void));
/*}}}*/
/*{{{  xmris*/
extern void add_score PROTOARGLIST((int, int, int));
extern void calc_distances PROTOARGLIST((void));
extern void fatal_error PROTOARGLIST((char const *, ...));
extern int itoa PROTOARGLIST((char *, int, int));
extern int main PROTOARGLIST((int, char **));
extern int process_xevents PROTOARGLIST((int));
extern unsigned random PROTOARGLIST((void));
/*}}}*/
/*}}}*/
