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
#define EXTERN
#include "xmris.h"
/*{{{  bitmaps*/
/*{{{  bitmaps/icon/.*/
#include "bitmaps/icon/mris.h"
#include "bitmaps/icon/msit.h"
#include "bitmaps/icon/cursor.h"
#include "bitmaps/icon/cursorm.h"
#include "bitmaps/icon/solidm.h"
#include "bitmaps/icon/solidr.h"
#include "bitmaps/icon/solidi.h"
#include "bitmaps/icon/solids.h"
#include "bitmaps/icon/solidt.h"
#include "bitmaps/icon/shellm.h"
#include "bitmaps/icon/shellr.h"
#include "bitmaps/icon/shelli.h"
#include "bitmaps/icon/shells.h"
#include "bitmaps/icon/shellt.h"
#include "bitmaps/icon/bigm.h"
#include "bitmaps/icon/bigr.h"
#include "bitmaps/icon/bigi.h"
#include "bitmaps/icon/bigs.h"
#include "bitmaps/icon/bigt.h"
/*}}}*/
/*{{{  bitmaps/board/.*/
#include "bitmaps/board/oblong.h"
#include "bitmaps/board/rect.h"
#include "bitmaps/board/munchtb.h"
#include "bitmaps/board/munchlr.h"
#include "bitmaps/board/edgetb.h"
#include "bitmaps/board/edgelr.h"
#include "bitmaps/board/fill0.h"
#include "bitmaps/board/fill1.h"
#include "bitmaps/board/fill2.h"
#include "bitmaps/board/fill3.h"
#include "bitmaps/board/digits.h"
#include "bitmaps/board/digitsm.h"
#include "bitmaps/board/cherry.h"
#include "bitmaps/board/cherrym.h"
/*}}}*/
/*{{{  bitmaps/apple/.*/
#include "bitmaps/apple/apple.h"
#include "bitmaps/apple/applem.h"
#include "bitmaps/apple/rock.h"
#include "bitmaps/apple/rockm.h"
#include "bitmaps/apple/split.h"
#include "bitmaps/apple/splitm.h"
#include "bitmaps/apple/decay.h"
#include "bitmaps/apple/decaym.h"
#include "bitmaps/apple/rot.h"
#include "bitmaps/apple/rotm.h"
/*}}}*/
/*{{{  bitmaps/player/.*/
#include "bitmaps/player/ball.h"
#include "bitmaps/player/plyrrt1.h"
#include "bitmaps/player/plyrrt1m.h"
#include "bitmaps/player/plyrrt2.h"
#include "bitmaps/player/plyrrt2m.h"
#include "bitmaps/player/plyrup1.h"
#include "bitmaps/player/plyrup1m.h"
#include "bitmaps/player/plyrup2.h"
#include "bitmaps/player/plyrup2m.h"
#include "bitmaps/player/plyrps1.h"
#include "bitmaps/player/plyrps1m.h"
#include "bitmaps/player/plyrps2.h"
#include "bitmaps/player/plyrps2m.h"
#include "bitmaps/player/plyrsq.h"
#include "bitmaps/player/plyrsqm.h"
/*}}}*/
/*{{{  bitmaps/normal/.*/
#include "bitmaps/normal/normrt1.h"
#include "bitmaps/normal/normrt1m.h"
#include "bitmaps/normal/normrt2.h"
#include "bitmaps/normal/normrt2m.h"
#include "bitmaps/normal/normup1.h"
#include "bitmaps/normal/normup1m.h"
#include "bitmaps/normal/normup2.h"
#include "bitmaps/normal/normup2m.h"
#include "bitmaps/normal/normsq.h"
#include "bitmaps/normal/normsqm.h"
#include "bitmaps/normal/den.h"
#include "bitmaps/normal/denm.h"
/*}}}*/
/*{{{  bitmaps/munch/.*/
#include "bitmaps/munch/mnchrt1.h"
#include "bitmaps/munch/mnchrt1m.h"
#include "bitmaps/munch/mnchrt2.h"
#include "bitmaps/munch/mnchrt2m.h"
#include "bitmaps/munch/mnchup1.h"
#include "bitmaps/munch/mnchup1m.h"
#include "bitmaps/munch/mnchup2.h"
#include "bitmaps/munch/mnchup2m.h"
#include "bitmaps/munch/mnchsq.h"
#include "bitmaps/munch/mnchsqm.h"
/*}}}*/
/*{{{  bitmaps/xtra/.*/
#include "bitmaps/xtra/xtra1.h"
#include "bitmaps/xtra/xtra1m.h"
#include "bitmaps/xtra/xtra2.h"
#include "bitmaps/xtra/xtra2m.h"
#include "bitmaps/xtra/xtrasq.h"
#include "bitmaps/xtra/xtrasqm.h"
#include "bitmaps/xtra/xtrbold.h"
#include "bitmaps/xtra/xtrfaint.h"
/*}}}*/
/*{{{  bitmaps/drone/.*/
#include "bitmaps/drone/dronrt1.h"
#include "bitmaps/drone/dronrt1m.h"
#include "bitmaps/drone/dronrt2.h"
#include "bitmaps/drone/dronrt2m.h"
#include "bitmaps/drone/dronup1.h"
#include "bitmaps/drone/dronup1m.h"
#include "bitmaps/drone/dronup2.h"
#include "bitmaps/drone/dronup2m.h"
#include "bitmaps/drone/dronsq.h"
#include "bitmaps/drone/dronsqm.h"
/*}}}*/
/*{{{  bitmaps/chomp/.*/
#include "bitmaps/chomp/chmpopn.h"
#include "bitmaps/chomp/chmpopnm.h"
#include "bitmaps/chomp/chmpcls.h"
#include "bitmaps/chomp/chmpclsm.h"
/*}}}*/
/*{{{  bitmaps/prize/.*/
#include "bitmaps/prize/cake.h"
#include "bitmaps/prize/cakem.h"
#include "bitmaps/prize/spanner.h"
#include "bitmaps/prize/spannerm.h"
#include "bitmaps/prize/brolly.h"
#include "bitmaps/prize/brollym.h"
#include "bitmaps/prize/mushrm.h"
#include "bitmaps/prize/mushrmm.h"
#include "bitmaps/prize/clock.h"
#include "bitmaps/prize/clockm.h"
/*}}}*/
/*}}}*/
#define XTRA_LETTER_X   ((xtra1_x_hot + xtra2_x_hot) >> 1)
#define XTRA_LETTER_Y   ((xtra1_y_hot + xtra2_y_hot) >> 1)
static unsigned long const colour_zero = 0;
static unsigned long const colour_one = ~0;
/*{{{  typedef struct Context*/
typedef struct Context
/* GC definer */
{
  int       function;         /* graphics function */
  unsigned long const *fgp;   /* foreground colour pointer */
  unsigned long const *bgp;   /* background colour pointer */
} CONTEXT;
/*}}}*/
/*{{{  gcsdefine*/
static CONTEXT gcsdefine[GCS] =
{
    {GXcopy,        &colour_one,    &colour_zero},
    {GXcopy,        &display.white, &colour_zero},
    {GXcopy,        &display.black, &colour_zero},
    {GXandInverted, &colour_one,    &colour_zero},
    {GXor,          &colour_one,    &colour_zero},
    {GXor,          &display.white, &colour_zero},
    {GXxor,         &display.xor,   &colour_zero},
    {GXcopy,        &display.black, &display.white},
    {GXand,         &colour_one,    &colour_zero},
    {GXcopy,        &display.black, &display.white},
};
/*}}}*/
/*{{{  tables*/
/*{{{  ARG const args[] =*/
ARG const args[] =
{
  {"help",    1, &flags.help,     "This help"},
  {"display",-1, &display.name,   "Display to run on"},
  {"font",   -1, &font.name,      "Font name"},
  {"rv",      1, &flags.reverse,  "Reverse video"},
  {"bw",      1, &flags.bw,       "Force black and white"},
  {"iconic",  1, &flags.iconic,   "Start iconic"},
  {"mris",    0, &flags.gender,   "Male version"},
  {"msit",    1, &flags.gender,   "Female version"},
  {NULL}
};
/*}}}*/
/*{{{  SPRITE sprites[] =*/
SPRITE sprites[] =
{
  /*{{{  2:center masks (oblong, rect)*/
  {NULL, oblong_bits, oblong_width, oblong_height, {CELL_WIDTH, CELL_HEIGHT}},
  {NULL, rect_bits, rect_width, rect_height, {CELL_WIDTH, CELL_HEIGHT}},
  /*}}}*/
  /*{{{  2:munch masks (tb,lr)*/
  {NULL, munchtb_bits, munchtb_width, munchtb_height, {CELL_WIDTH, MUNCH_HEIGHT}},
  {NULL, munchlr_bits, munchlr_width, munchlr_height, {MUNCH_WIDTH, CELL_HEIGHT}},
  /*}}}*/
  /*{{{  2:edge masks (tb, lr)*/
  {NULL, edgetb_bits, edgetb_width, edgetb_height, {GAP_WIDTH * 5, EDGE_HEIGHT}},
  {NULL, edgelr_bits, edgelr_width, edgelr_height, {EDGE_WIDTH, GAP_HEIGHT * 5}},
  /*}}}*/
  /*{{{  4:fills (0,1,2,3)*/
  {fill0_bits, NULL, fill0_width, fill0_height, {0, 0}},
  {fill1_bits, NULL, fill1_width, fill1_height, {0, 0}},
  {fill2_bits, NULL, fill2_width, fill2_height, {0, 0}},
  {fill3_bits, NULL, fill3_width, fill3_height, {0, 0}},
  /*}}}*/
  /*{{{  1:digits (0,1,2,3,4,5,6,7,8,9,' ')*/
  {digits_bits, digitsm_bits, digits_width, digits_height, {DIGIT_WIDTH * 11, DIGIT_HEIGHT}},
  /*}}}*/
  /*{{{  1:cherry*/
  {cherry_bits, cherrym_bits, cherry_width, cherry_height, {CELL_WIDTH, CELL_HEIGHT}},
  /*}}}*/
  /*{{{  1:den*/
  {den_bits, denm_bits, den_width, den_height, {CELL_WIDTH, CELL_HEIGHT}},
  /*}}}*/
  /*{{{  1:ball*/
  {ball_bits, ball_bits, ball_width, ball_height, {BALL_WIDTH, BALL_HEIGHT}},
  /*}}}*/
  /*{{{  6:apples (apple, rock, apple, split, decay, rot)*/
  {apple_bits, applem_bits, apple_width, apple_height, {CELL_WIDTH, CELL_HEIGHT}},
  {rock_bits,  rockm_bits,  rock_width,  rock_height,  {CELL_WIDTH, CELL_HEIGHT}},
  {NULL, NULL, 0, 0, {0, 0}, SPRITE_APPLE, 0},
  {split_bits, splitm_bits, split_width, split_height, {CELL_WIDTH, CELL_HEIGHT}},
  {decay_bits, decaym_bits, decay_width, decay_height, {DECAY_WIDTH, DECAY_HEIGHT}},
  {rot_bits,   rotm_bits,   rot_width,   rot_height, {ROT_WIDTH, ROT_HEIGHT}},
  /*}}}*/
  /*{{{  2:extra (bold, faint)*/
  {xtrbold_bits,  xtrbold_bits, xtrbold_width,  xtrbold_height, {CELL_WIDTH / 2 * 5, CELL_HEIGHT / 2}},
  {xtrfaint_bits, xtrbold_bits, xtrfaint_width, xtrfaint_height, {CELL_WIDTH / 2 * 5, CELL_HEIGHT / 2}},
  /*}}}*/
  /*{{{  2:xtra source*/
  {xtra1_bits, xtra1m_bits, xtra1_width, xtra1_height, {CELL_WIDTH, CELL_HEIGHT}},
  {xtra2_bits, xtra2m_bits, xtra2_width, xtra2_height, {CELL_WIDTH, CELL_HEIGHT}},
  /*}}}*/
  /*{{{  12:normal (ul, dr, l, r, ur, dl)*/
  {NULL, NULL, 0, 0, {0, 0}, SPRITE_NORMAL + 8, 1},
  {NULL, NULL, 0, 0, {0, 0}, SPRITE_NORMAL + 9, 1},
  {NULL, NULL, 0, 0, {0, 0}, SPRITE_NORMAL + 8, 2},
  {NULL, NULL, 0, 0, {0, 0}, SPRITE_NORMAL + 9, 2},
  {NULL, NULL, 0, 0, {0, 0}, SPRITE_NORMAL + 6, 1},
  {NULL, NULL, 0, 0, {0, 0}, SPRITE_NORMAL + 7, 1},
  {normrt1_bits, normrt1m_bits, normrt1_width, normrt1_height, {CELL_WIDTH, CELL_HEIGHT}},
  {normrt2_bits, normrt2m_bits, normrt2_width, normrt2_height, {CELL_WIDTH, CELL_HEIGHT}},
  {normup1_bits, normup1m_bits, normup1_width, normup1_height, {CELL_WIDTH, CELL_HEIGHT}},
  {normup2_bits, normup2m_bits, normup2_width, normup1_height, {CELL_WIDTH, CELL_HEIGHT}},
  {NULL, NULL, 0, 0, {0, 0}, SPRITE_NORMAL + 0, 2},
  {NULL, NULL, 0, 0, {0, 0}, SPRITE_NORMAL + 1, 2},
  /*}}}*/
  /*{{{  12:muncher (ul, dr, l, r, ur, dl)*/
  {NULL, NULL, 0, 0, {0, 0}, SPRITE_MUNCHER + 8, 1},
  {NULL, NULL, 0, 0, {0, 0}, SPRITE_MUNCHER + 9, 1},
  {NULL, NULL, 0, 0, {0, 0}, SPRITE_MUNCHER + 8, 2},
  {NULL, NULL, 0, 0, {0, 0}, SPRITE_MUNCHER + 9, 2},
  {NULL, NULL, 0, 0, {0, 0}, SPRITE_MUNCHER + 6, 1},
  {NULL, NULL, 0, 0, {0, 0}, SPRITE_MUNCHER + 7, 1},
  {mnchrt1_bits, mnchrt1m_bits, mnchrt1_width, mnchrt1_height, {CELL_WIDTH, CELL_HEIGHT}},
  {mnchrt2_bits, mnchrt2m_bits, mnchrt2_width, mnchrt2_height, {CELL_WIDTH, CELL_HEIGHT}},
  {mnchup1_bits, mnchup1m_bits, mnchup1_width, mnchup1_height, {CELL_WIDTH, CELL_HEIGHT}},
  {mnchup2_bits, mnchup2m_bits, mnchup2_width, mnchup2_height, {CELL_WIDTH, CELL_HEIGHT}},
  {NULL, NULL, 0, 0, {0, 0}, SPRITE_MUNCHER + 0, 2},
  {NULL, NULL, 0, 0, {0, 0}, SPRITE_MUNCHER + 1, 2},
  /*}}}*/
  /*{{{  12:xtra (ul, dr, l, r, ur, dl)*/
  {xtra1_bits, xtra1m_bits, xtra1_width, xtra1_height, {CELL_WIDTH, CELL_HEIGHT}},
  {xtra2_bits, xtra2m_bits, xtra2_width, xtra2_height, {CELL_WIDTH, CELL_HEIGHT}},
  {NULL, NULL, 0, 0, {0, 0}, SPRITE_XTRA + 0, 0},
  {NULL, NULL, 0, 0, {0, 0}, SPRITE_XTRA + 1, 0},
  {NULL, NULL, 0, 0, {0, 0}, SPRITE_XTRA + 0, 0},
  {NULL, NULL, 0, 0, {0, 0}, SPRITE_XTRA + 1, 0},
  {NULL, NULL, 0, 0, {0, 0}, SPRITE_XTRA + 0, 0},
  {NULL, NULL, 0, 0, {0, 0}, SPRITE_XTRA + 1, 0},
  {NULL, NULL, 0, 0, {0, 0}, SPRITE_XTRA + 0, 0},
  {NULL, NULL, 0, 0, {0, 0}, SPRITE_XTRA + 1, 0},
  {NULL, NULL, 0, 0, {0, 0}, SPRITE_XTRA + 0, 0},
  {NULL, NULL, 0, 0, {0, 0}, SPRITE_XTRA + 1, 0},
  /*}}}*/
  /*{{{  12:drone (ul, dr, l, r, ur, dl)*/
  {NULL, NULL, 0, 0, {0, 0}, SPRITE_DRONE + 8, 1},
  {NULL, NULL, 0, 0, {0, 0}, SPRITE_DRONE + 9, 1},
  {NULL, NULL, 0, 0, {0, 0}, SPRITE_DRONE + 8, 2},
  {NULL, NULL, 0, 0, {0, 0}, SPRITE_DRONE + 9, 2},
  {NULL, NULL, 0, 0, {0, 0}, SPRITE_DRONE + 6, 1},
  {NULL, NULL, 0, 0, {0, 0}, SPRITE_DRONE + 7, 1},
  {dronrt1_bits, dronrt1m_bits, dronrt1_width, dronrt1_height, {CELL_WIDTH, CELL_HEIGHT}},
  {dronrt2_bits, dronrt2m_bits, dronrt2_width, dronrt2_height, {CELL_WIDTH, CELL_HEIGHT}},
  {dronup1_bits, dronup1m_bits, dronup1_width, dronup1_height, {CELL_WIDTH, CELL_HEIGHT}},
  {dronup2_bits, dronup2m_bits, dronup2_width, dronup2_height, {CELL_WIDTH, CELL_HEIGHT}},
  {NULL, NULL, 0, 0, {0, 0}, SPRITE_DRONE + 0, 2},
  {NULL, NULL, 0, 0, {0, 0}, SPRITE_DRONE + 1, 2},
  /*}}}*/
  /*{{{  12:player (ul, dr, l, r, ur, dl)*/
  {NULL, NULL, 0, 0, {0, 0}, SPRITE_PLAYER + 8, 1},
  {NULL, NULL, 0, 0, {0, 0}, SPRITE_PLAYER + 9, 1},
  {NULL, NULL, 0, 0, {0, 0}, SPRITE_PLAYER + 8, 2},
  {NULL, NULL, 0, 0, {0, 0}, SPRITE_PLAYER + 9, 2},
  {NULL, NULL, 0, 0, {0, 0}, SPRITE_PLAYER + 6, 1},
  {NULL, NULL, 0, 0, {0, 0}, SPRITE_PLAYER + 7, 1},
  {plyrrt1_bits, plyrrt1m_bits, plyrrt1_width, plyrrt1_height, {CELL_WIDTH, CELL_HEIGHT}},
  {plyrrt2_bits, plyrrt2m_bits, plyrrt2_width, plyrrt2_height, {CELL_WIDTH, CELL_HEIGHT}},
  {plyrup1_bits, plyrup1m_bits, plyrup1_width, plyrup1_height, {CELL_WIDTH, CELL_HEIGHT}},
  {plyrup2_bits, plyrup2m_bits, plyrup2_width, plyrup2_height, {CELL_WIDTH, CELL_HEIGHT}},
  {NULL, NULL, 0, 0, {0, 0}, SPRITE_PLAYER + 0, 2},
  {NULL, NULL, 0, 0, {0, 0}, SPRITE_PLAYER + 1, 2},
  /*}}}*/
  /*{{{  4:player push (l, r)*/
  {NULL, NULL, 0, 0, {0, 0}, SPRITE_PLAYER_PUSH + 2, 1},
  {NULL, NULL, 0, 0, {0, 0}, SPRITE_PLAYER_PUSH + 3, 1},
  {plyrps1_bits, plyrps1m_bits, plyrps1_width, plyrps1_height, {CELL_WIDTH, CELL_HEIGHT}},
  {plyrps2_bits, plyrps2m_bits, plyrps2_width, plyrps2_height, {CELL_WIDTH, CELL_HEIGHT}},
  /*}}}*/
  /*{{{  2:player dead (l, r)*/
  {NULL, NULL, 0, 0, {0, 0}, SPRITE_PLAYER + 4, 2},
  {NULL, NULL, 0, 0, {0, 0}, SPRITE_PLAYER + 6, 2},
  /*}}}*/
  /*{{{  10:squished*/
  {NULL, NULL, 0, 0, {0, 0}, SPRITE_SQUISHED + 1, 1},
  {normsq_bits, normsqm_bits, normsq_width, normsq_height, {CELL_WIDTH, CELL_HEIGHT / 4}},
  {NULL, NULL, 0, 0, {0, 0}, SPRITE_SQUISHED + 3, 1},
  {mnchsq_bits, mnchsqm_bits, mnchsq_width, mnchsq_height, {CELL_WIDTH, CELL_HEIGHT / 4}},
  {NULL, NULL, 0, 0, {0, 0}, SPRITE_SQUISHED + 5, 0},
  {xtrasq_bits, xtrasqm_bits, xtrasq_width, xtrasq_height, {CELL_WIDTH, CELL_HEIGHT / 4}},
  {NULL, NULL, 0, 0, {0, 0}, SPRITE_SQUISHED + 7, 1},
  {dronsq_bits, dronsqm_bits, dronsq_width, dronsq_height, {CELL_WIDTH, CELL_HEIGHT / 4}},
  {NULL, NULL, 0, 0, {0, 0}, SPRITE_SQUISHED + 9, 1},
  {plyrsq_bits, plyrsqm_bits, plyrsq_width, plyrsq_height, {CELL_WIDTH, CELL_HEIGHT / 4}},
  /*}}}*/
  /*{{{  2:chomp (open, closed)*/
  {chmpopn_bits, chmpopnm_bits, chmpopn_width, chmpopn_height, {CELL_WIDTH, CELL_HEIGHT}},
  {chmpcls_bits, chmpclsm_bits, chmpcls_width, chmpcls_height, {CELL_WIDTH, CELL_HEIGHT}},
  /*}}}*/
  /*{{{  8:m r i s*/
  {solidm_bits, bigm_bits, solidm_width, solidm_height, {CELL_WIDTH, CELL_HEIGHT}},
  {solidr_bits, bigr_bits, solidr_width, solidr_height, {CELL_WIDTH, CELL_HEIGHT}},
  {solidi_bits, bigi_bits, solidi_width, solidi_height, {CELL_WIDTH, CELL_HEIGHT}},
  {solids_bits, bigs_bits, solids_width, solids_height, {CELL_WIDTH, CELL_HEIGHT}},
  {shellm_bits, bigm_bits, shellm_width, shellm_height, {CELL_WIDTH, CELL_HEIGHT}},
  {shellr_bits, bigr_bits, shellr_width, shellr_height, {CELL_WIDTH, CELL_HEIGHT}},
  {shelli_bits, bigi_bits, shelli_width, shelli_height, {CELL_WIDTH, CELL_HEIGHT}},
  {shells_bits, bigs_bits, shells_width, shells_height, {CELL_WIDTH, CELL_HEIGHT}},
  /*}}}*/
  /*{{{  5:prizes*/
  {cake_bits,    cakem_bits,    cake_width,    cake_height,    {CELL_WIDTH, CELL_HEIGHT}},
  {spanner_bits, spannerm_bits, spanner_width, spanner_height, {CELL_WIDTH, CELL_HEIGHT}},
  {brolly_bits,  brollym_bits,  brolly_width,  brolly_height,  {CELL_WIDTH, CELL_HEIGHT}},
  {mushrm_bits,  mushrmm_bits,  mushrm_width,  mushrm_height,  {CELL_WIDTH, CELL_HEIGHT}},
  {clock_bits,   clockm_bits,   clock_width,   clock_height,   {CELL_WIDTH, CELL_HEIGHT}},
  /*}}}*/
};
/*}}}*/
/*{{{  static SPRITE spritet[2] =*/
static SPRITE spritet[2] =
{
  {solidt_bits, bigt_bits, solidt_width, solidt_height, {CELL_WIDTH, CELL_HEIGHT}},
  {shellt_bits, bigt_bits, shellt_width, shellt_height, {CELL_WIDTH, CELL_HEIGHT}},
};
/*}}}*/
/*{{{  static SPRITE icons[2] =*/
static SPRITE icons[2] =
{
  {NULL, mris_bits, mris_width, mris_height},
  {NULL, msit_bits, msit_width, msit_height},
};
/*}}}*/
/*{{{  APPLE_SIZE const apple_sizes[6] =*/
APPLE_SIZE const apple_sizes[6] =
  {
    {{CELL_WIDTH, CELL_HEIGHT}, {0, 0}},
    {{CELL_WIDTH, CELL_HEIGHT}, {0, 0}},
    {{CELL_WIDTH, CELL_HEIGHT}, {0, 0}},
    {{CELL_WIDTH, CELL_HEIGHT}, {0, 0}},
    {{DECAY_WIDTH, DECAY_HEIGHT},
	  {(CELL_WIDTH - DECAY_WIDTH) / 2, CELL_HEIGHT - DECAY_HEIGHT}},
    {{ROT_WIDTH, ROT_HEIGHT},
	  {(CELL_WIDTH - ROT_WIDTH) / 2, CELL_HEIGHT - ROT_HEIGHT}},
  };
