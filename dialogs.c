/* Copyright (C) 1993, 1992 Nathan Sidwell */
/* RCS $Id: dialogs.c,v 4.6 1994/01/25 18:33:03 nathan Stable $ */
/*{{{  includes*/
#include "xmred.h"
#include <X11/Xaw/Dialog.h>
#include <X11/Shell.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/Text.h>
#include <X11/Xaw/Label.h>
/*}}}*/
/*{{{  structs*/
/*{{{  typedef struct Dialog*/
typedef struct Dialog
{
  char CONST *name;     /* shell name */
  char CONST *prompt;   /* dialog label */
  unsigned  input;      /* input permitted */
  unsigned  options;    /* selections permitted */
  unsigned  selection;  /* selection made */
  unsigned  up;         /* popped up */
  Widget    shell;      /* shell widget */
  Widget    dialog;     /* dialog widget */
  int       x;          /* warp back point */
  int       y;          /* warp back point */
  Widget    warp;       /* warp back widget */
} DIALOG;
/*}}}*/
/*{{{  typedef struct Dialog_Option*/
typedef struct Dialog_Option
{
  char CONST *text;   /* button text */
  unsigned  option;   /* option mask */
} DIALOG_OPTION;
/*}}}*/
/*}}}*/
/*{{{  tables*/
/*{{{  DIALOG_OPTION CONST dialog_options[] =*/
static DIALOG_OPTION CONST dialog_options[] =
{
  {"ok",        DIALOG_OK},
  {"retry",     DIALOG_RETRY},
  {"continue",  DIALOG_CONTINUE},
  {"default",   DIALOG_DEFAULT},
  {"cancel",    DIALOG_CANCEL},
  {"abort",     DIALOG_ABORT},
  {"agree",     DIALOG_AGREE},
  {"disagree",  DIALOG_DISAGREE},
  {"clear",     DIALOG_CLEAR},
  {NULL}
};
/*}}}*/
/*{{{  DIALOG dialogs[DIALOGS] =*/
static DIALOG dialogs[DIALOGS] =
{
  {"file", "Enter filename", 1, DIALOG_OK | DIALOG_CANCEL | DIALOG_CLEAR},
  {"altered", "Changes not saved", 0, DIALOG_CONTINUE | DIALOG_ABORT},
  {"error", NULL, 1, DIALOG_RETRY | DIALOG_ABORT},
  {"load", "Format errors", 1, DIALOG_OK},
  {"comment", "Enter comment", 1, DIALOG_OK | DIALOG_CANCEL | DIALOG_CLEAR},
  {"include", "Enter filename", 1,
      DIALOG_OK | DIALOG_DEFAULT | DIALOG_CANCEL | DIALOG_CLEAR},
  {"failed", "Operation failed", 1, DIALOG_OK},
};
/*}}}*/
/*}}}*/
/*{{{  prototypes*/
static VOIDFUNC dialog_action
    PROTOARG((Widget, XEvent *, String *, Cardinal *));
