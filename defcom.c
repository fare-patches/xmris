/* Copyright (C) 1993, 1992 Nathan Sidwell */
/* additional sprites by Stefan Gustavson <stefang@isy.liu.se> */
/* common define file for xmris and xmred, included from defmris or defmred */
/* RCS $Id: defcom.c 1.2 Tue, 16 Mar 1999 11:28:16 +0000 nathan $ */
/*{{{  some sprite info*/
/* you can add your own sprites, if you like, but you have to be careful
 * to make sure that they look ok with both -swap and -noswap, and also
 * fiddle the colour definitions, as there are checks to make sure that
 * we don't allocate colours which aren't used */
/*}}}*/
/*{{{  bitmaps*/
/*{{{  bitmaps/bitmaps*/
#include "bitmaps/bitmaps/oblong.h"
#include "bitmaps/bitmaps/rect.h"
#include "bitmaps/bitmaps/edge.h"
#include "bitmaps/bitmaps/fill0.h"
#include "bitmaps/bitmaps/fill1.h"
#include "bitmaps/bitmaps/fill2.h"
#include "bitmaps/bitmaps/fill3.h"
#ifndef XMRED
#include "bitmaps/bitmaps/mris.h"
#include "bitmaps/bitmaps/msit.h"
#include "bitmaps/bitmaps/ghosting.h"
#else
#include "bitmaps/bitmaps/mred.h"
#endif /* XMRED */
/*}}}*/
/*{{{  bitmaps/icon/.*/
#ifndef XMRED
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
#endif /* XMRED */
/*}}}*/
/*{{{  bitmaps/board/.*/
#include "bitmaps/board/cherry.h"
#ifndef XMRED
#include "bitmaps/board/ball.h"
#include "bitmaps/board/digits.h"
#include "bitmaps/board/seat.h"
#endif /* XMRED */
/*}}}*/
/*{{{  bitmaps/apple/.*/
#include "bitmaps/apple/aplnorm.h"
#ifndef XMRED
#include "bitmaps/apple/aplrock.h"
#include "bitmaps/apple/aplspl.h"
#include "bitmaps/apple/apldcy.h"
#include "bitmaps/apple/aplrot.h"
#include "bitmaps/apple/gstnorm.h"
#include "bitmaps/apple/gstrock.h"
#else
#include "bitmaps/apple/aplsmall.h"
#include "bitmaps/apple/bigedge.h"
#include "bitmaps/apple/smledge.h"
#endif /* XMRED */
/*}}}*/
/*{{{  bitmaps/player/.*/
#include "bitmaps/player/hplyrw1.h"
#include "bitmaps/player/splyrw1.h"
#ifndef XMRED
#include "bitmaps/player/hplyrw2.h"
#include "bitmaps/player/hplyrs1.h"
#include "bitmaps/player/hplyrs2.h"
#include "bitmaps/player/hplyrf.h"
#include "bitmaps/player/hplyrp1.h"
#include "bitmaps/player/hplyrp2.h"
#include "bitmaps/player/hplyrh1.h"
#include "bitmaps/player/hplyrh2.h"
#include "bitmaps/player/splyrw2.h"
#include "bitmaps/player/splyrs1.h"
#include "bitmaps/player/splyrs2.h"
#include "bitmaps/player/splyrf.h"
#include "bitmaps/player/splyrp1.h"
#include "bitmaps/player/splyrp2.h"
#include "bitmaps/player/splyrd1.h"
#include "bitmaps/player/splyrd2.h"
#include "bitmaps/player/splyrh1.h"
#include "bitmaps/player/splyrh2.h"
#endif /* XMRED */
/*}}}*/
/*{{{  bitmaps/normal/.*/
#include "bitmaps/normal/hden.h"
#include "bitmaps/normal/sden.h"
#ifndef XMRED
#include "bitmaps/normal/hnormw1.h"
#include "bitmaps/normal/hnormw2.h"
#include "bitmaps/normal/hnormf.h"
#include "bitmaps/normal/snormw1.h"
#include "bitmaps/normal/snormw2.h"
#include "bitmaps/normal/snormf.h"
#endif /* XMRED */
/*}}}*/
/*{{{  bitmaps/munch/.*/
#ifndef XMRED
#include "bitmaps/munch/hmnchw1.h"
#include "bitmaps/munch/hmnchw2.h"
#include "bitmaps/munch/hmnchf.h"
#include "bitmaps/munch/smnchw1.h"
#include "bitmaps/munch/smnchw2.h"
#include "bitmaps/munch/smnchf.h"
#endif /* XMRED */
/*}}}*/
/*{{{  bitmaps/xtra/.*/
#ifndef XMRED
#include "bitmaps/xtra/hxtraw1.h"
#include "bitmaps/xtra/hxtraw2.h"
#include "bitmaps/xtra/hxtraf.h"
#include "bitmaps/xtra/sxtraw1.h"
#include "bitmaps/xtra/sxtraw2.h"
#include "bitmaps/xtra/sxtraf.h"
#include "bitmaps/xtra/xtrfaint.h"
#endif /* XMRED */
#include "bitmaps/xtra/xtrbold.h"
/*}}}*/
/*{{{  bitmaps/drone/.*/
#ifndef XMRED
#include "bitmaps/drone/hdronw1.h"
#include "bitmaps/drone/hdronw2.h"
#include "bitmaps/drone/hdronf.h"
#include "bitmaps/drone/sdronw1.h"
#include "bitmaps/drone/sdronw2.h"
#include "bitmaps/drone/sdronf.h"
#endif /* XMRED */
/*}}}*/
/*{{{  bitmaps/chomp/.*/
#ifndef XMRED
#include "bitmaps/chomp/hchmpo.h"
#include "bitmaps/chomp/hchmpc.h"
#include "bitmaps/chomp/schmpo.h"
#include "bitmaps/chomp/schmpc.h"
#endif /* XMRED */
/*}}}*/
/*{{{  bitmaps/prize/.*/
#ifndef XMRED
#include "bitmaps/prize/cake.h"
#include "bitmaps/prize/spanner.h"
#include "bitmaps/prize/brolly.h"
#include "bitmaps/prize/mushrm.h"
#include "bitmaps/prize/clock.h"
#include "bitmaps/prize/gem1.h"
#include "bitmaps/prize/gem2.h"
#include "bitmaps/prize/gem3.h"
#endif /* XMRED */
/*}}}*/
/*{{{  bitmaps/edit/.*/
#ifdef XMRED
#include "bitmaps/edit/bigearw.h"
#include "bitmaps/edit/smlearw.h"
#include "bitmaps/edit/bigsarw.h"
#include "bitmaps/edit/smlsarw.h"
#include "bitmaps/edit/path.h"
#include "bitmaps/edit/red.h"
#include "bitmaps/edit/green.h"
#include "bitmaps/edit/blue.h"
#include "bitmaps/edit/include.h"
#include "bitmaps/edit/iconi.h"
#include "bitmaps/edit/boardi.h"
#include "bitmaps/edit/warnon.h"
#include "bitmaps/edit/warnoff.h"
#endif /* XMRED */
/*}}}*/
/*}}}*/
/*{{{  some sprite sizes*/
#define DECAY_WIDTH     apldcy_width
#define DECAY_HEIGHT    (apldcy_height / apldcy_depth)
#define ROT_WIDTH       aplrot_width
#define ROT_HEIGHT      (aplrot_height / aplrot_depth)
/*}}}*/
unsigned long color_zero = 0;
unsigned long color_one = ~0;
Boolean  default_gender = False;
static int CONST negone = -1;
/*{{{  create tables*/
/*{{{  extern CommandOptionDesc command_options[] =*/
CommandOptionDesc command_options[] =
{
  {"-help",    0,  XtOffsetOf(DATA, help)},
  {"-colors",  0,  XtOffsetOf(DATA, colors)},
  {"-colours", 0,  XtOffsetOf(DATA, colors)},
#ifndef XMRED
  {"-scores",  0,  XtOffsetOf(DATA, scores)},
  {"-sprites", 0,  XtOffsetOf(DATA, sprites)},
  {"-format",  1,  XtOffsetOf(DATA, format)},
  {"-expire",  1,  XtOffsetOf(DATA, expire)},
  {"-remove",  1,  XtOffsetOf(DATA, remove)},
  {"-nodisplay",0, XtOffsetOf(DATA, nodisplay)},
  {"+display", 0,  XtOffsetOf(DATA, nodisplay)},
  {"-dir",     1,  XtOffsetOf(DATA, dir)},
#endif /* XMRED */
  {NULL}
};
/*}}}*/
/*{{{  extern XrmOptionDescRec options[] =*/
XrmOptionDescRec options[] =
{
  {"-monochrome",".monochrome", XrmoptionNoArg, (XtPointer)XtEyes},
  {"-noswap",  "*reverseVideo", XrmoptionNoArg, (XtPointer)XtEno},
  {"+swap",    "*reverseVideo", XrmoptionNoArg, (XtPointer)XtEno},
  {"-swap",    "*reverseVideo", XrmoptionNoArg, (XtPointer)XtEyes},
  {"-mris",    ".gender",       XrmoptionNoArg, (XtPointer)"mris"},
  {"-msit",    ".gender",       XrmoptionNoArg, (XtPointer)"msit"},
  {"-gender",  ".gender",       XrmoptionSepArg,(XtPointer)NULL},
  {"-private", ".private",      XrmoptionNoArg, (XtPointer)XtEyes},
  {"-depth",   "*depth",        XrmoptionSepArg,(XtPointer)NULL},
  {"-visual",  "*visual",       XrmoptionSepArg,(XtPointer)NULL},
  {"-distinct",".distinct",     XrmoptionSepArg,(XtPointer)NULL},
#ifndef XMRED
  {"-random",  ".random",       XrmoptionNoArg, (XtPointer)XtEyes},
  {"+random",  ".random",       XrmoptionNoArg, (XtPointer)XtEno},
  {"-gardens", ".gardens",      XrmoptionSepArg,(XtPointer)NULL},
  {"-username",".username",     XrmoptionNoArg, (XtPointer)XtEyes},
  {"-realname",".username",     XrmoptionNoArg, (XtPointer)XtEno},
  {"-static",  ".static",       XrmoptionNoArg, (XtPointer)XtEyes},
#ifndef TRANSPUTER
  {"-busywait", ".busywait",    XrmoptionNoArg, (XtPointer)XtEyes},
#endif /* TRANSPUTER */
#endif /* XMRED */
  {NULL}
};
/*}}}*/
/*{{{  extern XtResource resources[] =*/
XtResource resources[] =
{
#ifndef XMRED
  /* dir must be first */
  {"dir", "Dir", XtRString, sizeof(String),
      XtOffsetOf(DATA, dir), XtRString, (XtPointer)SCORE_DIR},
  {"up", "Up", XtRKeySym, sizeof(KeySym),
      XtOffsetOf(DATA, keysyms[0]), XtRString, (XtPointer)"apostrophe"},
  {"down", "Down", XtRKeySym, sizeof(KeySym),
      XtOffsetOf(DATA, keysyms[1]), XtRString, (XtPointer)"slash"},
  {"left", "Left", XtRKeySym, sizeof(KeySym),
      XtOffsetOf(DATA, keysyms[2]), XtRString, (XtPointer)"z"},
  {"right", "Right", XtRKeySym, sizeof(KeySym),
      XtOffsetOf(DATA, keysyms[3]), XtRString, (XtPointer)"x"},
  {"throw", "Throw", XtRKeySym, sizeof(KeySym),
      XtOffsetOf(DATA, keysyms[KEY_THROW]), XtRString, (XtPointer)"space"},
  {"pause", "Pause", XtRKeySym, sizeof(KeySym),
      XtOffsetOf(DATA, keysyms[KEY_PAUSE]), XtRString, (XtPointer)"p"},
  {"quit", "Quit", XtRKeySym, sizeof(KeySym),
      XtOffsetOf(DATA, keysyms[KEY_QUIT]), XtRString, (XtPointer)"q"},
  {"iconize", "Iconize", XtRKeySym, sizeof(KeySym),
      XtOffsetOf(DATA, keysyms[KEY_ICONIZE]), XtRString, (XtPointer)"i"},
  {"keyboard", "Keyboard", XtRKeySym, sizeof(KeySym),
      XtOffsetOf(DATA, keysyms[KEY_KEYBOARD]), XtRString, (XtPointer)"k"},
  {"random", "Random", XtRBoolean, sizeof(Boolean),
      XtOffsetOf(DATA, random), XtRImmediate, (XtPointer)False},
  {"gardens", "Gardens", XtRString, sizeof(String),
      XtOffsetOf(DATA, boards), XtRImmediate, (XtPointer)NULL},
  {"userName", "UserName", XtRBoolean, sizeof(Boolean),
      XtOffsetOf(DATA, username), XtRImmediate, (XtPointer)False},
  {"static", "Static", XtRBoolean, sizeof(Boolean),
      XtOffsetOf(DATA, nodynamic), XtRImmediate, (XtPointer)False},
#ifndef TRANSPUTER
  {"busywait", "Busywait", XtRBoolean, sizeof(Boolean),
      XtOffsetOf(DATA, busywait), XtRImmediate, (XtPointer)False},
#endif /* TRANSPUTER */
#endif /* XMRED */
  {XtNfont, XtCFont, XtRFont, sizeof(Font),
      XtOffsetOf(DATA, font), XtRString, (XtPointer)FONT_NAME},
  {XtNreverseVideo, XtCReverseVideo, XtRBoolean, sizeof(Boolean),
      XtOffsetOf(DATA, swap), XtRImmediate, (XtPointer)False},
  {"monochrome", "Monochrome", XtRBoolean, sizeof(Boolean),
      XtOffsetOf(DATA, mono), XtRImmediate, (XtPointer)False},
  {"gender", "Gender", XtRGender, sizeof(Boolean),
      XtOffsetOf(DATA, gender), XtRGender, (XtPointer)&default_gender},
  {"private", "Private", XtRBoolean, sizeof(Boolean),
      XtOffsetOf(DATA, private), XtRImmediate, (XtPointer)False},
  {"distinct", "Distinct", XtRInt, sizeof(int),
      XtOffsetOf(DATA, distinct), XtRInt, (XtPointer)&negone},
  {NULL}
};
/*}}}*/
/*{{{  extern COLOR_DEF color_names[] =*/
COLOR_DEF color_names[] =
{
  {"White",       15, {NULL}, 2},
  {"Black",       15, {NULL}, 2},
  {"GreenBack",   31, {"#77BB77", "#BBFFBB", "#77BB77", "#BBFFBB"}},
  {"GreenFore",   31, {"#007700", "#00BB00", "#007700", "#00BB00"}},
  {"RedBack",     31, {"#BB7777", "#FFBBBB", "#BB7777", "#FFBBBB"}},
  {"RedFore",     31, {"#770000", "#BB0000", "#770000", "#BB0000"}},
  {"BlueBack",    31, {"#7777BB", "#BBBBFF", "#7777BB", "#BBBBFF"}},
  {"BlueFore",    31, {"#000077", "#0000BB", "#000077", "#0000BB"}},
  {"Background",  15, {"#FFFFFF", "#000000"}},
  {"Foreground",  15, {"#000000", "#FFFFFF"}},
  {"BorderColor", 15, {"#000000", "#FFFFFF"}},
  {"Cherry",      15, {"#EE0000"}},
  {"CherryStalk", 10, {"#DD9955", "#EEAA66"}},
  {"Apple1",      15, {"#EEDD00"}},
  {"Apple2",      15, {"#DD3300"}},
  {"AppleFaint",  
#ifndef XMRED
   5, {"#BBBBBB"}
#else
   15, {"#CCCCCC", "#FFFFFF"}
#endif /* XMRED */
   },
  {"Player",      15, {"#0000DD", "#6666FF", "#6666FF"}},
  {"PlayerBobble",15, {"#FFFFFF"}},
  {"PlayerSkin",  15, {"#FFCCCC", "#FFDDDD"}},
  {"PlayerBoot",  14, {"#DD9955", "#EEAA66", "#773322", "#DD9955"}},
  {"Normal",      15, {"#EE0000"}},
  {"LetterGot",   15, {"#000000", "#FFFFFF"}},
#ifndef XMRED
  {"LetterNotGot", 15, {"#BBBBBB"}},
  {"Ball",        15, {"#FFFF77", NULL, "#FF00FF", "#FF00FF"}},
  {"DroneBack",   51, {"#AA3333", "#FF6666", "#AA3333", "#FF6666"}},
  {"DroneFore",   51, {"#992222", "#FF2222", "#992222", "#FF2222"}},
  {"Gem1",        15, {"#DDDDDD"}},
  {"Gem2",        15, {"#BBBBBB"}},
  {"Munch1",      15, {"#FFFF00", NULL, "#FFFF00"}},
  {"Munch2",      15, {"#CCCCCC", NULL, "#FFFF00"}},
  {"Drone",       15, {"#0000DD", "#6666FF", "#00FF00"}},
  {"DroneTongue", 12, {"#EE0000"}},
  {"Extra",       15, {"#EEFF00"}},
  {"Chomp",       15, {"#FFFFFF", NULL, "#CCFF00"}},
  {"ChompLip",     3, {"#77FFFF"}},
  {"ChompTongue", 12, {"#EE0000"}},
  {"Seat",        15, {"#EE0000"}},
  {"Cake",        15, {"#FFFF77"}},
  {"CakeIcing",   15, {"#DD9955", "#EEAA66"}},
  {"CakeJam",     15, {"#EE0000"}},
  {"Spanner",     15, {"#AAAAAA", "#DDDDDD"}},
  {"SpannerShadow", 15, {"#000000"}},
  {"Brolly1",     15, {"#FFFFFF"}},
  {"Brolly2",     15, {"#EE0000"}},
  {"BrollyHandle", 15, {"#DD9955", "#EEAA66"}},
  {"MushroomStalk", 15, {"#FFFFFF"}},
  {"MushroomCap", 15, {"#EE0000"}},
  {"ClockFace",   15, {"#FFFFFF"}},
  {"ClockBell",   15, {"#00DD00"}},
  {"ClockRim",    15, {"#0000DD", "#00DD00", "#00DD00", "#00DD00"}},
  {"DynamicBack", 32, {NULL}, 2},  /*  dynamic backgrounds */
  {"DynamicFore", 32, {NULL}, 2},
#endif
};
/*}}}*/
/*{{{  extern CONTEXT gcsdefine[GCS] =*/
CONTEXT gcsdefine[GCS] =
{
    {GXcopy,        &color_one,       &color_zero},     /* GC_COPY */
    {GXcopy,        &display.white,   &color_zero},     /* GC_CLEAR */
    {GXcopy,        &display.black,   &color_zero},     /* GC_SET */
    {GXandInverted, &color_one,       &color_zero},     /* GC_MASK */
    {GXor,          &color_one,       &color_zero},     /* GC_OR */
    {GXcopy,        &display.black,   &display.white},  /* GC_TEXT */
    {GXand,         &color_one,       &color_zero},     /* GC_AND */
    {GXcopy,        &display.black,   &display.white},  /* GC_BOARD */
    {GXcopy,        &display.border,  &display.white},  /* GC_BORDER */
#ifndef XMRED
    {GXxor,         &display.xor,     &color_zero},     /* GC_LOAD */
#endif /* XMRED */
};
/*}}}*/
/*{{{  extern HELP CONST help[] =*/
HELP CONST help[] =
{
  {"List colour resource classes",  "-help",      "", "-colours"},
#ifndef XMRED
  {"List high scores",              "-scores",    "", ""},
#endif /* XMRED */
  {"Swap colours",                  "-swap",      "ReverseVideo:yes", ""},
  {"Don't swap colours",            "+swap",    "ReverseVideo:no", ""},
  {"Force black and white",         "-mono",      "Mono:yes", ""},
  {"Classic sprites",               "-mris",      "Gender:he", ""},
  {"Modern sprites",                "-msit",      "Gender:she", ""},
#ifndef XMRED
  {"Random apples",                 "-random",    "Random:yes", ""},
  {"High score directory",          "-dir",       "Dir:", "<directory>"},
  {"Use username in score",         "-username",  "UserName:yes", ""},
  {"Use realname in score",         "-realname",  "UserName:no", ""},
#endif /* XMRED */
  {"Show colour allocation",        "-colours",   "", ""},
#ifndef XMRED
  {"Show all sprites",              "-sprites",   "", ""},
#endif /* XMRED */
  {"Private colour map",            "-private",   "Private:yes", ""},
  {"Visual class",                  "-visual",    "Visual:", "<class>"},
  {"Visual depth",                  "-depth",     "Depth:",  "<n>"},
  {"Distinct colour limit",         "-distinct",  "Distinct:", "<n>"},
#ifndef XMRED
  {"Do not use dynamic colors",     "-static",    "Static:yes", ""},
  {"User defined gardens",          "-gardens",   "Gardens:", "<file>"},
  {"Expire own scores",             "-expire",    "", "<date>"},
  {"Remove user's scores",          "-remove",    "", "<name>"},
  {"Do not open display",           "+display", "", ""},
  {"Store date format change",      "-format",    "", ""},
#ifndef TRANSPUTER
  {"Force busywaiting",             "-busywait",  "Busywait:yes", ""},
#endif /* TRANSPUTER */
#endif /* XMRED */
  {NULL}
};
/*}}}*/
/*{{{  extern SPRITE_DEF CONST icons[] =*/
SPRITE_DEF CONST icons[] =
{
#ifndef XMRED
  {(unsigned char *)mris_bits, 0, {NULL, NULL}, {mris_width, mris_height}},
  {(unsigned char *)msit_bits, 0, {NULL, NULL}, {msit_width, msit_height}},
#else
  {(unsigned char *)mred_bits, 0, {NULL, NULL}, {mred_width, mred_height}},
#endif /* XMRED */
};
/*}}}*/
/*{{{  extern SPRITE_DEF CONST she_nadger[] =*/
SPRITE_DEF CONST she_nadger[] =
{
  {sden_bits, sden_depth, {sden_noswap, sden_swap},
      {sden_width, sden_height}, {0, 0},
      SPRITE_DEN, BORDER_HALO_MASK},
  {splyrw1_bits, splyrw1_depth, {splyrw1_noswap, splyrw1_swap},
      {splyrw1_width, splyrw1_height}, {0, 0},
      SPRITE_PLAYER
#ifndef XMRED
		    + 6
#endif /* XMRED */
		       , BORDER_HALO_MASK},
#ifndef XMRED
  {solids_bits, solids_depth, {solids_noswap, solids_swap},
      {solids_width, solids_height}, {0, 0},
      SPRITE_MRIS + 1, BORDER_HALO_MASK},
  {shells_bits, shells_depth, {shells_noswap, shells_swap},
      {shells_width, shells_height}, {0, 0},
      SPRITE_MRIS + 5, BORDER_HALO_MASK},
  {solidt_bits, solidt_depth, {solidt_noswap, solidt_swap},
      {solidt_width, solidt_height}, {0, 0},
      SPRITE_MRIS + 3, BORDER_HALO_MASK},
  {shellt_bits, shellt_depth, {shellt_noswap, shellt_swap},
      {shellt_width, shellt_height}, {0, 0},
      SPRITE_MRIS + 7, BORDER_HALO_MASK},
  {snormw1_bits, snormw1_depth, {snormw1_noswap, snormw1_swap},
      {snormw1_width, snormw1_height}, {0, 0},
      SPRITE_NORMAL + 6, BORDER_HALO_MASK},
  {snormw2_bits, snormw2_depth, {snormw2_noswap, snormw2_swap},
      {snormw2_width, snormw2_height}, {0, 0},
      SPRITE_NORMAL + 7, BORDER_HALO_MASK},
  {snormf_bits, snormf_depth, {snormf_noswap, snormf_swap},
      {snormf_width, snormf_height}, {0, 0},
      SPRITE_SQUISHED + 1, BORDER_HALO_MASK},
  {smnchw1_bits, smnchw1_depth, {smnchw1_noswap, smnchw1_swap},
      {smnchw1_width, smnchw1_height}, {0, 0}, SPRITE_MUNCHER + 6, 0},
  {smnchw2_bits, smnchw2_depth, {smnchw2_noswap, smnchw2_swap},
      {smnchw2_width, smnchw2_height}, {0, 0}, SPRITE_MUNCHER + 7, 0},
  {smnchf_bits, smnchf_depth, {smnchf_noswap, smnchf_swap},
      {smnchf_width,  smnchf_height}, {0, 0},
      SPRITE_SQUISHED + 3, BORDER_HALO_MASK},
  {sxtraw1_bits, sxtraw1_depth, {sxtraw1_noswap, sxtraw1_swap},
      {sxtraw1_width, sxtraw1_height}, {0, 0},
      SPRITE_XTRA_SOURCE + 0, BORDER_HALO_MASK},
  {sxtraw2_bits, sxtraw2_depth, {sxtraw2_noswap, sxtraw2_swap},
      {sxtraw2_width, sxtraw2_height}, {0, 0},
      SPRITE_XTRA_SOURCE + 1, BORDER_HALO_MASK},
  {sxtraf_bits, sxtraf_depth, {sxtraf_noswap, sxtraf_swap},
      {sxtraf_width, sxtraf_height}, {0, 0},
      SPRITE_SQUISHED + 5, BORDER_HALO_MASK},
  {sdronw1_bits, sdronw1_depth, {sdronw1_noswap, sdronw1_swap},
      {sdronw1_width, sdronw1_height}, {0, 0},
      SPRITE_DRONE + 6, BORDER_HALO_MASK},
  {sdronw2_bits, sdronw2_depth, {sdronw2_noswap, sdronw2_swap},
      {sdronw2_width, sdronw2_height}, {0, 0},
      SPRITE_DRONE + 7, BORDER_HALO_MASK},
  {schmpo_bits, schmpo_depth, {schmpo_noswap, schmpo_swap},
      {schmpo_width,  schmpo_height}, {0, 0},
      SPRITE_CHOMP + 0, BORDER_HALO_MASK},
  {schmpc_bits, schmpc_depth, {schmpc_noswap, schmpc_swap},
      {schmpc_width,  schmpc_height}, {0, 0},
      SPRITE_CHOMP + 1, BORDER_HALO_MASK},
  {sdronf_bits, sdronf_depth, {sdronf_noswap, sdronf_swap},
      {sdronf_width, sdronf_height}, {0, 0},
      SPRITE_SQUISHED + 7, BORDER_HALO_MASK},
  {splyrw2_bits, splyrw2_depth, {splyrw2_noswap, splyrw2_swap},
      {splyrw2_width, splyrw2_height}, {0, 0},
      SPRITE_PLAYER + 7, BORDER_HALO_MASK},
  {splyrf_bits, splyrf_depth, {splyrf_noswap, splyrf_swap},
      {splyrf_width, splyrf_height}, {0, 0},
      SPRITE_SQUISHED + 9, BORDER_HALO_MASK},
  {splyrs1_bits, splyrs1_depth, {splyrs1_noswap, splyrs1_swap},
      {splyrs1_width, splyrs1_height}, {0, 0},
      SPRITE_PLAYER_PUSH + 2, BORDER_HALO_MASK},
  {splyrs2_bits, splyrs2_depth, {splyrs2_noswap, splyrs2_swap},
      {splyrs2_width, splyrs2_height}, {0, 0},
      SPRITE_PLAYER_PUSH + 3, BORDER_HALO_MASK},
  {splyrp1_bits, splyrp1_depth, {splyrp1_noswap, splyrp1_swap},
      {splyrp1_width, splyrp1_height}, {0, 0},
      SPRITE_PLAYER_REST + 2, BORDER_HALO_MASK},
  {splyrp2_bits, splyrp2_depth, {splyrp2_noswap, splyrp2_swap},
      {splyrp2_width, splyrp2_height}, {0, 0},
      SPRITE_PLAYER_REST + 3, BORDER_HALO_MASK},
  {splyrd1_bits, splyrd1_depth, {splyrd1_noswap, splyrd1_swap},
      {splyrd1_width, splyrd1_height}, {0, 0},
      SPRITE_PLAYER_DEAD + 0, BORDER_HALO_MASK},
  {splyrd2_bits, splyrd2_depth, {splyrd2_noswap, splyrd2_swap},
      {splyrd2_width, splyrd2_height}, {0, 0},
      SPRITE_PLAYER_DEAD + 1, BORDER_HALO_MASK},
  {NULL, SPRITE_PLAYER_DEAD + 10, {NULL, NULL}, {0, 0}, {0, 0},
      SPRITE_PLAYER_DEAD + 2, REFLECT_VERTICAL},
  {NULL, SPRITE_PLAYER_DEAD + 11, {NULL, NULL}, {0, 0}, {0, 0},
      SPRITE_PLAYER_DEAD + 3, REFLECT_VERTICAL},
  {NULL, SPRITE_PLAYER_DEAD + 12, {NULL, NULL}, {0, 0}, {0, 0},
      SPRITE_PLAYER_DEAD + 4, REFLECT_VERTICAL},
  {NULL, SPRITE_PLAYER_DEAD + 13, {NULL, NULL}, {0, 0}, {0, 0},
      SPRITE_PLAYER_DEAD + 5, REFLECT_VERTICAL},
  {NULL, SPRITE_PLAYER_DEAD + 10, {NULL, NULL}, {0, 0}, {0, 0},
      SPRITE_PLAYER_DEAD + 6, REFLECT_HORIZONTAL},
  {NULL, SPRITE_PLAYER_DEAD + 11, {NULL, NULL}, {0, 0}, {0, 0},
      SPRITE_PLAYER_DEAD + 7, REFLECT_HORIZONTAL},
  {NULL, SPRITE_PLAYER_DEAD + 0, {NULL, NULL}, {0, 0}, {0, 0},
      SPRITE_PLAYER_DEAD + 8, REFLECT_VERTICAL},
  {NULL, SPRITE_PLAYER_DEAD + 1, {NULL, NULL}, {0, 0}, {0, 0},
      SPRITE_PLAYER_DEAD + 9, REFLECT_VERTICAL},
  {NULL, SPRITE_PLAYER_DEAD + 0, {NULL, NULL}, {0, 0}, {0, 0},
      SPRITE_PLAYER_DEAD + 10, REFLECT_DIAGONAL},
  {NULL, SPRITE_PLAYER_DEAD + 1, {NULL, NULL}, {0, 0}, {0, 0},
      SPRITE_PLAYER_DEAD + 11, REFLECT_DIAGONAL},
  {NULL, SPRITE_PLAYER_DEAD + 0, {NULL, NULL}, {0, 0}, {0, 0},
      SPRITE_PLAYER_DEAD + 12, REFLECT_HORIZONTAL},
  {NULL, SPRITE_PLAYER_DEAD + 1, {NULL, NULL}, {0, 0}, {0, 0},
      SPRITE_PLAYER_DEAD + 13, REFLECT_HORIZONTAL},
  {NULL, SPRITE_PLAYER_DEAD + 2, {NULL, NULL}, {0, 0}, {0, 0},
      SPRITE_PLAYER_DEAD + 14, REFLECT_HORIZONTAL},
  {NULL, SPRITE_PLAYER_DEAD + 3, {NULL, NULL}, {0, 0}, {0, 0},
      SPRITE_PLAYER_DEAD + 15, REFLECT_HORIZONTAL},
  {splyrh1_bits, splyrh1_depth, {splyrh1_noswap, splyrh1_swap},
      {splyrh1_width, splyrh1_height}, {0, 0},
      SPRITE_PLAYER_HAPPY + 0, BORDER_HALO_MASK},
  {splyrh2_bits, splyrh2_depth, {splyrh2_noswap, splyrh2_swap},
      {splyrh2_width, splyrh2_height}, {0, 0},
      SPRITE_PLAYER_HAPPY + 1, BORDER_HALO_MASK},
#endif /* XMRED */
  {NULL}
};
/*}}}*/
/*{{{  extern SPRITE_DEF sprites_def[SPRITES] =*/
SPRITE_DEF sprites_def[SPRITES] =
{
  /*{{{  2:center masks (oblong, rect)*/
  {(unsigned char *)oblong_bits, 1, {NULL},
      {oblong_width, oblong_height}, {CELL_WIDTH, CELL_HEIGHT}},
  {(unsigned char *)rect_bits, 1, {NULL},
      {rect_width, rect_height}, {CELL_WIDTH, CELL_HEIGHT}},
  /*}}}*/
  /*{{{  2:edge masks (lr, tb)*/
  {(unsigned char *)edge_bits, 1, {NULL},
      {edge_width, edge_height}, {GAP_WIDTH * 8, EDGE_HEIGHT}},
  {NULL, 0, {NULL}, {0, 0}, {EDGE_WIDTH, GAP_HEIGHT * 8},
      SPRITE_EDGE_BASE, REFLECT_DIAGONAL},
  /*}}}*/
#ifndef XMRED
  /*{{{  1:digits (0,1,2,3,4,5,6,7,8,9,' ')*/
  {digits_bits, digits_depth, {digits_noswap, digits_swap},
      {digits_width, digits_height}, {DIGIT_WIDTH * 11, DIGIT_HEIGHT},
      0, 0},
  /*}}}*/
#else
  /*{{{  1:path*/
  {path_bits, path_depth, {path_noswap, path_swap},
      {path_width, path_height}, {CELL_WIDTH, CELL_HEIGHT}, 0, 0},
  /*}}}*/
#endif /* XMRED */
  /*{{{  1:den*/
  {hden_bits, hden_depth, {hden_noswap,  hden_swap},
      {hden_width, hden_height}, {CELL_WIDTH, CELL_HEIGHT},
      0, BORDER_HALO_MASK},
  /*}}}*/
#ifndef XMRED
  /*{{{  1:ball*/
  {ball_bits, ball_depth, {ball_noswap, ball_swap},
      {ball_width, ball_height}, {BALL_WIDTH, BALL_HEIGHT},
      0, 0},
  /*}}}*/
#endif /* XMRED */
  /*{{{  1:cherry*/
  {cherry_bits, cherry_depth, {cherry_noswap, cherry_swap},
      {cherry_width, cherry_height}, {CELL_WIDTH, CELL_HEIGHT},
      0, BORDER_HALO_MASK | BORDER_MONOSWAP_EDGE_MASK},
  /*}}}*/
#ifndef XMRED
  /*{{{  6:apples (normal, rock, apple, split, decay, rot)*/
  {aplnorm_bits, aplnorm_depth, {aplnorm_noswap, aplnorm_swap},
      {aplnorm_width, aplnorm_height}, {CELL_WIDTH, CELL_HEIGHT},
      0, 0},
  {aplrock_bits, aplrock_depth, {aplrock_noswap, aplrock_swap},
      {aplrock_width, aplrock_height}, {CELL_WIDTH, CELL_HEIGHT},
      0, 0},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_APPLE, REFLECT_ALIAS},
  {aplspl_bits,  aplspl_depth, {aplspl_noswap, aplspl_swap},
      {aplspl_width,  aplspl_height},  {CELL_WIDTH, CELL_HEIGHT},
      0, 0},
  {apldcy_bits,  apldcy_depth, {apldcy_noswap, apldcy_swap},
      {apldcy_width,  apldcy_height},  {DECAY_WIDTH, DECAY_HEIGHT},
      0, 0},
  {aplrot_bits,  aplrot_depth, {aplrot_noswap, aplrot_swap},
      {aplrot_width,  aplrot_height},  {ROT_WIDTH, ROT_HEIGHT},
      0, 0},
  /*}}}*/
  /*{{{  3:apple ghosts*/
  {gstnorm_bits, gstnorm_depth, {gstnorm_noswap, gstnorm_swap},
      {gstnorm_width, gstnorm_height}, {CELL_WIDTH, CELL_HEIGHT},
      0, 0},
  {gstrock_bits, gstrock_depth, {gstrock_noswap, gstrock_swap},
      {gstrock_width, gstrock_height}, {CELL_WIDTH, CELL_HEIGHT},
      0, 0},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_GHOST, REFLECT_ALIAS},
  /*}}}*/
  /*{{{  3:diamond*/
  {gem1_bits, gem1_depth, {gem1_noswap, gem1_swap},
      {gem1_width, gem1_height}, {CELL_WIDTH, CELL_HEIGHT},
      0, BORDER_HALO_MASK},
  {gem2_bits, gem2_depth, {gem2_noswap, gem2_swap},
      {gem2_width, gem2_height}, {CELL_WIDTH, CELL_HEIGHT},
      0, BORDER_HALO_MASK},
  {gem3_bits, gem3_depth, {gem3_noswap, gem3_swap},
      {gem3_width, gem3_height}, {CELL_WIDTH, CELL_HEIGHT},
      0, BORDER_HALO_MASK},
  /*}}}*/
