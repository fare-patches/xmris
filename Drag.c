/* Copyright (C) 1993 Nathan Sidwell */
/* RCS $Id: Drag.c,v 4.13 1993/11/01 16:23:50 nathan stable $ */
/* the drag widget is an override shell to be popped up
 * and follow the pointer until released.
 * A pixmap is displayed in the widget.
 * This should probably just be a composite widget whose child
 * does the painting. Ho hum
 */
/*{{{  includes*/
#include "ansiknr.h"
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/SimpleP.h>
#include <X11/ShellP.h>
#include "Drag.h"
/*}}}*/
/*{{{  structs*/
/*{{{  typedef struct _DragClass*/
typedef struct _DragClass
{
  int     ansi_compliance; /* not used */
} DragClassPart;
/*}}}*/
/*{{{  typedef struct _DragClassRec*/
typedef struct _DragClassRec
{
  CoreClassPart   core_class;
  CompositeClassPart   composite_class;
  ShellClassPart shell_class;
  OverrideShellClassPart override_shell_class;
  DragClassPart   drag_class;
} DragClassRec;
/*}}}*/
/*{{{  typedef struct _DragPart*/
typedef struct
{
  /* resources */
  Pixmap    pixmap;           /* the icon to display */
  XtCallbackList callbacks;   /* callbacks to notify */
  WidgetList choices;         /* choices of selection */
  Cardinal  num_choices;      /* number of choices */
  Cursor    cursor;           /* cursor to use */
  
  /* private state */
  unsigned  width;          /* pixmap width */
  unsigned  height;         /* pixmap height */
  GC        gc;             /* GC to draw */
  Position  x;              /* corner of pixmap */
  Position  y;              /* corner of pixmap */
  Position  offset_x;       /* x offset */
  Position  offset_y;       /* y offset */
  Widget    widget;         /* invoker */
} DragPart;
/*}}}*/
/*{{{  typedef struct _DragRec*/
typedef struct _DragRec
{
  CorePart    core;
  CompositePart  composite;
  ShellPart  shell;
  OverrideShellPart  override;
  DragPart    drag;
} DragRec;
/*}}}*/
/*}}}*/
/*{{{  resources*/
static XtResource resources[] =
{
  {XtNpixmap,   XtCPixmap, XtRBitmap, sizeof(Pixmap),
      XtOffsetOf(DragRec, drag.pixmap), XtRImmediate, (XtPointer)None},
  {XtNcallback, XtCCallback, XtRCallback, sizeof(XtPointer),
      XtOffsetOf(DragRec, drag.callbacks), XtRCallback, (XtPointer)NULL},
  {MredNwidgetChoices, MredCWidgetList, XtRWidgetList, sizeof(WidgetList),
      XtOffsetOf(DragRec, drag.choices), XtRImmediate, (XtPointer)0},
  {MredNnumWidgetChoices, XtCIndex, XtRCardinal, sizeof(Cardinal),
      XtOffsetOf(DragRec, drag.num_choices), XtRImmediate, (XtPointer)0},
  {XtNcursor, XtCCursor, XtRCursor, sizeof(Cursor),
      XtOffsetOf(DragRec, drag.cursor), XtRString, (XtPointer)"left_ptr"},
};
/*}}}*/
/*{{{  prototypes*/
static VOIDFUNC Drag PROTOARG((Widget, XEvent *, String *, Cardinal *));
static VOIDFUNC Notify PROTOARG((Widget, XEvent *, String *, Cardinal *));

static VOIDFUNC ClassPartInitialize PROTOARG((WidgetClass));
static VOIDFUNC Destroy PROTOARG((Widget));
static VOIDFUNC Initialize PROTOARG((Widget, Widget, ArgList, Cardinal *));
static XtGeometryResult QueryGeometry
    PROTOARG((Widget, XtWidgetGeometry *, XtWidgetGeometry *));