/*}}}*/
/*{{{  BOARD const boards[10] =*/
#if BOARDS != 10
  #error BOARDS != 10
#endif
/*
 * the initial board maps are stored as character arrays
 * X for a path
 * @ for cherry
 */
BOARD const boards[BOARDS] =
  {
    /*{{{  board 0*/
    {
      0,
      {
	"..XXXXXXXX..",
	".XX......XX.",
	"XXXX......XX",
	"X..XX..@@@@X",
	"X...XX.@@@@X",
	"X@@@@X.....X",
	"X@@@@X...@@X",
	"X....XX..@@X",
	"X@@@@.XX.@@X",
	"X@@@@..XX@@X",
	"XX..@@@@XXXX",
	".XX.@@@@.XX.",
	"..XXXXXXXX..",
      }
    },
    /*}}}*/
    /*{{{  board 1*/
    {
      0,
      {
	"....XXXXXX..",
	"@@...X...XX.",
	"@@...X@@@@XX",
	"@@.@@X@@@@.X",
	"@@.@@X.....X",
	"...@@X.....X",
	"...@@X..@@.X",
	".....X..@@.X",
	"@@@@.X..@@.X",
	"@@@@.X..@@.X",
	"XXX..X....XX",
	"X.X..X...XX.",
	"XXXXXXXXXX..",
      }
    },
    /*}}}*/
    /*{{{  board 2*/
    {
      1,
      {
	"..XXXXXXXX..",
	".XX......XX.",
	"XX@@@@....XX",
	"X.@@@@..@@.X",
	"........@@.X",
	"@@......@@.X",
	"@@...X..@@XX",
	"@@...XXXXXX.",
	"@@.XXX....@@",
	".XXX@@@@..@@",
	"XX..@@@@..@@",
	"X.........@@",
	"XXXXXXXXXXXX",
      }
    },
    /*}}}*/
    /*{{{  board 3*/
    {
      2,
      {
	"..XXXXXXXX..",
	".XX......XX.",
	"XX........XX",
	"X..@@@@.@@.X",
	"@@.@@@@.@@.X",
	"@@......@@.X",
	"@@...X..@@XX",
	"@@..XXXXXXX.",
	"...@@.....XX",
	"...@@..@@@@X",
	"X..@@..@@@@X",
	"XX.@@.....XX",
	".XXXXXXXXXX.",
      }
    },
    /*}}}*/
    /*{{{  board 4*/
    {
      2,
      {
	"....XXXXXX..",
	"@@@@X....X..",
	"@@@@X....X@@",
	"...XX....X@@",
	"@@.X...@@X@@",
	"@@XX...@@X@@",
	"@@X..X.@@X..",
	"@@XXXX.@@X..",
	"XXX......X..",
	"X........X..",
	"XXXXXXXXXXXX",
	".@@@@....X..",
	".@@@@XXXXX..",
      }
    },
    /*}}}*/
    /*{{{  board 5*/
    {
      2,
      {
	"XXXXXXXXXXXX",
	"X...........",
	"X.@@@@......",
	"X.@@@@..@@@@",
	"X.......@@@@",
	"XXXXXXXXXXX.",
	"....XXX...XX",
	".@@.XXX..@@X",
	".@@......@@X",
	".@@.@@@@.@@X",
	"X@@.@@@@.@@X",
	"XX........XX",
	".XXXXXXXXXX.",
      }
    },
    /*}}}*/
    /*{{{  board 6*/
    {
      2,
      {
	"..XXXXXXXXXX",
	".XX.........",
	"XX..........",
	"X.@@.@@@@.@@",
	"X.@@.@@@@.@@",
	"X.@@......@@",
	"X.@@.X....@@",
	"XXXXXXXXXXX.",
	"X.........XX",
	"X@@@@......X",
	"X@@@@.@@@@.X",
	"XX....@@@@XX",
	".XXXXXXXXXX.",
      }
    },
    /*}}}*/
    /*{{{  board 7*/
    {
      3,
      {
	"XXXXXXXXXXXX",
	"X..........X",
	".@@.......XX",
	".@@.@@@@.XX.",
	".@@.@@@@XX..",
	".@@....XX.@@",
	".....XXX..@@",
	".....XX...@@",
	"@@@@.X.@@.@@",
	"@@@@.X.@@...",
	".....X.@@...",
	".....X.@@...",
	".....X......",
      }
    },
    /*}}}*/
    /*{{{  board 8*/
    {
      3,
      {
	"..XXXXXXXX..",
	".XX......XX.",
	"XX@@.@@.@@XX",
	"X.@@.@@.@@.X",
	"X.@@.@@.@@.X",
	"XX@@.@@.@@XX",
	".XX..X...XX.",
	"..XXXXXXXX..",
	"XXX......XXX",
	"X@@@@......X",
	"X@@@@.@@@@.X",
	"XX....@@@@XX",
	".XXXXXXXXXX.",
      }
    },
    /*}}}*/
    /*{{{  board 9*/
    {
      3,
      {
	"..XXXXXXXX..",
	".XX......XX.",
	"XX...@@@@.XX",
	"X.@@.@@@@..X",
	"X.@@.......X",
	"X.@@...@@@@X",
	"XX@@.X.@@@@X",
	".XXXXXXXXXXX",
	"@@.........X",
	"@@....@@@@.X",
	"@@....@@@@XX",
	"@@.......XX.",
	"XXXXXXXXXX..",
      }
    },
    /*}}}*/
  };