#else
  /*{{{  2:big apples (normal, edge)*/
  {aplnorm_bits, aplnorm_depth, {aplnorm_noswap, aplnorm_swap},
      {aplnorm_width, aplnorm_height}, {CELL_WIDTH, CELL_HEIGHT},
      0, 0},
  {bigedge_bits, bigedge_depth, {bigedge_noswap, bigedge_swap},
      {bigedge_width, bigedge_height}, {CELL_WIDTH, CELL_HEIGHT},
      0, 0},
  /*}}}*/
  /*{{{  2:big arrows (solid, edge)*/
  {bigsarw_bits, bigsarw_depth, {bigsarw_noswap, bigsarw_swap},
      {bigsarw_width, bigsarw_height}, {CELL_WIDTH, CELL_HEIGHT},
      0, 0},
  {bigearw_bits, bigearw_depth, {bigearw_noswap, bigearw_swap},
      {bigearw_width, bigearw_height}, {CELL_WIDTH, CELL_HEIGHT},
      0, 0},
  /*}}}*/
  /*{{{  2:small apples (normal, edge)*/
  {aplsmall_bits, aplsmall_depth, {aplsmall_noswap, aplsmall_swap},
      {aplsmall_width, aplsmall_height}, {CELL_WIDTH / 2, CELL_HEIGHT / 2},
      0, 0},
  {smledge_bits, smledge_depth, {smledge_noswap, smledge_swap},
      {smledge_width, smledge_height}, {CELL_WIDTH / 2, CELL_HEIGHT / 2},
      0, 0},
  /*}}}*/
  /*{{{  2:small arrows (solid, edge)*/
  {smlsarw_bits, smlsarw_depth, {smlsarw_noswap, smlsarw_swap},
      {smlsarw_width, smlsarw_height}, {CELL_WIDTH / 2, CELL_HEIGHT / 2},
      0, 0},
  {smlearw_bits, smlearw_depth, {smlearw_noswap, smlearw_swap},
      {smlearw_width, smlearw_height}, {CELL_WIDTH / 2, CELL_HEIGHT / 2},
      0, 0},
  /*}}}*/
