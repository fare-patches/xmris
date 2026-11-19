/* Copyright (C) 1993, 1992 Nathan Sidwell */
/* RCS $Id: common.h 1.2 Tue, 16 Mar 1999 11:28:16 +0000 nathan $ */
/* common header stuff for xmris and xmred
 * included from xmris.h or xmred.h
 */
#include "patchlevel.h"
#ifndef EXTERN
#define EXTERN extern
#endif /* EXTERN */
/*{{{  include*/
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/Intrinsic.h>
/*}}}*/
#include "ansiknr.h"
/*{{{  case changing things*/
#ifdef _tolower
#define lowercase(a) _tolower(a)
#define uppercase(a) _toupper(a)
#else
#define lowercase(a) tolower(a)
#define uppercase(a) toupper(a)
#endif /* _tolower */
/*}}}*/
/*{{{  check font*/
#ifndef FONT_NAME
  #error No default font
#endif /* FONT_NAME */
/*}}}*/
/*{{{  defines*/
/*{{{  board sizes*/
#define CELLS_ACROSS  12  /* size of the board */
#define CELLS_DOWN    13
#define CELL_WIDTH    32  /* size of the internal cell */
#define CELL_HEIGHT   32  /* monster sprites must be this size */
#define GAP_WIDTH     4   /* spacing of the cells */
#define GAP_HEIGHT    4
#define DEN_WIDTH     1   /* extra space round den */
#define DEN_HEIGHT    1
#define CELL_STRIDE   16  /* board array is bigger than the board */
#define CELL_TOP      2   /* and the board is offset by this much */
#define CELL_LEFT     2
#define BORDER_TOP    CELL_HEIGHT /* placing of the board */
#ifndef XMRED
#define BORDER_BOTTOM (CELL_HEIGHT + 1)     /* on the window */
#else
#define BORDER_BOTTOM (-1)
#endif
#define BORDER_LEFT   (-1)
#define BORDER_RIGHT  (-1)
#define BOARD_WIDTH   ((CELL_WIDTH + GAP_WIDTH) * CELLS_ACROSS + GAP_WIDTH)
#define BOARD_HEIGHT  ((CELL_HEIGHT + GAP_HEIGHT) * CELLS_DOWN + GAP_HEIGHT)
#define WINDOW_WIDTH  (BOARD_WIDTH + BORDER_LEFT + BORDER_RIGHT)
#define WINDOW_HEIGHT (BOARD_HEIGHT + BORDER_TOP + BORDER_BOTTOM)
#define XTRA_SPACING  ((CELL_WIDTH * 3 + GAP_WIDTH * 3) / 4)
#define CENTER_X      (BORDER_LEFT + BOARD_WIDTH / 2)
#define CENTER_Y      (BORDER_TOP + BOARD_HEIGHT / 2)
/*}}}*/
/*{{{  some defaults*/
#define DEFAULT_PLAYER_X  ((CELLS_ACROSS - 1) >> 1)
#define DEFAULT_PLAYER_Y  (CELLS_DOWN - 1)
#define DEFAULT_DEN_X     ((CELLS_ACROSS - 1) >> 1)
#define DEFAULT_DEN_Y     (CELLS_DOWN >> 1)
#define DEFAULT_APPLES    6
/*}}}*/
/*{{{  garden characters*/
#define GARDEN_PATH     'A'
#define GARDEN_APPLE    'a'
#define GARDEN_RANDOM   '.'
#define GARDEN_CHERRY   '@'
#define GARDEN_NOAPPLE  '+'
#define GARDEN_PATH_BLANK   0x0
#define GARDEN_PATH_CHERRY  0x4
#define GARDEN_PATH_DEN     0x8
#define GARDEN_PATH_PLAYER  0xc
/*}}}*/
/*{{{  default game gender*/
#define GAME_GENDER False
/*}}}*/
/*{{{  limits*/
#define BOARD_LIMIT     64    /* maximum number of boards */
#define APPLE_LIMIT     12    /* maximum number of initial apples */
/*}}}*/
#ifndef XMRED
/*{{{  key indices*/
#define KEY_THROW     4
#define KEY_PAUSE     5
#define KEY_QUIT      6
#define KEY_ICONIZE   7
#define KEY_KEYBOARD  8
#define KEYS          9
/*}}}*/
#endif /* XMRED */
/*{{{  sprite numbers*/
#define SPRITE_CENTER_BASE     0
#define SPRITE_EDGE_BASE       2
#ifndef XMRED
#define SPRITE_DIGITS          4
#else
#define SPRITE_PATH            4
#endif /* XMRED */
#define SPRITE_DEN             5
#ifndef XMRED
#define SPRITE_BALL            6
#define SPRITE_CHERRY          7
#define SPRITE_APPLE           8
#define SPRITE_GHOST          14
#define SPRITE_DIAMOND        17
#define SPRITE_EXTRA          20
#define SPRITE_XTRA_SOURCE    22
#define SPRITE_MONSTERS       24
#define SPRITE_NORMAL         24
#define SPRITE_MUNCHER        36
#define SPRITE_XTRA           48
#define SPRITE_DRONE          60
#define SPRITE_PLAYER         72
#define SPRITE_PLAYER_PUSH    84
#define SPRITE_PLAYER_REST    88
#define SPRITE_PLAYER_HAPPY   92
#define SPRITE_PLAYER_DEAD    94
#define SPRITE_SQUISHED       110
#define SPRITE_CHOMP          120
#define SPRITE_MRIS           122
#define SPRITE_SEAT           130
#define SPRITE_GHOSTING       131
#define SPRITE_PRIZE_BASE     132
#define SPRITES               137
#define SPRITE_PRIZES          5
#define SPRITE_MUSHROOM        3
#else
#define SPRITE_CHERRY          6
#define SPRITE_BIG_APPLE       7
#define SPRITE_BIG_ARROW       9
#define SPRITE_SMALL_APPLE     11
#define SPRITE_SMALL_ARROW     13
#define SPRITE_EXTRA           15
#define SPRITE_PLAYER          16
#define SPRITE_COLORS          17
#define SPRITE_APPLES          20
#define SPRITE_APPLE_DROP      25
#define SPRITE_INCLUDE         26
#define SPRITE_ICON_I          27
#define SPRITE_BOARD_I         28
#define SPRITE_WARNING         29
#define SPRITES                31
#endif /* XMRED */
/*}}}*/
/*{{{  color numbers*/
#define COLOR_WHITE          0
#define COLOR_BLACK          1
#define COLOR_BACKGROUNDS    2
#define COLOR_GREEN_BACK     2
#define COLOR_GREEN_FORE     3
#define COLOR_RED_BACK       4
#define COLOR_RED_FORE       5
#define COLOR_BLUE_BACK      6
#define COLOR_BLUE_FORE      7
#define COLOR_BACKGROUND     8
#define COLOR_FOREGROUND     9
#define COLOR_BORDER        10
#define COLOR_CHERRY        11
#define COLOR_CHERRY_STALK  12
#define COLOR_APPLE_1       13
#define COLOR_APPLE_2       14
#define COLOR_APPLE_FAINT   15
#define COLOR_PLAYER        16
#define COLOR_PLAYER_BOBBLE 17
#define COLOR_PLAYER_FACE   18
#define COLOR_PLAYER_BOOT   19
#define COLOR_NORMAL        20
#define COLOR_LETTER_GOT    21
#ifndef XMRED
#define COLOR_LETTER_NOT_GOT 22
#define COLOR_BALL          23
#define COLOR_DRONE_BACK    24
#define COLOR_DRONE_FORE    25
#define COLOR_GEM_1         26
#define COLOR_GEM_2         27
#define COLOR_MUNCH_1       28
#define COLOR_MUNCH_2       29
#define COLOR_DRONE         30
#define COLOR_DRONE_TONGUE  31
#define COLOR_XTRA          32
#define COLOR_CHOMP         33
#define COLOR_CHOMP_LIP     34
#define COLOR_CHOMP_TONGUE  35
#define COLOR_SEAT          36
#define COLOR_CAKE          37
#define COLOR_CAKE_ICING    38
#define COLOR_CAKE_JAM      39
#define COLOR_SPANNER       40
#define COLOR_SPANNER_SHADOW 41
#define COLOR_BROLLY_1      42
#define COLOR_BROLLY_2      43
#define COLOR_BROLLY_HANDLE 44
#define COLOR_MUSHROOM_STALK 45
#define COLOR_MUSHROOM_CAP  46
#define COLOR_CLOCK_FACE    47
#define COLOR_CLOCK_BELL    48
#define COLOR_CLOCK_EDGE    49
#define COLOR_DYNAMIC       50
#define COLORS              52
#else
#define COLORS              22
#endif /* XMRED */
/*}}}*/
/*{{{  random sizes*/
#define EDGE_WIDTH      (CELL_WIDTH + GAP_WIDTH * 2)
#define EDGE_HEIGHT     (CELL_HEIGHT + GAP_HEIGHT * 2)
#define INTERNAL_WIDTH  4     /* size of internal corner */
#define INTERNAL_HEIGHT 4
#define EXTERNAL_WIDTH  4     /* size of external corner */
#define EXTERNAL_HEIGHT 4
#define XTRA_LETTER_X   8     /* position of lettering on xtra */
#define XTRA_LETTER_Y   10
#ifndef XMRED
#define DIGIT_HEIGHT    (CELL_HEIGHT / 2)
#define DIGIT_WIDTH     (CELL_WIDTH / 4)
#define BALL_WIDTH      7     /* size of ball */
#define BALL_HEIGHT     7
#define GHOSTING_WIDTH  40
#define GHOSTING_HEIGHT 40
#else
#define WARNING_WIDTH   12
#define WARNING_HEIGHT  24
#endif /* XMRED */
/*}}}*/
/*{{{  colour flags*/
#define COLOUR_ZERO     0   /* colour is all zeros */
#define COLOUR_ONE      1   /* colour is all ones */
#define COLOUR_WEIRD    2   /* colour is neither zeros or ones */
/*}}}*/
#define BACKGROUNDS            3
#define FILLS                  4
#ifndef XMRED
#define BACKGROUND_WHIZZ       -1
#define BACKGROUND_NORMAL      -2
#define BACKGROUND_DRONES      BACKGROUNDS
#endif /* XMRED */
/*}}}*/
/*{{{  macros*/
#define PIXELX(CX, OX) \
    ((CX) * (CELL_WIDTH + GAP_WIDTH) + (OX) + (GAP_WIDTH + BORDER_LEFT))
