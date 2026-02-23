/* Copyright (C) 1993 Nathan Sidwell */
/* RCS $Id: PixmapList.c 1.2 Tue, 16 Mar 1999 11:28:16 +0000 nathan $ */
/* Holds a list of pixmaps which can be selected or dragged like
 * individual Icon widgets.
 * I tried using a box widget with an insertPosition function
 * but that didn't seem to work properly
 * you can also attach a scroll bar to this widget
 */
/*{{{  includes*/
#include "ansiknr.h"
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/SimpleP.h>
#include <X11/Xaw/Scrollbar.h>
#include <X11/Xmu/Drawing.h>
#include "PixmapList.h"
#include "Drag.h"
#include <math.h>
/*}}}*/
/*{{{  structs*/
/*{{{  typedef struct _PixmapListClass*/
typedef struct _PixmapListClass
{
  int     ansi_compliance; /* not used */
} PixmapListClassPart;
/*}}}*/
/*{{{  typedef struct _PixmapListClassRec*/
typedef struct _PixmapListClassRec
{
  CoreClassPart   core_class;
  SimpleClassPart simple_class;
  PixmapListClassPart   pixmap_list_class;
} PixmapListClassRec;
/*}}}*/
/*{{{  typedef _PixmapInfo*/
typedef struct _PixmapInfo
{
  Pixmap    pixmap;   /* the pixmap */
  int       width;    /* its width */
  int       height;   /* its height */
  Position  x;        /* its position */
  Position  y;        /* its position */
} PixmapInfo;
/*}}}*/
/*{{{  typedef struct _PixmapListPart*/
typedef struct
{
  /* resources */
  XtCallbackList callbacks;   /* callbacks to notify */
  Dimension drag_sensitivity; /* sensitivity of drag callback */
  Dimension highlight_thickness;  /* thickness of highlight bar */
  Pixel     foreground;       /* foreground pixel color */
  int       flash_delay;      /* time to flash */
  String    drag_name;        /* name of drag */
  Dimension distance;         /* separation of pixmaps */
  XtOrientation orient;       /* orientation */
  Dimension border_width;     /* border width */
  Pixel     border_color;     /* border color */
  
  /* private state */
  PixmapInfo *pixmap_info;  /* list of pixmaps */
  Cardinal  num_pixmaps;    /* their number */
  Cardinal  limit;          /* malloc limit of pi */
  GC        gc;             /* GC to draw foreground */
  GC        border_gc;      /* GC to draw border */
  Position  drag_x;         /* drag start x */
  Position  drag_y;         /* drag start y */
  int       drag_set;       /* drag selected */
  Boolean   highlit;        /* is it highlit? */
  int       selected;       /* selected pixmap */
  int       was;            /* selected pixmap */
  Position  x;              /* offset of start of list */
  Position  y;              /* offset of start of list */
  int       width;          /* pixmaps' widths or -1 for different */
  int       height;         /* pixmaps' heights or -1 for different */
  Dimension length;         /* length of list */
  XtIntervalId timeout;     /* timout id */
  Widget    scroll;         /* scrollbar widget */
} PixmapListPart;
/*}}}*/
/*{{{  typedef struct _PixmapListRec*/
typedef struct _PixmapListRec
{
  CorePart    core;
  SimplePart  simple;
  PixmapListPart pixmap_list;
} PixmapListRec;
/*}}}*/
/*}}}*/
/*{{{  resources*/
static XtResource resources[] =
{
  {XtNforeground, XtCForeground, XtRPixel, sizeof(Pixel),
      XtOffsetOf(PixmapListRec, pixmap_list.foreground),
      XtRString, (XtPointer)XtDefaultForeground},
  {MredNhighlightThickness, XtCThickness, XtRDimension, sizeof(Dimension),
      XtOffsetOf(PixmapListRec, pixmap_list.highlight_thickness),
      XtRImmediate, (XtPointer)1},
  {XtNcallback, XtCCallback, XtRCallback, sizeof(XtPointer),
      XtOffsetOf(PixmapListRec, pixmap_list.callbacks),
      XtRCallback, (XtPointer)NULL},
  {MredNdistance, XtCThickness, XtRDimension, sizeof(Dimension),
      XtOffsetOf(PixmapListRec, pixmap_list.distance),
      XtRImmediate, (XtPointer)4},
  {MredNdragSensitivity, XtCThickness, XtRDimension, sizeof(Dimension),
      XtOffsetOf(PixmapListRec, pixmap_list.drag_sensitivity),
      XtRImmediate, (XtPointer)4},
  {MredNflashDelay, XtCInterval, XtRInt, sizeof(int),
      XtOffsetOf(PixmapListRec, pixmap_list.flash_delay),
      XtRImmediate, (XtPointer)100},
  {MredNdragName, XtCLabel, XtRString, sizeof(String),
      XtOffsetOf(PixmapListRec, pixmap_list.drag_name),
      XtRImmediate, (XtPointer)"drag"},
  {XtNinternalBorderWidth, XtCBorderWidth, XtRDimension, sizeof(Dimension),
      XtOffsetOf(PixmapListRec, pixmap_list.border_width),
      XtRImmediate, (XtPointer)1},
  {XtNinternalBorderColor, XtCBorderColor, XtRPixel, sizeof(Pixel),
      XtOffsetOf(PixmapListRec, pixmap_list.border_color),
      XtRString, (XtPointer)XtDefaultForeground},
  {XtNorientation, XtCOrientation, XtROrientation, sizeof(XtOrientation),
      XtOffsetOf(PixmapListRec, pixmap_list.orient),
      XtRImmediate, (XtPointer)XtorientVertical},
};
/*}}}*/
/*{{{  prototypes*/
static VOIDFUNC Drag PROTOARG((Widget, XEvent *, String *, Cardinal *));
static VOIDFUNC Highlight PROTOARG((Widget, XEvent *, String *, Cardinal *));
static VOIDFUNC Notify PROTOARG((Widget, XEvent *, String *, Cardinal *));
static VOIDFUNC Set PROTOARG((Widget, XEvent *, String *, Cardinal *));
static VOIDFUNC Unhighlight PROTOARG((Widget, XEvent *, String *, Cardinal *));

