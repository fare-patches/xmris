/* Copyright (C) 1993 Nathan Sidwell */
/* RCS $Id: Drag.h,v 4.5 1993/10/01 19:19:35 nathan Stable $ */

#ifndef Mred_Drag_h
#define Mred_Drag_h
#include <X11/Xaw/Simple.h>
/*{{{  Drag widget resources:*/
/*
 Name		     Class		RepType		Default Value
 ----		     -----		-------		-------------
 accelerators	     Accelerators	AcceleratorTable NULL
 ancestorSensitive   AncestorSensitive	Boolean		True
 background	     Background		Pixel		XtDefaultBackground
 backgroundPixmap    Pixmap		Pixmap		XtUnspecifiedPixmap
 borderColor	     BorderColor	Pixel		XtDefaultForeground
 borderPixmap	     Pixmap		Pixmap		XtUnspecifiedPixmap
 borderWidth	     BorderWidth	Dimension	1
*callback	     Callback		XtCallbackList	NULL
 colormap	     Colormap		Colormap	parent's colormap
*cursor		     Cursor		Cursor		None
 cursorName	     Cursor		String		NULL
 depth		     Depth		int		parent's depth
 destroyCallback     Callback		XtCallbackList	NULL
*foreground	     Foreground		Pixel		XtDefaultForeground
 height		     Height		Dimension	text height
 insensitiveBorder   Insensitive	Pixmap		Gray
 mappedWhenManaged   MappedWhenManaged	Boolean		True
*numWidgetChoices    Index              Cardinal        0
*pixmap		     Pixmap		Pixmap		None
 pointerColor	     Foreground		Pixel		XtDefaultForeground
 pointerColorBackground Background	Pixel		XtDefaultBackground
 resize		     Resize		Boolean		True
 screen		     Screen		Screen		parent's Screen
 sensitive	     Sensitive		Boolean		True
 shapeStyle	     ShapeStyle		ShapeStyle	Rectangle
 translations	     Translations	TranslationTable see doc or source
*widgetChoices       WidgetList         WidgetList      NULL
 width		     Width		Dimension	text width
 x		     Position		Position	0
 y		     Position		Position	0
*/
/*}}}*/
/*{{{  new strings*/
#define MredNwidgetChoices "widgetChoices"
#define MredNnumWidgetChoices "numWidgetChoices"
#define MredCWidgetList "WidgetList"
/*}}}*/
extern WidgetClass     dragWidgetClass;
typedef struct _DragClassRec   *DragWidgetClass;
typedef struct _DragRec        *DragWidget;
extern VOIDFUNC DragPopup PROTOARG((Widget, Widget, Pixmap, Position, Position,
    Position, Position, Time));
/*{{{  typedef struct _DragCallback*/
typedef struct _DragCallback
{
  Widget    invoker;      /* invoking widget */
  Widget    selected;     /* selected widget */
  Position  offset_x;     /* offset of pointer */
  Position  offset_y;     /* offset of pointer */
} DragCallback;
/*}}}*/
#endif /* _Mred_Drag_h */
/* DON'T ADD STUFF AFTER THIS */
