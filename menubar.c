/* Copyright (C) 1993, 1992 Nathan Sidwell */
/* RCS $Id: menubar.c 1.2 Tue, 16 Mar 1999 11:28:16 +0000 nathan $ */
/*{{{  includes*/
#include "xmred.h"
#include <X11/StringDefs.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/Paned.h>
#include <X11/Xaw/MenuButton.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/SmeBSB.h>
/*}}}*/
/*{{{  defines*/
/*{{{  gizmos*/
#define GIZMO_FILE        0
#define GIZMO_FILE_MENU   1
#define GIZMO_FILE_BASE   2
#define GIZMO_FILE_CLEAR  2
#define GIZMO_FILE_LOAD   3
#define GIZMO_FILE_INSERT 4
#define GIZMO_FILE_SAVE   5
#define GIZMO_FILE_SAVEAS 6
#define GIZMO_FILE_QUIT   7
#define GIZMO_GARDEN      8
#define GIZMO_GARDEN_MENU 9
#define GIZMO_GARDEN_BASE 10
#define GIZMO_GARDEN_NEW  10
#define GIZMO_GARDEN_CLEAR  11
#define GIZMO_GARDEN_UNDO   12
#define GIZMO_GARDEN_REDO   13
#define GIZMO_INFO_FILE   14
#define GIZMO_INFO_TOTAL  15
#define GIZMO_INFO_GARDEN 16
/*}}}*/
/*}}}*/
/*{{{  prototypes*/
static VOIDFUNC copy_stack PROTOARG((unsigned));
static VOIDFUNC file_load PROTOARG((Widget, XtPointer, XtPointer));
static VOIDFUNC file_new PROTOARG((Widget, XtPointer, XtPointer));
static VOIDFUNC file_quit PROTOARG((Widget, XtPointer, XtPointer));
static VOIDFUNC file_save PROTOARG((Widget, XtPointer, XtPointer));
static VOIDFUNC garden_blank PROTOARG((BOARD *));
static VOIDFUNC garden_clear PROTOARG((Widget, XtPointer, XtPointer));
static VOIDFUNC garden_new PROTOARG((Widget, XtPointer, XtPointer));
static VOIDFUNC garden_redo PROTOARG((Widget, XtPointer, XtPointer));
static VOIDFUNC garden_undo PROTOARG((Widget, XtPointer, XtPointer));
static VOIDFUNC menu_filename PROTOARG((char CONST *));
static VOIDFUNC stack_sensitivity PROTOARG((VOIDARG));
/*}}}*/
/*{{{  statics*/
static char *filename;
static struct
{
  BOARD     *stack[UNDO_DEPTH + 1];
  unsigned  tos;
  unsigned  offset;
} stack;
/*}}}*/
/*{{{  tables*/
/*{{{  static XtCallbackProc file_callbacks[] =*/
static XtCallbackProc file_callbacks[] =
{
  file_new,
  file_load, file_load,
  file_save, file_save,
  file_quit,
};
/*}}}*/
/*{{{  static XtCallbackProc garden_callbacks[] =*/
static XtCallbackProc garden_callbacks[] =
{
  garden_new,
  garden_clear,
  garden_undo,
  garden_redo,
};
/*}}}*/
/*{{{  static Arg arg_sensitive[] =*/
static Arg arg_sensitive[] =
{
  {XtNsensitive},
};
/*}}}*/
/*{{{  static Arg arg_blank[] =*/
static Arg arg_blank[] =
{
  {XtNlabel, (XtArgVal)""},
};
/*}}}*/
/*{{{  static GIZMO gizmos[] =*/
static GIZMO gizmos[] =
{
  {"file", -1, &menuButtonWidgetClass},
  {"menu", GIZMO_FILE, &simpleMenuWidgetClass},
  {"clear", GIZMO_FILE_MENU, &smeBSBObjectClass},
  {"load", GIZMO_FILE_MENU, &smeBSBObjectClass},
  {"insert", GIZMO_FILE_MENU, &smeBSBObjectClass},
  {"save", GIZMO_FILE_MENU, &smeBSBObjectClass},
  {"saveas", GIZMO_FILE_MENU, &smeBSBObjectClass},
  {"quit", GIZMO_FILE_MENU, &smeBSBObjectClass},
  {"garden", -1, &menuButtonWidgetClass},
  {"menu", GIZMO_GARDEN, &simpleMenuWidgetClass},
  {"new", GIZMO_GARDEN_MENU, &smeBSBObjectClass},
  {"clear", GIZMO_GARDEN_MENU, &smeBSBObjectClass},
  {"undo", GIZMO_GARDEN_MENU, &smeBSBObjectClass},
  {"redo", GIZMO_GARDEN_MENU, &smeBSBObjectClass},
  {"filename", -1, &labelWidgetClass, arg_blank, XtNumber(arg_blank)},
  {"total", -1, &labelWidgetClass, arg_blank, XtNumber(arg_blank)},
  {"current", -1, &labelWidgetClass, arg_blank, XtNumber(arg_blank)},
};
/*}}}*/
/*}}}*/
/*{{{  void copy_stack(ix)*/
static VOIDFUNC copy_stack
FUNCARG((ix),
	unsigned  ix
)
/* copy stack offset to current garden
 */
{
  assert(state.edit);
  memcpy(state.edit->board, stack.stack[ix], sizeof(BOARD));
  state.edit->board->fill = state.fill;
  state.edit->board->colors = state.color;
  changed_flag = (changed_flag & ~state.change) | stack.stack[ix]->fill;
  update_garden();
  paint_garden_icon(state.edit);
  repaint_garden_icon();
  return;
}
/*}}}*/
/*{{{  void file_load(widget, client, call)*/
static VOIDFUNC file_load
FUNCARG((widget, client, call),
	Widget  widget
ARGSEP  XtPointer client
ARGSEP  XtPointer call
)
/* load and insert menu
 * We check for unsaved changes, and then prompt and load
 * a file. Continue, until success or abort
 */
{
  char CONST *title;
  char CONST *result;
  unsigned  option;
  int       dialog;
  char  CONST *error;
  int       insert;
  
  if((unsigned long)client == GIZMO_FILE_LOAD - GIZMO_FILE_BASE)
    {
      if(!check_saved(CHANGED_ANY))
	return;
      insert = 0;
    }
  else
    insert = 1;
  XtVaGetValues(widget, XtNlabel, &title, NULL);
  dialog = DIALOG_FILENAME;
  error = NULL;
  do
    {
      option = dialog_wait(dialog, title, error, filename, &result);
      if(option & DIALOG_AGREE && result)
	{
	  dialog = DIALOG_FILE_ERROR;
	  title = "Cannot load";
	  menu_filename(result);
	  if((unsigned long)client == GIZMO_FILE_LOAD - GIZMO_FILE_BASE)
	    free_descriptors();
	  error = load_boards(filename, insert);
	}
      else
	break;
    }
  while(error);
  if((unsigned long)client == GIZMO_FILE_LOAD - GIZMO_FILE_BASE)
    changed_flag = 0;
  return;
}
/*}}}*/
/*{{{  void file_new(widget, client, call)*/
static VOIDFUNC file_new
FUNCARG((widget, client, call),
	Widget  widget
ARGSEP  XtPointer client
ARGSEP  XtPointer call
)
/* clear menu
 * check for unsaved changes, and if ok, clear everything, and
 * return to the default edit descriptors
 */
{
  if(check_saved(CHANGED_ANY | CHANGED_GARDEN))
    {
      free_descriptors();
      new_descriptors();
      changed_flag = 0;
    }
  return;
}
/*}}}*/
/*{{{  void file_quit(widget, client, call)*/
static VOIDFUNC file_quit
FUNCARG((widget, client, call),
	Widget  widget
ARGSEP  XtPointer client
ARGSEP  XtPointer call
)
/* quit menu, after checking for unsaved changes
 */
{
  if(check_saved(CHANGED_ANY))
    exit(1);
  return;
}
/*}}}*/
/*{{{  void file_save(widget, client, call)*/
static VOIDFUNC file_save
FUNCARG((widget, client, call),
	Widget  widget
ARGSEP  XtPointer client
ARGSEP  XtPointer call
)
/* save and saveas menus
 * save the descriptors, and repeat until success or abort
 * clears the appropriate changed flags
 */
{
  char CONST *title;
  char CONST *result;
  unsigned  option;
  int       dialog;
  char CONST *error;
  
  XtVaGetValues(widget, XtNlabel, &title, NULL);
  dialog = filename && *filename && (unsigned long)client ==
      GIZMO_FILE_SAVE - GIZMO_FILE_BASE ? -1 : DIALOG_FILENAME;
  error = NULL;
  option = DIALOG_NONE;
  result = NULL;
  do
  {
    if(dialog >= 0)
      option = dialog_wait(dialog, title, error, filename, &result);
    if(dialog < 0 || (option & DIALOG_AGREE && result))
      {
	char      *oresult;
	
	oresult = NULL;
	while(result && dialog >= 0 &&
	    (oresult ? strcmp(result, oresult) :
	    !filename || strcmp(result, filename)) &&
	    option & DIALOG_AGREE && check_exists(result))
	  {
	    if(result)
	      free_dup(&oresult, result);
	    option = dialog_wait(DIALOG_FILENAME, "File exists", NULL,
		oresult, &result);
	  }
	XtFree(oresult);
	if(option & DIALOG_DISAGREE)
	  break;
	dialog = DIALOG_FILE_ERROR;
	title = "Cannot save";
	if(result)
	  menu_filename(result);
	error = save_boards(filename);
      }
    else
      break;
  }
  while(error);
  return;
}
/*}}}*/
/*{{{  void garden_blank(bptr)*/
static VOIDFUNC garden_blank
FUNCARG((bptr),
	BOARD     *bptr
)
/* sets garden to blank state
 */
{
  /*{{{  typedef struct Entry*/
  typedef struct Entry
  {
    unsigned  offset;
    char      c;
  } ENTRY;
  /*}}}*/
  /*{{{  static ENTRY CONST table[] =*/
  static ENTRY CONST table[] =
  {
    {4, GARDEN_PATH + 2},
    {5, GARDEN_PATH + 2},
    {6, GARDEN_PATH + 2},
    {7, GARDEN_PATH},
    {DEFAULT_DEN_X + DEFAULT_DEN_Y * (CELLS_ACROSS + 1),
	GARDEN_PATH + GARDEN_PATH_DEN},
    {DEFAULT_PLAYER_X + DEFAULT_PLAYER_Y * (CELLS_ACROSS + 1),
	GARDEN_PATH + GARDEN_PATH_PLAYER},
  };
  /*}}}*/
  unsigned  ix;
  char      *cptr;
  ENTRY CONST *tptr;
  
  for(ix = CELLS_DOWN, cptr = (char *)bptr->map; ix--;
      cptr += CELLS_ACROSS + 1)
    {
      cptr[CELLS_ACROSS] = 0;
      memset(cptr, ix ? GARDEN_RANDOM : GARDEN_NOAPPLE, CELLS_ACROSS);
    }
  bptr->apples = DEFAULT_APPLES;
  for(tptr = table, ix = XtNumber(table); ix--; tptr++)
    {
      ((char *)bptr->map)[tptr->offset] = tptr->c;
      if(tptr->offset > CELLS_ACROSS)
	((char *)bptr->map)[tptr->offset - CELLS_ACROSS - 1] = GARDEN_NOAPPLE;
    }
  return;
}
/*}}}*/
/*{{{  void garden_clear(widget, client, call)*/
static VOIDFUNC garden_clear
FUNCARG((widget, client, call),
	Widget  widget
ARGSEP  XtPointer client
ARGSEP  XtPointer call
)
/* clear the current edit garden
 */
{
  if(state.edit)
    {
      save_garden();
      garden_blank(state.edit->board);
      free_dup(&state.edit->comment, NULL);
      update_garden();
      paint_garden_icon(state.edit);
      repaint_garden_icon();
      changed_flag |= state.change;
    }
  return;
}
/*}}}*/
/*{{{  void garden_new(widget, client, call)*/
static VOIDFUNC garden_new
FUNCARG((widget, client, call),
	Widget  widget
ARGSEP  XtPointer client
ARGSEP  XtPointer call
)
/* creates a new blank edit garden
 */
{
  if(state.source != SOURCE_UNIQUE || check_saved(CHANGED_GARDEN))
    {
      DESCRIPTOR  *dptr;
      
      dptr = unique_garden();
      garden_blank(dptr->board);
      set_garden(dptr, SOURCE_UNIQUE, CHANGED_GARDEN);
      paint_garden_icon(dptr);
      repaint_garden_icon();
    }
  return;
}
/*}}}*/
/*{{{  void garden_redo(widget, client, call)*/
static VOIDFUNC garden_redo
FUNCARG((widget, client, call),
	Widget  widget
ARGSEP  XtPointer client
ARGSEP  XtPointer call
)
/* redoes the last undone change
 */
{
  if(stack.offset != stack.tos)
    {
      stack.offset++;
      copy_stack(stack.offset);
      stack_sensitivity();
    }
  return;
}
/*}}}*/
/*{{{  void garden_undo(widget, client, call)*/
static VOIDFUNC garden_undo
FUNCARG((widget, client, call),
	Widget  widget
ARGSEP  XtPointer client
ARGSEP  XtPointer call
)
/* undo change menu option
 * rolls back the stack
 */
{
  if(stack.offset)
    {
      /*{{{  first undo?*/
      if(stack.offset == stack.tos)
	{
	  BOARD   *bptr;
	  
	  if(!stack.stack[stack.offset])
	    stack.stack[stack.offset] = (BOARD *)XtMalloc(sizeof(BOARD));
	  bptr = stack.stack[stack.offset];
	  memcpy(bptr, state.edit->board, sizeof(BOARD));
	  bptr->fill = changed_flag & state.change;
	}
      /*}}}*/
      stack.offset--;
      copy_stack(stack.offset);
      stack_sensitivity();
    }
  return;
}
/*}}}*/
/*{{{  void menu_garden(string)*/
extern VOIDFUNC menu_garden
FUNCARG((string),
	char CONST *string
)
{
  XtVaSetValues(gizmos[GIZMO_INFO_GARDEN].widget, XtNlabel, string, NULL);
  return;
}
/*}}}*/
/*{{{  void menu_filename(name)*/
static VOIDFUNC menu_filename
FUNCARG((name),
	char CONST *name
)
{
  XtVaSetValues(gizmos[GIZMO_INFO_FILE].widget, XtNlabel,
      name ? name : "<no name>", NULL);
  free_dup(&filename, name);
  return;
}
/*}}}*/
/*{{{  void menu_total(total)*/
extern VOIDFUNC menu_total
FUNCARG((total),
	unsigned  total
)
{
  static char string[5];
  
  itoa(string, total, 0);
  XtVaSetValues(gizmos[GIZMO_INFO_TOTAL].widget,
      XtNlabel, total ? string : "<None>", NULL);
  arg_sensitive[0].value = (XtArgVal)(total ? True : False);
  XtSetValues(gizmos[GIZMO_FILE_SAVE].widget,
      arg_sensitive, XtNumber(arg_sensitive));
  XtSetValues(gizmos[GIZMO_FILE_SAVEAS].widget,
      arg_sensitive, XtNumber(arg_sensitive));
  XtSetValues(gizmos[GIZMO_FILE_CLEAR].widget,
      arg_sensitive, XtNumber(arg_sensitive));
  return;
}
/*}}}*/
/*{{{  void install_menubar(root)*/
extern VOIDFUNC install_menubar
FUNCARG((root),
	Widget    root
)
/* install menu bar objects
 */
{
  unsigned  ix;
  
  create_gizmos(root, gizmos, XtNumber(gizmos));
  for(ix = XtNumber(file_callbacks); ix--;)
    XtAddCallback(gizmos[GIZMO_FILE_BASE + ix].widget,
	XtNcallback, file_callbacks[ix], (XtPointer)(unsigned long)ix);
  for(ix = XtNumber(garden_callbacks); ix--;)
    XtAddCallback(gizmos[GIZMO_GARDEN_BASE + ix].widget,
	XtNcallback, garden_callbacks[ix], (XtPointer)(unsigned long)ix);
  menu_filename(NULL);
  menu_total(0);
  return;
}
/*}}}*/
/*{{{  void reset_garden_stack()*/
extern VOIDFUNC reset_garden_stack FUNCARGVOID
{
  stack.tos = 0;
  stack.offset = 0;
  stack_sensitivity();
  return;
}
/*}}}*/
/*{{{  void save_garden()*/
extern VOIDFUNC save_garden FUNCARGVOID
{
  if(state.edit)
    {
      BOARD   *bptr;
      
      if(stack.offset == UNDO_DEPTH)
	/*{{{  rotate*/
	{
	  unsigned  ix;
	  
	  bptr = stack.stack[0];
	  for(ix = 0; ix != UNDO_DEPTH - 1; ix++)
	    stack.stack[ix] = stack.stack[ix + 1];
	  stack.stack[UNDO_DEPTH - 1] = bptr;
	}
	/*}}}*/
      else
	/*{{{  push*/
	{
	  if(!stack.stack[stack.offset])
	    stack.stack[stack.offset] = (BOARD *)XtMalloc(sizeof(BOARD));
	  bptr = stack.stack[stack.offset];
	  stack.offset++;
	  stack.tos = stack.offset;
	}
	/*}}}*/
      memcpy(bptr, state.edit->board, sizeof(BOARD));
      bptr->fill = changed_flag & state.change;
      stack_sensitivity();
    }
  return;
}
/*}}}*/
/*{{{  void stack_sensitivity()*/
static VOIDFUNC stack_sensitivity FUNCARGVOID
{
  arg_sensitive[0].value = (XtArgVal)(stack.offset ? True : False);
  XtSetValues(gizmos[GIZMO_GARDEN_UNDO].widget,
      arg_sensitive, XtNumber(arg_sensitive));
  arg_sensitive[0].value = (XtArgVal)(stack.offset != stack.tos ?
      True : False);
  XtSetValues(gizmos[GIZMO_GARDEN_REDO].widget,
      arg_sensitive, XtNumber(arg_sensitive));
  return;
}
/*}}}*/