static VOIDFUNC ClassInitialize PROTOARG((VOIDARG));
static VOIDFUNC Destroy PROTOARG((Widget));
static VOIDFUNC Initialize PROTOARG((Widget, Widget, ArgList, Cardinal *));
static XtGeometryResult QueryGeometry
    PROTOARG((Widget, XtWidgetGeometry *, XtWidgetGeometry *));
static VOIDFUNC Redisplay PROTOARG((Widget, XEvent *, Region));
static VOIDFUNC Resize PROTOARG((Widget));
static Boolean SetValues
    PROTOARG((Widget, Widget, Widget, ArgList, Cardinal *));

static VOIDFUNC PaintPixmap
    PROTOARG((PixmapListWidget, PixmapInfo *, unsigned));
static VOIDFUNC PaintRect PROTOARG((PixmapListWidget, Position, Position));
static VOIDFUNC GetGC PROTOARG((PixmapListWidget));
static VOIDFUNC GetPixmapSize PROTOARG((PixmapListWidget, PixmapInfo *));
static int GetPointerPosition
    PROTOARG((PixmapListWidget, XEvent *, Position *, Position *));
static VOIDFUNC RemoveTimeOut PROTOARG((PixmapListWidget));
static VOIDFUNC ScrollJump PROTOARG((Widget, XtPointer, XtPointer));
static VOIDFUNC ScrollScroll PROTOARG((Widget, XtPointer, XtPointer));
static VOIDFUNC SetScrollbar PROTOARG((PixmapListWidget));
static VOIDFUNC TimeOut PROTOARG((XtPointer, XtIntervalId *));
/*}}}*/
/*{{{  translations*/
static char translations[] = "\
<BtnDown>:set()\n\
<BtnUp>:notify()\n\
<Motion>:highlight() drag()\n\
<LeaveNotify>:unhighlight()\n\
";
/*}}}*/
/*{{{  actions*/
static XtActionsRec actions[] =
{
  {"drag", Drag},
  {"highlight", Highlight},
  {"notify", Notify},
  {"set", Set},
  {"unhighlight", Unhighlight},
};
/*}}}*/
#define SuperClass (WidgetClass)&simpleClassRec
/*{{{  PixmapListClassRec pixmapListClassRec =*/
PixmapListClassRec pixmapListClassRec =
{
  /*{{{  core class part*/
  {
    SuperClass,                       /* superclass */
    "PixmapList",                     /* class_name */
    sizeof(PixmapListRec),            /* size */
    ClassInitialize,                  /* class_initialize */
    NULL,                             /* class_part_initialize */
    False,                            /* class_inited */
    Initialize,                       /* initialize */
    NULL,                             /* initialize_hook */
    XtInheritRealize,                 /* realize */
    actions,                          /* actions */
    XtNumber(actions),                /* num_actions */
    resources,                        /* resources */
    XtNumber(resources),              /* num_resources */
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
  /*{{{  pixmap list class part*/
  {
    0,        /* dummy */
  },
  /*}}}*/
};
/*}}}*/
WidgetClass pixmapListWidgetClass = (WidgetClass)&pixmapListClassRec;
/* actions */
/*{{{  void Drag(widget, event, params, num_params)*/
static VOIDFUNC Drag
FUNCARG((widget, event, params, num_params),
	Widget    widget
ARGSEP  XEvent    *event
ARGSEP  String    *params
ARGSEP  Cardinal  *num_params
)
/* determines if a drag sould start on the currently selected pixmap
 * the drag widget is invoked the same as Icon does.
 */
{
  PixmapListWidget plw;
  
  plw = (PixmapListWidget)widget;
  if(event->type != MotionNotify && event->type != ButtonPress)
    /* EMPTY */;
  else if(event->type == MotionNotify && !(event->xmotion.state &
    (Button1Mask | Button2Mask | Button3Mask | Button4Mask | Button5Mask)))
    /* EMPTY */;
  else
    {
      Position  x, y;
      int       selected;
       
      selected = GetPointerPosition(plw, event, &x, &y);
      if(selected != plw->pixmap_list.drag_set)
	{
	  plw->pixmap_list.drag_set = selected;
	  plw->pixmap_list.drag_x = x;
	  plw->pixmap_list.drag_y = y;
	}
      else if(selected < 0)
	/* EMPTY */;
      else if(!plw->pixmap_list.drag_sensitivity)
	/* EMPTY */;
      else if(!plw->pixmap_list.drag_name)
	/* EMPTY */;
      else if(plw->pixmap_list.drag_sensitivity *
	  plw->pixmap_list.drag_sensitivity <=
	  (x - plw->pixmap_list.drag_x) * (x - plw->pixmap_list.drag_x) + 
	  (y - plw->pixmap_list.drag_y) * (y - plw->pixmap_list.drag_y))
	/*{{{  pop up drag*/
	{
	  Widget  drag;
	  Widget  parent;
	  
	  drag = NULL;
	  for(parent = (Widget)plw; parent;
	      parent = XtParent(parent))
	    {
	      drag = XtNameToWidget(parent, plw->pixmap_list.drag_name);
	      if(drag)
		break;
	    }
	  if(drag)
	    {
	      Position  rx, ry;
	  
	      plw->pixmap_list.highlit = False;
	      PaintPixmap(plw, NULL, selected);
	      XtTranslateCoords((Widget)plw,
		  plw->pixmap_list.pixmap_info[selected].x +
		    plw->pixmap_list.x,
		  plw->pixmap_list.pixmap_info[selected].y +
		    plw->pixmap_list.y, &rx, &ry);
	      DragPopup((Widget)plw, (Widget)drag,
		  plw->pixmap_list.pixmap_info[selected].pixmap,
		  plw->pixmap_list.drag_x, plw->pixmap_list.drag_y,
		  rx + x, ry + y, event->xmotion.time);
	    }
	}
	/*}}}*/
    }
  return;
}
/*}}}*/
/*{{{  void Highlight(widget, event, params, num_params)*/
static VOIDFUNC Highlight
FUNCARG((widget, event, params, num_params),
	Widget    widget
ARGSEP  XEvent    *event
ARGSEP  String    *params
ARGSEP  Cardinal  *num_params
)
/* highlight the pixmap we're on
 */
{
  PixmapListWidget plw;
  Position  x, y;
  int       selected;
  
  plw = (PixmapListWidget)widget;
  selected = plw->pixmap_list.highlight_thickness ?
      GetPointerPosition(plw, event, &x, &y) : -1;
  if(selected != plw->pixmap_list.selected ||
      (plw->pixmap_list.highlit == False && selected >= 0))
    {
      int       old;
      
      plw->pixmap_list.drag_x = x;
      plw->pixmap_list.drag_y = y;
      plw->pixmap_list.drag_set = selected + 1;
      old = plw->pixmap_list.selected;
      plw->pixmap_list.selected = selected;
      plw->pixmap_list.highlit = selected < 0 ? False : True;
      if(selected >= 0)
	{
	  int       was;
	  
	  RemoveTimeOut(plw);
	  was = plw->pixmap_list.was;
	  plw->pixmap_list.was = -1;
	  if(was >= 0 && was != old && was != selected)
	    PaintPixmap(plw, NULL, was);
	  PaintPixmap(plw, NULL, selected);
	}
      if(old >= 0)
	PaintPixmap(plw, NULL, old);
    }
  return;
}
/*}}}*/
/*{{{  void Notify(widget, event, params, num_params)*/
static VOIDFUNC Notify
FUNCARG((widget, event, params, num_params),
	Widget    widget
ARGSEP  XEvent    *event
ARGSEP  String    *params
ARGSEP  Cardinal  *num_params
)
/* notify the callback of the currently selected pixmap
 * flashes the pixmap too
 * normally invoked on button release.
 */
{
  PixmapListWidget plw;
  
  plw = (PixmapListWidget)widget;
  plw->pixmap_list.drag_set = -1;
  if(plw->pixmap_list.selected >= 0)
    {
      PixmapListCallback data;
      
      if(plw->pixmap_list.flash_delay)
	{
	  plw->pixmap_list.was = plw->pixmap_list.selected;
	  PaintPixmap(plw, NULL, plw->pixmap_list.was);
	  plw->pixmap_list.timeout = XtAppAddTimeOut(
	      XtWidgetToApplicationContext((Widget)plw),
	      (unsigned long)plw->pixmap_list.flash_delay,
	      TimeOut, (XtPointer)plw);
	}
      data.selection = plw->pixmap_list.selected;
      if(event->type == ButtonPress || event->type == ButtonRelease)
	data.button = event->xbutton.button;
      else
	data.button = -1;
      XtCallCallbackList((Widget)plw, plw->pixmap_list.callbacks,
	  (XtPointer)&data);
    }
  return;
}
/*}}}*/
/*{{{  void Set(widget, event, params, num_params)*/
static VOIDFUNC Set
FUNCARG((widget, event, params, num_params),
	Widget    widget
ARGSEP  XEvent    *event
ARGSEP  String    *params
ARGSEP  Cardinal  *num_params
)
/* initialize the drag start point
 */
{
  PixmapListWidget plw;
  
  plw = (PixmapListWidget)widget;
  plw->pixmap_list.drag_set = GetPointerPosition(plw, event,
	  &plw->pixmap_list.drag_x, &plw->pixmap_list.drag_y) + 1;
  return;
}
/*}}}*/
/*{{{  void Unhighlight(widget, event, params, num_params)*/
static VOIDFUNC Unhighlight
FUNCARG((widget, event, params, num_params),
	Widget    widget
ARGSEP  XEvent    *event
ARGSEP  String    *params
ARGSEP  Cardinal  *num_params
)
/* deselect any selected widget
 */
{
  PixmapListWidget plw;
  
  plw = (PixmapListWidget)widget;
  if(plw->pixmap_list.highlit != False)
    {
      plw->pixmap_list.highlit = False;
      PaintPixmap(plw, NULL, plw->pixmap_list.selected);
    }
  return;
}
/*}}}*/
/* methods */
/*{{{  void ClassInitialize()*/
static VOIDFUNC ClassInitialize FUNCARGVOID
/* add the orientation type converter
 */
{
  XtAddConverter(XtRString, XtROrientation, XmuCvtStringToOrientation,
      NULL, 0);
  return;
}
/*}}}*/
/*{{{  void Destroy(widget)*/
static VOIDFUNC Destroy
FUNCARG((widget),
	Widget    widget
)
/* release our GCs and remove the flash timeout */
{
  PixmapListWidget plw;
  
  plw = (PixmapListWidget)widget;
  XtReleaseGC((Widget)plw, plw->pixmap_list.gc);
  XtReleaseGC((Widget)plw, plw->pixmap_list.border_gc);
  RemoveTimeOut(plw);
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
/* initialize ourselves
 */
{
  PixmapListWidget nplw;
  
  nplw = (PixmapListWidget)tnew;
  nplw->pixmap_list.selected = -1;
  nplw->pixmap_list.was = -1;
  nplw->pixmap_list.highlit = False;
  nplw->pixmap_list.drag_set = -1;
  nplw->pixmap_list.timeout = (XtIntervalId)0;
  nplw->pixmap_list.pixmap_info = NULL;
  nplw->pixmap_list.scroll = NULL;
  nplw->pixmap_list.limit = 0;
  nplw->pixmap_list.length = nplw->pixmap_list.distance;
  nplw->pixmap_list.width = nplw->pixmap_list.height = 0;
  nplw->pixmap_list.x = nplw->pixmap_list.y = 0;
  nplw->pixmap_list.num_pixmaps = 0;
  if(!nplw->core.width)
    nplw->core.width = 16;
  if(!nplw->core.height)
    nplw->core.height = 16;
  GetGC(nplw);
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
/* tell our parent how big we'd like to be.
 * This is big enough to hold all the pixmaps,
 * but we don't mind being bigger or smaller
 */
{
  PixmapListWidget plw;
  unsigned  ix;
  PixmapInfo *pi;
  unsigned  max_width;
  unsigned  max_height;
  
  plw = (PixmapListWidget)widget;
  answer->request_mode = CWWidth | CWHeight;
  max_width = max_height = 0;
  for(ix = plw->pixmap_list.num_pixmaps,
      pi = plw->pixmap_list.pixmap_info; ix--; pi++)
    {
      if(max_width < pi->width)
	max_width = pi->width;
      if(max_height < pi->height)
	max_height = pi->height;
    }
  pi--;
  if(plw->pixmap_list.orient == XtorientVertical)
    {
      answer->width = max_width + plw->pixmap_list.distance * 2 +
	  plw->pixmap_list.border_width * 2;
      answer->height = plw->pixmap_list.length;
    }
  else
    {
      answer->width = plw->pixmap_list.length;
      answer->height = max_height + plw->pixmap_list.distance * 2 +
	  plw->pixmap_list.border_width * 2;
    }
  if(!answer->width)
    answer->width = 16;
  if(!answer->height)
    answer->height = 16;
  if((proposed->request_mode & (CWWidth | CWHeight)) == (CWWidth | CWHeight) &&
      proposed->width == answer->width && proposed->height == answer->height)
    return XtGeometryYes;
  else if(answer->width == plw->core.width &&
      answer->height == plw->core.height)
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
/* repaint all the pixmaps.
 * We use the expose region to only repaint those
 * pixmaps falling partially or whole within it
 */
{
  PixmapListWidget plw;
  unsigned  ix;
  PixmapInfo *pi;
  
  if(!XtIsRealized(widget))
    return;
  plw = (PixmapListWidget)widget;
  for(ix = 0, pi = plw->pixmap_list.pixmap_info;
      ix != plw->pixmap_list.num_pixmaps; ix++, pi++)
    if(XRectInRegion(region, pi->x + plw->pixmap_list.x -
	  (plw->pixmap_list.border_width + 1) / 2,
	pi->y + plw->pixmap_list.y -
	  (plw->pixmap_list.border_width + 1) / 2,
	pi->width + 2 * plw->pixmap_list.border_width,
	pi->height + 2 * plw->pixmap_list.border_width) != RectangleOut)
      PaintPixmap(plw, pi, ix);
  return;
}
/*}}}*/
/*{{{  void Resize(widget)*/
static VOIDFUNC Resize
FUNCARG((widget),
	Widget    widget
)
/* reset the starting point and reset the scrollbar scale */
{
  PixmapListWidget plw;
  
  plw = (PixmapListWidget)widget;
  if(plw->pixmap_list.orient == XtorientVertical)
    plw->pixmap_list.x = 0;
  else
    plw->pixmap_list.y = 0;
  SetScrollbar(plw);
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
/* realloc the GCs if things change
 * This generally involes a complete repaint
 */
{
  Boolean  redraw;
  PixmapListWidget cplw;
  PixmapListWidget nplw;
  
  redraw = False;
  cplw = (PixmapListWidget)cw;
  nplw = (PixmapListWidget)nw;
  if(cplw->pixmap_list.foreground != nplw->pixmap_list.foreground ||
      cplw->pixmap_list.highlight_thickness !=
      nplw->pixmap_list.highlight_thickness ||
      cplw->pixmap_list.border_color != nplw->pixmap_list.border_color ||
      cplw->pixmap_list.border_width != nplw->pixmap_list.border_width)
    {
      redraw = True;
      XtReleaseGC((Widget)cplw, cplw->pixmap_list.gc);
      XtReleaseGC((Widget)cplw, cplw->pixmap_list.border_gc);
      GetGC(nplw);
    }
  if(nplw->pixmap_list.border_width != 
      cplw->pixmap_list.border_width ||
      nplw->pixmap_list.distance != cplw->pixmap_list.distance ||
      nplw->pixmap_list.orient != cplw->pixmap_list.orient)
    {
      PixmapInfo  *pi;
      unsigned  count;
      unsigned  spot;
      
      spot = nplw->pixmap_list.distance;
      for(count = nplw->pixmap_list.num_pixmaps,
	  pi = nplw->pixmap_list.pixmap_info; count--; pi++)
	{
	  spot += nplw->pixmap_list.border_width;
	  if(nplw->pixmap_list.orient == XtorientVertical)
	    pi->x = spot;
	  else
	    pi->y = spot;
	  spot += nplw->pixmap_list.border_width +
	      nplw->pixmap_list.distance;
	}
      nplw->pixmap_list.length = spot;
      redraw = True;
      SetScrollbar(nplw);
    }
  if(cplw->core.width != nplw->core.width ||
      cplw->core.height != nplw->core.height)
    redraw = False;
  return redraw;
}
/*}}}*/
/* public routines */
/*{{{  void PixmapListRepaint(widget)*/
extern VOIDFUNC PixmapListRepaint
FUNCARG((widget, ix),
	Widget    widget
ARGSEP  Cardinal  ix      /* pixmap to repaint */
)
/* repaint a single pixmap, avoiding the annoying expose event flicker
 */
{
  if(!XtIsSubclass(widget, (WidgetClass)&pixmapListClassRec))
    /* EMPTY */;
  else if(!XtIsRealized(widget))
    /* EMPTY */;
  else if(ix >= ((PixmapListWidget)widget)->pixmap_list.num_pixmaps)
    /* EMPTY */;
  else
    PaintPixmap((PixmapListWidget)widget, NULL, ix);
  return;
}
/*}}}*/
/*{{{  void PixmapListInsert(widget, ix, pixmap)*/
extern VOIDFUNC PixmapListInsert
FUNCARG((widget, ix, pixmap),
	Widget    widget
ARGSEP  Cardinal  ix        /* insert at this position */
ARGSEP  Pixmap    pixmap    /* pixmap to insert */
)
/* insert a pixmap at a given position in the list
 */
{
  PixmapListWidget plw;
  PixmapInfo  *pi;
  Position  start;
  
  plw = (PixmapListWidget)widget;
  if(!XtIsSubclass(widget, (WidgetClass)&pixmapListClassRec))
    return;
  if(ix > plw->pixmap_list.num_pixmaps)
    ix = plw->pixmap_list.num_pixmaps;
  if(plw->pixmap_list.drag_set >= (int)ix)
    plw->pixmap_list.drag_set++;
  if(plw->pixmap_list.selected >= (int)ix)
    plw->pixmap_list.selected++;
  if(plw->pixmap_list.was >= (int)ix)
    plw->pixmap_list.was++;
  /*{{{  insert it*/
  {
    unsigned  count;
    unsigned  width, height;
    
    plw->pixmap_list.num_pixmaps++;
    if(plw->pixmap_list.num_pixmaps > plw->pixmap_list.limit)
      {
	plw->pixmap_list.limit += 128;
	plw->pixmap_list.pixmap_info =
	    (PixmapInfo *)XtRealloc((VOID *)plw-> pixmap_list.pixmap_info,
	    plw->pixmap_list.limit * sizeof(PixmapInfo));
      }
    for(pi = &plw->pixmap_list.pixmap_info[plw->pixmap_list.num_pixmaps - 1],
	count = plw->pixmap_list.num_pixmaps - ix - 1; count--; pi--)
      memcpy(&pi[0], &pi[-1], sizeof(PixmapInfo));
    pi->pixmap = pixmap;
    GetPixmapSize(plw, pi);
    if(!plw->pixmap_list.width)
      plw->pixmap_list.width = pi->width;
    else if(plw->pixmap_list.width != pi->width)
      plw->pixmap_list.width = -1;
    if(!plw->pixmap_list.height)
      plw->pixmap_list.height = pi->height;
    else if(plw->pixmap_list.height != pi->height)
      plw->pixmap_list.height = -1;
    if(ix == plw->pixmap_list.num_pixmaps - 1)
      pi->x = pi->y = plw->pixmap_list.length +
	  plw->pixmap_list.border_width;
    if(plw->pixmap_list.orient == XtorientVertical)
      {
	pi->x = plw->pixmap_list.distance + plw->pixmap_list.border_width;
	width = 0;
	height = pi->height + plw->pixmap_list.distance +
	    plw->pixmap_list.border_width * 2;
      }
    else
      {
	pi->y = plw->pixmap_list.distance + plw->pixmap_list.border_width;
	height = 0;
	width = pi->width + plw->pixmap_list.distance +
	    plw->pixmap_list.border_width * 2;
      }
    for(count = plw->pixmap_list.num_pixmaps - ix - 1; pi++, count--;)
      {
	pi->x += width;
	pi->y += height;
      }
    plw->pixmap_list.length += width + height;
  }
  /*}}}*/
  pi = &plw->pixmap_list.pixmap_info[ix];
  start = plw->pixmap_list.orient == XtorientVertical ? pi->y : pi->x;
  start -= plw->pixmap_list.border_width;
  if(plw->pixmap_list.orient == XtorientVertical)
    {
      if(plw->core.height <= plw->pixmap_list.length)
	{
	  plw->pixmap_list.y -= plw->pixmap_list.border_width +
	      ((pi->height + plw->pixmap_list.distance + 1) >> 1);
	  start = -plw->pixmap_list.y;
	}
    }
  else
    {
      if(plw->core.width <= plw->pixmap_list.length)
	{
	  plw->pixmap_list.x -= plw->pixmap_list.border_width +
	      ((pi->width + plw->pixmap_list.distance + 1) >> 1);
	  start = -plw->pixmap_list.x;
	}
    }
  PaintRect(plw, start, plw->pixmap_list.length - start);
  SetScrollbar(plw);
  return;
}
/*}}}*/
/*{{{  void PixmapListRemove(widget, ix)*/
extern VOIDFUNC PixmapListRemove
FUNCARG((widget, ix),
	Widget    widget
ARGSEP  Cardinal  ix    /* pixmap to remove */
)
/* remove a pixmap from the list
 * note that we don't free the pixmap, they belong to the widget operator
 */
{
  PixmapListWidget plw;
  PixmapInfo  *pi;
  Position  start;
  unsigned  count;
  unsigned  delta;
  
  plw = (PixmapListWidget)widget;
  if(!XtIsSubclass(widget, (WidgetClass)&pixmapListClassRec))
    return;
  if(!plw->pixmap_list.num_pixmaps)
    return;
  if(ix >= plw->pixmap_list.num_pixmaps)
    ix = plw->pixmap_list.num_pixmaps - 1;
  if(plw->pixmap_list.drag_set == ix)
    plw->pixmap_list.drag_set = -1;
  if(plw->pixmap_list.selected == ix)
    plw->pixmap_list.selected = -1;
  if(plw->pixmap_list.selected == ix)
    RemoveTimeOut(plw);
  pi = &plw->pixmap_list.pixmap_info[ix];
  plw->pixmap_list.num_pixmaps--;
  if(plw->pixmap_list.orient == XtorientVertical)
    {
      delta = pi->height;
      start = pi->y;
    }
  else
    {
      delta = pi->width;
      start = pi->x;
    }
  delta += plw->pixmap_list.distance + plw->pixmap_list.border_width * 2;
  plw->pixmap_list.length -= delta;
  for(count = plw->pixmap_list.num_pixmaps - ix; count--; pi++)
    {
      memcpy(pi, pi + 1, sizeof(PixmapInfo));
      if(plw->pixmap_list.orient == XtorientVertical)
	pi->y -= delta;
      else
	pi->x -= delta;
    }
  /*{{{  set scroll point*/
  if(plw->pixmap_list.orient == XtorientVertical)
    {
      if(plw->pixmap_list.length < plw->core.height)
	{
	  if(plw->pixmap_list.y)
	    {
	      plw->pixmap_list.y = 0;
	      start = 0;
	    }
	}
      else
	{
	  plw->pixmap_list.y += delta / 2;
	  if(plw->pixmap_list.y > 0)
	    plw->pixmap_list.y = 0;
	  start = -plw->pixmap_list.y;
	}
    }
  else
    {
      if(plw->pixmap_list.length < plw->core.width)
	{
	  if(plw->pixmap_list.x)
	    {
	      plw->pixmap_list.x = 0;
	      start = 0;
	    }
	}
      else
	{
	  plw->pixmap_list.x += delta / 2;
	  if(plw->pixmap_list.x > 0)
	    plw->pixmap_list.x = 0;
	  start = -plw->pixmap_list.x;
	}
    }
  /*}}}*/
  PaintRect(plw, start - plw->pixmap_list.border_width,
      plw->pixmap_list.length + delta - start);
  SetScrollbar(plw);
  /*{{{  check if width & height can be rechecked*/
  if(!plw->pixmap_list.num_pixmaps)
    plw->pixmap_list.width = plw->pixmap_list.height = 0;
  else if(plw->pixmap_list.width < 0 || plw->pixmap_list.height < 0)
    {
      pi = plw->pixmap_list.pixmap_info;
      plw->pixmap_list.width = pi->width;
      plw->pixmap_list.height = pi->height;
      for(count = plw->pixmap_list.num_pixmaps; count--; pi++)
	{
	  if(pi->width != plw->pixmap_list.width)
	    plw->pixmap_list.width = -1;
	  if(pi->height != plw->pixmap_list.height)
	    plw->pixmap_list.height = -1;
	}
    }
  /*}}}*/
  return;
}
/*}}}*/
/*{{{  void PixmapListSetScroll(widget, scroll)*/
extern VOIDFUNC PixmapListSetScroll
FUNCARG((widget, scroll),
	Widget    widget    /* this widget */
ARGSEP  Widget    scroll    /* scrollbar widget */
)
/* inform of the scroll bar widget
 * add callbacks to the scrollbar, so we can do the
 * right thing
 */
{
  PixmapListWidget plw;
  
  plw = (PixmapListWidget)widget;
  if(!XtIsSubclass(widget, (WidgetClass)&pixmapListClassRec))
    return;
  if(plw->pixmap_list.scroll)
    return;
  plw->pixmap_list.scroll = scroll;
  SetScrollbar(plw);
  XtAddCallback(scroll, XtNjumpProc, ScrollJump, (XtPointer)plw);
  XtAddCallback(scroll, XtNscrollProc, ScrollScroll, (XtPointer)plw);
  return;
}
/*}}}*/
/*{{{  int PixmapListQueryOffset(widget, x, y, flag)*/
extern int PixmapListQueryOffset
FUNCARG((widget, x, y, flag),
	Widget    widget
ARGSEP  Position  x     /* x position */
ARGSEP  Position  y     /* y position */
ARGSEP  Boolean   flag  /* center mode False:on pixmaps, True:on gaps  */
)
/* find out which pixmap is at this position
 * The pixmaps can either be selected by centering on
 * the pixmaps, or on the gaps.
 * For selecting an insert position you probably want to center
 * on the gaps. For selecting a delete position, you
 * probably want to center on the pixmaps.
 */
{
  PixmapListWidget plw;
  PixmapInfo *pi;
  int       ix;
  int       point;
  int       count;
  int       gap;
  
  plw = (PixmapListWidget)widget;
  if(!XtIsSubclass(widget, (WidgetClass)&pixmapListClassRec))
    return -1;
  if(plw->pixmap_list.orient == XtorientVertical)
    point = y - plw->pixmap_list.y;
  else
    point = x - plw->pixmap_list.x;
  gap = plw->pixmap_list.border_width + ((plw->pixmap_list.distance + 1) >> 1);
  for(ix = 0, pi = plw->pixmap_list.pixmap_info,
      count = plw->pixmap_list.num_pixmaps; count; count--, ix++, pi++)
    if(point < (flag != False ? (plw->pixmap_list.orient == XtorientVertical ?
	pi->y + (pi->height >> 1) : pi->x + (pi->width >> 1)) :
    (plw->pixmap_list.orient == XtorientVertical ?
	pi->y + pi->height : pi->x + pi->width) + gap))
      break;
  return  ix;
}
/*}}}*/
/*{{{  int PixmapListQueryDrag(widget)*/
extern int PixmapListQueryDrag
FUNCARG((widget),
	Widget    widget
)
/* used to find out which pixmap was selected to initiate
 * a drag
 */
{
  if(!XtIsSubclass(widget, (WidgetClass)&pixmapListClassRec))
    return -1;
  return ((PixmapListWidget)widget)->pixmap_list.drag_set;
}
/*}}}*/
/* private routines */
/*{{{  void PaintPixmap(plw, pi, ix)*/
static VOIDFUNC PaintPixmap
FUNCARG((plw, pi, ix),
	PixmapListWidget plw
ARGSEP  PixmapInfo *pi    /* pixmap info pointer */
ARGSEP  unsigned  ix      /* pixmap number */
)
/* repaint a specific pixmap
 * We have to repaint the border too.
 */
{
  Position  x, y;
  
  if(!pi)
    pi = &plw->pixmap_list.pixmap_info[ix];
  x = pi->x + plw->pixmap_list.x;
  y = pi->y + plw->pixmap_list.y;
  if(x >= (int)plw->core.width || x + (int)pi->width <= 0 ||
      y >= (int)plw->core.height || y + (int)pi->height <= 0)
    return;
  if(pi->pixmap != None)
    XCopyArea(XtDisplay(plw), pi->pixmap, XtWindow(plw),
	plw->pixmap_list.gc, 0, 0, pi->width, pi->height,
	x, y);
  if(plw->pixmap_list.border_width)
    XDrawRectangle(XtDisplay(plw), XtWindow(plw), plw->pixmap_list.border_gc,
	x - (plw->pixmap_list.border_width + 1) / 2,
	y - (plw->pixmap_list.border_width + 1) / 2,
	pi->width + plw->pixmap_list.border_width,
	pi->height + plw->pixmap_list.border_width);
  if(ix == plw->pixmap_list.was)
    {
      XFillRectangle(XtDisplay(plw), XtWindow(plw), plw->pixmap_list.gc,
	  x, y, pi->width, pi->height);
      return;
    }
  if(ix == plw->pixmap_list.selected &&
      plw->pixmap_list.highlit != False)
    XDrawRectangle(XtDisplay(plw), XtWindow(plw), plw->pixmap_list.gc,
	x + (plw->pixmap_list.highlight_thickness >> 1),
	y + (plw->pixmap_list.highlight_thickness >> 1),
	pi->width - plw->pixmap_list.highlight_thickness,
	pi->height - plw->pixmap_list.highlight_thickness);
  return;
}
/*}}}*/
/*{{{  void PaintRect(plw, start, length)*/
static VOIDFUNC PaintRect
FUNCARG((plw, start, length),
	PixmapListWidget plw
ARGSEP  Position  start     /* starting offset */
ARGSEP  Position  length    /* length of rectangle */
)
/* paint all the pixmaps which fall within the specified
 * distance allong the widget
 * clears to the background too.
 */
{
  PixmapInfo *pi;
  unsigned  ix;
  Position  offset;
  Position  size;
  unsigned  count;
  
  /*{{{  set offset and size*/
  if(plw->pixmap_list.orient == XtorientVertical)
    {
      offset = plw->pixmap_list.y;
      size = plw->core.height;
    }
  else
    {
      offset = plw->pixmap_list.x;
      size = plw->core.width;
    }
  /*}}}*/
  /*{{{  clip start*/
  if(start + offset < 0)
    {
      length += start + offset;
      start = -offset;
    }
  /*}}}*/
  /*{{{  clip length*/
  if(offset + start + length > size)
    length = size - offset - start;
  /*}}}*/
  /*{{{  clear before first?*/
  if(start < 0)
    {
      if(plw->pixmap_list.orient == XtorientVertical)
	XClearArea(XtDisplay((Widget)plw), XtWindow((Widget)plw),
	    0, start + offset, plw->core.width, -start, False);
      else
	XClearArea(XtDisplay((Widget)plw), XtWindow((Widget)plw),
	    start + offset, 0, -start, plw->core.height, False);
      length += start;
      start = 0;
    }
  /*}}}*/
  /*{{{  find first pixmap*/
  for(ix = 0, pi = plw->pixmap_list.pixmap_info,
      count = plw->pixmap_list.num_pixmaps;
      count; ix++, pi++, count--)
    if(start < (plw->pixmap_list.orient == XtorientVertical ?
	pi->y + pi->height : pi->x + pi->width) +
	plw->pixmap_list.border_width)
      break;
  /*}}}*/
  for(; length > 0 && count; count--, ix++, pi++)
    if(plw->pixmap_list.orient == XtorientVertical)
      /*{{{  vertical*/
      {
	XClearArea(XtDisplay((Widget)plw), XtWindow((Widget)plw),
	    0, pi->y + plw->pixmap_list.y - plw->pixmap_list.border_width -
	      plw->pixmap_list.distance, plw->core.width,
	      plw->pixmap_list.distance, False);
	if(plw->pixmap_list.width >= 0)
	  {
	    int       width;
	    
	    width = pi->x + plw->pixmap_list.x -
		plw->pixmap_list.border_width;
	    if(width > 0)
	      XClearArea(XtDisplay((Widget)plw), XtWindow((Widget)plw),
		  0, pi->y + plw->pixmap_list.y -
		  plw->pixmap_list.border_width,
		  width, pi->height + 2 * plw->pixmap_list.border_width,
		  False);
	    width = pi->x + plw->pixmap_list.x + pi->width +
		plw->pixmap_list.border_width;
	    if((int)plw->core.width - width > 0)
	      XClearArea(XtDisplay((Widget)plw), XtWindow((Widget)plw),
		  width, pi->y + plw->pixmap_list.y -
		  plw->pixmap_list.border_width,
		  plw->core.width - width,
		  pi->height + 2 * plw->pixmap_list.border_width, False);
	  }
	PaintPixmap(plw, pi, ix);
	length -= pi->y + pi->height + plw->pixmap_list.border_width - start;
	start = pi->y + pi->height + plw->pixmap_list.border_width;
      }
      /*}}}*/
    else
      /*{{{  horizontal*/
      {
	XClearArea(XtDisplay((Widget)plw), XtWindow((Widget)plw),
	    pi->x + plw->pixmap_list.x - plw->pixmap_list.border_width -
	      plw->pixmap_list.distance, 0, plw->pixmap_list.distance,
	      plw->core.height, False);
	if(plw->pixmap_list.height >= 0)
	  {
	    int       height;
	    
	    height = pi->y + plw->pixmap_list.y -
		plw->pixmap_list.border_width;
	    if(height > 0)
	      XClearArea(XtDisplay((Widget)plw), XtWindow((Widget)plw),
		  pi->x + plw->pixmap_list.x - plw->pixmap_list.border_width,
		  0, pi->width + 2 * plw->pixmap_list.border_width,
		  height, False);
	    height = pi->y + plw->pixmap_list.y + pi->height +
		plw->pixmap_list.border_width;
	    if((int)plw->core.height - height > 0)
	      XClearArea(XtDisplay((Widget)plw), XtWindow((Widget)plw),
		  pi->x + plw->pixmap_list.x - plw->pixmap_list.border_width,
		  height, pi->width + 2 * plw->pixmap_list.border_width,
		  plw->core.height - height, False);
	  }
	PaintPixmap(plw, pi, ix);
	length -= pi->x + pi->width + plw->pixmap_list.border_width - start;
	start = pi->x + pi->width + plw->pixmap_list.border_width;
      }
      /*}}}*/
  /*{{{  clear after last?*/
  if(length > 0)
    {
      if(plw->pixmap_list.orient == XtorientVertical)
	XClearArea(XtDisplay((Widget)plw), XtWindow((Widget)plw),
	    0, start + offset, plw->core.width, length, False);
      else
	XClearArea(XtDisplay((Widget)plw), XtWindow((Widget)plw),
	    start + offset, 0, length, plw->core.height, False);
    }
  /*}}}*/
  return;
}
/*}}}*/
/*{{{  void GetGC(plw)*/
static VOIDFUNC GetGC
FUNCARG((widget),
	PixmapListWidget widget
)
/* get the two gcs which we need
 */
{
  XGCValues     values;
  
  values.foreground = widget->pixmap_list.foreground;
  values.line_width = widget->pixmap_list.highlight_thickness;
  widget->pixmap_list.gc = XtGetGC((Widget)widget, GCForeground | GCLineWidth,
      &values);
  values.foreground = widget->pixmap_list.border_color;
  values.line_width = widget->pixmap_list.border_width;
  widget->pixmap_list.border_gc = XtGetGC((Widget)widget,
      GCForeground | GCLineWidth, &values);
  return;
}
/*}}}*/
/*{{{  void GetPixmapSize(plw, pi)*/
static VOIDFUNC GetPixmapSize
FUNCARG((plw, pi),
	PixmapListWidget plw
ARGSEP  PixmapInfo *pi
)
/* find the size of a pixmap, returns 0 if None
 */
{
  if(pi->pixmap != None)
    {
      Window  root;
      int x, y;
      unsigned border;
      unsigned depth;
      Status status;
      
      status = XGetGeometry(XtDisplay(plw), pi->pixmap,
	  &root, &x, &y, (unsigned int *)&pi->width,
	  (unsigned int *)&pi->height, &border, &depth);
    }
  else
    pi->width = pi->height = 0;
  return;
}
/*}}}*/
/*{{{  int GetPointerPosition(plw, event, x, y)*/
static int GetPointerPosition
FUNCARG((plw, event, xp, yp),
	PixmapListWidget plw
ARGSEP  XEvent    *event
ARGSEP  Position  *xp       /* x offset return */
ARGSEP  Position  *yp       /* y offset return */
)
/* get the pointer position
 * returns the selected pixmap, or -1 for none
 * sets the pointer offset within that pixmap
 */
{
  unsigned  got;
  Position  x;
  Position  y;
  
  /*{{{  get position from event*/
  switch(event->type)
  {
    case ButtonPress:
    case ButtonRelease:
      x = event->xbutton.x;
      y = event->xbutton.y;
      got = 0;
      break;
    case MotionNotify:
      x = event->xmotion.x;
      y = event->xmotion.y;
      got = 0;
      break;
    case EnterNotify:
    case LeaveNotify:
      x = event->xcrossing.x;
      y = event->xcrossing.y;
      got = 0;
      break;
    default:
      x = y = 0;
      got = -1;
  }
  /*}}}*/
  if(!got)
    {
      Cardinal  index;
      PixmapInfo *pi;
      
      x -= plw->pixmap_list.x;
      y -= plw->pixmap_list.y;
      got = -1;
      for(index = plw->pixmap_list.num_pixmaps,
	  pi = plw->pixmap_list.pixmap_info;
	  index--; pi++)
	if(x >= pi->x && x < pi->x + pi->width &&
	    y >= pi->y && y < pi->y + pi->height)
	  {
	    got = plw->pixmap_list.num_pixmaps - 1 - index;
	    x -= pi->x;
	    y -= pi->y;
	    break;
	  }
      *xp = x;
      *yp = y;
    }
  return got;
}
/*}}}*/
/*{{{  void RemoveTimeOut(plw)*/
static VOIDFUNC RemoveTimeOut
FUNCARG((plw),
	PixmapListWidget plw
)
/* remove the flash timeout, if set
 */
{
  if(plw->pixmap_list.timeout != (XtIntervalId)0)
    {
      XtRemoveTimeOut(plw->pixmap_list.timeout);
      plw->pixmap_list.timeout = (XtIntervalId)0;
      plw->pixmap_list.was = -1;
    }
  return;
}
/*}}}*/
/*{{{  void ScrollJump(widget, client, call)*/
static VOIDFUNC ScrollJump
FUNCARG((widget, client, call),
	Widget    widget
ARGSEP  XtPointer client
ARGSEP  XtPointer call
)
/* jumpscroll callbac from the scrollbar
 * set the offset approriately, and redraw the widget
 */
{
  PixmapListWidget plw;
  int       start;
  int       length;
  unsigned  reset;
  
  plw = (PixmapListWidget)client;
  start = (int)ceil(plw->pixmap_list.length * *(float *)call);
  length = plw->pixmap_list.orient == XtorientVertical ?
      plw->core.height : plw->core.width;
  reset = start + length > plw->pixmap_list.length;
  if(reset)
    start = plw->pixmap_list.length - length;
  if(start < 0)
    start = 0;
  length = 0;
  if(plw->pixmap_list.orient == XtorientVertical)
    {
      if(start != -plw->pixmap_list.y)
	{
	  plw->pixmap_list.y = -start;
	  length = plw->core.height;
	}
    }
  else
    {
      if(start != -plw->pixmap_list.x)
	{
	  plw->pixmap_list.x = -start;
	  length = plw->core.width;
	}
    }
  if(length)
    PaintRect(plw, start, length);
  if(reset)
    SetScrollbar(plw);
  return;
}
/*}}}*/
/*{{{  void ScrollScroll(widget, client, call)*/
static VOIDFUNC ScrollScroll
FUNCARG((widget, client, call),
	Widget    widget
ARGSEP  XtPointer client
ARGSEP  XtPointer call
)
/* scroll callback from the scrollbar.
 * adjust the offset, and redraw appropriately
 */
{
  PixmapListWidget plw;
  int       start;
  int       length;
  
  plw = (PixmapListWidget)client;
  if(!(int)call)
    return;
  length = plw->pixmap_list.orient == XtorientVertical ?
      plw->core.height : plw->core.width;
  start = length / 2;
  if((int)call > 0)
    start = -start;
  start -= plw->pixmap_list.orient == XtorientVertical ?
      plw->pixmap_list.y : plw->pixmap_list.x;
  if(start < 0)
    start = 0;
  else if(start + length > plw->pixmap_list.length)
    start = plw->pixmap_list.length - length;
  length = 0;
  if(plw->pixmap_list.orient == XtorientVertical)
    {
      if(start != -plw->pixmap_list.y)
	{
	  plw->pixmap_list.y = -start;
	  length = plw->core.height;
	}
    }
  else
    {
      if(start != -plw->pixmap_list.x)
	{
	  plw->pixmap_list.x = -start;
	  length = plw->core.width;
	}
    }
  if(length)
    {
      PaintRect(plw, start, length);
      SetScrollbar(plw);
    }
  return;
}
/*}}}*/
/*{{{  void SetScrollbar(plw)*/
static VOIDFUNC SetScrollbar
FUNCARG((plw),
	PixmapListWidget plw
)
/* set the scrollbar scales appropriately
 * this needs to be done when
 *  we're resized
 *  we insert a pixmap
 *  we remove a pixmap
 *  our geometry changes
 */
{
  float     shown, top;
  Arg       args[2];
  
  if(!plw->pixmap_list.scroll)
    return;
  shown = (float)(plw->pixmap_list.orient == XtorientVertical ?
      plw->core.height + 1 : plw->core.width + 1) /
      (float)plw->pixmap_list.length;
  if(shown > (float)1.0)
    shown = (float)1.0;
  top = (float)(plw->pixmap_list.orient == XtorientVertical ?
      -plw->pixmap_list.y : -plw->pixmap_list.x) /
      (float)plw->pixmap_list.length;
  XtSetArg(args[0], XtNtopOfThumb, sizeof(float) > sizeof(XtArgVal) ?
      (XtArgVal)&top : *(XtArgVal *)&top);
  XtSetArg(args[1], XtNshown, sizeof(float) > sizeof(XtArgVal) ?
      (XtArgVal)&shown : *(XtArgVal *)&shown);
  XtSetValues(plw->pixmap_list.scroll, args, 2);
  return;
}
/*}}}*/
/*{{{  void TimeOut(data, id)*/
static VOIDFUNC TimeOut
FUNCARG((data, id),
	XtPointer data
ARGSEP  XtIntervalId *id
)
/* unflash the selected pixmap
 */
{
  PixmapListWidget plw;
  int         ix;
  
  plw = (PixmapListWidget)data;
  plw->pixmap_list.timeout = (XtIntervalId)0;
  ix = plw->pixmap_list.was;
  plw->pixmap_list.was = -1;
  if(XtIsRealized((Widget)plw) && ix >= 0)
    PaintPixmap(plw, NULL, ix);
  return;
}
/*}}}*/
