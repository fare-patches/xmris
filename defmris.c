/* Copyright (C) 1993, 1992 Nathan Sidwell */
/* RCS $Id: defmris.c,v 4.6 1995/12/14 13:53:27 nathan Exp $ */
#define EXTERN
#include "xmris.h"
#include "defmris.h"
#include "defcom.c"
/*{{{  unsigned char letter_mris[] =*/
unsigned char letter_mris[] =
{
  LETTERPOS(1, 3),
  LETTERPATH(0, 2), LETTERPATH(3, 1), LETTERPATH(1, 2), LETTERPATH(0, 2),
  LETTERPATH(3, 1), LETTERPATH(1, 2), LETTERPATH(3, 1), LETTERPATH(0, 1),
  LETTERPATH(3, 2), LETTERPATH(0, 1), LETTERPATH(2, 2), LETTERPATH(1, 1),
  LETTERPATH(3, 1), LETTERPATH(1, 1), LETTERPATH(3, 2), LETTERPATH(0, 2),
  LETTERPATH(1, 2), LETTERPATH(3, 3), LETTERPATH(0, 1), LETTERPATH(2, 2),
  LETTERPATH(0, 1), LETTERPATH(3, 2),
  LETTEREND
};
/*}}}*/
/*{{{  unsigned char letter_msit[] =*/
unsigned char letter_msit[] =
{
  LETTERPOS(1, 3),
  LETTERPATH(0, 2), LETTERPATH(3, 1), LETTERPATH(1, 2), LETTERPATH(0, 2),
  LETTERPATH(3, 1), LETTERPATH(1, 2), LETTERPATH(3, 3), LETTERPATH(0, 1),
  LETTERPATH(2, 2), LETTERPATH(0, 1), LETTERPATH(3, 3), LETTERPATH(1, 2),
  LETTERPATH(0, 2), LETTERPATH(3, 3), LETTERPATH(2, 1), LETTERPATH(1, 2),
  LETTEREND
};
/*}}}*/
/*{{{  static unsigned char letter_the[] =*/
static unsigned char letter_the[] =
{
  LETTERPOS(2, 7),
  LETTERPATH(0, 2), LETTERPATH(2, 1), LETTERPATH(3, 3), LETTERPATH(1, 2),
  LETTERPATH(0, 1), LETTERPATH(3, 2), LETTERPATH(0, 1), LETTERPATH(1, 2),
  LETTERPATH(3, 3), LETTERPATH(2, 2), LETTERPATH(0, 1), LETTERPATH(3, 2),
  LETTERPATH(2, 2), LETTERPATH(0, 1), LETTERPATH(3, 2),
  LETTEREND
};
/*}}}*/
/*{{{  static unsigned char letter_game[] =*/
static unsigned char letter_game[] =
{
  LETTERPOS(2, 9),
  LETTERPATH(2, 2), LETTERPATH(1, 2), LETTERPATH(3, 2), LETTERPATH(0, 1),
  LETTERPATH(2, 1), LETTERPATH(3, 1), LETTERPATH(1, 1), LETTERPATH(3, 1),
  LETTERPATH(0, 1), LETTERPATH(3, 2), LETTERPATH(0, 1), LETTERPATH(2, 2),
  LETTERPATH(1, 1), LETTERPATH(3, 2), LETTERPATH(1, 1), LETTERPATH(3, 1),
  LETTERPATH(0, 2), LETTERPATH(3, 1), LETTERPATH(1, 2), LETTERPATH(0, 2),
  LETTERPATH(3, 1), LETTERPATH(1, 2), LETTERPATH(3, 3), LETTERPATH(2, 2),
  LETTERPATH(0, 1), LETTERPATH(3, 2), LETTERPATH(2, 2), LETTERPATH(0, 1),
  LETTERPATH(3, 2),
  LETTEREND
};
/*}}}*/
/*{{{  extern COORD CONST she_hold[16] =*/
/*{{{  held ball offsets*/
#define SHE_RIGHT1_BALL_X   (splyrw1_x_hot - CELL_WIDTH / 2)
#define SHE_RIGHT1_BALL_Y   (splyrw1_y_hot - CELL_HEIGHT / 2)
#define SHE_UP1_BALL_X      (splyrw1_y_hot - CELL_WIDTH / 2)
#define SHE_UP1_BALL_Y      (splyrw1_x_hot - CELL_HEIGHT / 2)
#define SHE_RIGHT2_BALL_X   (splyrw2_x_hot - CELL_WIDTH / 2)
#define SHE_RIGHT2_BALL_Y   (splyrw2_y_hot - CELL_HEIGHT / 2)
#define SHE_UP2_BALL_X      (splyrw2_y_hot - CELL_WIDTH / 2)
#define SHE_UP2_BALL_Y      (splyrw2_x_hot - CELL_HEIGHT / 2)
#define SHE_PUSH1_BALL_X    (splyrs1_x_hot - CELL_WIDTH / 2)
#define SHE_PUSH1_BALL_Y    (splyrs1_y_hot - CELL_HEIGHT / 2)
#define SHE_PUSH2_BALL_X    (splyrs2_x_hot - CELL_WIDTH / 2)
#define SHE_PUSH2_BALL_Y    (splyrs2_y_hot - CELL_HEIGHT / 2)
/*}}}*/
COORD CONST she_hold[16] =
{
 {-SHE_UP1_BALL_X, -SHE_UP1_BALL_Y},
 {-SHE_UP2_BALL_X, -SHE_UP2_BALL_Y},
 { SHE_UP1_BALL_X,  SHE_UP1_BALL_Y},
 { SHE_UP2_BALL_X,  SHE_UP2_BALL_Y},
 {-SHE_RIGHT1_BALL_X, SHE_RIGHT1_BALL_Y},
 {-SHE_RIGHT2_BALL_X, SHE_RIGHT2_BALL_Y},
 { SHE_RIGHT1_BALL_X, SHE_RIGHT1_BALL_Y},
 { SHE_RIGHT2_BALL_X, SHE_RIGHT2_BALL_Y},
 { SHE_UP1_BALL_X, -SHE_UP1_BALL_Y},
 { SHE_UP2_BALL_X, -SHE_UP2_BALL_Y},
 {-SHE_UP1_BALL_X,  SHE_UP1_BALL_Y},
 {-SHE_UP2_BALL_X,  SHE_UP2_BALL_Y},
 {-SHE_PUSH1_BALL_X, SHE_PUSH1_BALL_Y},
 {-SHE_PUSH2_BALL_X, SHE_PUSH2_BALL_Y},
 { SHE_PUSH1_BALL_X, SHE_PUSH1_BALL_Y},
 { SHE_PUSH2_BALL_X, SHE_PUSH2_BALL_Y},
};
/*}}}*/
/*{{{  APPLE_SIZE CONST apple_sizes[7] =*/
APPLE_SIZE CONST apple_sizes[6] =
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
/*{{{  COORD ball_hold[16] =*/
/*{{{  held ball offsets*/
#define HE_RIGHT1_BALL_X   (hplyrw1_x_hot - CELL_WIDTH / 2)
#define HE_RIGHT1_BALL_Y   (hplyrw1_y_hot - CELL_HEIGHT / 2)
#define HE_UP1_BALL_X      (hplyrw1_y_hot - CELL_WIDTH / 2)
#define HE_UP1_BALL_Y      (hplyrw1_x_hot - CELL_HEIGHT / 2)
#define HE_RIGHT2_BALL_X   (hplyrw2_x_hot - CELL_WIDTH / 2)
#define HE_RIGHT2_BALL_Y   (hplyrw2_y_hot - CELL_HEIGHT / 2)
#define HE_UP2_BALL_X      (hplyrw2_y_hot - CELL_WIDTH / 2)
#define HE_UP2_BALL_Y      (hplyrw2_x_hot - CELL_HEIGHT / 2)
#define HE_PUSH1_BALL_X    (hplyrs1_x_hot - CELL_WIDTH / 2)
#define HE_PUSH1_BALL_Y    (hplyrs1_y_hot - CELL_HEIGHT / 2)
#define HE_PUSH2_BALL_X    (hplyrs2_x_hot - CELL_WIDTH / 2)
#define HE_PUSH2_BALL_Y    (hplyrs2_y_hot - CELL_HEIGHT / 2)
/*}}}*/
COORD ball_hold[16] =
{
 {-HE_UP1_BALL_X, -HE_UP1_BALL_Y},
 {-HE_UP2_BALL_X, -HE_UP2_BALL_Y},
 { HE_UP1_BALL_X,  HE_UP1_BALL_Y},
 { HE_UP2_BALL_X,  HE_UP2_BALL_Y},
 {-HE_RIGHT1_BALL_X, HE_RIGHT1_BALL_Y},
 {-HE_RIGHT2_BALL_X, HE_RIGHT2_BALL_Y},
 { HE_RIGHT1_BALL_X, HE_RIGHT1_BALL_Y},
 { HE_RIGHT2_BALL_X, HE_RIGHT2_BALL_Y},
 { HE_UP1_BALL_X, -HE_UP1_BALL_Y},
 { HE_UP2_BALL_X, -HE_UP2_BALL_Y},
 {-HE_UP1_BALL_X,  HE_UP1_BALL_Y},
 {-HE_UP2_BALL_X,  HE_UP2_BALL_Y},
 {-HE_PUSH1_BALL_X, HE_PUSH1_BALL_Y},
 {-HE_PUSH2_BALL_X, HE_PUSH2_BALL_Y},
 { HE_PUSH1_BALL_X, HE_PUSH1_BALL_Y},
 { HE_PUSH2_BALL_X, HE_PUSH2_BALL_Y},
};
/*}}}*/
/*{{{  COORD CONST ball_throw[8] =*/
COORD CONST ball_throw[8] =
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
unsigned CONST ball_dir[8] = {0, 1, 2, 1, 3, 2, 2, 1};
/*{{{  unsigned CONST ball_returns[BALL_RETURNS] =*/
#if BALL_RETURNS != 3
  #error BALL_RETURNS != 3
