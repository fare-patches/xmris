/* Copyright (C) 1993, 1992 Nathan Sidwell */
/* RCS $Id: all.c,v 4.18 1995/12/14 13:53:27 nathan Exp $ */
/*{{{  includes*/
#include "xmred.h"
#include <X11/StringDefs.h>
#include <X11/Xaw/Paned.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Scrollbar.h>
#include <sys/stat.h>
#include "Drag.h"
#include "Icon.h"
#include "PixmapList.h"
/*}}}*/
/*{{{  defines*/
/*{{{  board load error masks*/
#define ERROR_FILL              0x0001
#define ERROR_BACKGROUND        0x0002
#define ERROR_APPLES            0x0004
#define ERROR_APPLE_POSITION    0x0008
#define ERROR_PATH              0x0010
#define ERROR_CHAR              0x0020
#define ERROR_STRING            0x0040
#define ERROR_INCONSISTANT      0x0080
#define ERROR_NO_CHERRIES       0x0100
#define ERROR_TOO_FEW_APPLES    0x0200
#define ERROR_NO_DEN            0x0400
#define ERROR_NO_PLAYER         0x0800
#define ERROR_BAD_APPLES        0x1000
#define ERROR_SYNTAX            0x2000
#define ERROR_EOF               0x4000
#define ERROR_READING           0x8000
#define ERROR_FIX_MASK          0x00FF
#define ERROR_FATAL_MASK        0xE000
#define ERROR_KEEP_MASK         0xC000
/*}}}*/
/*{{{  gizmos*/
#define GIZMO_DRAG_BASE    0
#define GIZMO_BUFFER_BOX   0
#define GIZMO_GARDENS_BOX  1
#define GIZMO_EDIT_BOX     2
#define GIZMO_EDIT_BASE    3
#define GIZMO_GARDEN       3
#define GIZMO_INCLUDE      4
#define GIZMO_COMMENT      5
#define GIZMO_GARDENS      6
#define GIZMO_SCROLL       7
#define GIZMO_BUFFER       8
#define GIZMO_DRAG         9
/*}}}*/
/*}}}*/
/*{{{  structs*/
/*{{{  typedef struct List*/
typedef struct List
{
  DESCRIPTOR **list;            /* pointers to all descriptors */
  unsigned  descriptors;        /* number of descriptors */
  unsigned  limit;              /* descriptor limit */
} LIST;
/*}}}*/
/*}}}*/
/*{{{  board*/
struct
{
  LIST      list;               /* display list */
  LIST      insert;             /* insert list */
  DESCRIPTOR *edit[3];          /* edit descriptors */
  int       source[3];          /* edit sources */
  DESCRIPTOR *buffer;           /* buffer */
} board;
/*}}}*/
/*{{{  prototypes*/
static VOIDFUNC all_buffer PROTOARG((Widget, XtPointer, XtPointer));
static VOIDFUNC all_drag PROTOARG((Widget, XtPointer, XtPointer));
static VOIDFUNC all_comment PROTOARG((Widget, XtPointer, XtPointer));
static VOIDFUNC all_gardens PROTOARG((Widget, XtPointer, XtPointer));
static VOIDFUNC all_include PROTOARG((Widget, XtPointer, XtPointer));
static unsigned check_board PROTOARG((BOARD *));
static char CONST *close_file PROTOARG((FILE *));
static DESCRIPTOR *dup_descriptor PROTOARG((DESCRIPTOR CONST *));
static unsigned edit_descriptor PROTOARG((int, unsigned));
static VOIDFUNC free_descriptor PROTOARG((DESCRIPTOR *));
static VOIDFUNC insert_descriptor PROTOARG((DESCRIPTOR *, int, unsigned));
static DESCRIPTOR *malloc_descriptor PROTOARG((unsigned, char *, BOARD *));
static VOIDFUNC refresh_garden_copy PROTOARG((VOIDARG));
static DESCRIPTOR *remove_descriptor PROTOARG((int));
static VOIDFUNC save_comment PROTOARG((FILE *, char CONST *, unsigned));
static int strdiff PROTOARG((char CONST *, char CONST *));
/*}}}*/
/*{{{  tables*/
/*{{{  static Arg arg_buffer[] =*/
static Arg arg_buffer[] =
{
  {XtNpixmap, (XtArgVal)0},
};
/*}}}*/
/*{{{  static char CONST *initial_text =*/
static char CONST *initial_text =
#if __STDC__
"xmred " XMREDVERSION " " DATE "\n"
"Copyright 1993 Nathan Sidwell\n"
;
#else
"xmred %s %s\nCopyright 1993 Nathan Sidwell\n";
#endif /* __STDC__ */
/*}}}*/
/*{{{  static char CONST *text_error[] =*/
static char CONST *text_error[] =
{
  "Bad fill pattern",
  "Bad background colour",
  "Too many apples",
  "Bad apple position",
  "Required paths missing",
  "Bad character",
  "String wrong length",
  "Inconistant paths",
  "No cherries",
  "Too few apple locations",
  "No den",
  "No player",
  "Inconsistant apples",
  "Syntax error",
  "Unexpected EOF",
  "Error reading file"
};
/*}}}*/
/*{{{  static XtCallbackProc edit_callbacks[] =*/
static XtCallbackProc edit_callbacks[] =
{
  NULL,
  all_include,
  all_comment,
};
/*}}}*/
/*{{{  static GIZMO gizmos[] =*/
static GIZMO gizmos[] =
{
  {"buffer", -1, &formWidgetClass},
  {"gardens", -1, &panedWidgetClass},
  {"edit", -1, &formWidgetClass},
  {"garden", GIZMO_EDIT_BOX, &iconWidgetClass},
  {"include", GIZMO_EDIT_BOX, &iconWidgetClass},
  {"comment", GIZMO_EDIT_BOX, &iconWidgetClass},
  {"gardens", GIZMO_GARDENS_BOX, &pixmapListWidgetClass},
  {"scrollbar", GIZMO_GARDENS_BOX, &scrollbarWidgetClass},
  {"buffer", GIZMO_BUFFER_BOX, &iconWidgetClass,
      arg_buffer, XtNumber(arg_buffer)},
  {"drag", -1, &dragWidgetClass},
};
/*}}}*/
/*}}}*/
/*{{{  void all_buffer(widget, client, call)*/
static VOIDFUNC all_buffer
FUNCARG((widget, client, call),
	Widget  widget
ARGSEP  XtPointer client
ARGSEP  XtPointer call
)
/* callback on the garden buffer,
 * copy the buffer contents to an edit buffer
 */
{
  if(!board.insert.list)
    edit_descriptor(SOURCE_BUFFER, 1);
  return;
}
/*}}}*/
/*{{{  void all_drag(widget, client, call)*/
static VOIDFUNC all_drag
FUNCARG((widget, client, call),
	Widget  widget
ARGSEP  XtPointer client
ARGSEP  XtPointer call
)
/* drag callback
 * determine the source and destination of the drag
 * and then do the appropriate thing.
 * This generally means moving the descriptor around,
 * except when the source is the buffer or edit, in
 * which case a copy is done.
 */
{
  DragCallback *dc;
  int       dst;
  int       src;
  
  dc = (DragCallback *)call;
  /*{{{  set dst*/
  if(dc->selected == gizmos[GIZMO_EDIT_BOX].widget)
    dst = SOURCE_UNIQUE;
  else if(dc->selected == gizmos[GIZMO_BUFFER_BOX].widget)
    dst = SOURCE_BUFFER;
  else
    {
      static  Position x, y;
      static  Dimension width, height;
      /*{{{  static Arg args[] =*/
      static Arg args[] =
      {
	{XtNx, (XtArgVal)&x},
	{XtNy, (XtArgVal)&y},
	{XtNwidth, (XtArgVal)&width},
	{XtNheight, (XtArgVal)&height},
      };
      /*}}}*/
      
      assert(dc->selected == gizmos[GIZMO_GARDENS_BOX].widget);
      XtGetValues(gizmos[GIZMO_GARDENS].widget, args, XtNumber(args));
      dc->offset_x -= x;
      dc->offset_y -= y;
      dst = PixmapListQueryOffset(gizmos[GIZMO_GARDENS].widget,
	  dc->offset_x, dc->offset_y, True);
      assert(dst >= 0);
    }
  /*}}}*/
  /*{{{  set src*/
  if(XtParent(dc->invoker) == gizmos[GIZMO_EDIT_BOX].widget)
    src = SOURCE_UNIQUE;
  else if(dc->invoker == gizmos[GIZMO_BUFFER].widget)
    src = SOURCE_BUFFER;
  else if(dc->invoker == gizmos[GIZMO_GARDENS].widget)
    {
      src = PixmapListQueryDrag(gizmos[GIZMO_GARDENS].widget);
      if(src < 0)
	return;
    }
  else
    return;
  /*}}}*/
  if(src == SOURCE_BUFFER &&
      (!board.buffer || board.buffer->type == DESCRIPTOR_NONE))
    return;
  if(src == SOURCE_UNIQUE)
    /*{{{  from edit*/
    {
      int       ix;
      DESCRIPTOR *dptr;
      
      if(dst == SOURCE_UNIQUE)
	return;
      for(ix = 0; ix != 3; ix++)
	if(dc->invoker == gizmos[GIZMO_EDIT_BASE + ix].widget)
	  {
	    dptr = dup_descriptor(board.edit[ix]);
	    if(dst != SOURCE_BUFFER)
	      changed_flag &= ~(1 << ix);
	    insert_descriptor(dptr, dst, 1);
	    break;
	  }
    }
    /*}}}*/
  else if(src == SOURCE_BUFFER)
    /*{{{  from buffer*/
    {
      if(board.insert.list)
	{
	  if(dst >= 0)
	    {
	      unsigned  ix;
	      
	      for(ix = board.insert.descriptors; ix--;)
		{
		  insert_descriptor(board.insert.list[ix], dst, 0);
		  board.insert.list[ix] = NULL;
		}
	      free_descriptor(remove_descriptor(SOURCE_BUFFER));
	    }
	}
      else if(dst == SOURCE_UNIQUE)
	edit_descriptor(SOURCE_BUFFER, 1);
      else if(dst == SOURCE_BUFFER)
	return;
      else if(board.buffer)
	{
	  DESCRIPTOR *dptr;
	  
	  dptr = dup_descriptor(board.buffer);
	  insert_descriptor(dptr, dst,
	      board.source[dptr->type] == SOURCE_BUFFER);
	}
    }
    /*}}}*/
  else
    /*{{{  from gardens*/
    {
      if(dst == SOURCE_UNIQUE)
	edit_descriptor(src, 0);
      else if(dst == SOURCE_BUFFER || (src + 1 != dst && src != dst))
	{
	  unsigned  copy;
	  DESCRIPTOR *dptr;
	  
	  if(dst == SOURCE_BUFFER)
	    {
	      for(copy = 0; copy != 3; copy++)
		if(board.source[copy] == SOURCE_BUFFER)
		  break;
	      if(copy == 3 && !check_saved(CHANGED_BUFFER))
		return;
	    }
	  copy = board.source[board.list.list[src]->type] == src;
	  dptr = remove_descriptor(src);
	  if(dst > src)
	    dst--;
	  insert_descriptor(dptr, dst, copy);
	}
    }
    /*}}}*/
  return;
}
/*}}}*/
/*{{{  void all_comment(widget, client, call)*/
static VOIDFUNC all_comment
FUNCARG((widget, client, call),
	Widget  widget
ARGSEP  XtPointer client
ARGSEP  XtPointer call
)
/* callback on the comment edit.
 * Popup the dialog and await response.
 * Sets the appropriate changed flag
 */
{
  char CONST *result;
  unsigned  option;
  
  option = dialog_wait(DIALOG_COMMENT, "General Comment", NULL,
      board.edit[DESCRIPTOR_COMMENT]->comment, &result);
  if(option & DIALOG_AGREE && strdiff(result, 
      board.edit[DESCRIPTOR_COMMENT]->comment))
    {
      if(board.source[DESCRIPTOR_COMMENT] >= 0)
	{
	  assert(board.list.list[board.source[DESCRIPTOR_COMMENT]]->type ==
	      DESCRIPTOR_COMMENT);
	  free_dup(&board.list.list[board.source[DESCRIPTOR_COMMENT]]->comment,
	      result);
	  changed_flag |= CHANGED_ALL;
	}
      else if(board.source[DESCRIPTOR_COMMENT] == SOURCE_BUFFER)
	{
	  assert(board.buffer->type == DESCRIPTOR_COMMENT);
	  free_dup(&board.buffer->comment, result);
	  changed_flag |= CHANGED_BUFFER;
	}
      changed_flag |= CHANGED_COMMENT;
      free_dup(&board.edit[DESCRIPTOR_COMMENT]->comment, result);
    }
  return;
}
/*}}}*/
/*{{{  void all_gardens(widget, client, call)*/
static VOIDFUNC all_gardens
FUNCARG((widget, client, call),
	Widget  widget
ARGSEP  XtPointer client
ARGSEP  XtPointer call
)
/* callback on the gardens list.
 * copy the selected garden to an edit buffer.
 */
{
  edit_descriptor(((PixmapListCallback *)call)->selection, 1);
  return;
}
/*}}}*/
/*{{{  void all_garden_comment(dptr, change)*/
extern VOIDFUNC all_garden_comment
FUNCARG((dptr, change),
	DESCRIPTOR  *dptr   /* the garden descriptor */
ARGSEP  unsigned  change    /* change mask to apply */
)
/* do the garden comment for the control stuff.
 * Popup the dialog and await response.
 * Sets the appropriate changed flag
 */
{
  char CONST *result;
  unsigned  option;
  
  option = dialog_wait(DIALOG_COMMENT, "Garden Comment", NULL,
      dptr->comment, &result);
  if(option & DIALOG_AGREE && strdiff(result, dptr->comment))
    {
      changed_flag |= change;
      free_dup(&dptr->comment, result);
    }
  return;
}
/*}}}*/
/*{{{  void all_include(widget, client, call)*/
static VOIDFUNC all_include
FUNCARG((widget, client, call),
	Widget  widget
ARGSEP  XtPointer client
ARGSEP  XtPointer call
)
/* callback on the include edit.
 * Popup the dialog and await response.
 * Sets the appropriate changed flag
 */
{
  char CONST *result;
  unsigned  option;
  
  option = dialog_wait(DIALOG_INCLUDE, "Include", NULL,
      board.edit[DESCRIPTOR_INCLUDE]->comment, &result);
  if(option & (DIALOG_AGREE | DIALOG_DEFAULT) && strdiff(result,
      board.edit[DESCRIPTOR_INCLUDE]->comment))
    {
      if(board.source[DESCRIPTOR_INCLUDE] >= 0) 
	{
	  assert(board.list.list[board.source[DESCRIPTOR_INCLUDE]]->type ==
	      DESCRIPTOR_INCLUDE);
	  free_dup(&board.list.list[board.source[DESCRIPTOR_INCLUDE]]->comment,
	      result);
	  changed_flag |= CHANGED_ALL;
	}
      else if(board.source[DESCRIPTOR_COMMENT] == SOURCE_BUFFER)
	{
	  assert(board.buffer->type == DESCRIPTOR_INCLUDE);
	  free_dup(&board.buffer->comment, result);
	  changed_flag |= CHANGED_BUFFER;
	}
      changed_flag |= CHANGED_INCLUDE;
      free_dup(&board.edit[DESCRIPTOR_INCLUDE]->comment, result);
    }
  return;
}
/*}}}*/
/*{{{  unsigned check_board(map)*/
static unsigned check_board
FUNCARG((map),
	BOARD     *map
)
/* checks a board is ok
 * and fix if possible
 * returns a mask of the errors
 */
{
  unsigned  error;
  
  error = 0;
  /*{{{  check characters*/
  {
    char      *cptr;
    unsigned  ix;
    
    for(ix = CELLS_DOWN; ix--;)
      for(cptr = map->map[ix]; *cptr; cptr++)
	if(*cptr != GARDEN_RANDOM && *cptr != GARDEN_NOAPPLE &&
	    *cptr != GARDEN_CHERRY && !ISPATH(*cptr) && !ISAPPLE(*cptr))
	  {
	    error |= ERROR_CHAR;
	    *cptr = GARDEN_RANDOM;
	  }
  }
  /*}}}*/
  /*{{{  check fill*/
  if(map->fill >= FILLS)
    {
      error |= ERROR_FILL;
      map->fill %= FILLS;
    }
  /*}}}*/
  /*{{{  check background*/
  if(map->colors >= BACKGROUNDS)
    {
      error |= ERROR_BACKGROUND;
      map->colors %= BACKGROUNDS;
    }
  /*}}}*/
  /*{{{  check apples number*/
  if(map->apples > APPLE_LIMIT)
    {
      error |= ERROR_APPLES;
      map->apples = APPLE_LIMIT;
    }
  /*}}}*/
  /*{{{  check apple positioning*/
  {
    char      *cptr;
    
    for(cptr = map->map[CELLS_DOWN - 1]; *cptr; cptr++)
      if(ISAPPLE(*cptr) || *cptr == GARDEN_RANDOM)
	{
	  *cptr = GARDEN_NOAPPLE;
	  error |= ERROR_APPLE_POSITION;
	}
  }
  /*}}}*/
  /*{{{  check required paths*/
  {
    static CONST unsigned path[][2] =
      {
	{4, 0x2}, {5, 0x2}, {6, 0x2}, {7, 0x0},
      };
    unsigned  ix;
    
    for(ix = XtNumber(path); ix--;)
      {
	char      *cptr;
	unsigned  c;
	
	cptr = (char *)(map->map) + path[ix][0];
	c = *cptr;
	if(!ISPATH(c))
	  {
	    error |= ERROR_PATH;
	    *cptr = GARDEN_PATH + path[ix][1];
	  }
	else if((GARDENPATH(c) & path[ix][1]) != path[ix][1])
	  {
	    error |= ERROR_PATH;
	    *cptr = (GARDENPATH(c) | path[ix][1]) + GARDEN_PATH;
	  }
      }
  }
  /*}}}*/
  /*{{{  check integrity*/
  {
    unsigned  row;
    char      *cptr;
    
    for(row = CELLS_DOWN; row--;)
      for(cptr = map->map[row]; *cptr; cptr++)
	/*{{{  check a cell*/
	{
	  char      c;
	  
	  c = *cptr;
	  if(ISPATH(c))
	    {
	      unsigned  paths;
	      
	      paths = 0;
	      if(ISPATH(cptr[1]))
		paths |= 2;
	      if(row != CELLS_DOWN - 1 &&
		  ISPATH(cptr[CELLS_ACROSS + 1]))
		paths |= 1;
	      if((GARDENPATH(c) & paths) != (GARDENPATH(c) & 3))
		{
		  error |= ERROR_INCONSISTANT;
		  *cptr = ((GARDENPATH(c) & 0xC) | paths) + GARDEN_PATH;
		}
	    }
	}
	/*}}}*/
  }
  /*}}}*/
  /*{{{  check totals*/
  {
    char      *cptr;
    unsigned  count;
    unsigned  totals[8];
    
    for(count = 8; count--;)
      totals[count] = 0;
    for(count = (CELLS_ACROSS + 1) * CELLS_DOWN,
	cptr = (char *)map->map; count--; cptr++)
      if(ISPATHCHERRY(*cptr) || *cptr == GARDEN_CHERRY)
	totals[4]++;
      else if(ISPATHDEN(*cptr))
	totals[5]++;
      else if(ISPATHPLAYER(*cptr))
	totals[6]++;
      else if(ISAPPLE(*cptr))
	{
	  unsigned  value;
	  unsigned  ix;
	  
	  value = GARDENAPPLE(*cptr);
	  for(ix = 4; ix--; value >>= 1)
	    if(value & 1)
	      totals[ix]++;
	  totals[7]++;
	}
      else if(*cptr == GARDEN_RANDOM)
	totals[7]++;
    if(!totals[5])
      error |= ERROR_NO_DEN;
    if(!totals[6])
      error |= ERROR_NO_PLAYER;
    if(!totals[4])
      error |= ERROR_NO_CHERRIES;
    if(totals[7] < map->apples)
      error |= ERROR_TOO_FEW_APPLES;
    for(count = 4; count--;)
      if(totals[count] != map->apples)
	error |= ERROR_BAD_APPLES;
  }
  /*}}}*/
  return error;
}
/*}}}*/
/*{{{  unsigned check_exists(filename)*/
extern unsigned check_exists
FUNCARG((filename),
	char CONST *filename
)
/* check if a filename exists
 * returns 1 if found
 */
{
  struct stat buffer;
  
  assert(filename);
  return !stat(filename, &buffer);
}
/*}}}*/
/*{{{  unsigned check_saved(mask)*/
extern unsigned check_saved
FUNCARG((mask),
	unsigned  mask  /* changed mask to check */
)
/* check changes are saved, or that its ok to lose them
 * pops up a dialog to query if changed.
 * returns 1 if saved
 */
{
  unsigned  unsaved;
  
  unsaved = changed_flag & mask;
  if(unsaved)
    unsaved = dialog_wait(DIALOG_NOT_SAVED, NULL, NULL, NULL, NULL) &
	DIALOG_DISAGREE;
  return !unsaved;
}
/*}}}*/
/*{{{  char CONST *close_file(stream)*/
static char CONST *close_file
FUNCARG((stream),
	FILE      *stream
)
/* close the file and check error status
 * returns the error string, or NULL
 */
{
  extern int sys_nerr;
  extern char *sys_errlist[];
  
  int   error;
  
  if(!stream)
    error = errno;
  else if(ferror(stream))
    {
      error = errno;
      fclose(stream);
    }
  else if(fclose(stream))
    error = errno;
  else
    return NULL;
  return error >= 0 && error < sys_nerr ? sys_errlist[error] : "Unknown error";
}
/*}}}*/
/*{{{  DESCRIPTOR *dup_descriptor(src)*/
static DESCRIPTOR *dup_descriptor
FUNCARG((src),
	DESCRIPTOR CONST *src
)
/* duplicates a descriptor and its pixmap etc
 */
{
  DESCRIPTOR  *dptr;
  
  dptr = malloc_descriptor(src->type, XtNewString(src->comment),
      src->type == DESCRIPTOR_GARDEN ?
      memcpy(XtMalloc(sizeof(BOARD)), src->board, sizeof(BOARD)): NULL);
  if(src->type == DESCRIPTOR_GARDEN)
    XCopyArea(display.display, src->pixmap, dptr->pixmap, GCN(GC_COPY),
	0, 0, ICON_WIDTH, ICON_HEIGHT, 0, 0);
  return dptr;
}
/*}}}*/
/*{{{  unsigned edit_descriptor(src, dup)*/
static unsigned edit_descriptor
FUNCARG((src, dup),
	int       src   /* descriptor source */
ARGSEP  unsigned  dup   /* duplicate or remove source? */
)
/* edit the descriptor at src
 * this creates a copy of the descriptor
 * if !dup the source is removed
 * otherwise the edit.source is set to src
 * the previous edit descriptor is removed.
 * A check is made if that has unsaved changes
 * returns 1 if suceeded, 0 for abort du to unsaved changes
 */
{
  DESCRIPTOR *dptr;
  
  dptr = src < 0 ? board.buffer : board.list.list[src];
  if(!dptr || dptr->type == DESCRIPTOR_NONE ||
      (board.source[dptr->type] == SOURCE_UNIQUE &&
	!check_saved(1 << dptr->type)))
    return 0;
  if(dptr->type == DESCRIPTOR_GARDEN)
    refresh_garden_copy();
  free_descriptor(board.edit[dptr->type]);
  changed_flag &= ~(1 << dptr->type);
  if(dup)
    dptr = dup_descriptor(dptr);
  board.edit[dptr->type] = dptr;
  XtVaSetValues(gizmos[GIZMO_EDIT_BASE + dptr->type].widget,
      XtNpixmap, (XtArgVal)board.edit[dptr->type]->pixmap, NULL);
  if(dup)
    board.source[dptr->type] = src;
  else
    {
      board.source[dptr->type] = SOURCE_UNIQUE;
      remove_descriptor(src);
    }
  if(dptr->type == DESCRIPTOR_GARDEN)
    {
      set_garden(dptr, board.source[DESCRIPTOR_GARDEN],
	  dup ? src == SOURCE_BUFFER ? CHANGED_BUFFER | CHANGED_GARDEN :
	  CHANGED_ALL | CHANGED_GARDEN : CHANGED_GARDEN);
      if(dup)
	changed_flag &= ~CHANGED_GARDEN;
      else
	changed_flag |= CHANGED_GARDEN;
    }
  return 1;
}
/*}}}*/
/*{{{  void free_descriptor(dptr)*/
static VOIDFUNC free_descriptor
FUNCARG((dptr),
	DESCRIPTOR *dptr
)
/* frees the descriptor */
{
  if(!dptr)
    return;
  XtFree(dptr->comment);
  if(dptr->type == DESCRIPTOR_GARDEN)
    {
      XFreePixmap(display.display, dptr->pixmap);
      XtFree((VOID *)dptr->board);
    }
  XtFree((VOID *)dptr);
  return;
}
/*}}}*/
/*{{{  void free_descriptors()*/
extern VOIDFUNC free_descriptors FUNCARGVOID
/* remove and free all the display descriptors
 */
{
  while(board.list.descriptors)
    {
      DESCRIPTOR *dptr;
      
      dptr = remove_descriptor(board.list.descriptors - 1);
      free_descriptor(dptr);
    }
  return;
}
/*}}}*/
/*{{{  void free_dup(dptr, src)*/
extern VOIDFUNC free_dup
FUNCARG((dptr, src),
	char      **dptr
ARGSEP  char CONST *src
)
/* copy a string, freeing the original destination value */
{
  XtFree(*dptr);
  *dptr = XtNewString(src);
  return;
}
/*}}}*/
/*{{{  void insert_descriptor(dptr, place, copy)*/
static VOIDFUNC insert_descriptor
FUNCARG((dptr, place, copy),
	DESCRIPTOR *dptr    /* descriptor */
ARGSEP  int       place     /* place to insert */
ARGSEP  unsigned  copy      /* is an edit copy */
)
/* insert a descriptor into the display list at the given place
 * if the destination is the buffer, then the orignal buffer
 * is emptied.
 * Sets the edit source if copy != 0
 */
{
  unsigned  count;
  
  if(place == SOURCE_BUFFER)
    {
      if(board.buffer)
	free_descriptor(remove_descriptor(SOURCE_BUFFER));
      board.buffer = dptr;
      XtVaSetValues(gizmos[GIZMO_BUFFER].widget,
	  XtNpixmap, dptr->pixmap, NULL);
      changed_flag &= ~CHANGED_BUFFER;
    }
  else
    {
      DESCRIPTOR **ddptr;
      
      if(place > board.list.descriptors)
	place = board.list.descriptors;
      board.list.descriptors++;
      if(board.list.descriptors > board.list.limit)
	{
	  board.list.limit += 128;
	  board.list.list = (DESCRIPTOR **)XtRealloc((VOID *)board.list.list,
	      board.list.limit * sizeof(DESCRIPTOR *));
	}
      for(ddptr = &board.list.list[board.list.descriptors - 1],
	  count = board.list.descriptors - place - 1; count--; ddptr--)
	ddptr[0] = ddptr[-1];
      assert(ddptr == &board.list.list[place]);
      PixmapListInsert(gizmos[GIZMO_GARDENS].widget, place,
	  dptr->pixmap);
      *ddptr = dptr;
      for(count = 3; count--;)
	if(board.source[count] >= place)
	  {
	    board.source[count]++;
	    if(count == DESCRIPTOR_GARDEN &&
		(!copy || dptr->type != DESCRIPTOR_GARDEN))
	      {
		set_garden_source(board.source[DESCRIPTOR_GARDEN],
		    CHANGED_GARDEN | CHANGED_ALL);
	      }
	  }
      changed_flag |= CHANGED_ALL;
      menu_total(board.list.descriptors);
    }
  if(copy)
    {
      board.source[dptr->type] = place;
      if(place == SOURCE_BUFFER && changed_flag & (1 << dptr->type))
	changed_flag |= CHANGED_BUFFER;
      if(dptr->type == DESCRIPTOR_GARDEN)
	set_garden_source(place, place == SOURCE_BUFFER ?
	    CHANGED_GARDEN | CHANGED_BUFFER : CHANGED_GARDEN | CHANGED_ALL);
    }
  return;
}
/*}}}*/
/*{{{  void install_all(root)*/
extern VOIDFUNC install_all
FUNCARG((root),
	Widget    root
)
/* creates the allbox widgets
 * adds the callbacks
 * initializes the garden list and edit boxes.
 */
{
  GIZMO     *gptr;
  unsigned  ix;
  
#if !__STDC__
  /*{{{  string build*/
  {
    char      *text;
    
    text = XtMalloc(strlen(initial_text) +
	strlen(XMREDVERSION) + strlen(DATE) + 1);
    sprintf(text, initial_text, XMREDVERSION, DATE);
    initial_text = text;
  }
  /*}}}*/
#endif /* __STDC__ */
  arg_buffer[0].value = (XtArgVal)XCreatePixmap(display.display, display.copy,
	  ICON_WIDTH, ICON_HEIGHT, display.depth);
  XFillRectangle(display.display, (Pixmap)arg_buffer[0].value, GCN(GC_CLEAR),
      0, 0, ICON_WIDTH, ICON_HEIGHT);
  create_gizmos(root, gizmos, XtNumber(gizmos));
  /*{{{  set drags into all*/
  {
    static Widget list[3];
    
    for(ix = XtNumber(list); ix--;)
      list[ix] = gizmos[GIZMO_DRAG_BASE + ix].widget;
    XtVaSetValues(gizmos[GIZMO_DRAG].widget,
	MredNwidgetChoices, (XtArgVal)list,
	MredNnumWidgetChoices, (XtArgVal)XtNumber(list), NULL);
    XtAddCallback(gizmos[GIZMO_DRAG].widget, XtNcallback,
	all_drag, (XtPointer)NULL);
  }
  /*}}}*/
  /*{{{  add edit callbacks*/
  {
    for(gptr = &gizmos[GIZMO_EDIT_BASE + XtNumber(edit_callbacks) - 1],
	ix = XtNumber(edit_callbacks); ix--; gptr--)
      if(edit_callbacks[ix])
	XtAddCallback(gptr->widget, XtNcallback, edit_callbacks[ix],
	    (XtPointer)NULL);
  }
  /*}}}*/
  XtAddCallback(gizmos[GIZMO_BUFFER].widget, XtNcallback,
      all_buffer, (XtPointer)NULL);
  XtAddCallback(gizmos[GIZMO_GARDENS].widget, XtNcallback,
      all_gardens, (XtPointer)NULL);
  PixmapListSetScroll(gizmos[GIZMO_GARDENS].widget,
      gizmos[GIZMO_SCROLL].widget);
  for(ix = 3; ix--;)
    board.source[ix] = SOURCE_UNIQUE;
  new_descriptors();
  return;
}
/*}}}*/
/*{{{  char CONST *load_boards(filename, insert)*/
extern char CONST *load_boards
FUNCARG((filename, insert),
	char CONST *filename
ARGSEP  unsigned  insert    /* insert flag */
)
/* load a file up.
 * This is inserted or appended to the display list
 * returns NULL or the error text.
 */
{
  FILE      *stream;
  unsigned  popped;
  
  popped = 0;
  stream = fopen(filename, "r");
  if(stream)
    {
      unsigned  error_line;
      unsigned  line;
      unsigned  start;
      unsigned  error;
      size_t    limit;
      char      *text;
      DESCRIPTOR *last;
      
      error = error_line = 0;
      line = start = 0;
      limit = 80;
      text = XtMalloc(limit);
      last = NULL;
      if(insert)
	{
	  insert_descriptor(malloc_descriptor(DESCRIPTOR_INCLUDE, NULL, NULL),
	      SOURCE_BUFFER, 0);
	  board.insert.limit = 128;
	  board.insert.list = (DESCRIPTOR **)XtMalloc(board.insert.limit *
	      sizeof(DESCRIPTOR *));
	}
      do
	/*{{{  read a descriptor*/
	{
	  BOARD     *bptr;
	  char      *tptr;
	  size_t    tlen;
	  unsigned  type;
	  unsigned  append;
	  
	  append = 0;
	  bptr = NULL;
	  tptr = NULL;
	  tlen = 0;
	  type = DESCRIPTOR_NONE;
	  /*{{{  read descriptor*/
	  {
	    /* state 0    new block
	     * state 1    comment
	     * state 2-4  numbers
	     * state 5    open brace
	     * state 6    close brace
	     * state 7    close brace
	     * state 10+  lines
	     */
	    unsigned  state;
	    
	    state = 0;
	    start = line + 1;
	    do
	      {
		char      *ptr;
	    
		/*{{{  read line*/
		{
		  size_t    length;
		  char      *ptr;
		  char      *lptr;
		  
		  line++;
		  length = 0;
		  ptr = text;
		  /*{{{  read line*/
		  while((lptr = fgets(ptr, limit - length, stream)))
		    {
		      ptr += strlen(ptr);
		      length = ptr - text;
		      if(length && *(ptr - 1) == '\n')
			break;
		      if(length + 1 < limit)
			break;
		      limit += 128;
		      text = XtRealloc(text, limit);
		      ptr = text + length;
		    }
		  /*}}}*/
		  /*{{{  error?*/
		  if(!lptr)
		    {
		      if(feof(stream))
			*text = 0;
		      else
			error |= ERROR_READING;
		    }
		  /*}}}*/
		  if(!error)
		    error_line = line;
		  else if(error & ERROR_READING)
		    break;
		}
		/*}}}*/
		ptr = text;
		if(!*ptr)
		  {
		    if(state)
		      error |= ERROR_EOF;
		    break;
		  }
		while(*ptr == ' ' || *ptr == '\t')
		  ptr++;
		if(*ptr == '\n')
		  continue;
		/*{{{  new block*/
		if(state == 0)
		  {
		    if(ptr[0] == '/' && ptr[1] == '*')
		      {
			ptr += 2;
			state = 1;
			if(*ptr == '!')
			  ptr++;
			else if(last)
			  {
			    tptr = last->comment;
			    tlen = strlen(tptr);
			    append = 1;
			  }
			else
			  type = DESCRIPTOR_COMMENT;
		      }
		    else if(!strncmp(ptr, "#include", 8))
		      /*{{{  include*/
		      {
			ptr += 8;
			while(*ptr == ' ' || *ptr == '\t')
			  ptr++;
			if(*ptr == '\"')
			  {
			    char      *name;
			
			    name = ++ptr;
			    while(*ptr && *ptr != '\"')
			      ptr++;
			    *ptr = 0;
			    free_dup(&tptr, name);
			    type = DESCRIPTOR_INCLUDE;
			  }
			else
			  {
			    error = ERROR_SYNTAX;
			    break;
			  }
		      }
		      /*}}}*/
		    else if(*ptr == '{')
		      {
			last = NULL;
			ptr++;
			state = 2;
			bptr = (BOARD *)XtMalloc(sizeof(BOARD));
		      }
		    else
		      error = ERROR_SYNTAX;
		  }
		/*}}}*/
		/*{{{  in comment*/
		if(state == 1)
		  {
		    char    *start;
		    char    *end;
		    
		    for(start = ptr; *start; start++)
		      if(*start != ' ' && *start != '\t')
			break;
		    ptr = start;
		    while(*ptr && state)
		      {
			while(*ptr && *ptr != '*')
			  ptr++;
			if(*ptr)
			  {
			    if(ptr[1] == '/')
			      {
				ptr += 2;
				state = 0;
				break;
			      }
			    else
			      ptr++;
			  }
		      }
		    end = state ? ptr : ptr - 2;
		    while(end != start && end[-1] == ' ')
		      end--;
		    while(start != end && (*start == ' ' || *start == '*'))
		      start++;
		    if(end != start && *start != '\n')
		      {
			size_t    length;
			
			*end = 0;
			length = end - start;
			tptr = (char *)XtRealloc((VOID *)tptr,
			    tlen + length + 2);
			strcpy(&tptr[tlen], start);
			tlen += length;
			if(tptr[tlen - 1] != '\n')
			  {
			    tptr[tlen++] = '\n';
			    tptr[tlen] = 0;
			  }
		      }
		  }
		/*}}}*/
		/*{{{  reading board*/
		if(state >= 2)
		  {
		    while(state)
		      {
			if(*ptr == ' ' || *ptr == '\t')
			  ptr++;
			if(*ptr == '\n')
			  break;
			switch(state)
			{
			  /*{{{  case 2: case 3: case 4:*/
			  case 2: case 3: case 4:
			  {
			    char      *eptr;
			    unsigned  value;
			    
			    value = strtol(ptr, &eptr, 0);
			    if(eptr == ptr)
			      {
				error = ERROR_SYNTAX;
				state = 7;
			      }
			    else
			      {
				switch(state)
				{
				  case 2:
				    bptr->fill = value;
				    /* CASCADE */
				  case 3:
				    bptr->colors = value;
				    /* CASCADE */
				  case 4:
				    bptr->apples = value;
				    /* CASCADE */
				}
				ptr = eptr;
				state++;
			      }
			    break;
			  }
			  /*}}}*/
			  /*{{{  case 5:*/
			  case 5:
			    if(*ptr == '{')
			      {
				state = 10;
				ptr++;
			      }
			    else
			      {
				state = 7;
				error = ERROR_SYNTAX;
			      }
			    break;
			  /*}}}*/
			  /*{{{  case 6:*/
			  case 6:
			    if(*ptr == '}')
			      {
				ptr++;
				state = 7;
			      }
			    else if(*ptr == '"')
			      {
				char      *eptr;
				
				eptr = strchr(ptr + 1, '\"');
				if(eptr)
				  ptr = eptr + 1;
				else
				  {
				    ptr++;
				    error = ERROR_SYNTAX;
				  }
			      }
			    else
			      {
				ptr++;
				error = ERROR_SYNTAX;
			      }
			    break;
			  /*}}}*/
			  /*{{{  case 7:*/
			  case 7:
			    if(*ptr == '}')
			      {
				state = 0;
				if(!(error & ERROR_FATAL_MASK))
				  type = DESCRIPTOR_GARDEN;
				ptr++;
			      }
			    else if(*ptr++ == '{')
			      state = 6;
			    else
			      error = ERROR_SYNTAX;
			    break;
			  /*}}}*/
			  /*{{{  default:*/
			  default:
			  {
			    char      *eptr;
			    
			    assert(state >= 10 && state < 10 + CELLS_DOWN);
			    eptr = strchr(ptr + 1, '\"');
			    if(*ptr != '\"' || !eptr)
			      {
				error = ERROR_SYNTAX;
				state = 6;
			      }
			    else
			      {
				*eptr = 0;
				if(eptr - ptr > CELLS_ACROSS + 1)
				  {
				    error = ERROR_STRING;
				    ptr[1 + CELLS_ACROSS] = 0;
				  }
				strcpy(bptr->map[state - 10], ptr + 1);
				if(eptr - ptr < CELLS_ACROSS + 1)
				  {
				    error = ERROR_STRING;
				    memset(&bptr->map[state - 10]
					[eptr - ptr - 1], '.',
					CELLS_ACROSS + 1 - (eptr - ptr));
				  }
				bptr->map[state - 10][CELLS_ACROSS] = 0;
				state++;
				ptr = eptr + 1;
				if(state == 10 + CELLS_DOWN)
				  state = 6;
			      }
			    break;
			  }
			  /*}}}*/
			}
			if(*ptr == ',')
			  ptr++;
		      }
		  }
		/*}}}*/
	      }
	    while(state || (tptr && type == DESCRIPTOR_NONE && !append));
	    if(!error && !*text)
	      break;
	  }
	  /*}}}*/
	  if(append)
	    {
	      last->comment = tptr;
	      assert(type == DESCRIPTOR_NONE);
	    }
	  if(bptr)
	    error |= check_board(bptr);
	  /*{{{  store descriptor?*/
	  if(type != DESCRIPTOR_NONE)
	    {
	      DESCRIPTOR  *dptr;
	      
	      dptr = malloc_descriptor(type, tptr, bptr);
	      if(type == DESCRIPTOR_GARDEN)
		paint_garden_icon(dptr);
	      if(insert)
		{
		  board.insert.descriptors++;
		  if(board.insert.descriptors > board.insert.limit)
		    {
		      board.insert.limit += 128;
		      board.insert.list =
			  (DESCRIPTOR **)XtRealloc((VOID *)board.insert.list,
			  board.insert.limit * sizeof(DESCRIPTOR *));
		    }
		  board.insert.list[board.insert.descriptors - 1] = dptr;
		}
	      else
		insert_descriptor(dptr, board.list.descriptors, 0);
	      last = type == DESCRIPTOR_COMMENT ? dptr : NULL;
	    }
	  /*}}}*/
	  /*{{{  error?*/
	  if(error)
	    {
	      char      string[80];
	      size_t    length;
	      unsigned  ix;
	      
	      sprintf(string, "Line %d", error_line);
	      length = strlen(string);
	      if(start != line)
		{
		  sprintf(&string[length], " garden %d (lines %d-%d)",
		      (insert ? board.insert.descriptors :
		      board.list.descriptors) - 1, start, line);
		  length += strlen(&string[length]);
		}
	      string[length++] = '\n';
	      string[length] = 0;
	      /*{{{  pop up error box?*/
	      if(!popped)
		{
		  dialog_popup(DIALOG_LOAD_ERROR, NULL, "", NULL, XtGrabNone);
		  popped = 1;
		}
	      /*}}}*/
	      dialog_append(DIALOG_LOAD_ERROR, string);
	      for(ix = 16; ix--;)
		if(error & 1 << ix)
		  {
		    sprintf(string, "%s%s\n", text_error[ix],
			(1 << ix) & ERROR_FIX_MASK ? " (fixed)" : ""); 
		    dialog_append(DIALOG_LOAD_ERROR, string);
		  }
	      error &= ERROR_KEEP_MASK;
	    }
	  /*}}}*/
	}
	/*}}}*/
      while(!error);
      XtFree(text);
    }
  return close_file(stream);
}
/*}}}*/
/*{{{  DESCRIPTOR *malloc_descriptor(type, text, board)*/
static DESCRIPTOR *malloc_descriptor
FUNCARG((type, text, board),
	unsigned  type      /* descriptor type */
ARGSEP  char      *text     /* comment text, or NULL */
ARGSEP  BOARD     *board    /* board or NULL */
)
/* malloc a descriptor, initializing it appropriately
 * The comment text and board are not duplicated
 * The garden pixmap is not initialized, but is allocated
 */
{
  DESCRIPTOR *dptr;
  
  dptr = (DESCRIPTOR *)XtMalloc(sizeof(DESCRIPTOR));
  dptr->type = type;
  dptr->comment = text;
  switch(type)
  {
    case DESCRIPTOR_GARDEN:
    {
      dptr->pixmap = XCreatePixmap(display.display, display.copy,
	  ICON_WIDTH, ICON_HEIGHT, display.depth);
      dptr->board = board;
      break;
    }
    case DESCRIPTOR_COMMENT:
    {
      dptr->pixmap = sprites[SPRITE_BOARD_I].image;
      break;
    }
    case DESCRIPTOR_INCLUDE:
    {
      dptr->pixmap = sprites[SPRITE_INCLUDE].image;
      break;
    }
    default:
      assert(0);
  }
  return dptr;
}
/*}}}*/
/*{{{  void new_descriptors()*/
extern VOIDFUNC new_descriptors FUNCARGVOID
/* setup the default edit descriptors
 * clears the changed flag
 * clears the buffer
 */
{
  unsigned  ix;

  free_descriptor(remove_descriptor(SOURCE_BUFFER));
  for(ix = 3; ix--;)
    {
      free_descriptor(board.edit[ix]);
      assert(board.source[ix] == SOURCE_UNIQUE);
      board.edit[ix] = malloc_descriptor(ix, ix == DESCRIPTOR_INCLUDE ? NULL :
	  XtNewString(initial_text), ix != DESCRIPTOR_GARDEN ? NULL :
	  memcpy(XtMalloc(sizeof(BOARD)), &initial_board[0], sizeof(BOARD)));
      if(ix == DESCRIPTOR_GARDEN)
	paint_garden_icon(board.edit[ix]);
      XtVaSetValues(gizmos[GIZMO_EDIT_BASE + ix].widget,
	  XtNpixmap, (XtArgVal)board.edit[ix]->pixmap, NULL);
    }
  set_garden(board.edit[DESCRIPTOR_GARDEN], SOURCE_UNIQUE, 0);
  changed_flag = 0;
  return;
}
/*}}}*/
/*{{{  void refresh_garden_copy()*/
static VOIDFUNC refresh_garden_copy FUNCARGVOID
/* update the edit garden's source from the edit garden
 */
{
  if(board.source[DESCRIPTOR_GARDEN] != SOURCE_UNIQUE)
    {
      DESCRIPTOR *dptr;
      
      dptr = board.source[DESCRIPTOR_GARDEN] == SOURCE_BUFFER ? board.buffer :
	  board.list.list[board.source[DESCRIPTOR_GARDEN]];
      assert(dptr->type == DESCRIPTOR_GARDEN);
      memcpy(dptr->board, board.edit[DESCRIPTOR_GARDEN]->board, sizeof(BOARD));
      free_dup(&dptr->comment, board.edit[DESCRIPTOR_GARDEN]->comment);
    }
  return;
}
/*}}}*/
/*{{{  DESCRIPTOR *remove_descriptor(place)*/
static DESCRIPTOR *remove_descriptor
FUNCARG((place),
	int  place    /* descriptor to remove */
)
/* removes the specified descriptor
 * If this is the buffer, then the blank buffer is set
 * updates the edit sources as appropriate
 * returns the removed descriptor, this must be freed by the caller
 */
{
  DESCRIPTOR *dptr;
  DESCRIPTOR **ddptr;
  unsigned  ix;
  
  for(ix = 3; ix--;)
    if(board.source[ix] == place)
      {
	board.source[ix] = SOURCE_UNIQUE;
	if(ix == DESCRIPTOR_GARDEN)
	  set_garden_source(SOURCE_UNIQUE, CHANGED_GARDEN);
      }
  if(place == SOURCE_BUFFER)
    {
      dptr = board.buffer;
      if(dptr)
	{
	  XtSetValues(gizmos[GIZMO_BUFFER].widget,
	      arg_buffer, XtNumber(arg_buffer));
	  board.buffer = NULL;
	  changed_flag &= ~CHANGED_BUFFER;
	  if(board.insert.list)
	    {
	      unsigned  ix;
	      
	      free_descriptor(dptr);
	      dptr = NULL;
	      for(ix = board.insert.descriptors; ix--;)
		free_descriptor(board.insert.list[ix]);
	      XtFree((char *)board.insert.list);
	      board.insert.list = NULL;
	      board.insert.descriptors = 0;
	      board.insert.limit = 0;
	    }
	}
    }
  else
    {
      assert(place < board.list.descriptors);
      dptr = board.list.list[place];
      board.list.descriptors--;
      for(ddptr = &board.list.list[place], ix = board.list.descriptors - place;
	  ix--; ddptr++)
	ddptr[0] = ddptr[1];
      for(ix = 3; ix--;)
	if(board.source[ix] > (int)place)
	  {
	    board.source[ix]--;
	    if(ix == DESCRIPTOR_GARDEN)
	      set_garden_source(board.source[ix],
		  CHANGED_ALL | CHANGED_GARDEN);
	  }
      PixmapListRemove(gizmos[GIZMO_GARDENS].widget, place);
      changed_flag |= CHANGED_ALL;
      menu_total(board.list.descriptors);
    }
  return dptr;
}
/*}}}*/
/*{{{  void repaint_garden_icon()*/
extern VOIDFUNC repaint_garden_icon FUNCARGVOID
/* callback from the garden editor to repaint the edit icon,
 * and update the icon of its copy
 */
{
  DESCRIPTOR  *dptr;
  
  IconRepaint(gizmos[GIZMO_GARDEN].widget);
  dptr = board.source[DESCRIPTOR_GARDEN] == SOURCE_BUFFER ? board.buffer :
      board.source[DESCRIPTOR_GARDEN] >= 0 ?
      board.list.list[board.source[DESCRIPTOR_GARDEN]] : NULL;
  if(dptr)
    {
      XCopyArea(display.display, board.edit[DESCRIPTOR_GARDEN]->pixmap,
	  dptr->pixmap, GCN(GC_COPY), 0, 0, ICON_WIDTH, ICON_HEIGHT, 0, 0);
      if(board.source[DESCRIPTOR_GARDEN] == SOURCE_BUFFER)
	IconRepaint(gizmos[GIZMO_BUFFER].widget);
      else
	PixmapListRepaint(gizmos[GIZMO_GARDENS].widget,
	    board.source[DESCRIPTOR_GARDEN]);
    }
  return;
}
/*}}}*/
/*{{{  char CONST *save_boards(filename)*/
extern char CONST *save_boards
FUNCARG((filename),
	char CONST *filename
)
/* save all the descriptors
 * returns NULL or error message
 */
{
  char CONST *error;
  unsigned  count;
  DESCRIPTOR  **ddptr;
  FILE      *stream;
  
  refresh_garden_copy();
  stream = fopen(filename, "w");
  if(stream)
    for(ddptr = board.list.list, count = board.list.descriptors;
	count--; ddptr++)
      {
	DESCRIPTOR  *dptr;
	
	dptr = *ddptr;
	/*{{{  save a board;*/
	switch(dptr->type)
	{
	  /*{{{  case DESCRIPTOR_GARDEN:*/
	  case DESCRIPTOR_GARDEN:
	  {
	    BOARD     *bptr;
	    unsigned  row;
	    
	    bptr = dptr->board;
	    if(dptr->comment)
	      save_comment(stream, dptr->comment, 1);
	    fprintf(stream, "{\n  %d, %d, %d,\n  {\n",
		bptr->fill, bptr->colors, bptr->apples);
	    for(row = 0; row != CELLS_DOWN; row++)
	      {
		fputs("    \"", stream);
		fputs(bptr->map[row], stream);
		fputs("\",\n", stream);
	      }
	    fputs("  }\n},\n", stream);
	    break;
	  }
	  /*}}}*/
	  /*{{{  case DESCRIPTOR_INCLUDE:*/
	  case DESCRIPTOR_INCLUDE:
	    fputs("#include \"", stream);
	    if(dptr->comment)
	      fputs(dptr->comment, stream);
	    fputs("\"\n", stream);
	    break;
	  /*}}}*/
	  /*{{{  case DESCRIPTOR_COMMENT:*/
	  case DESCRIPTOR_COMMENT:
	    if(dptr->comment)
	      save_comment(stream, dptr->comment, 0);
	    break;
	  /*}}}*/
	  default:
	    break;
	}
	/*}}}*/
      }
  error = close_file(stream);
  if(!error)
    {
      changed_flag &= ~CHANGED_ALL;
      for(count = 3; count--;)
	if(board.source[count] >= 0)
	  changed_flag &= ~(1 << count);
    }
  return error;
}
/*}}}*/
/*{{{  void save_comment(stream, comment, flag)*/
static VOIDFUNC save_comment
FUNCARG((stream, comment, flag),
	FILE  *stream
ARGSEP  char CONST *comment
ARGSEP  unsigned  flag /* general or board comment */
)
/* save a comment
 */
{
  size_t    length;
  char CONST *eptr;
  
  fprintf(stream, "/*%c\n", " !"[flag]);
  for(eptr = comment; *eptr; comment = eptr)
    {
      eptr = strchr(comment, '\n');
      if(eptr)
	length = eptr - comment;
      else
	{
	  length = strlen(comment);
	  eptr = comment + length;
	}
      fprintf(stream, " * %.*s\n", (int)length, comment);
      if(*eptr)
	eptr++;
    }
  fputs(" */\n", stream);
  return;
}
/*}}}*/
/*{{{  int strdiff(one, two)*/
static int strdiff
FUNCARG((one, two),
	char CONST *one
ARGSEP  char CONST *two
)
/* compares two possibly NULL strings
 */
{
  if(one == two)
    return 0;
  if(!one || !two)
    return 1;
  return strcmp(one, two);
}
/*}}}*/
/*{{{  DESCRIPTOR *unique_garden()*/
extern DESCRIPTOR *unique_garden FUNCARGVOID
/* sets up for edit of a unique garden
 */
{
  refresh_garden_copy();
  assert(board.edit[DESCRIPTOR_GARDEN]);
  changed_flag &= ~(1 << DESCRIPTOR_GARDEN);
  free_dup(&board.edit[DESCRIPTOR_GARDEN]->comment, NULL);
  board.source[DESCRIPTOR_GARDEN] = SOURCE_UNIQUE;
  return board.edit[DESCRIPTOR_GARDEN];
}
/*}}}*/