/*}}}*/
char keystrokes[5] = {'\'', '/', 'z', 'x', ' '};
/*{{{  COORD const ball_hold[16] =*/
/*{{{  held ball offsets*/
#define PLAYER_RIGHT1_BALL_X   (plyrrt1_x_hot - CELL_WIDTH / 2)
#define PLAYER_RIGHT1_BALL_Y   (plyrrt1_y_hot - CELL_HEIGHT / 2)
#define PLAYER_UP1_BALL_X      (plyrup1_x_hot - CELL_WIDTH / 2)
#define PLAYER_UP1_BALL_Y      (plyrup1_y_hot - CELL_HEIGHT / 2)
#define PLAYER_RIGHT2_BALL_X   (plyrrt2_x_hot - CELL_WIDTH / 2)
#define PLAYER_RIGHT2_BALL_Y   (plyrrt2_y_hot - CELL_HEIGHT / 2)
#define PLAYER_UP2_BALL_X      (plyrup2_x_hot - CELL_WIDTH / 2)
#define PLAYER_UP2_BALL_Y      (plyrup2_y_hot - CELL_HEIGHT / 2)
#define PLAYER_PUSH1_BALL_X    (plyrps1_x_hot - CELL_WIDTH / 2)
#define PLAYER_PUSH1_BALL_Y    (plyrps1_y_hot - CELL_HEIGHT / 2)
#define PLAYER_PUSH2_BALL_X    (plyrps2_x_hot - CELL_WIDTH / 2)
#define PLAYER_PUSH2_BALL_Y    (plyrps2_y_hot - CELL_HEIGHT / 2)
/*}}}*/
COORD const ball_hold[16] =
{
 {-PLAYER_UP1_BALL_X - BALL_WIDTH / 2,  PLAYER_UP1_BALL_Y - BALL_HEIGHT / 2},
 {-PLAYER_UP2_BALL_X - BALL_WIDTH / 2,  PLAYER_UP2_BALL_Y - BALL_HEIGHT / 2},
 { PLAYER_UP1_BALL_X - BALL_WIDTH / 2, -PLAYER_UP1_BALL_Y - BALL_HEIGHT / 2},
 { PLAYER_UP2_BALL_X - BALL_WIDTH / 2, -PLAYER_UP2_BALL_Y - BALL_HEIGHT / 2},
 {-PLAYER_RIGHT1_BALL_X - BALL_WIDTH / 2, PLAYER_RIGHT1_BALL_Y - BALL_HEIGHT / 2},
 {-PLAYER_RIGHT2_BALL_X - BALL_WIDTH / 2, PLAYER_RIGHT2_BALL_Y - BALL_HEIGHT / 2},
 { PLAYER_RIGHT1_BALL_X - BALL_WIDTH / 2, PLAYER_RIGHT1_BALL_Y - BALL_HEIGHT / 2},
 { PLAYER_RIGHT2_BALL_X - BALL_WIDTH / 2, PLAYER_RIGHT2_BALL_Y - BALL_HEIGHT / 2},
 { PLAYER_UP1_BALL_X - BALL_WIDTH / 2,  PLAYER_UP1_BALL_Y - BALL_HEIGHT / 2},
 { PLAYER_UP2_BALL_X - BALL_WIDTH / 2,  PLAYER_UP2_BALL_Y - BALL_HEIGHT / 2},
 {-PLAYER_UP1_BALL_X - BALL_WIDTH / 2, -PLAYER_UP1_BALL_Y - BALL_HEIGHT / 2},
 {-PLAYER_UP2_BALL_X - BALL_WIDTH / 2, -PLAYER_UP2_BALL_Y - BALL_HEIGHT / 2},
 {-PLAYER_PUSH1_BALL_X - BALL_WIDTH / 2, PLAYER_PUSH1_BALL_Y - BALL_HEIGHT / 2},
 {-PLAYER_PUSH2_BALL_X - BALL_WIDTH / 2, PLAYER_PUSH2_BALL_Y - BALL_HEIGHT / 2},
 { PLAYER_PUSH1_BALL_X - BALL_WIDTH / 2, PLAYER_PUSH1_BALL_Y - BALL_HEIGHT / 2},
 { PLAYER_PUSH2_BALL_X - BALL_WIDTH / 2, PLAYER_PUSH2_BALL_Y - BALL_HEIGHT / 2},
};
/*}}}*/
/*{{{  COORD const ball_throw[8] =*/
COORD const ball_throw[8] =
{
  {-GAP_WIDTH / 2, -(CELL_HEIGHT + GAP_HEIGHT) / 2},
  {GAP_WIDTH / 2, (CELL_HEIGHT + GAP_HEIGHT) / 2},
  {-(CELL_WIDTH + GAP_WIDTH) / 2, GAP_HEIGHT / 2},
  {(CELL_WIDTH + GAP_WIDTH) / 2, GAP_HEIGHT / 2},
  {GAP_WIDTH / 2, -(CELL_HEIGHT + GAP_HEIGHT) / 2},
  {-GAP_WIDTH / 2, (CELL_HEIGHT + GAP_HEIGHT) / 2},
  {-(CELL_WIDTH + GAP_WIDTH) / 2, GAP_HEIGHT / 2},
  {(CELL_WIDTH + GAP_WIDTH) / 2, GAP_HEIGHT / 2},
};
/*}}}*/
int const ball_dir[8] = {0, 1, 2, 1, 3, 2, 2, 1};
/*{{{  int const player_dies[8] =*/
int const player_dies[8] =
  {
    SPRITE_PLAYER_DEAD + 1,
    SPRITE_PLAYER + 0,
    SPRITE_PLAYER + 4,
    SPRITE_PLAYER + 2,
    SPRITE_PLAYER + 8,
    SPRITE_PLAYER + 6,
    SPRITE_PLAYER + 10,
    SPRITE_PLAYER_DEAD + 0,
  };