#endif /* BALL_RETURNS */
unsigned CONST ball_returns[BALL_RETURNS] =
  {FRAMES_PER_SECOND * 3 / 2, FRAMES_PER_SECOND * 7 / 2,
    FRAMES_PER_SECOND * 11 / 2};
/*}}}*/
/*{{{  unsigned CONST player_dies[8] =*/
unsigned CONST player_dies[8] =
  {
    3, 5, 8, 4,
    2, 1, 8, 0,
  };
/*}}}*/
/*{{{  unsigned CONST squish_scores[SQUISH_SCORES] =*/
#if SQUISH_SCORES != 7
  #error SQUISH_SCORES != 7
#endif /* SQUISH_SCORES */
unsigned CONST squish_scores[SQUISH_SCORES] =
{
  0,
  1000 / SCORE_ROUND,
  2000 / SCORE_ROUND,
  4000 / SCORE_ROUND,
  6000 / SCORE_ROUND,
  8000 / SCORE_ROUND,
  9900 / SCORE_ROUND
};
/*}}}*/
/*{{{  TITLE title_text[] =*/
TITLE title_text[] =
  {
#if __STDC__
    {XMRISVERSION " " DATE},
#else
    {"%s %s"},
#endif /* __STDC__ */
    {"Left - %s",         2},
    {"Right - %s",        3},
    {"Up - %s",           0},
    {"Down - %s",         1},
    {"Throw - %s",        KEY_THROW},
    {"Pause - %s",        KEY_PAUSE},
    {"Quit - %s",         KEY_QUIT},
    {"Iconize - %s",      KEY_ICONIZE},
    {"Keyboard - %s",     KEY_KEYBOARD},
    {"Press %s to start", KEY_THROW},
    {""},
    {COPYRIGHT},
    {"Sprites also by Stefan Gustavson"},
    {NULL}
  };
/*}}}*/
/*{{{  unsigned char CONST *lettering_game[] =*/
unsigned char CONST *lettering_game[] =
{
  letter_mris,
  letter_the,
  letter_game,
  NULL
};
/*}}}*/
unsigned char CONST **lettering = lettering_game;