#define PIXELY(CY, OY) \
    ((CY) * (CELL_HEIGHT + GAP_HEIGHT) + (OY) + (GAP_HEIGHT + BORDER_TOP))
/*
 * The range macro does one test, rather than (straight foward) two,
 * provided l and u are compile time constants, we win. I suppose
 * a decent optimizer may even know about this trick, but gcc doesn't,
 * and that's usually a good indicator of what happens.
 */
#define INRANGE(x, l, u) \
    ((unsigned)(x) - (unsigned)(l) < (unsigned)(u) - (unsigned)(l))
#define ISPATH(c) INRANGE(c, GARDEN_PATH, GARDEN_PATH + 16)
#define ISPATHBLANK(c) INRANGE(c, GARDEN_PATH + GARDEN_PATH_BLANK, \
    GARDEN_PATH + GARDEN_PATH_BLANK + 4)
#define ISPATHCHERRY(c) INRANGE(c, GARDEN_PATH + GARDEN_PATH_CHERRY, \
    GARDEN_PATH + GARDEN_PATH_CHERRY + 4)
#define ISPATHDEN(c) INRANGE(c, GARDEN_PATH + GARDEN_PATH_DEN, \
    GARDEN_PATH + GARDEN_PATH_DEN + 4)
#define ISPATHPLAYER(c) INRANGE(c, GARDEN_PATH + GARDEN_PATH_PLAYER, \
    GARDEN_PATH + GARDEN_PATH_PLAYER + 4)
