/* Copyright (C) 1993, 1992 Nathan Sidwell */
/* RCS $Id: xmred.c,v 4.12 1995/12/14 13:53:27 nathan Exp $ */
/*{{{  include*/
#include "xmred.h"
#ifdef SCROLLBARBUGFIX
#include <X11/Xaw/Scrollbar.h>
#endif
/*}}}*/
/*{{{  prototypes*/
static int xio_error PROTOARG((Display *));
/*}}}*/
/*{{{  size_t itoa(text, n, width)*/
extern size_t itoa
FUNCARG((text, number, digits),
	char      *text    /* output text (include 0) */
ARGSEP  unsigned long number   /* number to convert */
ARGSEP  unsigned  digits   /* field width to convert into */
)
/*
 * formats an integer to a string
 * in the specified number of digits
 * pads leading zeros to ' '
 * returns the number of characters used
 */
{
  char      reverse[10];
  size_t    l, length;

  l = 0;
  do
    {
      reverse[l++] = (char)(number % 10 + '0');
      number /= 10;
    }
  while(number);
  if(!digits)
    length = 0;
  else if(l < digits)
    {
      length = digits - l;
      memset(text, ' ', length);
    }
  else
    {
      length = 0;
      l = digits;
    }
  while(l)
    text[length++] = reverse[--l];
  text[length] = 0;
  return length;
}
/*}}}*/
/*{{{  int main(argc, argv)*/
extern int main
FUNCARG((argc, argv),
	int     argc
ARGSEP  char CONST **argv
)
{
  myname = *argv ? *argv : "xmred";
  open_toolkit((Cardinal)argc, (String *)argv);
  /*{{{  help?*/
  if(data.help)
    {
      char CONST *ptr;
	
      ptr = myname;
      for(ptr += strlen(ptr) - 1; ptr != *argv; ptr--)
	if(ptr[-1] == '/')
	  break;
      list_help(ptr);
      return 0;
    }
  /*}}}*/
#ifndef NDEBUG
  XSetErrorHandler(error_handler);
#endif /* NDEBUG */
  XSetIOErrorHandler(xio_error);
  create_widget();
  XtRealizeWidget(display.toplevel);
#ifdef SCROLLBARBUGFIX
  nadger_widget_colors(display.toplevel, scrollbarWidgetClass);
#endif
#ifdef DEBUGEVENTLOOP
  printf("Toplevel is 0x%lx\n", XtWindow(display.toplevel));
#endif /* DEBUGEVENTLOOP */
#ifdef DEBUGEVENTLOOP
  while(1)
    {
      XEvent event;
	  
      XtAppNextEvent(display.context, &event);
      fprintf(stderr, "Event %lu, Window 0x%lx\n",
	  (long)event.xany.type, (long)event.xany.window);
      XtDispatchEvent(&event);
    }
#else
  XtAppMainLoop(display.context);
#endif /* DEBUGEVENTLOOP */
  return 0;
}
/*}}}*/
/*{{{  int xio_error(dptr)*/
static int xio_error
/* ARGSUSED */
FUNCARG((dptr),
	Display *dptr
)
{
  exit(0);
  return 0;
}
/*}}}*/