#endif /* XMRED */
  /*{{{  1:extra bold*/
  {xtrbold_bits,  xtrbold_depth, {xtrbold_noswap, xtrbold_swap},
      {xtrbold_width,  xtrbold_height},
      {CELL_WIDTH / 2 * 5, CELL_HEIGHT / 2}, 0, 0},
  /*}}}*/
#ifndef XMRED
  /*{{{  1:extra faint*/
  {xtrfaint_bits, xtrfaint_depth, {xtrfaint_noswap, xtrfaint_swap},
      {xtrfaint_width, xtrfaint_height},
      {CELL_WIDTH / 2 * 5, CELL_HEIGHT / 2}, 0, 0},
  /*}}}*/
  /*{{{  2:xtra source*/
  {hxtraw1_bits, hxtraw1_depth, {hxtraw1_noswap, hxtraw1_swap},
      {hxtraw1_width, hxtraw1_height}, {CELL_WIDTH, CELL_HEIGHT},
      0, BORDER_HALO_MASK},
  {hxtraw2_bits, hxtraw2_depth, {hxtraw2_noswap, hxtraw2_swap},
      {hxtraw2_width, hxtraw2_height}, {CELL_WIDTH, CELL_HEIGHT},
      0, BORDER_HALO_MASK},
  /*}}}*/
  /*{{{  12:normal (ul, dr, l, r, ur, dl)*/
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_NORMAL + 8, REFLECT_VERTICAL},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_NORMAL + 9, REFLECT_VERTICAL},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_NORMAL + 6, REFLECT_DIAGONAL},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_NORMAL + 7, REFLECT_DIAGONAL},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_NORMAL + 6, REFLECT_VERTICAL},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_NORMAL + 7, REFLECT_VERTICAL},
  {hnormw1_bits, hnormw1_depth, {hnormw1_noswap, hnormw1_swap},
      {hnormw1_width, hnormw1_height}, {CELL_WIDTH, CELL_HEIGHT},
      0, BORDER_HALO_MASK},
  {hnormw2_bits, hnormw2_depth, {hnormw2_noswap, hnormw2_swap},
      {hnormw2_width, hnormw2_height}, {CELL_WIDTH, CELL_HEIGHT},
      0, BORDER_HALO_MASK},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_NORMAL + 2, REFLECT_HORIZONTAL},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_NORMAL + 3, REFLECT_HORIZONTAL},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_NORMAL + 2, REFLECT_VERTICAL},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_NORMAL + 3, REFLECT_VERTICAL},
  /*}}}*/
  /*{{{  12:muncher (ul, dr, l, r, ur, dl)*/
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_MUNCHER + 8, REFLECT_VERTICAL},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_MUNCHER + 9, REFLECT_VERTICAL},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_MUNCHER + 6, REFLECT_DIAGONAL},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_MUNCHER + 7, REFLECT_DIAGONAL},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_MUNCHER + 6, REFLECT_VERTICAL},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_MUNCHER + 7, REFLECT_VERTICAL},
  {hmnchw1_bits, hmnchw1_depth, {hmnchw1_noswap, hmnchw1_swap},
      {hmnchw1_width, hmnchw1_height}, {CELL_WIDTH, CELL_HEIGHT},
      0, BORDER_HALO_MASK},
  {hmnchw2_bits, hmnchw2_depth, {hmnchw2_noswap, hmnchw2_swap},
      {hmnchw2_width, hmnchw2_height}, {CELL_WIDTH, CELL_HEIGHT},
      0, BORDER_HALO_MASK},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_MUNCHER + 2, REFLECT_HORIZONTAL},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_MUNCHER + 3, REFLECT_HORIZONTAL},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_MUNCHER + 2, REFLECT_VERTICAL},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_MUNCHER + 3, REFLECT_VERTICAL},
  /*}}}*/
  /*{{{  12:xtra (ul, dr, l, r, ur, dl)*/
  {hxtraw1_bits, hxtraw1_depth, {hxtraw1_noswap, hxtraw1_swap},
      {hxtraw1_width, hxtraw1_height}, {CELL_WIDTH, CELL_HEIGHT},
      0, BORDER_HALO_MASK},
  {hxtraw2_bits, hxtraw2_depth, {hxtraw2_noswap, hxtraw2_swap},
      {hxtraw2_width, hxtraw2_height}, {CELL_WIDTH, CELL_HEIGHT},
      0, BORDER_HALO_MASK},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_XTRA + 0, REFLECT_ALIAS},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_XTRA + 1, REFLECT_ALIAS},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_XTRA + 0, REFLECT_ALIAS},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_XTRA + 1, REFLECT_ALIAS},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_XTRA + 0, REFLECT_ALIAS},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_XTRA + 1, REFLECT_ALIAS},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_XTRA + 0, REFLECT_ALIAS},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_XTRA + 1, REFLECT_ALIAS},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_XTRA + 0, REFLECT_ALIAS},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_XTRA + 1, REFLECT_ALIAS},
  /*}}}*/
  /*{{{  12:drone (ul, dr, l, r, ur, dl)*/
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_DRONE + 8, REFLECT_VERTICAL},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_DRONE + 9, REFLECT_VERTICAL},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_DRONE + 6, REFLECT_DIAGONAL},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_DRONE + 7, REFLECT_DIAGONAL},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_DRONE + 6, REFLECT_VERTICAL},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_DRONE + 7, REFLECT_VERTICAL},
  {hdronw1_bits, hdronw1_depth, {hdronw1_noswap, hdronw1_swap},
      {hdronw1_width, hdronw1_height}, {CELL_WIDTH, CELL_HEIGHT},
      0, BORDER_HALO_MASK},
  {hdronw2_bits, hdronw2_depth, {hdronw2_noswap, hdronw2_swap},
      {hdronw2_width, hdronw2_height}, {CELL_WIDTH, CELL_HEIGHT},
      0, BORDER_HALO_MASK},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_DRONE + 2, REFLECT_HORIZONTAL},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_DRONE + 3, REFLECT_HORIZONTAL},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_DRONE + 2, REFLECT_VERTICAL},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_DRONE + 3, REFLECT_VERTICAL},
  /*}}}*/
  /*{{{  6:player (ul, dr, l)*/
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_PLAYER + 8, REFLECT_VERTICAL},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_PLAYER + 9, REFLECT_VERTICAL},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_PLAYER + 6, REFLECT_DIAGONAL},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_PLAYER + 7, REFLECT_DIAGONAL},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_PLAYER + 6, REFLECT_VERTICAL},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_PLAYER + 7, REFLECT_VERTICAL},
  /*}}}*/
