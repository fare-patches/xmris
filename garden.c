/* Copyright (C) 1993, 1992 Nathan Sidwell */
/* RCS $Id: garden.c,v 4.6 1994/01/06 10:20:24 nathan stable $ */
/*{{{  includes*/
#include "xmred.h"
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/SimpleP.h>
/*}}}*/
/*{{{  defines*/
#define WIDTH_EXPAND  (GAP_WIDTH / 2)
#define HEIGHT_EXPAND (GAP_HEIGHT / 2)
/*}}}*/
/*{{{  structs*/
/*{{{  typedef struct _IconClass*/
typedef struct _GardenClass
{
  int     ansi_compliance; /* not used */
} GardenClassPart;
/*}}}*/
/*{{{  typedef struct _GardenClassRec*/
typedef struct _GardenClassRec
{
  CoreClassPart   core_class;
  SimpleClassPart simple_class;
  GardenClassPart garden_class;
} GardenClassRec;
/*}}}*/
/*{{{  typedef struct _GardenPart*/
typedef struct
{
  /* private state */
  Position  x;              /* corner of pixmap */
  Position  y;              /* corner of pixmap */
  COORD     cell;           /* last known cell */
  unsigned  place;          /* place within cell
			     * 0-3 edges */
  COORD     dir;            /* tracking direction */
  unsigned  onoff;          /* adding or removing? */
  int       option;         /* button currently pressed */
} GardenPart;
/*}}}*/
/*{{{  typedef struct _GardenRec*/
typedef struct _GardenRec
{
  CorePart    core;
  SimplePart  simple;
  GardenPart  garden;
} GardenRec;
/*}}}*/
/*}}}*/
/*{{{  statics*/
static Widget garden;
/*}}}*/
/*{{{  prototypes*/
static VOIDFUNC Move PROTOARG((Widget, XEvent *, String *, Cardinal *));
static VOIDFUNC Press PROTOARG((Widget, XEvent *, String *, Cardinal *));
static VOIDFUNC Release PROTOARG((Widget, XEvent *, String *, Cardinal *));

static VOIDFUNC Destroy PROTOARG((Widget));
static VOIDFUNC Initialize PROTOARG((Widget, Widget, ArgList, Cardinal *));
static XtGeometryResult QueryGeometry
    PROTOARG((Widget, XtWidgetGeometry *, XtWidgetGeometry *));
static VOIDFUNC Redisplay PROTOARG((Widget, XEvent *, Region));
static VOIDFUNC Resize PROTOARG((Widget));
static Boolean SetValues
    PROTOARG((Widget, Widget, Widget, ArgList, Cardinal *));

static VOIDFUNC RedrawRect
    PROTOARG((Widget, Position, Position, Dimension, Dimension));

static unsigned do_option
    PROTOARG((GardenWidget, unsigned, unsigned, char *, unsigned));
static VOIDFUNC draw_board_icon
    PROTOARG((DESCRIPTOR CONST *, int, int, unsigned, unsigned));
static unsigned get_cell PROTOARG((GardenWidget, XEvent *, COORD *));
static unsigned get_paths PROTOARG((unsigned, unsigned, char **));
static unsigned long change_path
    PROTOARG((unsigned, unsigned, unsigned, unsigned, char *, unsigned));
static VOIDFUNC cut_back
    PROTOARG((int, int, unsigned, unsigned, int, int, SPRITE CONST *));