static VOIDFUNC Redisplay PROTOARG((Widget, XEvent *, Region));
static VOIDFUNC Resize PROTOARG((Widget));
static Boolean SetValues
    PROTOARG((Widget, Widget, Widget, ArgList, Cardinal *));

static VOIDFUNC GetGC PROTOARG((DragWidget));
static VOIDFUNC GetPixmapSize PROTOARG((DragWidget));
static unsigned GetPointerPosition
    PROTOARG((XEvent *, Position *, Position *));
/*}}}*/
/*{{{  translations*/
static char translations[] = "\
<BtnUp>:notify()\n\
<BtnMotion>:drag()\n\
";
/*}}}*/
/*{{{  actions*/
static XtActionsRec actions[] =
{
  {"drag", Drag},
  {"notify", Notify},
};
/*}}}*/
/*{{{  static CompositeClassExtensionRec compositeExtension =*/
static CompositeClassExtensionRec compositeExtension =
{
  NULL,
  NULLQUARK,
  XtCompositeExtensionVersion,
  sizeof(CompositeClassExtensionRec),
  False
};
/*}}}*/
#define SuperClass (WidgetClass)&overrideShellClassRec
/*{{{  DragClassRec dragClassRec =*/
DragClassRec dragClassRec =
{
  /*{{{  core class part*/
  {
    SuperClass,                        /* superclass */
    "Drag",                           /* class_name */
    sizeof(DragRec),                  /* size */
    NULL,                             /* class_initialize */
    ClassPartInitialize,              /* class_part_initialize */
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
  /*{{{  composite class part*/
  {
    XtInheritGeometryManager,
    XtInheritChangeManaged,
    XtInheritInsertChild,
    XtInheritDeleteChild,
    NULL
  },
  /*}}}*/
  /*{{{  shell class part*/
  {
    NULL
  },
  /*}}}*/
  /*{{{  override shell class part*/
  {
    NULL
  },
  /*}}}*/
  /*{{{  drag class part*/
  {
    0,        /* dummy */
  },
  /*}}}*/
};
/*}}}*/
WidgetClass dragWidgetClass = (WidgetClass)&dragClassRec;
/* actions */
/*{{{  void Drag(widget, event, params, num_params)*/
static VOIDFUNC Drag
FUNCARG((widget, event, params, num_params),
	Widget    widget
ARGSEP  XEvent    *event
ARGSEP  String    *params
ARGSEP  Cardinal  *num_params
)
/* Follows the widget to the current pointer position
 * If invoked by non pointer event, nothing happens
 */
{
  static Arg args[] = {{XtNx, 0}, {XtNy, 0}};
  DragWidget dw;
  Position  x, y;
  
  dw = (DragWidget)widget;
  if(GetPointerPosition(event, &x, &y))
    {
      args[0].value = x - dw->drag.offset_x - dw->core.border_width;
      args[1].value = y - dw->drag.offset_y - dw->core.border_width;
      XtSetValues((Widget)dw, args, XtNumber(args));
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
/* Ungrabs the pointer and pops down the widget
 * notifies the call back that we've selected a widget
 * Normally called on button up
 * searches the list of selectable widgets for the one underneath
 * the center of the widget.
 * if no widget is found then the callback is not called.
 */
{
  DragWidget dw;
  Position  x, y;
  
  XtUngrabPointer(widget, event->xbutton.time);
  XtPopdown(widget);
  dw = (DragWidget)widget;
  if(GetPointerPosition(event, &x, &y))
    {
      int       count;
      Widget    *wptr;
      
      x += dw->core.width / 2 - dw->drag.offset_x;
      y += dw->core.height / 2 - dw->drag.offset_y;
      for(wptr = dw->drag.choices, count = dw->drag.num_choices;
	  count--; wptr++)
	{
	  Position  rx, ry;
	  
	  XtTranslateCoords(*wptr, 0, 0, &rx, &ry);
	  if(XtIsRealized(*wptr) && rx <= x && ry <= y &&
	      rx + (int)((CorePart *)*wptr)->width > x &&
	      ry + (int)((CorePart *)*wptr)->height > y)
	    {
	      DragCallback data;
	      
	      data.selected = *wptr;
	      data.invoker = dw->drag.widget;
	      data.offset_x = x - rx;
	      data.offset_y = y - ry;
	      XtCallCallbackList((Widget)dw, dw->drag.callbacks,
		  (XtPointer)&data);
	      break;
	    }
	}
    }
  return;
}
/*}}}*/
/* class methods */
/*{{{  void ClassPartInitialize(widgetclass)*/
static VOIDFUNC ClassPartInitialize
FUNCARG((widgetclass),
	WidgetClass widgetclass
)
/* just add the composite class extension
 */
{
  DragWidgetClass dwclass;
  
  dwclass = (DragWidgetClass)widgetclass;
  compositeExtension.next_extension =
      dwclass->composite_class.extension;
  dwclass->composite_class.extension = (XtPointer)&compositeExtension;
  return;
}
/*}}}*/
/* methods */
/*{{{  void Destroy(widget)*/
static VOIDFUNC Destroy
FUNCARG((widget),
	Widget    widget
)
/* free the gc we use for drawing
 */
{
  DragWidget dw;
  
  dw = (DragWidget)widget;
  XtReleaseGC((Widget)dw, dw->drag.gc);
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
/* initialize widget instance,
 * allocate gc and set default size
 */
{
  DragWidget ndw;
  
  ndw = (DragWidget)tnew;
  GetPixmapSize(ndw);
  if(!ndw->core.width)
    ndw->core.width = ndw->drag.width ? ndw->drag.width : 16;
  if(!ndw->core.height)
    ndw->core.height = ndw->drag.height ? ndw->drag.height : 16;
  GetGC(ndw);
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
/* validate geometry request from parent
 * try to set to size of pixmap
 */
{
  DragWidget dw;
  
  dw = (DragWidget)widget;
  answer->request_mode = CWWidth | CWHeight;
  answer->height = dw->drag.width ? dw->drag.width : 16;
  answer->height = dw->drag.height ? dw->drag.height : 16;
  if((proposed->request_mode & (CWWidth | CWHeight)) == (CWWidth | CWHeight) &&
      proposed->width == answer->width && proposed->height == answer->height)
    return XtGeometryYes;
  else if(answer->width == dw->core.width && answer->height == dw->core.height)
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
/* repaint the widget on expose
 */
{
  DragWidget  dw;
  
  if(!XtIsRealized(widget))
    return;
  dw = (DragWidget)widget;
  if(dw->drag.pixmap != None)
    XCopyArea(XtDisplay(dw), dw->drag.pixmap, XtWindow(dw), dw->drag.gc, 0, 0,
      dw->drag.width, dw->drag.height, dw->drag.x, dw->drag.y);
  return;
}
/*}}}*/
/*{{{  void Resize(widget)*/
static VOIDFUNC Resize
FUNCARG((widget),
	Widget    widget
)
/* when resized, reset the pixmap corner so that its centered
 */
{
  DragWidget dw;
  
  dw = (DragWidget)widget;
  dw->drag.x = (dw->core.width - dw->drag.width) / 2;
  dw->drag.y = (dw->core.height - dw->drag.height) / 2;
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
/* recompute things if the pixmap changes
 */
{
  Boolean  redraw;
  DragWidget cdw;
  DragWidget ndw;
  
  redraw = False;
  cdw = (DragWidget)cw;
  ndw = (DragWidget)nw;
  if(cdw->drag.pixmap != ndw->drag.pixmap)
    {
      redraw = True;
      GetPixmapSize(ndw);
      if(ndw->drag.width)
	{
	  ndw->core.width = ndw->drag.width;
	  ndw->drag.x = 0;
	}
      if(ndw->drag.height)
	{
	  ndw->core.height = ndw->drag.height;
	  ndw->drag.y = 0;
	}
    }
  if(cdw->core.width != ndw->core.width ||
      cdw->core.height != ndw->core.height)
    redraw = False;
  return redraw;
}
/*}}}*/
/* public routines */
/*{{{  void DragPopup(invoker, widget, pixmap, offset_x, offset_y, x, y, time)*/
extern VOIDFUNC DragPopup
FUNCARG((invoker, widget, pixmap, offset_x, offset_y, x, y, time),
	Widget    invoker   /* invoker widget */
ARGSEP  Widget    widget    /* drag widget */
ARGSEP  Pixmap    pixmap    /* pixmap to display */
ARGSEP  Position  offset_x  /* pointer offset on pixmap */
ARGSEP  Position  offset_y
ARGSEP  Position  x         /* root postion of pointer */
ARGSEP  Position  y
ARGSEP  Time      time      /* time of last event (needed for grab) */
)
/* called to invoke a drag selection
 * set pixmap and offset values
 * popup the widget
 * grab the pointer
 * If the pointer grab fails, the widget is popped down
 */
{
  /*{{{  set_args*/
  static Arg set_args[] =
  {
    {XtNx},
    {XtNy},
    {XtNpixmap},
  };
  /*}}}*/
  DragWidget dw;
  
  dw = (DragWidget)widget;
  if(!XtIsSubclass((Widget)dw, (WidgetClass)&dragClassRec))
    return;
  dw->drag.offset_x = offset_x;
  dw->drag.offset_y = offset_y;
  dw->drag.widget = invoker;
  set_args[0].value = x - dw->core.border_width - offset_x;
  set_args[1].value = y - dw->core.border_width - offset_y;
  set_args[2].value = pixmap;
  XtSetValues((Widget)dw, set_args, pixmap == None ? 2 : 3);
  XtPopupSpringLoaded((Widget)dw);
  if(XtGrabPointer((Widget)dw, False, ButtonMotionMask | ButtonReleaseMask |
      EnterWindowMask | LeaveWindowMask, GrabModeAsync, GrabModeAsync, None,
      dw->drag.cursor, time) != GrabSuccess)
    XtPopdown((Widget)dw);
  return;
}
/*}}}*/
/* private routines */
/*{{{  void GetGC(dw)*/
static VOIDFUNC GetGC
FUNCARG((widget),
	DragWidget    widget
)
/* get a default gc to copy with
 */
{
  XGCValues     values;
  
  widget->drag.gc = XtGetGC((Widget)widget, 0, &values);
  return;
}
/*}}}*/
/*{{{  void GetPixmapSize(dw)*/
static VOIDFUNC GetPixmapSize
FUNCARG((widget),
	DragWidget  widget
)
/* get the pixmap size
 * Note this requires a server query
 * if the pixmap is None, then set 0 size
 */
{
  if(widget->drag.pixmap != None)
    {
      Window  root;
      int x, y;
      unsigned border;
      unsigned depth;
      Status status;
      
      status = XGetGeometry(XtDisplay(widget), widget->drag.pixmap, &root,
	  &x, &y, &widget->drag.width, &widget->drag.height, &border, &depth);
    }
  else
    widget->drag.width = widget->drag.height = 0;
  return;
}
/*}}}*/
/*{{{  unsigned GetPointerPosition(dw, event, x, y)*/
static unsigned GetPointerPosition
FUNCARG((event, xp, yp),
	XEvent    *event  /* the event to query */
ARGSEP  Position  *xp     /* root pointer x return */
ARGSEP  Position  *yp     /* root pointer y return */
)
/* get the pointer position on the root window.
 * returns 0 if not a pointer event
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
      x = event->xbutton.x_root;
      y = event->xbutton.y_root;
      got = 1;
      break;
    case MotionNotify:
      x = event->xmotion.x_root;
      y = event->xmotion.y_root;
      got = 1;
      break;
    case EnterNotify:
    case LeaveNotify:
      x = event->xcrossing.x_root;
      y = event->xcrossing.y_root;
      got = 1;
      break;
    default:
      got = 0;
  }
  /*}}}*/
  if(got)
    {
      *xp = x;
      *yp = y;
    }
  return got;
}
/*}}}*/