#endif /* XMRED */
  {hplyrw1_bits, hplyrw1_depth, {hplyrw1_noswap, hplyrw1_swap},
      {hplyrw1_width, hplyrw1_height}, {CELL_WIDTH, CELL_HEIGHT},
      0, BORDER_HALO_MASK},
#ifndef XMRED
  {hplyrw2_bits, hplyrw2_depth, {hplyrw2_noswap, hplyrw2_swap},
      {hplyrw2_width, hplyrw2_height}, {CELL_WIDTH, CELL_HEIGHT},
      0, BORDER_HALO_MASK},
  /*{{{  4:player (ur, dl)*/
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_PLAYER + 2, REFLECT_HORIZONTAL},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_PLAYER + 3, REFLECT_HORIZONTAL},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_PLAYER + 2, REFLECT_VERTICAL},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_PLAYER + 3, REFLECT_VERTICAL},
  /*}}}*/
  /*{{{  4:player push (l, r)*/
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_PLAYER_PUSH + 2, REFLECT_VERTICAL},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_PLAYER_PUSH + 3, REFLECT_VERTICAL},
  {hplyrs1_bits, hplyrs1_depth, {hplyrs1_noswap, hplyrs1_swap},
      {hplyrs1_width, hplyrs1_height}, {CELL_WIDTH, CELL_HEIGHT},
      0, BORDER_HALO_MASK},
  {hplyrs2_bits, hplyrs2_depth, {hplyrs2_noswap, hplyrs2_swap},
      {hplyrs2_width, hplyrs2_height}, {CELL_WIDTH, CELL_HEIGHT},
      0, BORDER_HALO_MASK},
  /*}}}*/
  /*{{{  4:player rest (l, r)*/
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_PLAYER_REST + 2, REFLECT_VERTICAL},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_PLAYER_REST + 3, REFLECT_VERTICAL},
  {hplyrp1_bits, hplyrp1_depth, {hplyrp1_noswap, hplyrp1_swap},
      {hplyrp1_width, hplyrp1_height}, {CELL_WIDTH, CELL_HEIGHT},
      0, BORDER_HALO_MASK},
  {hplyrp2_bits, hplyrp2_depth, {hplyrp2_noswap, hplyrp2_swap},
      {hplyrp2_width, hplyrp2_height}, {CELL_WIDTH, CELL_HEIGHT},
      0, BORDER_HALO_MASK},
  /*}}}*/
  /*{{{  2:player happy*/
  {hplyrh1_bits, hplyrh1_depth, {hplyrh1_noswap, hplyrh1_swap},
      {hplyrh1_width, hplyrh1_height}, {CELL_WIDTH, CELL_HEIGHT},
      0, BORDER_HALO_MASK},
  {hplyrh2_bits, hplyrh2_depth, {hplyrh2_noswap, hplyrh2_swap},
      {hplyrh2_width, hplyrh2_height}, {CELL_WIDTH, CELL_HEIGHT},
      0, BORDER_HALO_MASK},
  /*}}}*/
  /*{{{  16:player dead*/
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_PLAYER + 6, REFLECT_ALIAS},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_PLAYER_DEAD + 0, REFLECT_ALIAS},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_PLAYER + 10, REFLECT_ALIAS},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_PLAYER_DEAD + 2, REFLECT_ALIAS},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_PLAYER + 4, REFLECT_HORIZONTAL},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_PLAYER_DEAD + 4, REFLECT_ALIAS},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_PLAYER + 8, REFLECT_ALIAS},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_PLAYER_DEAD + 6, REFLECT_ALIAS},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_PLAYER + 4, REFLECT_ALIAS},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_PLAYER_DEAD + 8, REFLECT_ALIAS},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_PLAYER + 2, REFLECT_ALIAS},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_PLAYER_DEAD + 10, REFLECT_ALIAS},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_PLAYER + 6, REFLECT_HORIZONTAL},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_PLAYER_DEAD + 12, REFLECT_ALIAS},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_PLAYER + 0, REFLECT_ALIAS},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_PLAYER_DEAD + 14, REFLECT_ALIAS},
  /*}}}*/
  /*{{{  10:squished*/
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_SQUISHED + 1, REFLECT_VERTICAL},
  {hnormf_bits, hnormf_depth, {hnormf_noswap, hnormf_swap},
      {hnormf_width, hnormf_height}, {CELL_WIDTH, CELL_HEIGHT / 4},
      0, BORDER_HALO_MASK},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_SQUISHED + 3, REFLECT_VERTICAL},
  {hmnchf_bits, hmnchf_depth, {hmnchf_noswap, hmnchf_swap},
      {hmnchf_width, hmnchf_height}, {CELL_WIDTH, CELL_HEIGHT / 4},
      0, BORDER_HALO_MASK},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_SQUISHED + 5, REFLECT_VERTICAL},
  {hxtraf_bits, hxtraf_depth, {hxtraf_noswap, hxtraf_swap},
      {hxtraf_width, hxtraf_height}, {CELL_WIDTH, CELL_HEIGHT / 4},
      0, BORDER_HALO_MASK},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_SQUISHED + 7, REFLECT_VERTICAL},
  {hdronf_bits, hdronf_depth, {hdronf_noswap, hdronf_swap},
      {hdronf_width, hdronf_height}, {CELL_WIDTH, CELL_HEIGHT / 4},
      0, BORDER_HALO_MASK},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_SQUISHED + 9, REFLECT_VERTICAL},
  {hplyrf_bits, hplyrf_depth, {hplyrf_noswap, hplyrf_swap},
      {hplyrf_width, hplyrf_height}, {CELL_WIDTH, CELL_HEIGHT / 4},
      0, BORDER_HALO_MASK},
  /*}}}*/
  /*{{{  2:chomp (open, closed)*/
  {hchmpo_bits, hchmpo_depth, {hchmpo_noswap, hchmpo_swap},
      {hchmpo_width, hchmpo_height}, {CELL_WIDTH, CELL_HEIGHT},
      0, BORDER_HALO_MASK},
  {hchmpc_bits, hchmpc_depth, {hchmpc_noswap, hchmpc_swap},
      {hchmpc_width, hchmpc_height}, {CELL_WIDTH, CELL_HEIGHT},
      0, BORDER_HALO_MASK},
  /*}}}*/
  /*{{{  8:m r i s*/
  {solidm_bits, solidm_depth, {solidm_noswap, solidm_swap},
      {solidm_width, solidm_height}, {CELL_WIDTH, CELL_HEIGHT},
      0, BORDER_HALO_MASK},
  {solidr_bits, solidr_depth, {solidr_noswap, solidr_swap},
      {solidr_width, solidr_height}, {CELL_WIDTH, CELL_HEIGHT},
      0, BORDER_HALO_MASK},
  {solidi_bits, solidi_depth, {solidi_noswap, solidi_swap},
      {solidi_width, solidi_height}, {CELL_WIDTH, CELL_HEIGHT},
      0, BORDER_HALO_MASK},
  {solids_bits, solids_depth, {solids_noswap, solids_swap},
      {solids_width, solids_height}, {CELL_WIDTH, CELL_HEIGHT},
      0, BORDER_HALO_MASK},
  {shellm_bits, shellm_depth, {shellm_noswap, shellm_swap},
      {shellm_width, shellm_height}, {CELL_WIDTH, CELL_HEIGHT},
      0, BORDER_HALO_MASK},
  {shellr_bits, shellr_depth, {shellr_noswap, shellr_swap},
      {shellr_width, shellr_height}, {CELL_WIDTH, CELL_HEIGHT},
      0, BORDER_HALO_MASK},
  {shelli_bits, shelli_depth, {shelli_noswap, shelli_swap},
      {shelli_width, shelli_height}, {CELL_WIDTH, CELL_HEIGHT},
      0, BORDER_HALO_MASK},
  {shells_bits, shells_depth, {shells_noswap, shells_swap},
      {shells_width, shells_height}, {CELL_WIDTH, CELL_HEIGHT},
      0, BORDER_HALO_MASK},
  /*}}}*/
  /*{{{  1:seat*/
  {seat_bits, seat_depth, {seat_noswap, seat_swap},
      {seat_width, seat_height}, {CELL_WIDTH, 0}, 0, BORDER_HALO_MASK},
  /*}}}*/
  /*{{{  1:ghosting*/
  {(unsigned char *)ghosting_bits, 1, {NULL},
      {ghosting_width, ghosting_height}, {GHOSTING_WIDTH, GHOSTING_HEIGHT}},
  /*}}}*/
  /*{{{  5:prizes*/
  {cake_bits, cake_depth, {cake_noswap, cake_swap},
      {cake_width, cake_height}, {CELL_WIDTH, CELL_HEIGHT},
      0, BORDER_HALO_MASK | BORDER_MONOSWAP_EDGE_MASK},
  {spanner_bits, spanner_depth, {spanner_noswap, spanner_swap},
      {spanner_width, spanner_height}, {CELL_WIDTH, CELL_HEIGHT},
      0, BORDER_HALO_MASK | BORDER_MONOSWAP_EDGE_MASK},
  {brolly_bits, brolly_depth, {brolly_noswap, brolly_swap},
      {brolly_width, brolly_height}, {CELL_WIDTH, CELL_HEIGHT},
      0, BORDER_HALO_MASK | BORDER_MONOSWAP_EDGE_MASK},
  {mushrm_bits, mushrm_depth, {mushrm_noswap, mushrm_swap},
      {mushrm_width, mushrm_height}, {CELL_WIDTH, CELL_HEIGHT},
      0, BORDER_HALO_MASK | BORDER_MONOSWAP_EDGE_MASK},
  {clock_bits, clock_depth, {clock_noswap, clock_swap},
      {clock_width, clock_height}, {CELL_WIDTH, CELL_HEIGHT},
      0, BORDER_HALO_MASK | BORDER_MONOSWAP_EDGE_MASK},
  /*}}}*/