static VOIDFUNC dialog_option PROTOARG((Widget, XtPointer, XtPointer));
/*}}}*/
/*{{{  void dialog_action(widget, event, argv, argc)*/
static VOIDFUNC dialog_action
FUNCARG((widget, event, argv, argc),
	Widget    widget
ARGSEP  XEvent    *event
ARGSEP  String    *argv
ARGSEP  Cardinal  *argc
)
/* action routine for the dialog buttons,
 * presses all the dialog buttons matching the given arguments
 */
{
  DIALOG_OPTION CONST *optr;
  unsigned  count;
  
  for(count = *argc; count--; argv++)
    for(optr = dialog_options; optr->text; optr ++)
      if(!strcmp(optr->text, *argv))
	{
	  dialog_option(widget, (XtPointer)optr->option, (XtPointer)NULL);
	  break;
	}
  return;
}
/*}}}*/
/*{{{  void dialog_append(dialog, string)*/
extern VOIDFUNC dialog_append
FUNCARG((dialog, string),
	unsigned  dialog
ARGSEP  char CONST *string    /* string to append */
)
/* appends a string to a dialog's text box
 */
{
  XawTextBlock text;
  Widget  widget;
  
  widget = XtNameToWidget(dialogs[dialog].dialog, "*value");
  assert(widget);
  text.firstPos = 0;
  text.length = strlen(string);
  text.ptr = (char *)string;
  text.format = FMT8BIT;
  XawTextReplace(widget, (XawTextPosition)32767,
      (XawTextPosition)32767, &text);
  return;
}
/*}}}*/
/*{{{  void dialog_option(widget, client, call)*/
static VOIDFUNC dialog_option
FUNCARG((widget, client, call),
	Widget  widget
ARGSEP  XtPointer client
ARGSEP  XtPointer call
)
/* callback on the dialog buttons.
 * most of these just set an appropriate bit in the select mask
 * For thos on the dialogs selection list, the dialog is popped down
 * and the pointer unwarped
 * The clear button, finds the dialog's text widget, and
 * removes all the text from it.
 */
{
  while(widget && strcmp(XtName(widget), "dialog"))
    widget = XtParent(widget);
  if(!widget)
    /* EMPTY */;
  else if((unsigned)client == DIALOG_CLEAR)
    {
      Widget    text;
      
      text = XtNameToWidget(widget, "*value");
      if(text)
	{
	  XawTextBlock block;
	  
	  block.firstPos = 0;
	  block.length = 0;
	  block.ptr = NULL;
	  block.format = FMT8BIT;
	  XawTextReplace(text, 0, 32767, &block);
	}
    }
  else
    {
      unsigned  count;
      DIALOG    *dptr;
      
      for(dptr = dialogs, count = DIALOGS; count--; dptr++)
	if(dptr->dialog == widget)
	  {
	    dptr->selection |=
		(unsigned)client & dptr->options;
	    if(dptr->selection && dptr->up)
	      {
		dptr->up = 0;
		if(dptr->warp)
		  XWarpPointer(XtDisplay(dptr->shell), XtWindow(dptr->shell),
		      XtWindow(dptr->warp), 0, 0, 0, 0, dptr->x, dptr->y);
		XtPopdown(dptr->shell);
	      }
	    break;
	  }
    }
  return;
}
/*}}}*/
/*{{{  void dialog_popup(dialog, title, suggestion, anchor, grab)*/
extern VOIDFUNC dialog_popup
FUNCARG((dialog, title, suggestion, anchor, grab),
	unsigned  dialog    /* dialog to popup */
ARGSEP  char CONST *title   /* title or NULL */
ARGSEP  char CONST *suggestion  /* suggested text or NULL */
ARGSEP  Widget    anchor    /* widget to anchor the popup from */
ARGSEP  XtGrabKind grab     /* grab mode */
)
/* pops up a dialog widget 1/4 in from the top left corner of
 * the anchor widget.
 * Sets the warp return values if within the anchor widget, and
 * warps the pointer to the bottom of the dialog
 */
{
  DIALOG    *dptr;
  Position  topx, topy;
  Dimension topw, toph;
  Dimension w, h;
  
  dptr = &dialogs[dialog];
  dptr->selection = 0;
  dptr->warp = anchor;
  if(anchor)
    XtVaGetValues(anchor, XtNx, (XtArgVal)&topx, XtNy, (XtArgVal)&topy,
	XtNwidth, (XtArgVal)&topw, XtNheight, (XtArgVal)&toph, NULL);
  /*{{{  setup for warp back*/
  if(anchor)
    {
      Boolean   warp;
      Window    root;
      Window    child;
      unsigned  mask;
      int       rx, ry;
    
      warp = XQueryPointer(display.display, XtWindow(anchor),
	  &root, &child, &rx, &ry, &dptr->x, &dptr->y, &mask);
      if(warp == False || dptr->x < 0 || dptr->y < 0 ||
	  dptr->x >= topw || dptr->y >= toph)
	dptr->warp = NULL;
    }
  /*}}}*/
  if(title)
    XtVaSetValues(dptr->shell, XtNtitle, (XtArgVal)title, NULL);
  assert(dptr->input ? !!suggestion : !suggestion);
  XtVaSetValues(dptr->dialog, XtNvalue, (XtArgVal)suggestion, NULL);
  XtRealizeWidget(dptr->shell);
  if(!dptr->up)
    {
      if(anchor)
	{
	  XtVaGetValues(dptr->shell, XtNwidth, &w, XtNheight, &h, NULL);
	  topx = topx + (topw - w) / 4;
	  topy = topy + (toph - h) / 4;
	  XtVaSetValues(dptr->shell, XtNx, (XtArgVal)topx,
	      XtNy, (XtArgVal)topy, NULL);
	}
      XtPopup(dptr->shell, grab);
      dptr->up = 1;
    }
  if(dptr->warp)
    XWarpPointer(XtDisplay(dptr->shell), XtWindow(dptr->warp),
	XtWindow(dptr->shell), 0, 0, 0, 0, w / 2, h - 8);
  return;
}
/*}}}*/
/*{{{  unsigned dialog_wait(dialog, title, label, suggestion, result)*/
extern unsigned dialog_wait
FUNCARG((dialog, title, label, suggestion, result),
	unsigned  dialog    /* dialog to use */
ARGSEP  char CONST *title   /* title or NULL */
ARGSEP  char CONST *label   /* label or NULL */
ARGSEP  char CONST *suggestion  /* initial text or NULL */
ARGSEP  char CONST **result   /* returned text buffer or NULL */
)
/* popup a dialog and wait for a reply.
 * returns the text selection, this must be copied to a safe
 * place, if you want to keep it.
 */
{
  DIALOG    *dptr;
  unsigned  option;
  
  dptr = &dialogs[dialog];
  if(label)
    XtVaSetValues(dptr->dialog, XtNlabel, label, NULL);
  dialog_popup(dialog, title, suggestion || !result ? suggestion : "",
      display.toplevel, XtGrabExclusive);
  while(!dptr->selection)
    {
      XEvent event;
      
      XtAppNextEvent(display.context, &event);
#ifdef DEBUGEVENTLOOP
      fprintf(stderr, "Event %lu, Window 0x%lx\n",
	  (long)event.xany.type, (long)event.xany.window);
#endif /* DEBUGEVENTLOOP */
      XtDispatchEvent(&event);
    }
  option = dptr->selection;
  if(result)
    {
      char CONST *ptr;
      
      ptr = XawDialogGetValueString(dptr->dialog);
      *result = ptr && *ptr ? ptr : NULL;
    }
  return option;
}
/*}}}*/
/*{{{  void install_dialogs(root)*/
extern VOIDFUNC install_dialogs
FUNCARG((root),
	Widget    root
)
/* install the dialog popup widgets
 */
{
  /*{{{  static XtActionsRec actions[] =*/
  static XtActionsRec actions[] =
  {
    {"set-dialog-button", dialog_action},
  };
  /*}}}*/
  unsigned  ix;
  DIALOG  *dptr;
  
  XtAppAddActions(display.context, actions, XtNumber(actions));
  for(dptr = &dialogs[DIALOGS - 1], ix = DIALOGS; ix--; dptr--)
    {
      unsigned  count;
      
      dptr->shell = XtVaCreatePopupShell(dptr->name,
	  transientShellWidgetClass, root, NULL);
      dptr->dialog = XtVaCreateManagedWidget("dialog", dialogWidgetClass,
	  dptr->shell, NULL);
      for(count = 0; count != 8; count++)
	if(dptr->options & (1 << count))
	  {
	    DIALOG_OPTION CONST *optr;
	    
	    for(optr = dialog_options; optr->text; optr++)
	      if(optr->option & (1 << count))
		{
		  XawDialogAddButton(dptr->dialog, optr->text,
		      dialog_option, (XtPointer)(1 << count));
		  break;
		}
	  }
      if(dptr->input)
	XtVaSetValues(dptr->dialog, XtNvalue, (XtArgVal)"", NULL);
      nadger_widget_colors(dptr->shell, (WidgetClass)NULL);
      if(dptr->prompt)
	XtVaSetValues(dptr->dialog,
	    XtNlabel, (XtArgVal)dptr->prompt, NULL);
    }
  return;
}
/*}}}*/