/*}}}*/
/*{{{  char const *title_text[] =*/
char const *title_text[] =
  {
    "(C) 1992 Nathan Sidwell",
#if __STDC__
    XMRISVERSION " " __DATE__, "",
#else
    "                           ", /* should be enough space */
#endif
    "Z - Left", "X - Right", "' - Up", "/ - Down", "Space - Throw",
    "Or use the mouse", "P - Pause", "Q - Quit",
    "Press a key or button to start", NULL
  };
/*}}}*/
#if SQUISH_SCORES != 7
  #error SQUISH_SCORES != 7
#endif
int const squish_scores[SQUISH_SCORES] = {0, 1000, 2000, 4000, 6000, 8000, 9900};
/*}}}*/
/*{{{  void create_resources(arc, argv)*/
extern void create_resources FUNCARGLIST((argc, argv))
int     argc    FUNCARGSEP
char    **argv  FUNCARGTERM
{
  display.display = XOpenDisplay(display.name);
  /*{{{  opened?*/
  if(!display.display)
    fatal_error("Cannot open display \"%s\"",
	display.name ? display.name : "DEFAULT", stderr);
  /*}}}*/
  /*{{{  open the display*/
  {
    display.screen = DefaultScreen(display.display);
    display.colormap = DefaultColormap(display.display, display.screen);
    display.root = DefaultRootWindow(display.display);
    display.depth = DefaultDepth(display.display, display.screen);
    display.black = BlackPixel(display.display, display.screen);
    display.white = WhitePixel(display.display, display.screen);
    display.xor = display.black ^ display.white; /* for xor context */
    /*
     * We want to suspend the game in case the window is iconified.
     * This is more difficult than it sounds.  On the Sun, iconification
     * seems to produce an UnmapNotify event -- very nice.  DECwindows,
     * however, informs the application by generating a PropertyNotify
     * event on a DEC-specific property -- very nasty.  The atom is
     * not defined in any of the DECwindows headers, so we will try
     * to get its value from the server, and use it later.  We are
     * hoping here that all non-DECwindows servers will return None
     * for this atom.
     */
    display.DEC_icon_atom =
	XInternAtom(display.display, "DEC_WM_ICON_STATE", True);
    display.event_mask = ExposureMask | (display.DEC_icon_atom == None ?
	StructureNotifyMask : PropertyChangeMask);
  }
  /*}}}*/
  /*{{{  reverse video?*/
  if(flags.reverse)
    {
      unsigned long temp;
	
      temp = display.black;
      display.black = display.white;
      display.white = temp;
    }
  /*}}}*/
  /*{{{  set foreground & background types*/
  {
    display.background = display.white == 0 ? COLOUR_ZERO :
	display.white == (1 << display.depth) - 1 ? COLOUR_ONE :
	COLOUR_WEIRD;
    display.foreground = display.black == 0 ? COLOUR_ZERO :
	display.black == (1 << display.depth) - 1 ? COLOUR_ONE :
	COLOUR_WEIRD;
  }
  /*}}}*/
  /*{{{  get a font*/
  {
    font.font = XLoadFont(display.display, font.name);
    if(!font.font)
      fatal_error("Cannot load font \"%s\"", font.name);
  }
  /*}}}*/
  /*{{{  create graphics contexts*/
  {
    XGCValues gcv;
    CONTEXT   *cptr;
    unsigned  count;
	
    gcv.plane_mask = AllPlanes;
    gcv.font       = font.font;
    gcv.graphics_exposures = False;
    gcv.line_width = 1;
    gcv.line_style = LineSolid;
    gcv.join_style = JoinMiter;
    gcv.fill_style = FillSolid;
    for(cptr = gcsdefine, count = 0; count != GCS; count++, cptr++)
      {
	gcv.function = cptr->function;
	gcv.foreground = *cptr->fgp;
	gcv.background = *cptr->bgp;
	GCN(count) = XCreateGC(display.display, display.root, 
	    GCForeground | GCBackground | GCFunction |
	    GCFont | GCGraphicsExposures | GCPlaneMask |
	    GCLineStyle | GCLineWidth | GCJoinStyle | GCFillStyle, &gcv);
	if(!GCN(count))
	  fatal_error("Cannot create context %d", count);
      }
  }
  /*}}}*/
  /*{{{  create window*/
  {
    XSizeHints hints;
    XColor colors[2];
    Pixmap cursor, mask;
	
    hints.flags = PSize | PMinSize | PMaxSize;
    hints.width = WINDOW_WIDTH;
    hints.height = WINDOW_HEIGHT;
    hints.min_width = hints.max_width = hints.width;
    hints.min_height = hints.max_height = hints.height;
    colors[0].pixel = display.black;
    colors[1].pixel = display.white;
    XQueryColors(display.display, display.colormap, colors, 2);
    display.icon = XCreatePixmapFromBitmapData(display.display,
	display.root, icons[flags.gender].mask_bits,
	icons[flags.gender].width, icons[flags.gender].height, 1, 0, 1);
    if(!display.icon)
      fatal_error("Cannot create window icon");
    cursor = mask = 0;
    cursor = XCreatePixmapFromBitmapData(display.display,
	display.root, cursor_bits,
	cursor_width, cursor_height, 1, 0, 1);
    mask = XCreatePixmapFromBitmapData(display.display,
	display.root, cursorm_bits,
	cursorm_width, cursorm_height, 1, 0, 1);
    if(cursor && mask)
      display.cursor = XCreatePixmapCursor(display.display,
	  cursor, mask, &colors[0], &colors[1],
	  cursor_x_hot, cursor_y_hot);
    if(cursor)
      XFreePixmap(display.display, cursor);
    if(mask)
      XFreePixmap(display.display, mask);
    if(!display.cursor)
      fatal_error("Cannot create cursor icon");
    display.window = XCreateSimpleWindow(display.display, display.root,
	0, 0, hints.width, hints.height, 1, display.black, display.white);
    if(!display.window)
      fatal_error("Cannot create window");
    XSetStandardProperties(display.display, display.window,
	game_name, game_name, display.icon, argv, argc, &hints);
    XDefineCursor(display.display, display.window, display.cursor);
  }
  /*}}}*/
  display.back = XCreatePixmap(display.display, display.root,
      WINDOW_WIDTH, WINDOW_HEIGHT, display.depth);
  display.copy = XCreatePixmap(display.display, display.root,
      WINDOW_WIDTH, WINDOW_HEIGHT, display.depth);
  if(!display.back || !display.copy)
    fatal_error("Cannot create window copies");
  /*{{{  are we she?*/
  if(flags.gender)
    {
      memcpy(&sprites[SPRITE_MRIS + 1], &sprites[SPRITE_MRIS + 3],
	  sizeof(SPRITE));
      memcpy(&sprites[SPRITE_MRIS + 5], &sprites[SPRITE_MRIS + 7],
	  sizeof(SPRITE));
      memcpy(&sprites[SPRITE_MRIS + 3],  &spritet[0], sizeof(SPRITE));
      memcpy(&sprites[SPRITE_MRIS + 7],  &spritet[1], sizeof(SPRITE));
    }
  /*}}}*/
  /*{{{  create sprites*/
  {
    unsigned  i;
    SPRITE    *sptr;
	
    /*{{{  generate all the ones from bitmaps*/
    for(i = 0, sptr = sprites; i != SPRITES; i++, sptr++)
      {
	/* check that its the size we expected */
	assert((!sptr->expected.x || sptr->expected.x == sptr->width) &&
	    (!sptr->expected.y || sptr->expected.y == sptr->height));
	if(sptr->mask_bits || sptr->image_bits)
	  {
	    assert(sptr->width && sptr->height);
	    if(sptr->image_bits)
	      sptr->image = XCreatePixmapFromBitmapData(display.display,
		  display.root, sptr->image_bits,
		  sptr->width, sptr->height,
		  display.black, display.white, display.depth);
	    else
	      {
		sptr->image = XCreatePixmap(display.display, display.root,
		    sptr->width, sptr->height, display.depth);
		if(sptr->image)
		  XFillRectangle(display.display, sptr->image, GCN(GC_CLEAR),
		      0, 0, sptr->width, sptr->height);
	      }
	    if(sptr->mask_bits && sptr->image)
	      {
		sptr->mask = XCreatePixmapFromBitmapData(display.display,
		    display.root, sptr->mask_bits, sptr->width, sptr->height,
		    ((unsigned long)1 << display.depth) - (unsigned long)1,
		    (unsigned long)0, display.depth);
		if(sptr->mask)
		  XCopyArea(display.display, sptr->mask, sptr->image, GCN(GC_AND),
		      0, 0, sptr->width, sptr->height, 0, 0);
	      }
	    if((!sptr->mask && sptr->mask_bits) || !sptr->image)
	      fatal_error("Cannot create sprite %d", i);
	  }
      }
    /*}}}*/
    /*{{{  do the ball*/
    {
      sptr = &sprites[SPRITE_BALL];
      ball_xor = XCreatePixmap(display.display, display.window,
	  BALL_WIDTH, BALL_HEIGHT, display.depth);
      if(!ball_xor)
	fatal_error("Cannot create ball sprite");
      XFillRectangle(display.display, ball_xor, GCN(GC_MASK),
	  0, 0, BALL_WIDTH, BALL_HEIGHT);
      XFillRectangle(display.display, ball_xor, GCN(GC_BALL),
	  0, 0, BALL_WIDTH, BALL_HEIGHT);
      XCopyArea(display.display, sptr->mask, ball_xor, GCN(GC_AND),
	  0, 0, BALL_WIDTH, BALL_HEIGHT, 0, 0);
    }
    /*}}}*/
    /*{{{  now do the copies*/
    for(sptr = sprites, i = SPRITES; i--; sptr++)
      if(!sptr->copy)
	{
	  /* Sun's assert macro is broken, so I have to
	   * put it in a scope */
	  assert(sptr->mask_bits || sptr->image_bits);
	}
      else
	{
	  SPRITE    *optr;
	      
	  optr = &sprites[sptr->copy];
	  assert(optr->width && optr->height);
	  sptr->width = optr->width;
	  sptr->height = optr->height;
	  switch(sptr->reflect)
	  {
	    /*{{{  case 0: (no reflections)*/
	    case 0:
	      sptr->mask = optr->mask;
	      sptr->image = optr->image;
	      break;
	    /*}}}*/
	    /*{{{  case 1: (vertical axis)*/
	    case 1:
	    {
	      int       i;
						  
	      sptr->mask = XCreatePixmap(display.display, display.root,
		  sptr->width, sptr->height, display.depth);
	      sptr->image = XCreatePixmap(display.display, display.root,
		  sptr->width, sptr->height, display.depth);
	      if(sptr->image && sptr->mask)
		for(i = sptr->width; i--;)
		  {
		    XCopyArea(display.display, optr->mask, sptr->mask,
			GCN(GC_COPY),
			i, 0, 1, sptr->height, sptr->width - i - 1, 0);
		    XCopyArea(display.display, optr->image, sptr->image,
			GCN(GC_COPY),
			i, 0, 1, sptr->height, sptr->width - i - 1, 0);
		  }
	      break;
	    }
	    /*}}}*/
	    /*{{{  case 2: (horizontal axis)*/
	    case 2:
	    {
	      int       i;
						  
	      sptr->mask = XCreatePixmap(display.display, display.root,
		  sptr->width, sptr->height, display.depth);
	      sptr->image = XCreatePixmap(display.display, display.root,
		  sptr->width, sptr->height, display.depth);
	      if(sptr->mask && sptr->image)
		for(i = sptr->width; i--;)
		  {
		    XCopyArea(display.display, optr->mask, sptr->mask,
			GCN(GC_COPY),
			0, i, sptr->width, 1, 0, sptr->height - i - 1);
		    XCopyArea(display.display, optr->image, sptr->image,
			GCN(GC_COPY),
			0, i, sptr->width, 1, 0, sptr->height - i - 1);
		  }
	      break;
	    }
	    /*}}}*/
	    /*{{{  default*/
	    default:
	      assert(0);
	    /*}}}*/
	  }
	  if(!sptr->image || !sptr->mask)
	    fatal_error("Cannot generate sprite %d", SPRITES - 1 - i);
	}
    /*}}}*/
  }
  /*}}}*/
  /*{{{  check ball hold is ok*/
  {
    unsigned  i;
	
    for(i = sizeof(ball_hold) / sizeof(COORD); i--;)
      assert(ball_hold[i].x + CELL_WIDTH / 2 >= 0 &&
	  ball_hold[i].x + CELL_WIDTH / 2 <= CELL_WIDTH - BALL_WIDTH / 2 && 
	  ball_hold[i].y + CELL_HEIGHT / 2 >= 0 &&
	  ball_hold[i].y + CELL_HEIGHT / 2 <= CELL_HEIGHT - BALL_HEIGHT / 2);
  }
  /*}}}*/
  /*{{{  create score pixmaps*/
  {
    unsigned  i;
	
    for(i = BOARD_SCORES; i--;)
      {
	update.score.list[i].image = XCreatePixmap(display.display,
	    display.root, DIGIT_WIDTH * 4, DIGIT_HEIGHT, display.depth);
	update.score.list[i].mask = XCreatePixmap(display.display,
	    display.root, DIGIT_WIDTH * 4, DIGIT_HEIGHT, display.depth);
	if(!update.score.list[i].image || !update.score.list[i].mask)
	  fatal_error("Cannot create score pixmap");
      }
  }
  /*}}}*/
  return;
}
/*}}}*/
/*{{{  void create_xtra_monster(index)*/
extern void create_xtra_monster FUNCARGLIST((index))
int   index FUNCARGTERM
{
  SPRITE    *dptr;
  SPRITE    *sptr;
  SPRITE    *lptr;
  unsigned  i;
  
  sptr = &sprites[SPRITE_XTRA];
  lptr = &sprites[SPRITE_EXTRA];
  for(dptr = &sprites[SPRITE_XTRA], sptr = &sprites[SPRITE_XTRA_SOURCE],
      i = MONSTER_IMAGES; i--; dptr++, sptr++)
    {
      XCopyArea(display.display, sptr->image, dptr->image, GCN(GC_COPY),
	  0, 0, CELL_WIDTH, CELL_HEIGHT, 0, 0);
      XCopyArea(display.display, lptr->mask, dptr->image, GCN(GC_MASK),
	  index * (CELL_WIDTH / 2), 0, CELL_WIDTH / 2, CELL_HEIGHT / 2,
	  XTRA_LETTER_X, XTRA_LETTER_Y);
      XCopyArea(display.display, lptr[!(extra.got & 1 << index)].image,
	  dptr->image, GCN(GC_OR),
	  index * (CELL_WIDTH / 2), 0, CELL_WIDTH / 2, CELL_HEIGHT / 2,
	  XTRA_LETTER_X, XTRA_LETTER_Y);
    }
  return;
}
/*}}}*/
/*{{{  void draw_extra_letter(index)*/
extern void draw_extra_letter FUNCARGLIST((index))
int     index  FUNCARGTERM
{
  SPRITE    *lptr;
  int       x;
  
  lptr = &sprites[SPRITE_EXTRA];
  x = XTRA_X + index * XTRA_SPACING;
  XFillRectangle(display.display, display.back, GCN(GC_CLEAR),
      x, XTRA_Y, CELL_WIDTH, CELL_HEIGHT);
  XCopyArea(display.display, lptr->mask,
      display.back, GCN(GC_MASK), index * (CELL_WIDTH / 2), 0,
      CELL_WIDTH / 2, CELL_HEIGHT / 2,
      x + XTRA_LETTER_X, XTRA_Y + XTRA_LETTER_Y);
  XCopyArea(display.display, lptr[!(extra.got & 1 << index)].image,
      display.back, GCN(GC_OR), index * (CELL_WIDTH / 2), 0,
      CELL_WIDTH / 2, CELL_HEIGHT / 2,
      x + XTRA_LETTER_X, XTRA_Y + XTRA_LETTER_Y);
  add_background(x, XTRA_Y, CELL_WIDTH, CELL_HEIGHT);
  return;
}
/*}}}*/
/*{{{  void release_resources()*/
extern void release_resources FUNCARGVOID
/*
 * frees all the resources we have allocated by create_resources
 */
{
  unsigned  i;
  SPRITE    *sptr;

  if(display.window)
    {
      XUnmapWindow(display.display, display.window);
      XUndefineCursor(display.display, display.window);
      XDestroyWindow(display.display, display.window);
    }
  if(font.font)
    XUnloadFont(display.display, font.font);
  if(display.cursor)
    XFreeCursor(display.display, display.cursor);
  if(display.copy)
    XFreePixmap(display.display, display.copy);
  if(display.back)
    XFreePixmap(display.display, display.back);
  if(display.icon)
    XFreePixmap(display.display, display.icon);
  /*{{{  free gcs*/
  {
    GC        *gcptr;
    unsigned  count;
	
    for(gcptr = gcs, count = GCS; count--; gcptr++)
      if(*gcptr)
	{
	  XFreeGC(display.display, *gcptr);
	  *gcptr = 0;
	}
  }
  /*}}}*/
  for(i = SPRITES, sptr = sprites; i--; sptr++)
    {
      if(!sptr->copy || sptr->reflect)
	{
	  if(sptr->image)
	    XFreePixmap(display.display, sptr->image);
	  if(sptr->mask)
	    XFreePixmap(display.display, sptr->mask);
	}
    }
  for(i = BOARD_SCORES; i--;)
    {
      if(update.score.list[i].image)
	XFreePixmap(display.display, update.score.list[i].image);
      if(update.score.list[i].mask)
	XFreePixmap(display.display, update.score.list[i].mask);
    }
  if(display.display)
    {
      XFlush(display.display);
      XCloseDisplay(display.display);
    }
  return;
}
/*}}}*/