#else
  /*{{{  3:red, green, blue*/
  {red_bits, red_depth, {red_noswap, red_swap},
      {red_width, red_height}, {CELL_WIDTH, CELL_HEIGHT},
      0, BORDER_WHITE_EDGE_MASK | BORDER_HALO_MASK},
  {green_bits, green_depth, {green_noswap, green_swap},
      {green_width, green_height}, {CELL_WIDTH, CELL_HEIGHT},
      0, BORDER_WHITE_EDGE_MASK | BORDER_HALO_MASK},
  {blue_bits, blue_depth, {blue_noswap, blue_swap},
      {blue_width, blue_height}, {CELL_WIDTH, CELL_HEIGHT},
      0, BORDER_WHITE_EDGE_MASK | BORDER_HALO_MASK},
  /*}}}*/
  /*{{{  5:apples*/
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_CHERRY, REFLECT_HORIZONTAL},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_CHERRY, REFLECT_HORIZONTAL},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_CHERRY, REFLECT_HORIZONTAL},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_CHERRY, REFLECT_HORIZONTAL},
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_CHERRY, REFLECT_HORIZONTAL},
  /*}}}*/
  /*{{{  1:apple drop*/
  {NULL, 0, {NULL}, {0, 0}, {0, 0}, SPRITE_BIG_APPLE, REFLECT_HORIZONTAL},
  /*}}}*/
  /*{{{  1:include*/
  {include_bits, include_depth, {include_noswap, include_swap},
      {include_width, include_height}, {ICON_WIDTH, ICON_HEIGHT}, 0, 0},
  /*}}}*/
  /*{{{  2:iconi, boardi*/
  {iconi_bits, iconi_depth, {iconi_noswap, iconi_swap},
      {iconi_width, iconi_height}, {CELL_WIDTH, CELL_HEIGHT}, 0, 0},
  {boardi_bits, boardi_depth, {boardi_noswap, boardi_swap},
      {boardi_width, boardi_height}, {ICON_WIDTH, ICON_HEIGHT}, 0, 0},
  /*}}}*/
  /*{{{  2:flagoff, flagon*/
  {warnoff_bits, warnoff_depth, {warnoff_noswap, warnoff_swap},
      {warnoff_width, warnoff_height}, {WARNING_WIDTH, WARNING_HEIGHT}, 0, 0},
  {warnon_bits, warnon_depth, {warnon_noswap, warnon_swap},
      {warnon_width, warnon_height}, {WARNING_WIDTH, WARNING_HEIGHT}, 0, 0},
  /*}}}*/
