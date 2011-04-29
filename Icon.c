/* Copyright (C) 1993 Nathan Sidwell */
/* RCS $Id: Icon.c 1.2 Tue, 16 Mar 1999 11:28:16 +0000 nathan $ */
/* this widget holds a pixmap which it can select a row/column from
 * similar to a command widget, or initiate a drag on press and move
 */
/*{{{  includes*/
#include "ansiknr.h"
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/SimpleP.h>
#include "Icon.h"
#include "Drag.h"
/*}}}*/
/*{{{  structs*/
/*{{{  typedef struct _IconClass*/
typedef struct _IconClass
{
  int     ansi_compliance; /* not used */
} IconClassPart;
/*}}}*/
/*{{{  typedef struct _IconClassRec*/
typedef struct _IconClassRec
{
  CoreClassPart   core_class;
  SimpleClassPart simple_class;
  IconClassPart   icon_class;
} IconClassRec;
/*}}}*/
/*{{{  typedef struct _IconPart*/
typedef struct
{
  /* resources */
  Pixmap    pixmap;           /* the icon to display */
  XtCallbackList callbacks;   /* callbacks to notify */
  Dimension columns;          /* cells across to highlight */
  Dimension rows;             /* cells down to highlight */
  Dimension drag_sensitivity; /* sensitivity of drag callback */
  Dimension highlight_thickness;  /* thickness of highlight bar */
  Pixel     foreground;       /* foreground pixel color */
  int       flash_delay;      /* time to flash */
  String    drag_name;        /* name of drag */
  
  /* private state */
  unsigned  width;          /* pixmap width */
  unsigned  height;         /* pixmap height */
  GC        gc;             /* GC to draw */
  Position  x;              /* corner of pixmap */
  Position  y;              /* corner of pixmap */
  Position  drag_x;         /* drag start x */
  Position  drag_y;         /* drag start y */
  Position  selected;       /* selected cell */
  Boolean   drag_set;       /* drag started */
  Boolean   highlit;        /* is it highlit? */
  Boolean   solid;          /* solid foreground */
  XtIntervalId timeout;     /* timout id */
} IconPart;
/*}}}*/
/*{{{  typedef struct _IconRec*/
typedef struct _IconRec
{
  CorePart    core;
  SimplePart  simple;
  IconPart    icon;
} IconRec;
/*}}}*/
/*}}}*/
/*{{{  resources*/
static XtResource resources[] =
{
  {XtNforeground, XtCForeground, XtRPixel, sizeof(Pixel),
      XtOffsetOf(IconRec, icon.foreground),
      XtRString, (XtPointer)XtDefaultForeground},
  {MredNhighlightThickness, XtCThickness, XtRDimension, sizeof(Dimension),
      XtOffsetOf(IconRec, icon.highlight_thickness),
      XtRImmediate, (XtPointer)1},
  {XtNpixmap,   XtCPixmap, XtRBitmap, sizeof(Pixmap),
      XtOffsetOf(IconRec, icon.pixmap), XtRImmediate, (XtPointer)None},
  {MredNcolumns,  XtCWidth, XtRDimension, sizeof(Dimension),
      XtOffsetOf(IconRec, icon.columns), XtRImmediate, (XtPointer)1},
  {MredNrows,    XtCHeight, XtRDimension, sizeof(Dimension),
      XtOffsetOf(IconRec, icon.rows), XtRImmediate, (XtPointer)1},
  {XtNcallback, XtCCallback, XtRCallback, sizeof(XtPointer),
      XtOffsetOf(IconRec, icon.callbacks), XtRCallback, (XtPointer)NULL},
  {MredNdragSensitivity, XtCThickness, XtRDimension, sizeof(Dimension),
      XtOffsetOf(IconRec, icon.drag_sensitivity), XtRImmediate, (XtPointer)4},
  {MredNflashDelay, XtCInterval, XtRInt, sizeof(int),
      XtOffsetOf(IconRec, icon.flash_delay), XtRImmediate, (XtPointer)100},
  {MredNdragName, XtCLabel, XtRString, sizeof(String),
      XtOffsetOf(IconRec, icon.drag_name), XtRImmediate, (XtPointer)"drag"},
};
/*}}}*/
/*{{{  prototypes*/
static VOIDFUNC Drag PROTOARG((Widget, XEvent *, String *, Cardinal *));
static VOIDFUNC Highlight PROTOARG((Widget, XEvent *, String *, Cardinal *));
static VOIDFUNC Notify PROTOARG((Widget, XEvent *, String *, Cardinal *));
static VOIDFUNC Set PROTOARG((Widget, XEvent *, String *, Cardinal *));
static VOIDFUNC Unhighlight PROTOARG((Widget, XEvent *, String *, Cardinal *));

