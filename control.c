/* Copyright (C) 1993, 1992 Nathan Sidwell */
/* RCS $Id: control.c,v 4.15 1993/12/10 11:52:23 nathan stable $ */
/*{{{  includes*/
#include "xmred.h"
#include <X11/StringDefs.h>
#include "Icon.h"
#include "Drag.h"
#include <X11/Xaw/Scrollbar.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Label.h>
#include <math.h>
/*}}}*/
#define XtROption "Option"
/*{{{  defines*/
/*{{{  control gizmos*/
#define GIZMO_CONTROL          0
#define GIZMO_TOTAL            1
#define GIZMO_COMMENT          2
#define GIZMO_DISPLAY          3
#define GIZMO_DISPLAY_TITLE    4
#define GIZMO_DISPLAY_BASE     5
#define GIZMO_DISPLAY_COMBINED (5 + MODE_COMBINED)
#define GIZMO_DISPLAY_SEPARATE (5 + MODE_SEPARATE)
#define GIZMO_DISPLAY_RANDOM   (5 + MODE_RANDOM)
#define GIZMO_CONTROL_OPTIONS  8
#define GIZMO_CONTROL_FILLS    9
#define GIZMO_CONTROL_COLORS  10
#define GIZMO_CONTROL_BUTTONS 11
#define GIZMO_OPTION_LABEL    12
#define GIZMO_FILL_LABEL      13
#define GIZMO_COLOR_LABEL     14
#define GIZMO_BUTTON_LABEL    15
#define GIZMO_TOTAL_LABEL     16
#define GIZMO_TOTAL_BOXES     17
#define GIZMO_TOTAL_ICONS     27
#define GIZMO_TOTAL_COUNTS    37
#define GIZMO_TOTAL_WARNINGS  47
#define GIZMO_OPTION_DRAG     57
#define GIZMO_BUTTON_DRAG     58
#define GIZMO_OPTIONS         59
#define GIZMO_FILLS           (GIZMO_OPTIONS + OPTIONS)
#define GIZMO_COLORS          (GIZMO_FILLS + FILLS)
#define GIZMO_BUTTONS         (GIZMO_COLORS + BACKGROUNDS)
#define GIZMOS                (GIZMO_BUTTONS + BUTTONS)
/*}}}*/
#define VALUE_LOCK    256
/*}}}*/
/*{{{  structs*/
/*{{{  typedef struct Icon*/
typedef struct Icon
{
  int       gizmo;    /* gizmo number */
  COORD     size;     /* size of pixmap */
  unsigned  sprite;   /* sprite source */
  unsigned  fill;     /* background fill pattern */
  unsigned  color;    /* background color */
  Pixmap    pixmap;   /* displayed pixmap */
} ICON;
/*}}}*/
/*}}}*/
/*{{{  tables*/
/*{{{  static Arg arg_apple[] =*/
static Arg arg_apple[] =
{
  {MredNcolumns, (XtArgVal)2},
  {MredNrows, (XtArgVal)2},
};
/*}}}*/
/*{{{  static Arg arg_notapple[] =*/
static Arg arg_notapple[] =
{
  {MredNcolumns, (XtArgVal)1},
  {MredNrows, (XtArgVal)1},
};
/*}}}*/
/*{{{  static Arg arg_nohighlight[] =*/
static Arg arg_nohighlight[] =
{
  {MredNhighlightThickness, (XtArgVal)0},
};
/*}}}*/
/*{{{  static Arg arg_nodrag[] =*/
static Arg arg_nodrag[] =
{
  {MredNdragSensitivity, (XtArgVal)0},
};
/*}}}*/
/*{{{  static Arg arg_font[] =*/
static Arg arg_count[] =
{
  {XtNfont, (XtArgVal)NULL},
};
/*}}}*/
/*{{{  static Arg arg_warning[] =*/
static Arg arg_warning[] =
{
  {XtNbitmap, (XtArgVal)NULL},
};
/*}}}*/
/*{{{  static GIZMO gizmos[GIZMOS] =*/
static GIZMO gizmos[GIZMOS] =
{
  {"controls", -1, &formWidgetClass},
  {"totals", -1, &formWidgetClass},
  {"comment", -1, &iconWidgetClass,
      arg_nodrag, XtNumber(arg_nodrag)},
  {"mode", -1, &formWidgetClass},
  {"label", GIZMO_DISPLAY, &labelWidgetClass},
  {"combined", GIZMO_DISPLAY, &iconWidgetClass,
      arg_nodrag, XtNumber(arg_nodrag)},
  {"separate", GIZMO_DISPLAY, &iconWidgetClass,
      arg_nodrag, XtNumber(arg_nodrag)},
  {"random", GIZMO_DISPLAY, &iconWidgetClass,
      arg_nodrag, XtNumber(arg_nodrag)},
  {"options", GIZMO_CONTROL, &formWidgetClass},
  {"fills", GIZMO_CONTROL, &formWidgetClass},
  {"colors", GIZMO_CONTROL, &formWidgetClass},
  {"buttons", GIZMO_CONTROL, &formWidgetClass},
  {"label", GIZMO_CONTROL_OPTIONS, &labelWidgetClass},
  {"label", GIZMO_CONTROL_BUTTONS, &labelWidgetClass},
  {"label", GIZMO_CONTROL_FILLS, &labelWidgetClass},
  {"label", GIZMO_CONTROL_COLORS, &labelWidgetClass},
  {"label", GIZMO_TOTAL, &labelWidgetClass},
  {"total0", GIZMO_TOTAL, &formWidgetClass},
  {"total1", GIZMO_TOTAL, &formWidgetClass},
  {"total2", GIZMO_TOTAL, &formWidgetClass},
  {"total3", GIZMO_TOTAL, &formWidgetClass},
  {"total4", GIZMO_TOTAL, &formWidgetClass},
  {"total5", GIZMO_TOTAL, &formWidgetClass},
  {"total6", GIZMO_TOTAL, &formWidgetClass},
  {"total7", GIZMO_TOTAL, &formWidgetClass},
  {"total8", GIZMO_TOTAL, &formWidgetClass},
  {"total9", GIZMO_TOTAL, &formWidgetClass},
  {"icon", GIZMO_TOTAL_BOXES + 0, &iconWidgetClass,
      arg_nohighlight, XtNumber(arg_nohighlight)},
  {"icon", GIZMO_TOTAL_BOXES + 1, &iconWidgetClass,
      arg_nohighlight, XtNumber(arg_nohighlight)},
  {"icon", GIZMO_TOTAL_BOXES + 2, &iconWidgetClass,
      arg_nohighlight, XtNumber(arg_nohighlight)},
  {"icon", GIZMO_TOTAL_BOXES + 3, &iconWidgetClass,
      arg_nohighlight, XtNumber(arg_nohighlight)},
  {"icon", GIZMO_TOTAL_BOXES + 4, &iconWidgetClass,
      arg_nohighlight, XtNumber(arg_nohighlight)},
  {"icon", GIZMO_TOTAL_BOXES + 5, &iconWidgetClass,
      arg_nohighlight, XtNumber(arg_nohighlight)},
  {"icon", GIZMO_TOTAL_BOXES + 6, &iconWidgetClass,
      arg_nohighlight, XtNumber(arg_nohighlight)},
  {"icon", GIZMO_TOTAL_BOXES + 7, &iconWidgetClass,
      arg_nohighlight, XtNumber(arg_nohighlight)},
  {"icon", GIZMO_TOTAL_BOXES + 8, &iconWidgetClass,
      arg_nohighlight, XtNumber(arg_nohighlight)},
  {"icon", GIZMO_TOTAL_BOXES + 9, &iconWidgetClass,
      arg_nohighlight, XtNumber(arg_nohighlight)},
  {"count", GIZMO_TOTAL_BOXES + 0, &labelWidgetClass,
      arg_count, XtNumber(arg_count)},
  {"count", GIZMO_TOTAL_BOXES + 1, &labelWidgetClass,
      arg_count, XtNumber(arg_count)},
  {"count", GIZMO_TOTAL_BOXES + 2, &labelWidgetClass,
      arg_count, XtNumber(arg_count)},
  {"count", GIZMO_TOTAL_BOXES + 3, &labelWidgetClass,
      arg_count, XtNumber(arg_count)},
  {"count", GIZMO_TOTAL_BOXES + 4, &labelWidgetClass,
      arg_count, XtNumber(arg_count)},
  {"count", GIZMO_TOTAL_BOXES + 5, &labelWidgetClass,
      arg_count, XtNumber(arg_count)},
  {"count", GIZMO_TOTAL_BOXES + 6, &labelWidgetClass,
      arg_count, XtNumber(arg_count)},
  {"count", GIZMO_TOTAL_BOXES + 7, &labelWidgetClass,
      arg_count, XtNumber(arg_count)},
  {"count", GIZMO_TOTAL_BOXES + 8, &labelWidgetClass,
      arg_count, XtNumber(arg_count)},
  {"count", GIZMO_TOTAL_BOXES + 9, &labelWidgetClass,
      arg_count, XtNumber(arg_count)},
  {"warning", GIZMO_TOTAL_BOXES + 0, &labelWidgetClass,
      arg_warning, XtNumber(arg_warning)},
  {"warning", GIZMO_TOTAL_BOXES + 1, &labelWidgetClass,
      arg_warning, XtNumber(arg_warning)},
  {"warning", GIZMO_TOTAL_BOXES + 2, &labelWidgetClass,
      arg_warning, XtNumber(arg_warning)},
  {"warning", GIZMO_TOTAL_BOXES + 3, &labelWidgetClass,
      arg_warning, XtNumber(arg_warning)},
  {"warning", GIZMO_TOTAL_BOXES + 4, &labelWidgetClass,
      arg_warning, XtNumber(arg_warning)},
  {"scrollbar", GIZMO_TOTAL_BOXES + 5, &scrollbarWidgetClass},
  {"warning", GIZMO_TOTAL_BOXES + 6, &labelWidgetClass,
      arg_warning, XtNumber(arg_warning)},
  {"warning", GIZMO_TOTAL_BOXES + 7, &labelWidgetClass,
      arg_warning, XtNumber(arg_warning)},
  {"warning", GIZMO_TOTAL_BOXES + 8, &labelWidgetClass,
      arg_warning, XtNumber(arg_warning)},
  {"warning", GIZMO_TOTAL_BOXES + 9, &labelWidgetClass,
      arg_warning, XtNumber(arg_warning)},
  {"drag", GIZMO_CONTROL_OPTIONS, &dragWidgetClass},
  {"drag", GIZMO_CONTROL_BUTTONS, &dragWidgetClass},
  {"apple", GIZMO_CONTROL_OPTIONS, &iconWidgetClass,
      arg_apple, XtNumber(arg_apple)},
  {"random", GIZMO_CONTROL_OPTIONS, &iconWidgetClass},
  {"cherry", GIZMO_CONTROL_OPTIONS, &iconWidgetClass},
  {"path", GIZMO_CONTROL_OPTIONS, &iconWidgetClass},
  {"player", GIZMO_CONTROL_OPTIONS, &iconWidgetClass},
  {"den", GIZMO_CONTROL_OPTIONS, &iconWidgetClass},
  {"fill0", GIZMO_CONTROL_FILLS, &iconWidgetClass,
      arg_nodrag, XtNumber(arg_nodrag)},
  {"fill1", GIZMO_CONTROL_FILLS, &iconWidgetClass,
      arg_nodrag, XtNumber(arg_nodrag)},
  {"fill2", GIZMO_CONTROL_FILLS, &iconWidgetClass,
      arg_nodrag, XtNumber(arg_nodrag)},
  {"fill3", GIZMO_CONTROL_FILLS, &iconWidgetClass,
      arg_nodrag, XtNumber(arg_nodrag)},
  {"color0", GIZMO_CONTROL_COLORS, &iconWidgetClass,
      arg_nodrag, XtNumber(arg_nodrag)}, 
  {"color1", GIZMO_CONTROL_COLORS, &iconWidgetClass,
      arg_nodrag, XtNumber(arg_nodrag)}, 
  {"color2", GIZMO_CONTROL_COLORS, &iconWidgetClass,
      arg_nodrag, XtNumber(arg_nodrag)}, 
  {"button1", GIZMO_CONTROL_BUTTONS, &iconWidgetClass},
  {"button2", GIZMO_CONTROL_BUTTONS, &iconWidgetClass},
  {"button3", GIZMO_CONTROL_BUTTONS, &iconWidgetClass},
  {"button4", GIZMO_CONTROL_BUTTONS, &iconWidgetClass},
  {"button5", GIZMO_CONTROL_BUTTONS, &iconWidgetClass},
};
/*}}}*/
/*{{{  static ICON icons[] =*/
static ICON icons[] =
{
  {GIZMO_DISPLAY_COMBINED, {CELL_WIDTH, CELL_HEIGHT}, SPRITE_APPLES + 4},
  {GIZMO_DISPLAY_SEPARATE, {CELL_WIDTH, CELL_HEIGHT}, SPRITE_APPLES},
  {GIZMO_DISPLAY_RANDOM, {CELL_WIDTH, CELL_HEIGHT}, SPRITE_BIG_APPLE + 1},
  {GIZMO_COMMENT, {CELL_WIDTH, CELL_HEIGHT}, SPRITE_ICON_I},
  {GIZMO_TOTAL_ICONS + COUNT_APPLES + 0,
      {CELL_WIDTH, CELL_HEIGHT}, SPRITE_APPLES + 0},
  {GIZMO_TOTAL_ICONS + COUNT_APPLES + 1,
      {CELL_WIDTH, CELL_HEIGHT}, SPRITE_APPLES + 1},
  {GIZMO_TOTAL_ICONS + COUNT_APPLES + 2,
      {CELL_WIDTH, CELL_HEIGHT}, SPRITE_APPLES + 2},
  {GIZMO_TOTAL_ICONS + COUNT_APPLES + 3,
      {CELL_WIDTH, CELL_HEIGHT}, SPRITE_APPLES + 3},
  {GIZMO_TOTAL_ICONS + COUNT_CHERRY, {CELL_WIDTH, CELL_HEIGHT}, SPRITE_CHERRY},
  {GIZMO_TOTAL_ICONS + COUNT_RANDOM,
      {CELL_WIDTH, CELL_HEIGHT}, SPRITE_BIG_APPLE},
  {GIZMO_TOTAL_ICONS + COUNT_SPACES,
      {CELL_WIDTH, CELL_HEIGHT}, SPRITE_BIG_APPLE + 1},
  {GIZMO_TOTAL_ICONS + COUNT_FALL,
      {CELL_WIDTH, CELL_HEIGHT}, SPRITE_APPLE_DROP},
  {GIZMO_TOTAL_ICONS + COUNT_PLAYER, {CELL_WIDTH, CELL_HEIGHT}, SPRITE_PLAYER},
  {GIZMO_TOTAL_ICONS + COUNT_DEN, {CELL_WIDTH, CELL_HEIGHT}, SPRITE_DEN},
  {GIZMO_OPTIONS + OPTION_APPLES, {CELL_WIDTH, CELL_HEIGHT}, SPRITE_APPLES},
  {GIZMO_OPTIONS + OPTION_RANDOM,
      {CELL_WIDTH, CELL_HEIGHT}, SPRITE_BIG_APPLE + 1},
  {GIZMO_OPTIONS + OPTION_CHERRY, {CELL_WIDTH, CELL_HEIGHT}, SPRITE_CHERRY},
  {GIZMO_OPTIONS + OPTION_PATH, {CELL_WIDTH, CELL_HEIGHT}, SPRITE_PATH},
  {GIZMO_OPTIONS + OPTION_PLAYER, {CELL_WIDTH, CELL_HEIGHT}, SPRITE_PLAYER},
  {GIZMO_OPTIONS + OPTION_DEN, {CELL_WIDTH, CELL_HEIGHT}, SPRITE_DEN},
  {GIZMO_FILLS + 0, {CELL_WIDTH, CELL_HEIGHT}, 0, VALUE_LOCK | 0},
  {GIZMO_FILLS + 1, {CELL_WIDTH, CELL_HEIGHT}, 0, VALUE_LOCK | 1},
  {GIZMO_FILLS + 2, {CELL_WIDTH, CELL_HEIGHT}, 0, VALUE_LOCK | 2},
  {GIZMO_FILLS + 3, {CELL_WIDTH, CELL_HEIGHT}, 0, VALUE_LOCK | 3},
  {GIZMO_COLORS + 0, {CELL_WIDTH, CELL_HEIGHT}, SPRITE_COLORS + 0,
      0, VALUE_LOCK | 0},
  {GIZMO_COLORS + 1, {CELL_WIDTH, CELL_HEIGHT}, SPRITE_COLORS + 1,
      0, VALUE_LOCK | 1},
  {GIZMO_COLORS + 2, {CELL_WIDTH, CELL_HEIGHT}, SPRITE_COLORS + 2,
      0, VALUE_LOCK | 2},
  {GIZMO_BUTTONS + 0, {CELL_WIDTH, CELL_HEIGHT}, 0},
  {GIZMO_BUTTONS + 1, {CELL_WIDTH, CELL_HEIGHT}, 0},
  {GIZMO_BUTTONS + 2, {CELL_WIDTH, CELL_HEIGHT}, 0},
  {GIZMO_BUTTONS + 3, {CELL_WIDTH, CELL_HEIGHT}, 0},
  {GIZMO_BUTTONS + 4, {CELL_WIDTH, CELL_HEIGHT}, 0},
};
/*}}}*/
static char count_text[COUNTS][4];
/*}}}*/
/*{{{  prototypes*/
static VOIDFUNC apple_jump PROTOARG((Widget, XtPointer, XtPointer));
static VOIDFUNC apple_scroll PROTOARG((Widget, XtPointer, XtPointer));
static VOIDFUNC apple_set_thumb PROTOARG((VOIDARG));
static VOIDFUNC bind_button PROTOARG((unsigned, int));
static VOIDFUNC command_button PROTOARG((Widget, XtPointer, XtPointer));
static VOIDFUNC command_color PROTOARG((Widget, XtPointer, XtPointer));
static VOIDFUNC command_fill PROTOARG((Widget, XtPointer, XtPointer));
static VOIDFUNC command_mode PROTOARG((Widget, XtPointer, XtPointer));
static VOIDFUNC command_option PROTOARG((Widget, XtPointer, XtPointer));
static Boolean convert_string2option PROTOARG((Display *, XrmValue *,
    Cardinal *, XrmValue *, XrmValue *, XtPointer *));