#endif /* XMRED */
};
/*}}}*/
/*{{{  extern SPRITE_DEF fills_def[FILLS] =*/
SPRITE_DEF fills_def[FILLS] =
{
  {(unsigned char *)fill0_bits, 0, {NULL}, {fill0_width, fill0_height}},
  {(unsigned char *)fill1_bits, 0, {NULL}, {fill1_width, fill1_height}},
  {(unsigned char *)fill2_bits, 0, {NULL}, {fill2_width, fill2_height}},
  {(unsigned char *)fill3_bits, 0, {NULL}, {fill3_width, fill3_height}},
};
/*}}}*/
/*{{{  extern VISUAL_CLASS CONST visual_class[] =*/
VISUAL_CLASS CONST visual_class[] =
{
  {"PseudoColor", PseudoColor},
  {"StaticColor", StaticColor},
  {"GrayScale", GrayScale},
  {"StaticGray", StaticGray},
  {"DirectColor", DirectColor},
  {"TrueColor", TrueColor},
  {NULL}
};
/*}}}*/
/*}}}*/
/*{{{  public tables*/
/*{{{  unsigned backgrounds[][2] =*/
unsigned CONST backgrounds[][2] =
{
  {COLOR_RED_BACK, COLOR_RED_FORE},
  {COLOR_GREEN_BACK, COLOR_GREEN_FORE},
  {COLOR_BLUE_BACK, COLOR_BLUE_FORE},
#ifndef XMRED
  {COLOR_DRONE_BACK, COLOR_DRONE_FORE},
#endif /* XMRED */
};
/*}}}*/
/*}}}*/