static VOIDFUNC Destroy PROTOARG((Widget));
static VOIDFUNC Initialize PROTOARG((Widget, Widget, ArgList, Cardinal *));
static XtGeometryResult QueryGeometry
    PROTOARG((Widget, XtWidgetGeometry *, XtWidgetGeometry *));
static VOIDFUNC Redisplay PROTOARG((Widget, XEvent *, Region));
static VOIDFUNC Resize PROTOARG((Widget));
static Boolean SetValues
    PROTOARG((Widget, Widget, Widget, ArgList, Cardinal *));

#define FakeExpose(iw)  Redisplay((Widget)iw, NULL, NULL)
static VOIDFUNC GetGC PROTOARG((IconWidget));
static VOIDFUNC GetPixmapSize PROTOARG((IconWidget));
static unsigned GetPointerPosition
    PROTOARG((IconWidget, XEvent *, Position *, Position *));
static VOIDFUNC RemoveTimeOut PROTOARG((IconWidget));
static VOIDFUNC TimeOut PROTOARG((XtPointer, XtIntervalId *));
/*}}}*/
/*{{{  translations*/
static char translations[] = "\
<BtnDown>:set()\n\
<BtnUp>:notify()\n\
<Motion>:highlight() drag()\n\
<EnterWindow>:highlight()\n\
<LeaveWindow>:unhighlight()\
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
/*{{{  IconClassRec iconClassRec =*/
IconClassRec iconClassRec =
{
  /*{{{  core class part*/
  {
    SuperClass,                       /* superclass */
    "Icon",                           /* class_name */
    sizeof(IconRec),                  /* size */
    NULL,                             /* class_initialize */
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
  /*{{{  icon class part*/
  {
    0,        /* dummy */
  },
  /*}}}*/
};
/*}}}*/
WidgetClass iconWidgetClass = (WidgetClass)&iconClassRec;
/* actions */
/*{{{  void Drag(widget, event, params, num_params)*/
static VOIDFUNC Drag
FUNCARG((widget, event, params, num_params),
	Widget    widget
ARGSEP  XEvent    *event
ARGSEP  String    *params
ARGSEP  Cardinal  *num_params
)
/* detect dragging
 * if we moved >= drag sensitivity since the button press then
 * invoke a drag widget
 * the drag widget is found by cheking each level of the widget tree
 * for a drag child, going up from this widget.
 * if started, any current selection is cleared
 * Should be called on button type events
 */
{
  IconWidget iw;
  Position  x, y;
  
  iw = (IconWidget)widget;
  x = y = 0;
  if(event->type != MotionNotify && event->type != ButtonPress)
    /* EMPTY */;
  else if(event->type == MotionNotify && !(event->xmotion.state &
    (Button1Mask | Button2Mask | Button3Mask | Button4Mask | Button5Mask)))
    /* EMPTY */;
  else if(!GetPointerPosition(iw, event, &x, &y))
    /* EMPTY */;
  else if(iw->icon.drag_set == False)
    {
      iw->icon.drag_x = x;
      iw->icon.drag_y = y;
      iw->icon.drag_set = True;
    }
  else if(iw->icon.drag_sensitivity && iw->icon.drag_name &&
      iw->icon.drag_sensitivity * iw->icon.drag_sensitivity <=
      (x - iw->icon.drag_x) * (x - iw->icon.drag_x) + 
      (y - iw->icon.drag_y) * (y - iw->icon.drag_y))
    /*{{{  popup drag*/
    {
      Widget  drag;
      Widget  parent;
      
      drag = NULL;
      for(parent = (Widget)iw; parent; parent = XtParent(parent))
	{
	  drag = XtNameToWidget(parent, iw->icon.drag_name);
	  if(drag)
	    break;
	}
      if(drag)
	{
	  Position  rx, ry;
	  
	  iw->icon.highlit = False;
	  FakeExpose(iw);
	  XtTranslateCoords((Widget)iw, iw->icon.x, iw->icon.y, &rx, &ry);
	  DragPopup((Widget)iw, (Widget)drag, iw->icon.pixmap,
	      iw->icon.drag_x, iw->icon.drag_y,
	      rx + x, ry + y, event->xmotion.time);
	}
    }
    /*}}}*/
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
/* Determines which cell of the widget is selected, and
 * redraws if changed.
 * The drag position is reset, if a new cell is selected.
 * Should be invoked on pointer movement events.
 */
{
  IconWidget iw;
  Position  x, y;
  int       selected;
  
  iw = (IconWidget)widget;
  selected = GetPointerPosition(iw, event, &x, &y);
  if(selected)
    {
      if(selected > 1 || !iw->icon.highlight_thickness)
	selected = -1;
      else
	selected = y / (iw->icon.height / iw->icon.rows) * iw->icon.columns +
	    x / (iw->icon.width / iw->icon.columns);
      if(selected != iw->icon.selected ||
	  (iw->icon.highlit == False && selected >= 0))
	{
	  iw->icon.drag_x = x;
	  iw->icon.drag_y = y;
	  iw->icon.selected = selected;
	  iw->icon.highlit = selected < 0 ? False : True;
	  if(selected >= 0)
	    RemoveTimeOut(iw);
	  FakeExpose(iw);
	}
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
/* notifies of the current selection.
 * If none is selected nothing happens.
 * otherwise the callback is called, and the widget flashed
 * in its foreground color
 * Normally called on button release
 */
{
  IconWidget iw;
  
  iw = (IconWidget)widget;
  iw->icon.drag_set = False;
  if(iw->icon.selected >= 0)
    {
      IconCallback data;
      
      if(iw->icon.flash_delay)
	{
	  iw->icon.solid = True;
	  FakeExpose(iw);
	  iw->icon.timeout =
	      XtAppAddTimeOut(XtWidgetToApplicationContext((Widget)iw),
	      (unsigned long)iw->icon.flash_delay, TimeOut, (XtPointer)iw);
	}
      data.selection = iw->icon.selected;
      if(event->type == ButtonPress || event->type == ButtonRelease)
	data.button = event->xbutton.button;
      else
	data.button = -1;
      XtCallCallbackList((Widget)iw, iw->icon.callbacks, (XtPointer)&data);
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
/* initialize the drag position
 * normally called on button press
 */
{
  IconWidget iw;
  
  iw = (IconWidget)widget;
  GetPointerPosition(iw, event, &iw->icon.drag_x, &iw->icon.drag_y);
  iw->icon.drag_set = True;
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
/* deselect any selected area and reset the drag
 * Normally called on LeaveWindow
 */
{
  IconWidget iw;
  
  iw = (IconWidget)widget;
  if(iw->icon.highlit != False)
    {
      iw->icon.highlit = False;
      FakeExpose(iw);
    }
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
  IconWidget iw;
  
  iw = (IconWidget)widget;
  XtReleaseGC((Widget)iw, iw->icon.gc);
  RemoveTimeOut(iw);
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
/* allocate ourselves a GC, and set out default size
 */
{
  IconWidget niw;
  
  niw = (IconWidget)tnew;
  niw->icon.selected = -1;
  niw->icon.highlit = False;
  niw->icon.drag_set = False;
  niw->icon.solid = False;
  niw->icon.timeout = (XtIntervalId)0;
  GetPixmapSize(niw);
  if(!niw->core.width)
    niw->core.width = niw->icon.width ? niw->icon.width : 16;
  if(!niw->core.height)
    niw->core.height = niw->icon.height ? niw->icon.height : 16;
  GetGC(niw);
  if(!niw->icon.columns)
    niw->icon.columns = 1;
  if(!niw->icon.rows)
    niw->icon.rows = 1;
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
/* tell out parent what size we'd like to be
 */
{
  IconWidget iw;
  
  iw = (IconWidget)widget;
  answer->request_mode = CWWidth | CWHeight;
  answer->height = iw->icon.width ? iw->icon.width : 16;
  answer->width = iw->icon.height ? iw->icon.height : 16;
  if((proposed->request_mode & (CWWidth | CWHeight)) == (CWWidth | CWHeight) &&
      proposed->width == answer->width && proposed->height == answer->height)
    return XtGeometryYes;
  else if(answer->width == iw->core.width && answer->height == iw->core.height)
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
 * We don't bother with a clip, just do the whole lot
 * This involves blitting the pixmap
 * drawing the select box
 * Drawing the flash box
 */
{
  IconWidget  iw;
  
  if(!XtIsRealized(widget))
    return;
  iw = (IconWidget)widget;
  if(iw->icon.pixmap != None)
    XCopyArea(XtDisplay(iw), iw->icon.pixmap, XtWindow(iw), iw->icon.gc, 0, 0,
      iw->icon.width, iw->icon.height, iw->icon.x, iw->icon.y);
  if(iw->icon.solid != False)
    XFillRectangle(XtDisplay(iw), XtWindow(iw), iw->icon.gc,
	iw->icon.x + iw->icon.selected % iw->icon.columns *
	  iw->icon.width / iw->icon.columns,
	iw->icon.y + iw->icon.selected / iw->icon.columns *
	iw->icon.height / iw->icon.rows,
	iw->icon.width / iw->icon.columns, iw->icon.height / iw->icon.rows);
  else if(iw->icon.highlit != False)
    XDrawRectangle(XtDisplay(iw), XtWindow(iw), iw->icon.gc,
	iw->icon.x + iw->icon.selected % iw->icon.columns *
	  iw->icon.width / iw->icon.columns +
	  (iw->icon.highlight_thickness >> 1),
	iw->icon.y + iw->icon.selected / iw->icon.columns *
	  iw->icon.height / iw->icon.rows +
	  (iw->icon.highlight_thickness >> 1),
	iw->icon.width / iw->icon.columns - iw->icon.highlight_thickness,
	iw->icon.height / iw->icon.rows - iw->icon.highlight_thickness);
  return;
}
/*}}}*/
/*{{{  void Resize(widget)*/
static VOIDFUNC Resize
FUNCARG((widget),
	Widget    widget
)
/* recenter the pixmap, when w get resized
 */
{
  IconWidget iw;
  
  iw = (IconWidget)widget;
  iw->icon.x = (iw->core.width - iw->icon.width) / 2;
  iw->icon.y = (iw->core.height - iw->icon.height) / 2;
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
/* need to get a new GC if color or highlight width changes
 * need to calculate a new size if the pixmap changes
 */
{
  Boolean  redraw;
  IconWidget ciw;
  IconWidget niw;
  
  redraw = False;
  ciw = (IconWidget)cw;
  niw = (IconWidget)nw;
  if(ciw->icon.foreground != niw->icon.foreground ||
      ciw->icon.highlight_thickness != niw->icon.highlight_thickness)
    {
      redraw = True;
      XtReleaseGC(cw, ciw->icon.gc);
      GetGC(niw);
    }
  if(ciw->icon.pixmap != niw->icon.pixmap)
    {
      redraw = True;
      GetPixmapSize(niw);
      if(niw->icon.width)
	{
	  niw->core.width = niw->icon.width;
	  niw->icon.x = 0;
	}
      if(niw->icon.height)
	{
	  niw->core.height = niw->icon.height;
	  niw->icon.y = 0;
	}
    }
  if(!niw->icon.columns)
    niw->icon.columns = 1;
  if(!niw->icon.rows)
    niw->icon.rows = 1;
  if(ciw->icon.columns != niw->icon.columns ||
      ciw->icon.rows != niw->icon.rows)
    {
      niw->icon.selected = -1;
      niw->icon.highlit = False;
    }
  if(ciw->core.width != niw->core.width ||
      ciw->core.height != niw->core.height)
    redraw = False;
  return redraw;
}
/*}}}*/
/* public routines */
/*{{{  void IconRepaint(widget)*/
extern VOIDFUNC IconRepaint
FUNCARG((widget),
	Widget widget
)
/* repaint the widget, without the anoying flicker caused by an expose
 * event
 */
{
  if(XtIsSubclass(widget, (WidgetClass)&iconClassRec))
    FakeExpose((IconWidget)widget);
  return;
}
/*}}}*/
/* private routines */
/*{{{  void GetGC(iw)*/
static VOIDFUNC GetGC
FUNCARG((widget),
	IconWidget    widget
)
/* get an approptiate GC
 */
{
  XGCValues     values;
  
  values.foreground = widget->icon.foreground;
  values.line_width = widget->icon.highlight_thickness;
  widget->icon.gc = XtGetGC((Widget)widget, GCForeground | GCLineWidth,
      &values);
  return;
}
/*}}}*/
/*{{{  void GetPixmapSize(iw)*/
static VOIDFUNC GetPixmapSize
FUNCARG((widget),
	IconWidget  widget
)
/* find out the pixmap size
 * if pixmap is None then set to 0
 * note this interrogates the server
 */
{
  if(widget->icon.pixmap != None)
    {
      Window  root;
      int x, y;
      unsigned border;
      unsigned depth;
      
      XGetGeometry(XtDisplay(widget), widget->icon.pixmap, &root,
	  &x, &y, &widget->icon.width, &widget->icon.height, &border, &depth);
    }
  else
    widget->icon.width = widget->icon.height = 0;
  return;
}
/*}}}*/
/*{{{  unsigned GetPointerPosition(iw, event, x, y)*/
static unsigned GetPointerPosition
FUNCARG((iw, event, xp, yp),
	IconWidget iw
ARGSEP  XEvent    *event    /* event to query */
ARGSEP  Position  *xp       /* x coordinate return */ 
ARGSEP  Position  *yp       /* y coordinate return */
)
/* find the pointer position on widget window.
 * returns 0 if not a pointer event
 * returns 1 if inside the pixmap
 * returns 2 if outside pixmap && inside widget
 * returns 3 if outisde widget
 * clips to pixmap
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
      got = 1;
      break;
    case MotionNotify:
      x = event->xmotion.x;
      y = event->xmotion.y;
      got = 1;
      break;
    case EnterNotify:
    case LeaveNotify:
      x = event->xcrossing.x;
      y = event->xcrossing.y;
      got = 1;
      break;
    default:
      x = y = 0;
      got = 0;
  }
  /*}}}*/
  if(got)
    /*{{{  convert to pixmap coord*/
    {
      unsigned  out;
      
      out = event->type != EnterNotify &&
	  (x < 0 || x >= (Position)iw->core.width ||
	  y < 0 || y >= (Position)iw->core.height);
      x -= iw->icon.x;
      y -= iw->icon.y;
      if(x < 0)
	{
	  x = 0;
	  got = 2;
	}
      else if(x >= iw->icon.width)
	{
	  x = iw->icon.width - 1;
	  got = 2;
	}
      if(y < 0)
	{
	  y = 0;
	  got = 2;
	}
      else if(y >= iw->icon.height)
	{
	  y = iw->icon.height - 1;
	  got = 2;
	}
      if(out)
	got = 3;
      *xp = x;
      *yp = y;
    }
    /*}}}*/
  return got;
}
/*}}}*/
/*{{{  void RemoveTimeOut(iw)*/
static VOIDFUNC RemoveTimeOut
FUNCARG((iw),
	IconWidget iw
)
/* remove the flash timeout, if its set
 */
{
  if(iw->icon.timeout != (XtIntervalId)0)
    {
      XtRemoveTimeOut(iw->icon.timeout);
      iw->icon.timeout = (XtIntervalId)0;
      iw->icon.solid = False;
    }
  return;
}
/*}}}*/
/*{{{  void TimeOut(data, id)*/
static VOIDFUNC TimeOut
FUNCARG((data, id),
	XtPointer data
ARGSEP  XtIntervalId *id
)
/* unflashes the widget
 */
{
  IconWidget iw;
  
  iw = (IconWidget)data;
  iw->icon.solid = False;
  iw->icon.timeout = (XtIntervalId)0;
  FakeExpose(iw);
  return;
}
/*}}}*/