static VOIDFUNC drag_button PROTOARG((Widget, XtPointer, XtPointer));
static VOIDFUNC drag_option PROTOARG((Widget, XtPointer, XtPointer));
static VOIDFUNC garden_comment PROTOARG((Widget, XtPointer, XtPointer));
static VOIDFUNC generate_icon PROTOARG((ICON *));
static ICON *gizmo2icon PROTOARG((GIZMO *));
static unsigned select_apple PROTOARG((unsigned));
static VOIDFUNC set_count_flag PROTOARG((unsigned, unsigned));
static VOIDFUNC set_icon_color PROTOARG((unsigned));
static VOIDFUNC set_icon_fill PROTOARG((unsigned));
/*}}}*/
/*{{{  void adjust_count(count, delta)*/
extern VOIDFUNC adjust_count
FUNCARG((count, delta),
	unsigned  count   /* which count to alter */
ARGSEP  int       delta   /* amount to alter it by */
)
/* alters a count and updates the display
 * adds or removes the appropriate warning flag(s)
 */
{
  size_t    ix;
  
  assert(count < COUNTS && (delta >= 0 || state.counts[count] >= -delta));
  state.counts[count] += delta;
  for(ix = itoa(count_text[count], state.counts[count], 0); ix != 3; ix++)
    count_text[count][ix] = ' ';
  count_text[count][4] = 0;
  XtVaSetValues(gizmos[GIZMO_TOTAL_COUNTS + count].widget,
      XtNlabel, (XtArgVal)count_text[count], NULL);
  if(INRANGE(count, COUNT_APPLES, 4))
    set_count_flag(count, state.counts[count] != state.counts[COUNT_RANDOM]);
  else if(count == COUNT_CHERRY)
    set_count_flag(COUNT_CHERRY, !state.counts[COUNT_CHERRY]);
  else if(count == COUNT_DEN)
    set_count_flag(COUNT_DEN, !state.counts[COUNT_DEN]);
  else if(count == COUNT_PLAYER)
    set_count_flag(COUNT_PLAYER, state.counts[COUNT_PLAYER] != 1);
  else if(count == COUNT_RANDOM)
    {
      unsigned  ix;
      
      if(state.edit && delta)
	{
	  state.edit->board->apples = state.counts[COUNT_RANDOM];
	  changed_flag |= state.change;
	}
      for(ix = 4; ix--;)
	set_count_flag(COUNT_APPLES + ix,
	    state.counts[COUNT_APPLES + ix] != state.counts[COUNT_RANDOM]);
      set_count_flag(COUNT_SPACES,
	  state.counts[COUNT_SPACES] < state.counts[COUNT_RANDOM]);
    }
  else if(count == COUNT_SPACES)
    set_count_flag(COUNT_SPACES,
	state.counts[COUNT_SPACES] < state.counts[COUNT_RANDOM]);
  return;
}
/*}}}*/
/*{{{  void apple_jump(widget, client, call)*/
static VOIDFUNC apple_jump
FUNCARG((widget, client, call),
	Widget  widget
ARGSEP  XtPointer client
ARGSEP  XtPointer call
)
/* jump scroll callback for setting number of apples to display
 * Note this slider is upsidedown.
 */
{
  int     count;
  
  count = APPLE_LIMIT - (int)floor(*(float *)call * (float)APPLE_LIMIT);
  if(count != state.counts[5])
    adjust_count(5, count - state.counts[5]);
  return;
}
/*}}}*/
/*{{{  void apple_scroll(widget, client, call)*/
static VOIDFUNC apple_scroll
FUNCARG((widget, client, call),
	Widget  widget
ARGSEP  XtPointer client
ARGSEP  XtPointer call
)
/* smooth scroll callback on apple set callback
 * note that this slider is upsidedown to normal, so
 * we get slider = 1.0 gives 0 apples and slider = 0.0 gives APPLE_LIMIT
 */
{
  int     delta;
  
  delta = (int)call ? (int)call < 0 ? -1 : 1 : 0;
  if((delta >= 0 || state.counts[5]) && (delta <= 0 ||
      state.counts[5] != APPLE_LIMIT))
    adjust_count(5, delta);
  apple_set_thumb();
  return;
}
/*}}}*/
/*{{{  void apple_set_thumb()*/
static VOIDFUNC apple_set_thumb FUNCARGVOID
/* sets the apple selectionn scrollbar to the current
 * count value
 */
{
  XawScrollbarSetThumb(gizmos[GIZMO_TOTAL_WARNINGS + 5].widget,
      (float)(APPLE_LIMIT - state.counts[5]) / (float)APPLE_LIMIT, -1.0);
  return;
}
/*}}}*/
/*{{{  void bind_button(button, option)*/
static VOIDFUNC bind_button
FUNCARG((button, option),
	unsigned  button    /* button to bind */
ARGSEP  int       option    /* option to bind */
)
/* binds an option to a button
 */
{
  GIZMO     *gptr;
  ICON      *iptr;
  
  state.button[button] = option;
  gptr = &gizmos[GIZMO_BUTTONS + button];
  XtSetValues(gptr->widget, option == OPTION_APPLES ?
      arg_apple : arg_notapple, option == OPTION_APPLES ?
      XtNumber(arg_apple) : XtNumber(arg_notapple));
  iptr = gizmo2icon(gptr);
  iptr->sprite = option < 0 ? 0 :
      gizmo2icon(&gizmos[GIZMO_OPTIONS + option])->sprite;
  generate_icon(iptr);
  IconRepaint(gptr->widget);
  return;
}
/*}}}*/
/*{{{  void command_button(widget, client, call)*/
static VOIDFUNC command_button
FUNCARG((widget, client, call),
	Widget  widget
ARGSEP  XtPointer client
ARGSEP  XtPointer call
)
/* button press callback on a command button widget.
 * Copy the selected widget's option to the pressed button's widget
 */
{
  IconCallback *call_data;
  int       option;
  
  option = state.button[(unsigned)client];
  call_data = (IconCallback *)call;
  if(option == OPTION_APPLES)
    select_apple(call_data->selection);
  else if(call_data->button >= Button1 && call_data->button <= Button5)
    bind_button(call_data->button - Button1, option);
  return;
}
/*}}}*/
/*{{{  void command_color(widget, client, call)*/
static VOIDFUNC command_color
FUNCARG((widget, client, call),
	Widget  widget
ARGSEP  XtPointer client
ARGSEP  XtPointer call
)
/* color widget selection
 * set the garden's color to the selected one.
 * update all the other icon widgets
 */
{
  if(state.edit)
    {
      state.edit->board->colors = (unsigned)client;
      paint_garden_icon(state.edit);
      paint_garden_image();
      repaint_garden_icon();
      changed_flag |= state.change;
    }
  set_icon_color((unsigned)client);
  return;
}
/*}}}*/
/*{{{  void command_fill(widget, client, call)*/
static VOIDFUNC command_fill
FUNCARG((widget, client, call),
	Widget  widget
ARGSEP  XtPointer client
ARGSEP  XtPointer call
)
/* fill widget selection
 * set garden's fill pattern and update all the other iconn widgets
 */
{
  if(state.edit)
    {
      state.edit->board->fill = (unsigned)client;
      paint_garden_icon(state.edit);
      paint_garden_image();
      repaint_garden_icon();
      changed_flag |= state.change;
    }
  set_icon_fill((unsigned)client);
  return;
}
/*}}}*/
/*{{{  void command_mode(widget, client, call)*/
static VOIDFUNC command_mode
FUNCARG((widget, client, call),
	Widget  widget
ARGSEP  XtPointer client
ARGSEP  XtPointer call
)
/* display mode set callback.
 * change the display mode and update the garden
 */
{
  if(state.mode != (unsigned)client)
    {
      state.mode = (unsigned)client;
      paint_garden_icon(state.edit);
      paint_garden_image();
      repaint_garden_icon();
    }
  return;
}
/*}}}*/
/*{{{  void command_option(widget, client, call)*/
static VOIDFUNC command_option
FUNCARG((widget, client, call),
	Widget  widget
ARGSEP  XtPointer client
ARGSEP  XtPointer call
)
/* option callback. If the selected option is the apples, 
 * set the current apple to the selected one.
 * Otherwise, or if no current button has any apples
 * set the pressed button's action to the selected option.
 * Otherwise update the apple icons
 */
{
  IconCallback *call_data;
  unsigned  option;
  
  option = (unsigned)client;
  call_data = (IconCallback *)call;
  if(option == OPTION_APPLES)
    if(select_apple(call_data->selection))
      return;
  if(call_data->button >= Button1 && call_data->button <= Button5)
    bind_button(call_data->button - Button1, option);
  return;
}
/*}}}*/
/*{{{  Boolean convert_string2option(display, args, num_args, from, to, data)*/
static Boolean convert_string2option
/* ARGSUSED */
FUNCARG((display, args, num_args, from, to, data),
	Display   *display
ARGSEP  XrmValue  *args
ARGSEP  Cardinal  *num_args
ARGSEP  XrmValue  *from
ARGSEP  XrmValue  *to
ARGSEP  XtPointer *data
)
/*
 * converts an option string to option number
 */
{
  static char CONST *options[] =
    {"apple", "random", "cherry", "path", "player", "den", NULL};
  char CONST **ptr;
  static int result;
  
  if(to->size < sizeof(int))
    {
      to->size = sizeof(int);
      return False;
    }
  for(ptr = options; *ptr; ptr++)
    if(!strcmp(*ptr, (char CONST *)from->addr))
      {
	to->size = sizeof(int);
	result = ptr - options;
	if(to->addr)
	   *(int *)to->addr = result;
	else
	  to->addr = (XtPointer)&result;
	return True;
      }
  XtDisplayStringConversionWarning(display, from->addr, XtROption);
  return False;
}
/*}}}*/
/*{{{  void drag_button(widget, client, call)*/
static VOIDFUNC drag_button
FUNCARG((widget, client, call),
	Widget  widget
ARGSEP  XtPointer client
ARGSEP  XtPointer call
)
/* drag callback for button selection.
 * Copy the initiating button's option to the selected button.
 */
{
  DragCallback *call_data;
  int       option;
  unsigned  ix;
  GIZMO     *gptr;
  
  call_data = (DragCallback *)call;
  for(gptr = &gizmos[GIZMO_BUTTONS + BUTTONS - 1], option = BUTTONS;
      option--; gptr--)
    if(gptr->widget == call_data->invoker)
      {
	option = state.button[option];
	break;
      }
  for(gptr = &gizmos[GIZMO_BUTTONS + BUTTONS - 1], ix = BUTTONS;
      ix--; gptr--)
    if(gptr->widget == call_data->selected)
      {
	bind_button(ix, option);
	break;
      }
  return;
}
/*}}}*/
/*{{{  void drag_option(widget, client, call)*/
static VOIDFUNC drag_option
FUNCARG((widget, client, call),
	Widget  widget
ARGSEP  XtPointer client
ARGSEP  XtPointer call
)
/* drag callback from option.
 * set the selected button's action from the initiating option
 */
{
  DragCallback *call_data;
  int       option;
  unsigned  ix;
  GIZMO     *gptr;
  
  call_data = (DragCallback *)call;
  for(gptr = &gizmos[GIZMO_OPTIONS + OPTIONS - 1], option = OPTIONS;
      option--; gptr--)
    if(gptr->widget == call_data->invoker)
      break;
  for(gptr = &gizmos[GIZMO_BUTTONS + BUTTONS - 1], ix = BUTTONS;
      ix--; gptr--)
    if(gptr->widget == call_data->selected)
      {
	bind_button(ix, option);
	break;
      }
  return;
}
/*}}}*/
/*{{{  void generate_icon(iptr)*/
static VOIDFUNC generate_icon
FUNCARG((iptr),
	ICON      *iptr
)
/* generate an icon pixmap.
 * Copies its background an blats on its sprite
 */
{
  XGCValues gcv;
  
  gcv.fill_style = FillOpaqueStippled;
  gcv.background = data.mono != False ? display.white :
      colors[backgrounds[iptr->color & ~VALUE_LOCK][0]].pixel;
  gcv.foreground = data.mono != False ? display.black :
      colors[backgrounds[iptr->color & ~VALUE_LOCK][1]].pixel;
  gcv.stipple = fills[iptr->fill & ~VALUE_LOCK].mask;
  XChangeGC(display.display, GCN(GC_BOARD),
      GCFillStyle | GCForeground | GCBackground | GCStipple, &gcv);
  XFillRectangle(display.display, iptr->pixmap,
      GCN(iptr->sprite == SPRITE_PLAYER || iptr->sprite == SPRITE_DEN ?
	GC_CLEAR : GC_BOARD), 0, 0, iptr->size.x, iptr->size.y);
  if(iptr->sprite && sprites[iptr->sprite].image)
    {
      XCopyArea(display.display, sprites[iptr->sprite].mask, iptr->pixmap,
	  GCN(GC_MASK), 0, 0, iptr->size.x, iptr->size.y, 0, 0);
      XCopyArea(display.display, sprites[iptr->sprite].image, iptr->pixmap,
	  GCN(GC_OR), 0, 0, iptr->size.x, iptr->size.y, 0, 0);
    }
  return;
}
/*}}}*/
/*{{{  void garden_comment(widget, client, call)*/
static VOIDFUNC garden_comment
FUNCARG((widget, client, call),
	Widget  widget
ARGSEP  XtPointer client
ARGSEP  XtPointer call
)
{
  if(state.edit)
    all_garden_comment(state.edit, state.change);
  return;
}
/*}}}*/
/*{{{  ICON gizmo2icon(gptr)*/
static ICON *gizmo2icon
FUNCARG((gptr),
	GIZMO     *gptr
)
/* find the icon associated with a gizmo.
 * returns NULL if none is
 */
{
  unsigned    ix;
  ICON        *iptr;
  
  for(iptr = icons, ix = XtNumber(icons); ix--; iptr++)
    if(iptr->gizmo == gptr - gizmos)
      return iptr;
  return NULL;
}
/*}}}*/
/*{{{  void install_control(root)*/
extern VOIDFUNC install_control
FUNCARG((root),
	Widget    root
)
/* create the widgets for the control panel
 */
{
  unsigned  ix;
  GIZMO     *gptr;
  
  /*{{{  initialize arg_**/
  {
    XFontStruct *ptr;
    
    ptr = XQueryFont(display.display, data.font);
    arg_count[0].value = (XtArgVal)ptr;
    arg_warning[0].value = (XtArgVal)sprites[SPRITE_WARNING].image;
  }
  /*}}}*/
  create_gizmos(root, gizmos, XtNumber(gizmos));
  /*{{{  initialize pixmaps*/
  {
    ICON      *iptr;
    
    for(iptr = icons, ix = XtNumber(icons); ix--; iptr++)
      {
	iptr->pixmap = XCreatePixmap(display.display, display.copy,
	    iptr->size.x, iptr->size.y, display.depth);
	generate_icon(iptr);
	XtVaSetValues(gizmos[iptr->gizmo].widget,
	    XtNpixmap, (XtArgVal)iptr->pixmap, NULL);
      }
  }
  /*}}}*/
  /*{{{  get additional resources*/
  {
    /*{{{  static XtResource button_resources[] =*/
    static XtResource button_resources[] =
    {
      {"option", "Option", XtROption, sizeof(int),
	  0, XtRImmediate, (XtPointer)-1},
    };
    /*}}}*/
    /*{{{  static XtResource index_resources[] =*/
    static XtResource index_resources[] =
    {
      {"index", "Index", XtRInt, sizeof(int),
	  0, XtRImmediate, (XtPointer)0},
    };
    /*}}}*/
    /*{{{  typedef struct _Option*/
    typedef struct _Option
    {
      unsigned  gizmo;
      unsigned  *dest;
      unsigned  range;
    } OPTION;
    /*}}}*/
    /*{{{  static OPTION options[] =*/
    static CONST OPTION options[] =
    {
      {GIZMO_OPTIONS + 0, &initial_board[0].apples, 4},
      {GIZMO_CONTROL_FILLS, &initial_board[0].fill, FILLS}, 
      {GIZMO_CONTROL_COLORS, &initial_board[0].colors, BACKGROUNDS},
      {GIZMO_DISPLAY, &state.mode, MODES},
    };
    /*}}}*/
    OPTION CONST *optr;
    
    XtAppSetTypeConverter(display.context, XtRString, XtROption,
	convert_string2option, (XtConvertArgRec *)NULL, 0, XtCacheNone,
	(void (*)PROTOARG((XtAppContext, XrmValue *, XtPointer,
	    XrmValue *, Cardinal *)))NULL);
    for(gptr = &gizmos[GIZMO_BUTTONS + BUTTONS - 1], ix = BUTTONS;
	ix--; gptr--)
      {
	unsigned  binding;
	
	XtGetApplicationResources(gptr->widget, (XtPointer)&binding,
	    button_resources, XtNumber(button_resources), NULL, 0);
	bind_button(ix, binding >= OPTIONS ? -1 : binding);
      }
    for(optr = options, ix = XtNumber(options); ix--; optr++)
      {
	XtGetApplicationResources(gizmos[optr->gizmo].widget,
	    optr->dest, index_resources, XtNumber(index_resources), NULL, 0);
	*optr->dest %= optr->range;
      }
    gizmo2icon(&gizmos[GIZMO_OPTIONS])->sprite += state.apple;
    for(ix = BUTTONS; ix--;)
      if(state.button[ix] >= 0)
	gizmo2icon(&gizmos[GIZMO_BUTTONS + ix])->sprite =
	    gizmo2icon(&gizmos[GIZMO_OPTIONS + state.button[ix]])->sprite;
  }
  /*}}}*/
  /*{{{  set option callback*/
  for(gptr = &gizmos[GIZMO_OPTIONS + OPTIONS - 1], ix = OPTIONS;
      ix--; gptr--)
    XtAddCallback(gptr->widget, XtNcallback, command_option, (XtPointer)ix);
  /*}}}*/
  /*{{{  set fill callback*/
  for(gptr = &gizmos[GIZMO_FILLS + FILLS - 1], ix = FILLS; ix--; gptr--)
    XtAddCallback(gptr->widget, XtNcallback, command_fill, (XtPointer)ix);
  /*}}}*/
  /*{{{  set color callback*/
  for(gptr = &gizmos[GIZMO_COLORS + BACKGROUNDS - 1], ix = BACKGROUNDS;
      ix--; gptr--)
    XtAddCallback(gptr->widget, XtNcallback, command_color, (XtPointer)ix);
  /*}}}*/
  /*{{{  set mode callback*/
  for(gptr = &gizmos[GIZMO_DISPLAY_BASE + MODES - 1], ix = MODES;
      ix--; gptr--)
    XtAddCallback(gptr->widget, XtNcallback, command_mode, (XtPointer)ix);
  /*}}}*/
  /*{{{  set button callback*/
  for(gptr = &gizmos[GIZMO_BUTTONS + BUTTONS - 1], ix = BUTTONS;
      ix--; gptr--)
    XtAddCallback(gptr->widget, XtNcallback, command_button, (XtPointer)ix);
  /*}}}*/
  /*{{{  setup apple scrollbar*/
  {
    float     shown;
    Arg       arg[1];
    
    gptr = &gizmos[GIZMO_TOTAL_WARNINGS + 5];
    shown = (float)1.0;
    XtSetArg(arg[0], XtNshown, sizeof(float) > sizeof(XtArgVal) ?
	(XtArgVal)&shown : *(XtArgVal *)&shown);
    XtSetValues(gptr->widget, arg, 1);
    XtAddCallback(gptr->widget, XtNjumpProc, apple_jump, (XtPointer)NULL);
    XtAddCallback(gptr->widget, XtNscrollProc, apple_scroll, (XtPointer)NULL);
  }
  /*}}}*/
  /*{{{  set drags into buttons*/
  {
    static Widget list[5];
    
    for(ix = XtNumber(list); ix--;)
      list[ix] = gizmos[GIZMO_BUTTONS + ix].widget;
    XtVaSetValues(gizmos[GIZMO_OPTION_DRAG].widget,
	MredNwidgetChoices, (XtArgVal)list,
	MredNnumWidgetChoices, (XtArgVal)XtNumber(list), NULL);
    XtVaSetValues(gizmos[GIZMO_BUTTON_DRAG].widget,
	MredNwidgetChoices, (XtArgVal)list,
	MredNnumWidgetChoices, (XtArgVal)XtNumber(list), NULL);
    XtAddCallback(gizmos[GIZMO_BUTTON_DRAG].widget, XtNcallback,
	drag_button, (XtPointer)NULL);
    XtAddCallback(gizmos[GIZMO_OPTION_DRAG].widget, XtNcallback,
	drag_option, (XtPointer)NULL);
  }
  /*}}}*/
  XtAddCallback(gizmos[GIZMO_COMMENT].widget, XtNcallback,
      garden_comment, (XtPointer)NULL);
  return;
}
/*}}}*/
/*{{{  unsigned select_apple(apple)*/
static unsigned select_apple
FUNCARG((apple),
	unsigned  apple
)
/* sets explicit apple and ipdates buttons bound to it
 * returns 1 if at least one button bound
 */
{
  GIZMO     *gptr;
  ICON      *iptr;
  unsigned  ix;
  unsigned  found;
  unsigned  sprite;
  
  sprite = SPRITE_APPLES + apple;
  if(state.apple != apple)
    {
      state.apple = apple;
      gptr = &gizmos[GIZMO_OPTIONS];
      iptr = gizmo2icon(gptr);
      iptr->sprite = sprite;
      generate_icon(iptr);
      IconRepaint(gptr->widget);
      gptr = &gizmos[GIZMO_DISPLAY_SEPARATE];
      iptr = gizmo2icon(gptr);
      iptr->sprite = sprite;
      generate_icon(iptr);
      IconRepaint(gptr->widget);
      if(state.mode == MODE_SEPARATE)
	paint_garden_image();
    }
  found = 0;
  for(ix = BUTTONS; ix--;)
    if(!state.button[ix])
      {
	gptr = &gizmos[GIZMO_BUTTONS + ix];
	iptr = gizmo2icon(gptr);
	iptr->sprite = sprite;
	generate_icon(iptr);
	IconRepaint(gptr->widget);
	found = 1;
      }
  return found;
}
/*}}}*/
/*{{{  void set_count_flag(count, flag)*/
static VOIDFUNC set_count_flag
FUNCARG((count, flag),
	unsigned  count
ARGSEP  unsigned  flag
)
{
  static unsigned state;

  if(flag != !!(state & (1 << count)))
    /*{{{  change flag*/
    {
      state ^= 1 << count;
      XtVaSetValues(gizmos[GIZMO_TOTAL_WARNINGS + count].widget,
	  XtNbitmap, sprites[SPRITE_WARNING + flag].image, NULL);
    }
    /*}}}*/
  return;
}
/*}}}*/
/*{{{  void set_icon_color(color)*/
static VOIDFUNC set_icon_color
FUNCARG((color),
	unsigned  color
)
{
  ICON      *iptr;
  unsigned  ix;
  
  state.color = color;
  for(iptr = icons, ix = XtNumber(icons); ix--; iptr++)
    if(!(iptr->color & VALUE_LOCK))
      {
	iptr->color = color;
	generate_icon(iptr);
	IconRepaint(gizmos[iptr->gizmo].widget);
      }
  return;
}
/*}}}*/
/*{{{  void set_icon_fill(fill)*/
static VOIDFUNC set_icon_fill
FUNCARG((fill),
	unsigned  fill
)
{
  ICON      *iptr;
  unsigned  ix;
  
  state.fill = fill;
  for(iptr = icons, ix = XtNumber(icons); ix--; iptr++)
    if(!(iptr->fill & VALUE_LOCK))
      {
	iptr->fill = fill;
	generate_icon(iptr);
	IconRepaint(gizmos[iptr->gizmo].widget);
      }
  return;
}
/*}}}*/
/*{{{  void set_garden(dptr, source, change)*/
extern VOIDFUNC set_garden
FUNCARG((dptr, source, change),
	DESCRIPTOR *dptr    /* garden descriptor to edit */
ARGSEP  int       source    /* source value */
ARGSEP  unsigned  change    /* change mask */
)
/* set up for a new garden to edit
 */
{
  BOARD     *bptr;
  
  assert(dptr);
  state.edit = dptr;
  state.source = source;
  state.change = change;
  bptr = dptr->board;
  assert(bptr);
  set_icon_fill(bptr->fill);
  set_icon_color(bptr->colors);
  update_garden();
  reset_garden_stack();
  return;
}
/*}}}*/
/*{{{  void set_garden_source(source, change)*/
extern VOIDFUNC set_garden_source
FUNCARG((source, change),
	int       source
ARGSEP  unsigned  change
)
{
  state.source = source;
  state.change = change;
  paint_garden_source();
  return;
}
/*}}}*/
/*{{{  void update_garden()*/
extern VOIDFUNC update_garden FUNCARGVOID
/* updates counts and repaints the current garden from scratch
 */
{
  char      *cptr;
  unsigned  ix;

  memset(state.counts, 0, sizeof(state.counts));
  state.counts[COUNT_RANDOM] = state.edit->board->apples;
  /*{{{  set count values*/
  for(ix = CELLS_DOWN; ix--;)
    for(cptr = state.edit->board->map[ix]; *cptr; cptr++)
      {
	if(*cptr == GARDEN_CHERRY || ISPATHCHERRY(*cptr))
	  state.counts[COUNT_CHERRY]++;
	else if(ISPATHDEN(*cptr))
	  state.counts[COUNT_DEN]++;
	else if(ISPATHPLAYER(*cptr))
	  state.counts[COUNT_PLAYER]++;
	else if(ISAPPLE(*cptr))
	  {
	    unsigned  value;
	    unsigned  ix;
	    
	    value = GARDENAPPLE(*cptr);
	    for(ix = 4; ix--;)
	      if(value & (1 << ix))
		state.counts[COUNT_APPLES + ix]++;
	    state.counts[COUNT_SPACES]++;
	  }
	else if(*cptr == GARDEN_RANDOM)
	  state.counts[COUNT_SPACES]++;
	if(ix != CELLS_DOWN - 1 && ISPATH(cptr[CELLS_ACROSS + 1]) &&
	    (*cptr == GARDEN_RANDOM || ISAPPLE(*cptr)))
	  state.counts[COUNT_FALL]++;
      }
  /*}}}*/
  apple_set_thumb();
  for(ix = COUNTS; ix--;)
    adjust_count(ix, 0);
  paint_garden_image();
  return;
}
/*}}}*/