#define ISAPPLE(c) INRANGE(c, GARDEN_APPLE, GARDEN_APPLE + 16)
#define GARDENPATH(c) ((c) - GARDEN_PATH)
#define GARDENAPPLE(c) ((c) - GARDEN_APPLE)
/*}}}*/
/*{{{  structs*/
/*{{{  typedef struct Data*/
typedef struct Data
{
  Font    font;     /* font to use */
#ifndef XMRED
  String  expire;   /* expiration */
  String  remove;   /* removal */
  String  boards;   /* board definitions */
  String  format;   /* new date format */
  KeySym  keysyms[KEYS];
  Boolean random;   /* random apples */
  Boolean scores;   /* high scores only */
  Boolean sprites;  /* show sprites */
  Boolean username; /* use username of user */
  Boolean nodynamic; /* don't take advantage of dynamic visuals */
  Boolean nodisplay; /* don't open the display, even to get X resources */
  int pixel_scale;
  Boolean sound;
  Boolean busywait; /* do busy waiting */
#endif /* XMRED */
  Boolean gender;   /* he or she? */
  int     distinct; /* distinct colors allocated */
  Boolean help;     /* gimme some help */
  Boolean swap;     /* swap colours? */
  Boolean mono;     /* force black & white */
  Boolean colors;   /* color allocation */
  Boolean private;  /* own colormap */
} DATA;
/*}}}*/
/*{{{  typedef struct _CommandOptionDesc*/
typedef struct _CommandOptionDesc
{
  char CONST *option;   /* option text */
  unsigned  optional;   /* optional arg? */
  Cardinal  offset;     /* offset */
} CommandOptionDesc;
/*}}}*/
/*{{{  typedef struct Coord*/
typedef struct Coord
/* general coordinate store */
{
  int       x;
  int       y;
} COORD;
/*}}}*/
/*{{{  typedef struct Size*/
typedef struct Size
/* general size store */
{
  unsigned  x;
  unsigned  y;
} SIZE;
/*}}}*/
/*{{{  typedef struct Sprite*/
typedef struct Sprite
/* sprite definition */
{
  Pixmap    image;            /* image pixmap */
  Pixmap    mask;             /* mask pixmap */
  SIZE      size;             /* size of sprite */
} SPRITE;
/*}}}*/
/*{{{  typedef struct Board*/
typedef struct Board
/* initial board information */
{
  unsigned  fill;   /* fill pattern */
  unsigned  colors; /* background colours: 0-red, 1-green, 2-blue */
  unsigned  apples; /* initial number of apples */
  char      map[CELLS_DOWN][CELLS_ACROSS + 1]; /* board lines with nul */
} BOARD;
/*}}}*/
/*}}}*/
EXTERN char CONST *myname;      /* name of me */
/*{{{  display*/
EXTERN struct
{
  XtAppContext context; /* my context */
  Widget    toplevel;   /* toplevel widget */
#ifndef XMRED
  Widget    form;       /* form widget */
  Widget    garden;     /* garden widget */
  Window    window;     /* game window */
#endif /* XMRED */
  Display   *display;   /* server name stuff */
  Colormap  colormap;   /* colormap */
  int       screen;     /* screen */
  Visual    *visual;    /* visual */
  int       depth;      /* screen depth */
  Pixel     black;      /* black pixel index */
  Pixel     white;      /* white pixel index */
  Pixel     border;     /* border pixel index */
  Pixel     xor;        /* black xor white */
#ifndef XMRED
  unsigned  repeat;     /* default keyrepeat state */
  unsigned  mapped;     /* is mapped */
  Pixmap    back;       /* background store */
#endif /* XMRED */
  Pixmap    copy;       /* backing store */
  Pixmap    icon;       /* icon */
  unsigned  background; /* type of background colour */
  unsigned  foreground; /* type of foreground colour */
  unsigned  dynamic;    /* dynamic colors */
} display;
/*}}}*/
/*{{{  font*/
EXTERN struct
{
  unsigned  width;    /* width of a character */
  int       ascent;   /* max ascent */
  int       descent;  /* max descent */
  int       center;   /* center offset to add */
  int       digitup;  /* digit ascent */
  int       digitdown;/* digit descent */
} font;
/*}}}*/
/*{{{  gc*/
#define GCN(n)    gcs[n]
#define GC_COPY   0   /* src */
#define GC_CLEAR  1   /* background */
#define GC_SET    2   /* foreground */
#define GC_MASK   3   /* NOT src AND dst */
#define GC_OR     4   /* src OR dst */
#define GC_TEXT   5   /* text drawer */
#define GC_AND    6   /* and */
#define GC_BOARD  7   /* board background */
#define GC_BORDER 8   /* border lines */
#ifndef XMRED
#define GC_LOAD   9   /* game load line */
#define GCS       10
#else
#define GCS       9
#endif /* XMRED */
EXTERN GC gcs[GCS];
/*}}}*/
EXTERN DATA data;
/*{{{  tables*/
EXTERN SPRITE sprites[SPRITES];
EXTERN SPRITE fills[FILLS];
extern unsigned CONST backgrounds[][2];
EXTERN XColor colors[COLORS];
extern unsigned char CONST **lettering;
/*}}}*/