static VOIDFUNC cut_sprite PROTOARG((unsigned, int, int));
static unsigned long force_blank PROTOARG((unsigned, unsigned, char *));
static VOIDFUNC paint_cell PROTOARG((unsigned, unsigned, unsigned, unsigned));
static unsigned long set_random PROTOARG((unsigned, unsigned, char *));
/*}}}*/
/*{{{  translations*/
static char translations[] = "\
<BtnDown>:press() move()\n\
<BtnUp>:release()\n\
<BtnMotion>:move()\n\
";
/*}}}*/
/*{{{  actions*/
static XtActionsRec actions[] =
{
  {"move", Move},
  {"press", Press},
  {"release", Release},
};
/*}}}*/
#define SuperClass (WidgetClass)&simpleClassRec
/*{{{  GardenClassRec gardenClassRec =*/
GardenClassRec gardenClassRec =
{
  /*{{{  core class part*/
  {
    SuperClass,                       /* superclass */
    "Garden",                         /* class_name */
    sizeof(GardenRec),                /* size */
    NULL,                             /* class_initialize */
    NULL,                             /* class_part_initialize */
    False,                            /* class_inited */
    Initialize,                       /* initialize */
    NULL,                             /* initialize_hook */
    XtInheritRealize,                 /* realize */
    actions,                          /* actions */
    XtNumber(actions),                /* num_actions */
    NULL,                             /* resources */
    0,                                /* num_resources */
    NULLQUARK,                        /* xrm_class */
    True,                             /* compress_motion */
    XtExposeCompressMultiple,         /* compress_exposure */
    True,                             /* compress_enterleave */
    False,                            /* visible_interest */
    Destroy,                          /* destroy */
    Resize,                           /* resize */
    Redisplay,                        /* expose */
    SetValues,                        /* set_values */
    NULL,                             /* set_values_hook */
    XtInheritSetValuesAlmost,         /* set_values_almost */
    NULL,                             /* get_values_hook */
    NULL,                             /* accept_focus */
    XtVersion,                        /* version */
    NULL,                             /* callback_private */
    translations,                     /* default_translations */
    QueryGeometry,                    /* query_geometry */
    XtInheritDisplayAccelerator,      /* display_accelerator */
    NULL,                             /* extension */
  },
  /*}}}*/
  /*{{{  simple class part*/
  {
    XtInheritChangeSensitive      /* change_sensitive */
  },
  /*}}}*/
  /*{{{  garden class part*/
  {
    0,        /* dummy */
  },
  /*}}}*/
};
/*}}}*/
WidgetClass gardenWidgetClass = (WidgetClass)&gardenClassRec;
/* actions */
/*{{{  void Move(widget, event, params, num_params)*/
static VOIDFUNC Move
FUNCARG((widget, event, params, num_params),
	Widget    widget
ARGSEP  XEvent    *event
ARGSEP  String    *params
ARGSEP  Cardinal  *num_params
)
/* deal with pointer pressed motion events
 */
{
  GardenWidget gw;
  
  gw = (GardenWidget)widget;
  if(event->type == MotionNotify && gw->garden.option >= 0)
    {
      unsigned  place;
      COORD     cell;
      COORD     dir;
      unsigned  cont;
      unsigned  icon;
      
      place = get_cell(gw, event, &cell);
      dir.x = cell.x < gw->garden.cell.x ? -1 :
	  cell.x > gw->garden.cell.x ? 1 : 0;
      dir.y = cell.y < gw->garden.cell.y ? -1 :
	  cell.y > gw->garden.cell.y ? 1 : 0;
      cont = ((dir.x != 0) << 1) | (dir.y != 0);
      icon = 0;
      if(cont)
	/*{{{  moved to new cell*/
	{
	  unsigned  first;
	  
	  first = 1;
	  if(dir.x * gw->garden.dir.x < 0)
	    {
	      gw->garden.dir.x = 0;
	      gw->garden.dir.y = dir.y;
	    }
	  else if(dir.y * gw->garden.dir.y < 0)
	    {
	      gw->garden.dir.x = dir.x;
	      gw->garden.dir.y = 0;
	    }
	  if(!gw->garden.dir.y && !gw->garden.dir.x)
	    {
	      gw->garden.dir.y = dir.x ? 0 : dir.y;
	      gw->garden.dir.x = dir.x;
	    }
	  while(cont)
	    {
	      unsigned  mask;
	      
	      /*{{{  arrived @ x?*/
	      if(cell.x == gw->garden.cell.x)
		{
		  cont &= 1;
		  if(dir.y)
		    {
		      gw->garden.dir.x = 0;
		      gw->garden.dir.y = dir.y;
		    }
		}
	      /*}}}*/
	      /*{{{  arrived @ y?*/
	      if(cell.y == gw->garden.cell.y)
		{
		  cont &= 2;
		  if(dir.x)
		    {
		      gw->garden.dir.x = dir.x;
		      gw->garden.dir.y = 0;
		    }
		}
	      /*}}}*/
	      mask = gw->garden.dir.x < 0 ? 0x8 : gw->garden.dir.x > 0 ? 0x4 :
		  gw->garden.dir.y < 0 ? 0x2 : gw->garden.dir.y > 0 ? 0x1 : 0;
	      assert(mask);
	      if(first)
		mask = (mask ^ 0xF) & gw->garden.place ? 0 : 0x10;
	      else if(cont)
		mask |= 0x10;
	      else if((mask & place) != place)
		mask |= 0x10 | place;
	      if(mask)
		icon |= do_option(gw, 0, mask, NULL, 0);
	      first = 0;
	      if(cont)
		{
		  gw->garden.cell.x += gw->garden.dir.x;
		  gw->garden.cell.y += gw->garden.dir.y;
		}
	    }
	}
	/*}}}*/
      else if(place != gw->garden.place)
	icon = do_option(gw, 0, place | 0x10, NULL, 0);
      gw->garden.place = place;
      if(icon)
	repaint_garden_icon();
    }
  return;
}
/*}}}*/
/*{{{  void Press(widget, event, params, num_params)*/
static VOIDFUNC Press
FUNCARG((widget, event, params, num_params),
	Widget    widget
ARGSEP  XEvent    *event
ARGSEP  String    *params
ARGSEP  Cardinal  *num_params
)
/* deal with button press event,
 * override current button press, fetch option
 * and determine whether adding or removing
 */
{
  GardenWidget gw;
  
  gw = (GardenWidget)widget;
  if(event->type != ButtonPress)
    /* EMPTY */;
  else if((gw->garden.option = state.button[event->xbutton.button - 1]) < 0)
    /* EMPTY */;
  else
    {
      unsigned  place;
      COORD     cell;
      
      place = get_cell(gw, event, &cell);
      if(place)
	{
	  char      *cptr;
	  unsigned  current;
	  
	  save_garden();
	  gw->garden.place = place;
	  gw->garden.cell.x = cell.x;
	  gw->garden.cell.y = cell.y;
	  current = get_paths(cell.x, cell.y, &cptr);
	  /*{{{  set or release?*/
	  switch(gw->garden.option)
	  {
	    /*{{{  case OPTION_APPLES:*/
	    case OPTION_APPLES:
	      gw->garden.onoff = !ISAPPLE(*cptr) ||
		  !(GARDENAPPLE(*cptr) & (1 << state.apple));
	      break;
	    /*}}}*/
	    /*{{{  case OPTION_RANDOM:*/
	    case OPTION_RANDOM:
	      gw->garden.onoff = ISPATH(*cptr) || *cptr == GARDEN_NOAPPLE ||
		  *cptr == GARDEN_CHERRY;
	      break;
	    /*}}}*/
	    /*{{{  case OPTION_CHERRY:*/
	    case OPTION_CHERRY:
	      gw->garden.onoff = *cptr != GARDEN_CHERRY &&
		  !ISPATHCHERRY(*cptr);
	      break;
	    /*}}}*/
	    /*{{{  case OPTION_PATH:*/
	    case OPTION_PATH:
	      gw->garden.onoff = current & place;
	      break;
	    /*}}}*/
	    /*{{{  case OPTION_DEN:*/
	    case OPTION_DEN:
	      gw->garden.onoff = !ISPATHDEN(*cptr);
	      break;
	    /*}}}*/
	    /*{{{  case OPTION_PLAYER:*/
	    case OPTION_PLAYER:
	      gw->garden.onoff = !ISPATHPLAYER(*cptr);
	      break;
	    /*}}}*/
	  }
	  /*}}}*/
	  if(do_option(gw, 1, place, cptr, current))
	    repaint_garden_icon();
	}
    }
  return;
}
/*}}}*/
/*{{{  void Release(widget, event, params, num_params)*/
static VOIDFUNC Release
FUNCARG((widget, event, params, num_params),
	Widget    widget
ARGSEP  XEvent    *event
ARGSEP  String    *params
ARGSEP  Cardinal  *num_params
)
/* deal with button release event
 * if its the current button being released, then
 * set current to none
 */
{
  GardenWidget gw;
  
  gw = (GardenWidget)widget;
  if(event->type == ButtonRelease &&
      state.button[event->xbutton.button - 1] == gw->garden.option)
    gw->garden.option = -1;
  return;
}
/*}}}*/
/* methods */
/*{{{  void Destroy(widget)*/
static VOIDFUNC Destroy
FUNCARG((widget),
	Widget    widget
)
/* free the GC and remove the flash timeout
 */
{
  return;
}
/*}}}*/
/*{{{  void Initialize(treq, tnew, args, num_args)*/
static VOIDFUNC Initialize
FUNCARG((treq, tnew, args, num_args),
	Widget    treq
ARGSEP  Widget    tnew
ARGSEP  ArgList   args
ARGSEP  Cardinal  *num_args
)
/* set out default size
 */
{
  GardenWidget ngw;
  
  ngw = (GardenWidget)tnew;
  ngw->garden.x = ngw->garden.y = 0;
  ngw->garden.place = 0;
  ngw->garden.dir.x = ngw->garden.dir.y = 0;
  ngw->garden.option = -1;
  if(!ngw->core.width)
    ngw->core.width = WINDOW_WIDTH;
  if(!ngw->core.height)
    ngw->core.height = WINDOW_HEIGHT;
  return;
}
/*}}}*/
/*{{{  XtGeometryResult QueryGeometry(widget, proposed, answer)*/
static XtGeometryResult QueryGeometry
FUNCARG((widget, proposed, answer),
	Widget    widget
ARGSEP  XtWidgetGeometry *proposed
ARGSEP  XtWidgetGeometry *answer
)
/* tell our parent what size we'd like to be
 */
{
  GardenWidget gw;
  
  gw = (GardenWidget)widget;
  answer->request_mode = CWWidth | CWHeight;
  answer->width = WINDOW_WIDTH;
  answer->height = WINDOW_HEIGHT;
  if((proposed->request_mode & (CWWidth | CWHeight)) == (CWWidth | CWHeight) &&
      proposed->width == answer->width && proposed->height == answer->height)
    return XtGeometryYes;
  else if(answer->width == gw->core.width && answer->height == gw->core.height)
    return XtGeometryNo;
  else
    return XtGeometryAlmost;
}
/*}}}*/
/*{{{  void Redisplay(widget, event, region)*/
static VOIDFUNC Redisplay
FUNCARG((widget, event, region),
	Widget    widget
ARGSEP  XEvent    *event
ARGSEP  Region    region
)
/* repaint ourselves
 */
{
  GardenWidget  gw;
  XRectangle    rect;
  
  if(!XtIsRealized(widget))
    return;
  gw = (GardenWidget)widget;
  XClipBox(region, &rect);
  XCopyArea(display.display, display.copy, XtWindow((Widget)gw), GCN(GC_COPY),
      rect.x - gw->garden.x, rect.y - gw->garden.y, rect.width, rect.height,
      rect.x, rect.y);
  return;
}
/*}}}*/
/*{{{  void Resize(widget)*/
static VOIDFUNC Resize
FUNCARG((widget),
	Widget    widget
)
/* recenter the pixmap, when we get resized
 */
{
  GardenWidget gw;
  
  gw = (GardenWidget)widget;
  gw->garden.x = (gw->core.width - WINDOW_WIDTH) / 2;
  gw->garden.y = (gw->core.height - WINDOW_HEIGHT) / 2;
  return;
}
/*}}}*/
/*{{{  Boolean SetValues(cw, rw, nw, args, num_args)*/
static Boolean SetValues
FUNCARG((cw, rw, nw, args, num_args),
	Widget    cw
ARGSEP  Widget    rw
ARGSEP  Widget    nw
ARGSEP  ArgList   args
ARGSEP  Cardinal  *num_args
)
{
  return False;
}
/*}}}*/
/* public routines */
/*{{{  void RedrawRect(widget, x, y, w, h)*/
static VOIDFUNC RedrawRect
FUNCARG((widget, x, y, w, h),
	Widget    widget
ARGSEP  Position  x
ARGSEP  Position  y
ARGSEP  Dimension w
ARGSEP  Dimension h
)
{
  GardenWidget  gw;
  
  if(!XtIsRealized(widget))
    return;
  gw = (GardenWidget)widget;
  XCopyArea(display.display, display.copy, XtWindow((Widget)gw), GCN(GC_COPY),
      x, y, w, h, gw->garden.x + x, gw->garden.y + y);
  return;
}
/*}}}*/
/* non-widget routines */
/*{{{  unsigned long change_path(x, y, mask, set, cptr, current)*/
static unsigned long change_path
FUNCARG((x, y, mask, set, cptr, current),
	unsigned  x     /* cell to changed */
ARGSEP  unsigned  y
ARGSEP  unsigned  mask  /* mask to change
			 * 0..3 edges,
			 * 4 center,
			 * 5 place random
			 * 6 clear cherry */
ARGSEP  unsigned  set   /* set or clear those specified */
ARGSEP  char      *cptr /* map pointer */
ARGSEP  unsigned  current /* current paths */
)
/* sets or clears the specified parts of a cell.
 * forces the center to blank
 * clears the center of adjacent cells if wall is cleared (by recursive call)
 * returns a mask specifying which bits should be redrawn.
 * 0..3 edges
 * 4..7 adjacent centers
 * 8 center
 * 9..11 above, left and right tops edges
 * 13..15 above, left and right above centers
 * 16..19 adjacent centers changed
 */
{
  unsigned long redraw;
  unsigned  random;
  unsigned  cherry;
  unsigned  cleared;
  
  random = mask & 0x20;
  redraw = cherry = 0;
  cleared = mask & 0x40;
  if(cleared)
    redraw = force_blank(x, y, cptr);
  /*{{{  clip*/
  {
    if(!x)
      mask &= ~4;
    if(!y)
      mask &= ~1;
    if(x == CELLS_ACROSS - 1)
      mask &= ~8;
    if(y == CELLS_DOWN - 1)
      mask &= ~2;
  }
  /*}}}*/
  if(set)
    {
      if(mask & 0x10)
	mask |= 0xF;
      mask &= current;
    }
  else
    {
      if(mask & 0x0F)
	mask |= 0x10;
      mask &= ~current;
    }
  if(mask & 0x10)
    {
      cherry = *cptr == GARDEN_CHERRY || ISPATHCHERRY(*cptr);
      if(cherry)
	random = 0;
      else if(!cleared)
	redraw = force_blank(x, y, cptr);
    }
  if(mask & 0x10 && !y && INRANGE(x, 4, 8))
    redraw |= 0x1;
  if(mask)
    changed_flag |= state.change;
  /*{{{  clear center?*/
  if(!set && mask & 0x10)
    {
      redraw |= 0x100;
      if(y && (cptr[-1 - CELLS_ACROSS] == GARDEN_RANDOM ||
	  ISAPPLE(cptr[-1 - CELLS_ACROSS])))
	{
	  adjust_count(COUNT_FALL, 1);
	  redraw |= 0x10;
	}
      *cptr = cherry ? GARDEN_PATH + GARDEN_PATH_CHERRY : GARDEN_PATH;
    }
  /*}}}*/
  /*{{{  change above?*/
  if(mask & 0x01)
    {
      if(!set && !ISPATH(cptr[-1 - CELLS_ACROSS]))
	{
	  redraw |= (change_path(x, y - 1, 0x10, 0,
	      cptr - CELLS_ACROSS - 1, get_paths(x, y - 1, NULL)) & 0x11) << 9;
	  redraw |= 0x10010;
	}
      if(ISPATHDEN(cptr[-1 - CELLS_ACROSS]))
	redraw |= 0x10;
      assert(ISPATH(*cptr) && y);
      redraw |= 0x01;
      cptr[-1 - CELLS_ACROSS] =
	  (GARDENPATH(cptr[-1 - CELLS_ACROSS]) ^ 1) + GARDEN_PATH;
      if(!(GARDENPATH(*cptr) & 2) || !x || !ISPATH(cptr[-1]) ||
	  !(GARDENPATH(cptr[-1]) & 2))
	redraw |= 0x100;
      if(!(GARDENPATH(cptr[-1 - CELLS_ACROSS]) & 2) || !x ||
	  !ISPATH(cptr[-2 - CELLS_ACROSS]) ||
	  !(GARDENPATH(cptr[-2 - CELLS_ACROSS]) & 2))
	redraw |= 0x10;
    }
  /*}}}*/
  /*{{{  change below?*/
  if(mask & 0x02)
    {
      if(!set && !ISPATH(cptr[1 + CELLS_ACROSS]))
	{
	  change_path(x, y + 1, 0x10, 0,
	      cptr + CELLS_ACROSS + 1, get_paths(x, y + 1, NULL));
	  redraw |= 0x20020;
	}
      if(ISPATHDEN(cptr[1 + CELLS_ACROSS]))
	redraw |= 0x20;
      assert(ISPATH(*cptr) && y != CELLS_DOWN - 1);
      redraw |= 0x02;
      *cptr = (GARDENPATH(*cptr) ^ 1) + GARDEN_PATH;
      if(!(GARDENPATH(*cptr) & 2) || !x || !ISPATH(cptr[-1]) ||
	  !(GARDENPATH(cptr[-1]) & 2))
	redraw |= 0x100;
      if(!(GARDENPATH(cptr[1 + CELLS_ACROSS]) & 2) || !x ||
	  !ISPATH(cptr[CELLS_ACROSS]) ||
	    !(GARDENPATH(cptr[CELLS_ACROSS]) & 2))
	redraw |= 0x20;
    }
  /*}}}*/
  /*{{{  change left?*/
  if(mask & 0x04)
    {
      if(!set && !ISPATH(cptr[-1]))
	{
	  redraw |= (change_path(x - 1, y, 0x10, 0,
	      cptr - 1, get_paths(x - 1, y, NULL)) & 0x11) << 10;
	  redraw |= 0x40040;
	}
      if(ISPATHDEN(cptr[-1]))
	redraw |= 0x40;
      assert(x && ISPATH(*cptr));
      redraw |= 0x04;
      cptr[-1] = (GARDENPATH(cptr[-1]) ^ 2) + GARDEN_PATH;
      if(!(GARDENPATH(*cptr) & 1) || !y || !ISPATH(cptr[-1 - CELLS_ACROSS]) ||
	  !(GARDENPATH(cptr[-1 - CELLS_ACROSS]) & 1))
	redraw |= 0x100;
      if(!(GARDENPATH(cptr[-1]) & 1) || !y ||
	  !ISPATH(cptr[-2 - CELLS_ACROSS]) ||
	  !(GARDENPATH(cptr[-2 - CELLS_ACROSS]) & 1))
	redraw |= 0x40;
    }
  /*}}}*/
  /*{{{  change right?*/
  if(mask & 0x08)
    {
      if(!set && !ISPATH(cptr[1]))
	{
	  redraw |= (change_path(x + 1, y, 0x10, 0,
	      cptr + 1, get_paths(x + 1, y, NULL)) & 0x011) << 11;
	  redraw |= 0x80080;
	}
      if(ISPATHDEN(cptr[1]))
	redraw |= 0x80;
      assert(ISPATH(*cptr) && x != CELLS_ACROSS - 1);
      redraw |= 0x08;
      *cptr = (GARDENPATH(*cptr) ^ 2) + GARDEN_PATH;
      if(!(GARDENPATH(*cptr) & 1) || !y || !ISPATH(cptr[-1 - CELLS_ACROSS]) ||
	  !(GARDENPATH(cptr[-1 - CELLS_ACROSS]) & 1))
	redraw |= 0x100;
      if(!(GARDENPATH(cptr[1]) & 1) || !y || !ISPATH(cptr[-CELLS_ACROSS]) ||
	  !(GARDENPATH(cptr[-CELLS_ACROSS]) & 1))
	redraw |= 0x80;
    }
  /*}}}*/
  /*{{{  set center?*/
  if(set && mask & 0x10)
    {
      redraw |= 0x100;
      assert((ISPATHBLANK(*cptr) || ISPATHCHERRY(*cptr)) &&
	  !(GARDENPATH(*cptr) & 3));
      *cptr = cherry ? GARDEN_CHERRY : GARDEN_NOAPPLE;
      if(random)
	redraw |= set_random(x, y, cptr);
      if(y && (cptr[-1 - CELLS_ACROSS] == GARDEN_RANDOM ||
	  ISAPPLE(cptr[-1 - CELLS_ACROSS])))
	{
	  adjust_count(COUNT_FALL, -1);
	  redraw |= 0x10;
	}
    }
  /*}}}*/
  return redraw;
}
/*}}}*/
/*{{{  void cut_back(sx, sy, width, height, dx, dy, sprite)*/
static VOIDFUNC cut_back
FUNCARG((sx, sy, width, height, dx, dy, sprite),
	int       sx            /* offset within source sprite */
ARGSEP  int       sy
ARGSEP  unsigned  width         /* size of blat */
ARGSEP  unsigned  height
ARGSEP  int       dx            /* destination address */
ARGSEP  int       dy
ARGSEP  SPRITE CONST *sprite    /* sprite to blat on */
)
/*
 * munches the background image with the specified sprite
 */
{
  if(display.background != COLOUR_ONE)
    XCopyArea(display.display, sprite->mask, display.copy, GCN(GC_MASK),
	sx, sy, width, height, dx, dy);
  if(display.background != COLOUR_ZERO)
    XCopyArea(display.display, sprite->image, display.copy, GCN(GC_OR),
	sx, sy, width, height, dx, dy);
  return;
}
/*}}}*/
/*{{{  void cut_sprite(ix, flag, x, y)*/
static VOIDFUNC cut_sprite
FUNCARG((ix, x, y),
	unsigned  ix
ARGSEP  int       x
ARGSEP  int       y
)
/*
 * copy a sprite onto the background
 */
{
  SPRITE CONST *sptr;
  
  sptr = &sprites[ix];
  if(!INRANGE(ix, SPRITE_CENTER_BASE, 4) || display.background != COLOUR_ZERO)
    XCopyArea(display.display, sptr->mask, display.copy, GCN(GC_MASK),
	0, 0, sptr->size.x, sptr->size.y, x, y);
  XCopyArea(display.display, sptr->image, display.copy, GCN(GC_OR),
      0, 0, sptr->size.x, sptr->size.y, x, y);
  return;
}
/*}}}*/
/*{{{  unsigned do_option(gw, start, place, cptr, current)*/
static unsigned do_option
FUNCARG((gw, start, place, cptr, current),
	GardenWidget gw
ARGSEP  unsigned  start   /* first action of sequence */
ARGSEP  unsigned  place   /* place mask on cell */
ARGSEP  char      *cptr   /* char pointer to map (calculated if NULL) */
ARGSEP  unsigned  current /* current walls (calculated if cptr is NULL) */
)
/* perform an option on the current garden cell,
 * using the specified place mask.
 * the caller must deal with tracking
 * returns flag which specifies if the icon needs refreshing
 */
{
  unsigned long redraw;
  
  redraw = 0;
  assert(gw->garden.option >= 0);
  if(!cptr)
    current = get_paths(gw->garden.cell.x, gw->garden.cell.y, &cptr);
  /*{{{  perform the option*/
  switch(gw->garden.option)
  {
    /*{{{  case OPTION_APPLES:*/
    case OPTION_APPLES:
    {
      if(!(start || place & 0x10))
	/* EMPTY */;
      else if(gw->garden.onoff)
	{
	  redraw = change_path(gw->garden.cell.x, gw->garden.cell.y, 0x10,
	      1, cptr, current);
	  if(*cptr == GARDEN_CHERRY)
	    force_blank(gw->garden.cell.x, gw->garden.cell.y, cptr);
	  redraw |= set_random(gw->garden.cell.x, gw->garden.cell.y, cptr);
	  if(!ISAPPLE(*cptr)) 
	    *cptr = GARDEN_APPLE;
	  if(!(GARDENAPPLE(*cptr) & (1 << state.apple)))
	    {
	      *cptr += 1 << state.apple;
	      adjust_count(COUNT_APPLES + state.apple, 1);
	    }
	  redraw |= 0x100;
	}
      else if(ISAPPLE(*cptr) && GARDENAPPLE(*cptr) & (1 << state.apple))
	{
	  *cptr -= 1 << state.apple;
	  adjust_count(COUNT_APPLES + state.apple, -1);
	  if(*cptr == GARDEN_APPLE)
	    *cptr = GARDEN_RANDOM;
	  changed_flag |= state.change;
	  redraw = 0x100;
	}
      break;
    }
    /*}}}*/
    /*{{{  case OPTION_RANDOM;*/
    case OPTION_RANDOM:
    {
      if(!(start || place & 0x10))
	/* EMPTY */;
      else if(gw->garden.onoff)
	{
	  redraw = change_path(gw->garden.cell.x, gw->garden.cell.y,
	      0x5F, 1, cptr, current);
	  redraw |= set_random(gw->garden.cell.x, gw->garden.cell.y, cptr);
	}
      else if(ISAPPLE(*cptr) || *cptr == GARDEN_RANDOM)
	redraw = force_blank(gw->garden.cell.x, gw->garden.cell.y, cptr);
      break;
    }
    /*}}}*/
    /*{{{  case OPTION_CHERRY:*/
    case OPTION_CHERRY:
    {
      if(!(start || place & 0x10))
	/* EMPTY */;
      else if(gw->garden.onoff)
	{
	  if(*cptr == GARDEN_CHERRY || ISPATHCHERRY(*cptr))
	    /* EMPTY */;
	  else
	    {
	      redraw = force_blank(gw->garden.cell.x, gw->garden.cell.y, cptr);
	      if(ISPATH(*cptr))
		*cptr += GARDEN_PATH_CHERRY - GARDEN_PATH_BLANK;
	      else
		*cptr = GARDEN_CHERRY;
	      adjust_count(COUNT_CHERRY, 1);
	      redraw |= 0x100;
	      changed_flag |= state.change;
	    }
	}
      else if(*cptr == GARDEN_CHERRY || ISPATHCHERRY(*cptr))
	{
	  redraw = force_blank(gw->garden.cell.x, gw->garden.cell.y, cptr);
	  redraw |= set_random(gw->garden.cell.x, gw->garden.cell.y, cptr);
	}
      break;
    }
    /*}}}*/
    /*{{{  case OPTION_PATH:*/
    case OPTION_PATH:
    {
      redraw = change_path(gw->garden.cell.x, gw->garden.cell.y,
	  place | 0x20, gw->garden.onoff, cptr, current);
      break;
    }
    /*}}}*/
    /*{{{  case OPTION_DEN:*/
    case OPTION_DEN:
    {
      if(!(start || place & 0x10))
	/* EMPTY */;
      else if(gw->garden.onoff)
	{
	  if(ISPATHDEN(*cptr))
	    /* EMPTY */;
	  else
	    {
	      redraw = change_path(gw->garden.cell.x, gw->garden.cell.y,
		  0x50, 0, cptr, current);
	      assert(ISPATHBLANK(*cptr));
	      *cptr += GARDEN_PATH_DEN - GARDEN_PATH_BLANK;
	      adjust_count(COUNT_DEN, 1);
	      redraw |= 0x100;
	      changed_flag |= state.change;
	    }
	}
      else if(ISPATHDEN(*cptr))
	redraw = force_blank(gw->garden.cell.x, gw->garden.cell.y, cptr);
      break;
    }
    /*}}}*/
    /*{{{  case OPTION_PLAYER:*/
    case OPTION_PLAYER:
    {
      if(!(start || place & 0x10))
	/* EMPTY */;
      else if(gw->garden.onoff)
	{
	  if(ISPATHPLAYER(*cptr))
	    /* EMPTY */;
	  else
	    {
	      redraw = change_path(gw->garden.cell.x, gw->garden.cell.y,
		  0x50, 0, cptr, current);
	      assert(ISPATHBLANK(*cptr));
	      *cptr += GARDEN_PATH_PLAYER - GARDEN_PATH_BLANK;
	      adjust_count(COUNT_PLAYER, 1);
	      redraw |= 0x100;
	      changed_flag |= state.change;
	    }
	}
      else if(ISPATHPLAYER(*cptr))
	redraw = force_blank(gw->garden.cell.x, gw->garden.cell.y, cptr);
      break;
    }
    /*}}}*/
    default:
      assert(0);
  }
  /*}}}*/
  /*{{{  update display.copy*/
  {
    /*{{{  typedef struct Entry*/
    typedef struct Entry
    {
      COORD     offset;   /* cell offset */
      unsigned  mask;     /* top edge mask */
    } ENTRY;
    /*}}}*/
    /*{{{  static ENTRY CONST table[] =*/
    static ENTRY CONST table[] =
    {
      {{0, -1}, 0x200},
      {{0, 1},  0x000},
      {{-1, 0}, 0x400},
      {{1, 0},  0x800},
    };
    /*}}}*/
    /*{{{  static COORD CONST offsets[] =*/
    static COORD CONST offsets[] =
    {
      { 0, -2},
      {-1, -1},
      { 1, -1},
    };
    /*}}}*/
    unsigned  ix;
    ENTRY CONST *tptr;
    COORD CONST *optr;
    
    if(redraw & 0x10F)
      paint_cell(gw->garden.cell.x, gw->garden.cell.y,
	  (redraw & 0xF) | ((redraw >> 4) & 0x10), 1);
    for(tptr = &table[XtNumber(table) - 1], ix = XtNumber(table); ix--; tptr--)
      if(redraw & (0x10 << ix))
	paint_cell(gw->garden.cell.x + tptr->offset.x,
	    gw->garden.cell.y + tptr->offset.y,
	    redraw & tptr->mask ? 0x11 : 0x10, 1);
    for(optr = &offsets[XtNumber(offsets) - 1], ix = XtNumber(offsets);
	ix--; optr--)
      if(redraw & (0x2000 << ix))
	paint_cell(gw->garden.cell.x + optr->x, gw->garden.cell.y + optr->y,
	    0x10, 1);
    draw_board_icon(state.edit, gw->garden.cell.x - ((redraw & 0x44000) != 0),
	gw->garden.cell.y - ((redraw & 0x10000) != 0),
	1 + ((redraw & 0x44000) != 0) + ((redraw & 0x88000) != 0),
	1 + ((redraw & 0x10000) != 0) + ((redraw & 0x20000) != 0));
  }
  /*}}}*/
  return redraw;
}
/*}}}*/
/*{{{  void draw_board_icon(dptr, x, y, w, h)*/
static VOIDFUNC draw_board_icon
FUNCARG((dptr, x, y, w, h),
	DESCRIPTOR CONST *dptr
ARGSEP  int       x
ARGSEP  int       y
ARGSEP  unsigned  w
ARGSEP  unsigned  h
)
/* draws the specified board icon cells and their edges
 */
{
  BOARD     *board;
  Pixmap    pixmap;
  char      *cptr;
  unsigned  row;
  unsigned  column;
  unsigned  ix;
  XGCValues gcv;
  
  assert(dptr && dptr->type == DESCRIPTOR_GARDEN && dptr->board);
  board = dptr->board;
  pixmap = dptr->pixmap;
  gcv.fill_style = FillSolid;
  gcv.foreground = data.mono != False ? display.black :
      colors[backgrounds[board->colors][1]].pixel;
  XChangeGC(display.display, GCN(GC_BOARD), GCFillStyle | GCForeground, &gcv);
  if(x)
    {
      x--;
      w++;
    }
  if(y)
    {
      y--;
      h++;
    }
  XFillRectangle(display.display, pixmap, GCN(GC_BOARD),
      x * 3, y * 3 + 3, w * 3, h * 3);
  for(row = y; h--; row++)
    for(cptr = &board->map[row][x], column = x, ix = w; ix--; column++, cptr++)
      /*{{{  do a cell*/
      {
	XRectangle rectangles[3];
	unsigned  n;
	unsigned  mask;
	char      c;
	
	c = *cptr;
	if(ISPATH(c))
	  mask = 4 | (GARDENPATH(c) & 3);
	else
	  mask = 0;
	if(mask == 0x7 && ISPATH(cptr[1]) && GARDENPATH(cptr[1]) & 1 &&
	    ISPATH(cptr[CELLS_ACROSS + 1]) &&
	      GARDENPATH(cptr[CELLS_ACROSS + 1]) & 2)
	  mask |= 8;
	n = 0;
	/*{{{  all blank?*/
	if(mask == 0xF)
	  {
	    rectangles[n].x = column * 3;
	    rectangles[n].y = row * 3 + 3;
	    rectangles[n].width = 3;
	    rectangles[n].height = 4;
	    n++;
	    mask = 0;
	  }
	/*}}}*/
	/*{{{  visit and right?*/
	if((mask & 0x6) == 0x6)
	  {
	    rectangles[n].x = column * 3;
	    rectangles[n].y = row * 3 + 3;
	    rectangles[n].width = 3;
	    rectangles[n].height = 2;
	    n++;
	    mask ^= 0x6;
	  }
	/*}}}*/
	/*{{{  visit and down?*/
	if((mask & 0x5) == 0x5)
	  {
	    rectangles[n].x = column * 3;
	    rectangles[n].y = row * 3 + 3;
	    rectangles[n].width = 2;
	    rectangles[n].height = 3;
	    n++;
	    mask ^= 0x5;
	  }
	/*}}}*/
	/*{{{  visit?*/
	if(mask & 0x4)
	  {
	    rectangles[n].x = column * 3;
	    rectangles[n].y = row * 3 + 3;
	    rectangles[n].width = 2;
	    rectangles[n].height = 2;
	    n++;
	    mask ^= 0x4;
	  }
	/*}}}*/
	/*{{{  down?*/
	if(mask & 0x1)
	  {
	    rectangles[n].x = column * 3;
	    rectangles[n].y = row * 3 + 3 + 2;
	    rectangles[n].width = 2;
	    rectangles[n].height = 1;
	    n++;
	    mask ^= 0x1;
	  }
	/*}}}*/
	XFillRectangles(display.display, pixmap, GCN(GC_CLEAR),
	    rectangles, n);
	assert(!mask);
	/*{{{  cherry?*/
	if(c == GARDEN_CHERRY || ISPATHCHERRY(c))
	  {
	    XPoint points[2];
	    
	    points[0].x = column * 3;
	    points[0].y = row * 3 + 3 + 1;
	    points[1].x = 1;
	    points[1].y = -1;
	    XDrawPoints(display.display, pixmap,
		GCN(ISPATH(c) ? GC_SET : GC_CLEAR), points, 2,
		CoordModePrevious);
	  }
	/*}}}*/
      }
      /*}}}*/
  return;
}
/*}}}*/
/*{{{  unsigned long force_blank(x, y, cptr*/
static unsigned long force_blank
FUNCARG((x, y, cptr),
	unsigned  x
ARGSEP  unsigned  y
ARGSEP  char      *cptr
)
/* sets the cell to blank path or noapple hedge
 * adjusts the counts appropriately
 * returns mask of parts to redraw, a la change_path
 */
{
  unsigned long redraw;
  
  redraw = 0;
  if(ISPATH(*cptr) && !ISPATHBLANK(*cptr))
    /*{{{  set to blank path*/
    {
      unsigned  count;
      unsigned  delta;
      
      if(ISPATHCHERRY(*cptr))
	{
	  count = COUNT_CHERRY;
	  delta = GARDEN_PATH_CHERRY - GARDEN_PATH_BLANK;
	  redraw = 0x100;
	}
      else if(ISPATHDEN(*cptr))
	{
	  count = COUNT_DEN;
	  delta = GARDEN_PATH_DEN - GARDEN_PATH_BLANK;
	  redraw = 0x10F;
	  if(y && ISPATHDEN(cptr[-1 - CELLS_ACROSS]))
	    redraw |= 0x10;
	  else if(y != CELLS_DOWN - 1 && ISPATHDEN(cptr[1 + CELLS_ACROSS]))
	    redraw |= 0x20;
	  if(x && ISPATHDEN(cptr[-1]))
	    redraw |= 0x40;
	  else if(x != CELLS_ACROSS - 1 && ISPATHDEN(cptr[1]))
	    redraw |= 0x80;
	}
      else if(ISPATHPLAYER(*cptr))
	{
	  count = COUNT_PLAYER;
	  delta = GARDEN_PATH_PLAYER - GARDEN_PATH_BLANK;
	  redraw = 0x100;
	}
      else
	{
	  assert(0);
	  delta = count = 0; /* keep compiler happy */
	}
      adjust_count(count, -1);
      *cptr -= delta;
      changed_flag |= state.change;
    }
    /*}}}*/
  else if(*cptr == GARDEN_CHERRY)
    /*{{{  set to noapple hedge*/
    {
      *cptr = GARDEN_NOAPPLE;
      adjust_count(COUNT_CHERRY, -1);
      changed_flag |= state.change;
      redraw = 0x100;
    }
    /*}}}*/
  else if(ISAPPLE(*cptr))
    /*{{{  set to random hedge*/
    {
      unsigned  ix;
      unsigned  mask;
      
      mask = GARDENAPPLE(*cptr);
      for(ix = 4; ix--;)
	if(mask & (1 << ix))
	  adjust_count(COUNT_APPLES + ix, -1);
      *cptr = GARDEN_RANDOM;
      changed_flag |= state.change;
      redraw = 0x100;
    }
    /*}}}*/
  if(*cptr == GARDEN_RANDOM)
    /*{{{  set to noapple hedge*/
    {
      *cptr = GARDEN_NOAPPLE;
      adjust_count(COUNT_SPACES, -1);
      if(y != CELLS_DOWN && ISPATH(cptr[CELLS_ACROSS + 1]))
	adjust_count(COUNT_FALL, -1);
      changed_flag |= state.change;
      redraw = 0x100;
    }
    /*}}}*/
  return redraw;
}
/*}}}*/
/*{{{  unsigned get_cell(gw, event, cell_ptr)*/
static unsigned get_cell
FUNCARG((gw, event, cptr),
	GardenWidget gw
ARGSEP  XEvent    *event
ARGSEP  COORD     *cptr
)
/* get the cell and offset from window coordinate
 * returns mask of edges we're on and 0x10 for center
 */
{
  COORD     coord;
  unsigned  place;
  
  /*{{{  get coordinate*/
  switch(event->type)
  {
    case ButtonPress:
    case ButtonRelease:
      coord.x = event->xbutton.x;
      coord.y = event->xbutton.y;
      break;
    case MotionNotify:
      coord.x = event->xmotion.x;
      coord.y = event->xmotion.y;
      break;
    default:
      assert(0);
  }
  /*}}}*/
  coord.x -= gw->garden.x + BORDER_LEFT + GAP_WIDTH / 2;
  coord.y -= gw->garden.y + BORDER_TOP + GAP_HEIGHT / 2;
  place = 0;
  /*{{{  clip x*/
  if(coord.x < 0)
    {
      coord.x = 0;
      place = 0x10;
    }
  else if(coord.x >= (CELL_WIDTH + GAP_WIDTH) * CELLS_ACROSS)
    {
      coord.x = (CELL_WIDTH + GAP_WIDTH) * CELLS_ACROSS - 1;
      place = 0x10;
    }
  /*}}}*/
  /*{{{  clip y*/
  if(coord.y < 0)
    {
      coord.y = 0;
      place = 0x10;
    }
  else if(coord.y >= (CELL_HEIGHT + GAP_HEIGHT) * CELLS_DOWN)
    {
      coord.y = (CELL_HEIGHT + GAP_HEIGHT) * CELLS_DOWN - 1;
      place = 0x10;
    }
  /*}}}*/
  cptr->x = coord.x / (CELL_WIDTH + GAP_WIDTH);
  cptr->y = coord.y / (CELL_HEIGHT + GAP_HEIGHT);
  coord.x %= (CELL_WIDTH + GAP_WIDTH);
  coord.y %= (CELL_HEIGHT + GAP_HEIGHT);
  if(coord.x < GAP_WIDTH / 2 + WIDTH_EXPAND)
    place |= 0x04;
  else if(coord.x >= GAP_WIDTH / 2 + CELL_WIDTH - WIDTH_EXPAND)
    place |= 0x08;
  if(coord.y < GAP_HEIGHT / 2 + HEIGHT_EXPAND)
    place |= 0x01;
  else if(coord.y >= GAP_HEIGHT / 2 + CELL_HEIGHT - HEIGHT_EXPAND)
    place |= 0x02;
  if(place & 0x10)
    place = 0;
  else if(!place)
    place = 0x10;
  return place;
}
/*}}}*/
/*{{{  unsigned get_paths(x, y, cptrptr)*/
static unsigned get_paths
FUNCARG((x, y, cptrptr),
	unsigned  x           /* the cell coordinate */
ARGSEP  unsigned  y
ARGSEP  char      **cptrptr   /* return for cell mapp pointer */
)
/* return the current cell walls and pointer to cell map character
 */
{
  unsigned  current;
  char      *cptr;

  cptr = &state.edit->board->map[y][x];
  if(cptrptr)
    *cptrptr = cptr;
  current = 0;
  /*{{{  set current paths*/
  if(ISPATH(*cptr))
    {
      current |= 0x10;
      current |= (GARDENPATH(*cptr) & 1) << 1;
      current |= (GARDENPATH(*cptr) & 2) << 2;
      if(y && ISPATH(cptr[-1 - CELLS_ACROSS]))
	current |= GARDENPATH(cptr[-1 - CELLS_ACROSS]) & 1;
      if(x && ISPATH(cptr[-1]))
	current |= (GARDENPATH(cptr[-1]) & 2) << 1;
    }
  /*}}}*/
  return current;
}
/*}}}*/
/*{{{  void install_garden(root)*/
extern VOIDFUNC install_garden
FUNCARG((root),
	Widget    root
)
{
  garden = root;
  return;
}
/*}}}*/
/*{{{  void paint_cell(x, y, mask, copy)*/
static VOIDFUNC paint_cell
FUNCARG((x, y, mask, copy),
	unsigned  x       /* cell coordinates */
ARGSEP  unsigned  y
ARGSEP  unsigned  mask    /* parts to paint
			   * 0-3 edges, 4 center
			   */
ARGSEP  unsigned  copy    /* copy to window */
)
/* paints the specified cell from scratch,
 * updates display.copy
 * this will have to be copied to display.window
 */
{
  char CONST *cptr;
  COORD     pixel;
  unsigned  walls;
  
  assert(x < CELLS_ACROSS && y < CELLS_DOWN);
  if(!mask)
    return;
  pixel.x = PIXELX(x, 0);
  pixel.y = PIXELY(y, 0);
  cptr = &state.edit->board->map[y][x];
  /*{{{  set board GC*/
  {
    XGCValues gcv;
    
    gcv.fill_style = FillOpaqueStippled;
    gcv.background = data.mono != False ? display.white :
	colors[backgrounds[state.edit->board->colors][0]].pixel;
    gcv.foreground = data.mono != False ? display.black :
	colors[backgrounds[state.edit->board->colors][1]].pixel;
    gcv.stipple = fills[state.edit->board->fill].mask;
    XChangeGC(display.display, GCN(GC_BOARD),
	GCFillStyle | GCForeground | GCBackground | GCStipple, &gcv);
  }
  /*}}}*/
  /*{{{  set walls*/
  {
    unsigned  temp;
    unsigned  paths;
  
    paths = 0;
    if(ISPATH(cptr[0]))
      {
	temp = GARDENPATH(cptr[0]) & 3;
	paths |= (temp & 2) << 2 | (temp & 1) << 1;
      }
    if(x && ISPATH(cptr[-1]))
      {
	temp = GARDENPATH(cptr[-1]) & 3;
	paths |= (temp & 2) << 1 | (temp & 1) << 10;
      }
    if(y && ISPATH(cptr[-1 - CELLS_ACROSS]))
      {
	temp = GARDENPATH(cptr[-1 - CELLS_ACROSS]) & 3;
	paths |= (temp & 1) | (temp & 2) << 7;
      }
    if(y && x && ISPATH(cptr[-2 - CELLS_ACROSS]))
      {
	temp = GARDENPATH(cptr[-2 - CELLS_ACROSS]) & 3;
	paths |= (temp & 1) << 6 | (temp & 2) << 3;
      }
    if(x != CELLS_ACROSS - 1 && ISPATH(cptr[1]))
      {
	temp = GARDENPATH(cptr[1]) & 3;
	paths |= (temp & 1) << 11;
      }
    if(y != CELLS_DOWN - 1 && ISPATH(cptr[CELLS_ACROSS + 1]))
      {
	temp = GARDENPATH(cptr[CELLS_ACROSS + 1]) & 3;
	paths |= (temp & 2) << 8;
      }
    if(x && y != CELLS_DOWN - 1 && ISPATH(cptr[CELLS_ACROSS]))
      {
	temp = GARDENPATH(cptr[CELLS_ACROSS]) & 3;
	paths |= (temp & 2) << 4;
      }
    if(y && x != CELLS_ACROSS - 1 && ISPATH(cptr[-CELLS_ACROSS]))
      {
	temp = GARDENPATH(cptr[-CELLS_ACROSS]) & 3;
	paths |= (temp & 1) << 7;
      }
    if(!y && INRANGE(x, 4, 8) && ISPATH(*cptr))
      {
	paths |= 0x1;
	if(x < 7 && paths & 0x8)
	  {
	    paths |= 0x100;
	    if(ISPATH(cptr[1]))
	      paths |= 0x080;
	  }
	if(x > 4 && paths & 0x4)
	  {
	    paths |= 0x010;
	    if(ISPATH(cptr[-1]))
	      paths |= 0x040;
	  }
      }
    walls = paths ^ 0xFFF;
  }
  /*}}}*/
  /*{{{  paint center?*/
  if(mask & 0x10)
    {
      XFillRectangle(display.display, display.copy, GCN(GC_BOARD),
	pixel.x, pixel.y, CELL_WIDTH, CELL_HEIGHT);
      if(ISPATH(*cptr))
	{
	  /*{{{  typedef struct Entry*/
	  typedef struct Entry
	  {
	    COORD     offset;
	    unsigned  mask;
	  } ENTRY;
	  /*}}}*/
	  /*{{{  static CONST ENTRY table[] =*/
	  static CONST ENTRY table[] =
	  {
	    {{0, 0}, 0x5},
	    {{CELL_WIDTH >> 1, 0}, 0x9},
	    {{CELL_WIDTH >> 1, CELL_HEIGHT >> 1}, 0xA},
	    {{0, CELL_HEIGHT >> 1}, 0x6},
	  };
	  /*}}}*/
	  SPRITE    *sptr;
	  unsigned  ix;
	  ENTRY CONST *tptr;
      
	  sptr = &sprites[SPRITE_CENTER_BASE];
	  for(tptr = table, ix = 4; ix--; tptr++)
	    cut_back(tptr->offset.x, tptr->offset.y,
		CELL_WIDTH >> 1, CELL_HEIGHT >> 1,
		pixel.x + tptr->offset.x, pixel.y + tptr->offset.y,
		&sptr[(walls & tptr->mask) != tptr->mask]);
	}
    }
  /*}}}*/
  /*{{{  set edges*/
  {
    /*{{{  typedef struct Entry*/
    typedef struct Entry
    {
      COORD start;
      COORD size;
    } ENTRY;
    /*}}}*/
    /*{{{  static CONST ENTRY table[] =*/
    static CONST ENTRY table[] =
    {
      {{-GAP_WIDTH, -GAP_HEIGHT}, {GAP_WIDTH * 2 + CELL_WIDTH, GAP_HEIGHT}},
      {{-GAP_WIDTH, CELL_HEIGHT}, {GAP_WIDTH * 2 + CELL_WIDTH, GAP_HEIGHT}},
      {{-GAP_WIDTH, -GAP_HEIGHT}, {GAP_WIDTH, GAP_HEIGHT * 2 + CELL_HEIGHT}},
      {{CELL_WIDTH, -GAP_HEIGHT}, {GAP_WIDTH, GAP_HEIGHT * 2 + CELL_HEIGHT}},
    };
    /*}}}*/
    unsigned  ix;
    ENTRY CONST *tptr;
    
    for(tptr = &table[3], ix = 4; ix--; tptr--)
      if(mask & (1 << ix))
	XFillRectangle(display.display, display.copy, GCN(GC_BOARD),
	    pixel.x + tptr->start.x, pixel.y + tptr->start.y,
	    tptr->size.x, tptr->size.y);
  }
  /*}}}*/
  if(!(mask & 0x10))
    /* EMPTY */;
  else if(*cptr == GARDEN_CHERRY || ISPATH(*cptr))
    /*{{{  draw sprite*/
    {
      if(*cptr == GARDEN_CHERRY || ISPATHCHERRY(*cptr))
	cut_sprite(SPRITE_CHERRY, pixel.x, pixel.y);
      else if(ISPATHDEN(*cptr))
	{
	  XFillRectangle(display.display, display.copy, GCN(GC_CLEAR),
	      pixel.x - DEN_WIDTH, pixel.y - DEN_HEIGHT,
	      CELL_WIDTH + DEN_HEIGHT * 2, CELL_HEIGHT + DEN_HEIGHT * 2);
	  cut_sprite(SPRITE_DEN, pixel.x, pixel.y);
	}
      else if(ISPATHPLAYER(*cptr))
	cut_sprite(SPRITE_PLAYER, pixel.x, pixel.y);
    }
    /*}}}*/
  else if(*cptr == GARDEN_RANDOM || ISAPPLE(*cptr))
    /*{{{  draw apples*/
    {
      unsigned  value;
      unsigned  fall;
      
      value = state.mode == MODE_RANDOM ? 0 :
	  *cptr == GARDEN_RANDOM ? 0 : GARDENAPPLE(*cptr);
      fall = y != CELLS_DOWN - 1 && ISPATH(cptr[CELLS_ACROSS + 1]);
      if(state.mode == MODE_SEPARATE || !value)
	{
	  unsigned  solid;
	  
	  solid = value & (1 << state.apple);
	  cut_sprite(SPRITE_BIG_APPLE + !solid, pixel.x, pixel.y);
	  if(fall)
	    cut_sprite(SPRITE_BIG_ARROW + !solid, pixel.x, pixel.y);
	}
      else
	{
	  unsigned  ix;
	  
	  for(ix = 4; ix--;)
	    {
	      COORD     offset;
	      unsigned  solid;
	      
	      solid = !(value & 1 << ix);
	      offset.x = pixel.x + CELL_WIDTH / 2 * (ix & 1);
	      offset.y = pixel.y + CELL_HEIGHT / 4 * (ix & 2);
	      cut_sprite(SPRITE_SMALL_APPLE + solid, offset.x, offset.y);
	      if(fall)
		cut_sprite(SPRITE_SMALL_ARROW + solid, offset.x, offset.y);
	    }
	}
    }
    /*}}}*/
  /*{{{  cut edges*/
  {
    mask &= 0xF;
    mask |= mask << 4;
    /*{{{  set extra bits to cut*/
    {
      /*{{{  typedef struct Entry*/
      typedef struct Entry
      {
	unsigned  mask;
	unsigned  value;
	unsigned  current;
	unsigned  extra;
      } ENTRY;
      /*}}}*/
      /*{{{  static CONST ENTRY table[] =*/
      static CONST ENTRY table[] =
      {
	{0x055, 0x004, 0x01, 0x04},
	{0x055, 0x010, 0x01, 0x04},
	{0x055, 0x001, 0x04, 0x01},
	{0x055, 0x040, 0x04, 0x01},
	{0x189, 0x008, 0x10, 0x08},
	{0x189, 0x100, 0x10, 0x08},
	{0x189, 0x001, 0x08, 0x10},
	{0x189, 0x080, 0x08, 0x10},
	{0x426, 0x004, 0x02, 0x40},
	{0x426, 0x020, 0x02, 0x40},
	{0x426, 0x002, 0x40, 0x02},
	{0x426, 0x400, 0x40, 0x02},
	{0xA0A, 0x008, 0x20, 0x80},
	{0xA0A, 0x200, 0x20, 0x80},
	{0xA0A, 0x002, 0x80, 0x20},
	{0xA0A, 0x800, 0x80, 0x20},
      };
      /*}}}*/
      unsigned  ix;
      ENTRY CONST *tptr;
      
      for(tptr = table, ix = XtNumber(table); ix--; tptr++)
	if(mask & tptr->current && (walls & tptr->mask) == tptr->value)
	  mask |= tptr->extra;
    }
    /*}}}*/
    /*{{{  cut all the edges*/
    {
      /*{{{  typedef struct Entry*/
      typedef struct Entry
      {
	COORD     offset;   /* place on copy pixmap */
	unsigned  mask;     /* bits we're interested in */
	char      base[16]; /* base of edge mask */
      } ENTRY;
      /*}}}*/
      /*{{{  static CONST ENTRY table[8] =*/
      static CONST ENTRY table[8] =
      {
	/*{{{  top left horizontal*/
	{
	  {-GAP_WIDTH, -GAP_HEIGHT},
	  0x055,
	  {
	    4, 7, 0, 5,
	    0, 6, 0, 8, 
	    3, 8, 1, 8, 
	    2, 8, 0, 8, 
	  }
	},
	/*}}}*/
	/*{{{  bottom left horizontal*/
	{
	  {-GAP_WIDTH, CELL_HEIGHT},
	  0x426,
	  {
	    4, 7, 0, 6, 
	    0, 5, 0, 8, 
	    3, 8, 2, 8, 
	    1, 8, 0, 8, 
	  }
	},
	/*}}}*/
	/*{{{  top left vertical*/
	{
	  {-GAP_WIDTH, -GAP_HEIGHT},
	  0x055,
	  {
	    4, 0, 7, 5, 
	    3, 1, 8, 8, 
	    0, 0, 6, 8, 
	    2, 0, 8, 8, 
	  }
	},
	/*}}}*/
	/*{{{  top right vertical*/
	{
	  {CELL_WIDTH, -GAP_HEIGHT},
	  0x189,
	  {
	    4, 0, 7, 6, 
	    0, 0, 5, 8, 
	    3, 2, 8, 8, 
	    1, 0, 8, 8, 
	  }
	},
	/*}}}*/
	/*{{{  top right horizontal*/
	{
	  {CELL_WIDTH / 2, -GAP_HEIGHT},
	  0x189,
	  {
	    4, 7, 0, 5, 
	    3, 8, 1, 8, 
	    0, 6, 0, 8, 
	    2, 8, 0, 8, 
	  }
	},
	/*}}}*/
	/*{{{  bottom right horizontal*/
	{
	  {CELL_WIDTH / 2, CELL_HEIGHT},
	  0xA0A,
	  {
	    4, 7, 0, 6, 
	    0, 5, 0, 8, 
	    3, 8, 2, 8, 
	    1, 8, 0, 8, 
	  }
	},
	/*}}}*/
	/*{{{  bottom left vertical*/
	{
	  {-GAP_WIDTH, CELL_HEIGHT / 2},
	  0x426,
	  {
	    4, 0, 7, 5, 
	    3, 1, 8, 8, 
	    0, 0, 6, 8, 
	    2, 0, 8, 8, 
	  }
	},
	/*}}}*/
	/*{{{  bottom right vertical*/
	{
	  {CELL_WIDTH, CELL_HEIGHT / 2},
	  0xA0A,
	  {
	    4, 0, 7, 6, 
	    3, 2, 8, 8, 
	    0, 0, 5, 8, 
	    1, 0, 8, 8, 
	  }
	},
	/*}}}*/
      };
      /*}}}*/
      unsigned  ix;
      ENTRY CONST *tptr;
      
      for(tptr = &table[XtNumber(table) - 1], ix = XtNumber(table);
	  ix--; tptr--)
	if(mask & (1 << ix))
	  {
	    unsigned  mask;
	    unsigned  bits;
	    unsigned  bit;
	    
	    mask = 0;
	    /*{{{  compact the interesting bits*/
	    for(bits = tptr->mask, bit = 0; bit != 4; bit++)
	      {
		unsigned  bitmask;
		
		assert(bits);
		bitmask = bits & -bits;
		if(walls & bitmask)
		  mask |= 1 << bit;
		bits ^= bitmask;
	      }
	    /*}}}*/
	    /*{{{  top edge specials*/
	    if(!y && INRANGE(x, 3, 9) && mask == 0xC)
	      {
		if(ix == 0 || ix == 4)
		  mask |= 0x2;
		else if(ix == 2 || ix == 3)
		  mask |= 0x1;
	      }
	    /*}}}*/
	    if(tptr->base[mask] == 8)
	      /* EMPTY */;
	    else if(ix & 2)
	      cut_back(tptr->base[mask] * GAP_WIDTH,
		  ix & 4 ? GAP_HEIGHT + CELL_HEIGHT / 2 : 0,
		  GAP_WIDTH, GAP_HEIGHT + CELL_HEIGHT / 2,
		  pixel.x + tptr->offset.x, pixel.y + tptr->offset.y,
		  &sprites[SPRITE_EDGE_BASE + 0]);
	    else
	      cut_back(ix & 4 ? GAP_WIDTH + CELL_WIDTH / 2 : 0,
		  tptr->base[mask] * GAP_HEIGHT,
		  GAP_WIDTH + CELL_WIDTH / 2, GAP_HEIGHT,
		  pixel.x + tptr->offset.x, pixel.y + tptr->offset.y,
		  &sprites[SPRITE_EDGE_BASE + 1]);
	  }
    }
    /*}}}*/
  }
  /*}}}*/
  if(!y && mask & 0xD)
    XDrawLine(display.display, display.copy, GCN(GC_BORDER),
	pixel.x - GAP_WIDTH, pixel.y - GAP_HEIGHT,
	pixel.x + CELL_WIDTH + GAP_WIDTH, pixel.y - GAP_HEIGHT);
  if(copy)
    RedrawRect(garden, pixel.x - GAP_WIDTH, pixel.y - GAP_HEIGHT,
	CELL_WIDTH + 2 * GAP_HEIGHT, CELL_HEIGHT + 2 * GAP_HEIGHT);
  return;
}
/*}}}*/
/*{{{  void paint_garden_icon(dptr)*/
extern VOIDFUNC paint_garden_icon
FUNCARG((dptr),
	DESCRIPTOR *dptr
)
{
  XFillRectangle(display.display, dptr->pixmap, GCN(GC_CLEAR), 0, 0,
      ICON_WIDTH, 3);
  XDrawLine(display.display, dptr->pixmap, GCN(GC_BORDER), 0, 2,
      ICON_WIDTH, 2);
  XDrawLine(display.display, dptr->pixmap, GCN(GC_BORDER), 4 * 3 - 1, 0,
      4 * 3 - 1, 2);
  XDrawLine(display.display, dptr->pixmap, GCN(GC_BORDER), 8 * 3 - 1, 0,
      8 * 3 - 1, 2);
  draw_board_icon(dptr, 0, 0, CELLS_ACROSS, CELLS_DOWN);
  return;
}
/*}}}*/
/*{{{  void paint_garden_image()*/
extern VOIDFUNC paint_garden_image FUNCARGVOID
{
  unsigned  ix;
  unsigned  iy;
  
  XFillRectangle(display.display, display.copy, GCN(GC_CLEAR),
      0, 0, WINDOW_WIDTH, PIXELY(0, 0));
  XDrawLine(display.display, display.copy, GCN(GC_BORDER),
      BORDER_LEFT, BORDER_TOP, BORDER_LEFT + BOARD_WIDTH, BORDER_TOP);
  XDrawLine(display.display, display.copy, GCN(GC_BORDER),
      PIXELX(4, -1), PIXELY(-1, 0), PIXELX(4, -1), PIXELY(-1, CELL_HEIGHT));
  XDrawLine(display.display, display.copy, GCN(GC_BORDER),
      PIXELX(4, XTRA_SPACING * 4 + CELL_WIDTH), PIXELY(-1, 0),
      PIXELX(4, XTRA_SPACING * 4 + CELL_WIDTH), PIXELY(-1, CELL_HEIGHT));
  /*{{{  blat on the extra*/
  {
    unsigned  ix;
    
    for(ix = 5; ix--;)
      {
	int       x;
	SPRITE    *lptr;
	
	lptr = &sprites[SPRITE_EXTRA];
	x = PIXELX(4, ix * XTRA_SPACING);
	XCopyArea(display.display, lptr->mask,
	    display.copy, GCN(GC_MASK), (int)ix * (CELL_WIDTH / 2), 0,
	    CELL_WIDTH / 2, CELL_HEIGHT / 2,
	    x + XTRA_LETTER_X, PIXELY(-1, XTRA_LETTER_Y));
	XCopyArea(display.display, lptr->image,
	    display.copy, GCN(GC_OR), (int)ix * (CELL_WIDTH / 2), 0,
	    CELL_WIDTH / 2, CELL_HEIGHT / 2,
	    x + XTRA_LETTER_X, PIXELY(-1, XTRA_LETTER_Y));
      }
  }
  /*}}}*/
  XDrawImageString(display.display, display.copy, GCN(GC_TEXT),
      PIXELX(4, -GAP_WIDTH) - 3 * font.width,
      PIXELY(-1, CELL_HEIGHT / 2) + font.center, "0", (int)1);
  for(iy = CELLS_DOWN; iy--;)
    for(ix = CELLS_ACROSS; ix--;)
      paint_cell(ix, iy, 0x1A | (iy ? 0x0 : 0x1) | (ix ? 0x0 : 0x4), 0);
  paint_garden_source();
  RedrawRect(garden, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
  return;  
}
/*}}}*/
/*{{{  void paint_garden_source()*/
extern VOIDFUNC paint_garden_source FUNCARGVOID
{
  char CONST *text;
  
  switch(state.source)
  {
    /*{{{  case SOURCE_UNIQUE:*/
    case SOURCE_UNIQUE:
    {
      text = "Unique";
      break;
    }
    /*}}}*/
    /*{{{  case SOURCE_BUFFER:*/
    case SOURCE_BUFFER:
    {
      text = "Buffer";
      break;
    }
    /*}}}*/
    /*{{{  default:*/
    default:
    {
      static char string[] = "Garden 000";
      
      itoa(string + 7, (unsigned long)state.source, 0);
      text = string;
      break;
    }
    /*}}}*/
  }
  XFillRectangle(display.display, display.copy, GCN(GC_CLEAR),
      PIXELX(8, 0), PIXELY(-1, 0),
      (CELLS_ACROSS - 8) * (CELL_WIDTH + GAP_WIDTH), CELL_HEIGHT);
  XDrawImageString(display.display, display.copy, GCN(GC_TEXT),
      PIXELX(8, (int)font.width * 2),
      PIXELY(-1, CELL_HEIGHT / 2 + font.center), text, strlen(text));
  RedrawRect(garden, PIXELX(8, 0), PIXELY(-1, 0),
      (CELLS_ACROSS - 8) * (CELL_WIDTH + GAP_WIDTH), CELL_HEIGHT);
  menu_garden(text);
  return;
}
/*}}}*/
/*{{{  unsigned long set_random(x, y, cptr)*/
static unsigned long set_random
FUNCARG((x, y, cptr),
	unsigned  x
ARGSEP  unsigned  y
ARGSEP  char      *cptr
)
/* sets a random apple possiblilty
 * if currently noapple
 * returns mask of bits to redraw a la change_path
 */
{
  unsigned long redraw;
  
  redraw = 0;
  if(*cptr == GARDEN_NOAPPLE)
    {
      *cptr = GARDEN_RANDOM;
      adjust_count(COUNT_SPACES, 1);
      if(y != CELLS_DOWN && ISPATH(cptr[CELLS_ACROSS + 1]))
	adjust_count(COUNT_FALL, 1);
      changed_flag |= state.change;
      redraw = 0x100;
    }
  return redraw;
}
/*}}}*/
