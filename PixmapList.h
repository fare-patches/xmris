/* Copyright (C) 1993 Nathan Sidwell */
/* RCS $Id: PixmapList.h,v 4.4 1993/12/14 16:55:52 nathan Stable $ */

#ifndef Mred_PixmapList_h
#define Mred_PixmapList_h
#include <X11/Xaw/Simple.h>
#include <X11/Xaw/Paned.h>
/*{{{  PixmapList widget resources:*/
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
 cursor		     Cursor		Cursor		None
 cursorName	     Cursor		String		NULL
 depth		     Depth		int		parent's depth
 destroyCallback     Callback		XtCallbackList	NULL
*distance            Thickness          Cardinal        4
*dragName            Label		String    	"drag"
*dragSensitivity     Thickness		Dimension	4
*flashDelay	     Interval		int		200
*foreground	     Foreground		Pixel		XtDefaultForeground
 height		     Height		Dimension	text height
*highlightThickness  Thickness		Dimension	0 if shaped, else 2
 insensitiveBorder   Insensitive	Pixmap		Gray
*internalBorderColor   BorderColor	Pixel		XtDefaultForeground
*internalBorderWidth   BorderWidth	Dimension	1
 mappedWhenManaged   MappedWhenManaged	Boolean		True
*orientation	     Orientation	XtOrientation	XtorientVertical
 pointerColor	     Foreground		Pixel		XtDefaultForeground
 pointerColorBackground Background	Pixel		XtDefaultBackground
 resize		     Resize		Boolean		True
 screen		     Screen		Screen		parent's Screen
 sensitive	     Sensitive		Boolean		True
 translations	     Translations	TranslationTable see doc or source
 width		     Width		Dimension	text width
 x		     Position		Position	0
 y		     Position		Position	0
*/
/*}}}*/
/*{{{  new strings*/
#define MredNhighlightThickness "highlightThickness"
#define MredNdragName "dragName"
#define MredNdragSensitivity "dragSensitivity"
#define MredNflashDelay "flashDelay"
#define MredCPixmapList "PixmapList"
#define MredNdistance "distance"
/*}}}*/
extern VOIDFUNC PixmapListRepaint PROTOARG((Widget, Cardinal));
extern VOIDFUNC PixmapListInsert PROTOARG((Widget, Cardinal, Pixmap));
extern VOIDFUNC PixmapListRemove PROTOARG((Widget, Cardinal));
extern VOIDFUNC PixmapListSetScroll PROTOARG((Widget, Widget));
extern int PixmapListQueryOffset
    PROTOARG((Widget, Position, Position, Boolean));
extern int PixmapListQueryDrag PROTOARG((Widget));
extern WidgetClass     pixmapListWidgetClass;
typedef struct _PixmapListClassRec   *PixmapListWidgetClass;
typedef struct _PixmapListRec        *PixmapListWidget;
/*{{{  typedef struct _PixmapListCallback*/
typedef struct _PixmapListCallback
{
  unsigned  selection;    /* selected widget */
  int       button;       /* button number */
} PixmapListCallback;
/*}}}*/
#endif /* _Mred_Pixmap_List_h */
/* DON'T ADD STUFF AFTER THIS */
