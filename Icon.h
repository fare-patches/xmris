/* Copyright (C) 1993 Nathan Sidwell */
/* RCS $Id: Icon.h 1.2 Tue, 16 Mar 1999 11:28:16 +0000 nathan $ */

#ifndef Mred_Icon_h
#define Mred_Icon_h
#include <X11/Xaw/Simple.h>
/*{{{  Icon widget resources:*/
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
*columns	     Width		Dimension	1
 cursorName	     Cursor		String		NULL
 depth		     Depth		int		parent's depth
 destroyCallback     Callback		XtCallbackList	NULL
*dragName            Label		String    	"drag"
*dragSensitivity     Thickness		Dimension	4
*flashDelay	     Interval		int		200
*foreground	     Foreground		Pixel		XtDefaultForeground
 height		     Height		Dimension	text height
*highlightThickness  Thickness		Dimension	0 if shaped, else 2
 insensitiveBorder   Insensitive	Pixmap		Gray
 mappedWhenManaged   MappedWhenManaged	Boolean		True
*pixmap		     Pixmap		Pixmap		None
 pointerColor	     Foreground		Pixel		XtDefaultForeground
 pointerColorBackground Background	Pixel		XtDefaultBackground
 resize		     Resize		Boolean		True
*rows		     Height		Dimension	1
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
#define MredNcolumns "columns"
#define MredNrows "rows"
#define MredNdragName "dragName"
#define MredNdragSensitivity "dragSensitivity"
#define MredNflashDelay "flashDelay"
/*}}}*/
extern VOIDFUNC IconRepaint PROTOARG((Widget));
extern WidgetClass iconWidgetClass;
typedef struct _IconClassRec *IconWidgetClass;
typedef struct _IconRec *IconWidget;
/*{{{  typedef struct _IconCallback*/
typedef struct _IconCallback
{
  unsigned  selection;    /* selected area */
  int       button;       /* button number */
} IconCallback;
/*}}}*/
#endif /* _Mred_Icon_h */
/* DON'T ADD STUFF AFTER THIS */
