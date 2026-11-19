#include "xmris.h"
#include <X11/XKBlib.h>
#include "sound.h"
static unsigned map_start;

/* XKB often reports the navigation symbol at keypad level zero. */
KeySym canonical_key(KeySym key)
{
  switch(key)
    {
    case XK_KP_Insert: return XK_KP_0;
    case XK_KP_End: return XK_KP_1;
    case XK_KP_Down: return XK_KP_2;
    case XK_KP_Next: return XK_KP_3;
    case XK_KP_Left: return XK_KP_4;
    case XK_KP_Begin: return XK_KP_5;
    case XK_KP_Right: return XK_KP_6;
    case XK_KP_Home: return XK_KP_7;
    case XK_KP_Up: return XK_KP_8;
    case XK_KP_Prior: return XK_KP_9;
    case XK_KP_Delete: return XK_KP_Decimal;
    default: return key;
    }
}

/*{{{  prototypes*/
static VOIDFUNC action_expose
    PROTOARG((Widget, XEvent *, String *, Cardinal *));
static VOIDFUNC action_keypress
    PROTOARG((Widget, XEvent *, String *, Cardinal *));
static VOIDFUNC action_keyrelease
    PROTOARG((Widget, XEvent *, String *, Cardinal *));
static VOIDFUNC action_map
    PROTOARG((Widget, XEvent *, String *, Cardinal *));
static VOIDFUNC action_pause
    PROTOARG((Widget, XEvent *, String *, Cardinal *));
static VOIDFUNC action_unmap
    PROTOARG((Widget, XEvent *, String *, Cardinal *));
static VOIDFUNC action_quit
    PROTOARG((Widget, XEvent *, String *, Cardinal *));
static int xio_error PROTOARG((Display *));
/*}}}*/
/*{{{  static tables*/
/*{{{  static XtActionsRec actions[] =*/
static XtActionsRec actions[] =
{
  {"keypress", action_keypress},
  {"keyrelease", action_keyrelease},
  {"pause", action_pause},
  {"map", action_map},
  {"unmap", action_unmap},
  {"expose", action_expose},
  {"quit", action_quit},
};
/*}}}*/
/*{{{  static char CONST garden_translations[] =*/
static char CONST garden_translations[] =
"\
<KeyPress>:keypress()\n\
<KeyRelease>:keyrelease()\n\
<Expose>:expose()\n\
";
/*}}}*/
/*{{{  static char CONST form_translations[] =*/
static char CONST form_translations[] =
"\
<KeyPress>:keypress()\n\
<KeyRelease>:keyrelease()\n\
<FocusOut>:pause()\n\
";
/*}}}*/
/*{{{  static char CONST toplevel_translations[] =*/
static char CONST toplevel_translations[] =
"\
<MapNotify>:map()\n\
<UnmapNotify>:unmap()\n\
<Message>WM_PROTOCOLS: quit()\n\
";
/*}}}*/
/*}}}*/
/*{{{  void action_expose(widget, event, params, num_params)*/
static VOIDFUNC action_expose
FUNCARG((widget, event, params, num_params),
	Widget    widget
ARGSEP  XEvent    *event
ARGSEP  String    *params
ARGSEP  Cardinal  *num_params
)
{
#ifdef DEBUGEVENTLOOP
  fprintf(stderr, "action_expose(0x%lx)\n", XtWindow(widget));
#endif /* DEBUGEVENTLOOP */
  if(display.mapped)
    refresh_window();
  return;
}
/*}}}*/
/*{{{  void action_quit(widget, event, params, num_params)*/
static VOIDFUNC action_quit
FUNCARG((widget, event, params, num_params),
	Widget    widget
ARGSEP  XEvent    *event
ARGSEP  String    *params
ARGSEP  Cardinal  *num_params
)
{
#ifdef DEBUGEVENTLOOP
  fprintf(stderr, "action_quit(0x%lx)\n", XtWindow(widget));
#endif /* DEBUGEVENTLOOP */
  set_key_default();
  global.quit = 1;
}
/*}}}*/
/*{{{  void action_keypress(widget, event, params, num_params)*/
static VOIDFUNC action_keypress
FUNCARG((widget, event, params, num_params),
	Widget    widget
ARGSEP  XEvent    *event
ARGSEP  String    *params
ARGSEP  Cardinal  *num_params
)
/*
 * When a key is pressed, we check to see if it is
 * a control key. If so, then we set the relevant pressed bit.
 */
{
  KeySym    keysym;

  keysym = canonical_key(XkbKeycodeToKeysym(display.display, event->xkey.keycode, 0, 0));
#ifdef DEBUGEVENTLOOP
  fprintf(stderr, "action_keypress(0x%lx) keysym=%0xlx\n",
      (long)XtWindow(widget), (long)keysym);
#endif /* DEBUGEVENTLOOP */
  game_keypress(keysym);
  return;
}
/*}}}*/
/*{{{  void action_keyrelease(widget, event, params, num_params)*/
static VOIDFUNC action_keyrelease
FUNCARG((widget, event, params, num_params),
	Widget    widget
ARGSEP  XEvent    *event
ARGSEP  String    *params
ARGSEP  Cardinal  *num_params
)
/*
 * When a key is released, we check to see if it is
 * a control key. If so, then we set the relevant pressed bit.
 */
{
  KeySym    keysym;

  keysym = canonical_key(XkbKeycodeToKeysym(display.display, event->xkey.keycode, 0, 0));
#ifdef DEBUGEVENTLOOP
  fprintf(stderr, "action_keyrelease(0x%lx) keysym=%0xlx\n",
      (long)XtWindow(widget), (long)keysym);
#endif /* DEBUGEVENTLOOP */
  game_keyrelease(keysym);
  return;
}
/*}}}*/
/*{{{  void action_map(widget, event, params, num_params)*/
static VOIDFUNC action_map
FUNCARG((widget, event, params, num_params),
	Widget    widget
ARGSEP  XEvent    *event
ARGSEP  String    *params
ARGSEP  Cardinal  *num_params
)
{
#ifdef DEBUGEVENTLOOP
  fprintf(stderr, "action_map(0x%lx)\n", (long)XtWindow(widget));
#endif /* DEBUGEVENTLOOP */
  sound_pause(0);
  display.mapped = 1;
  if(map_start)
    timer_set((unsigned long)0, TIMING_ON);
  map_start = 0;
  return;
}
/*}}}*/
/*{{{  void action_pause(widget, event, params, num_params)*/
static VOIDFUNC action_pause
FUNCARG((widget, event, params, num_params),
	Widget    widget
ARGSEP  XEvent    *event
ARGSEP  String    *params
ARGSEP  Cardinal  *num_params
)
{
#ifdef DEBUGEVENTLOOP
  fprintf(stderr, "action_pause(0x%lx)\n", (long)XtWindow(widget));
#endif /* DEBUGEVENTLOOP */
  global.pause = global.state < 6;
  return;
}
/*}}}*/
/*{{{  void action_unmap(widget, event, params, num_params)*/
static VOIDFUNC action_unmap
FUNCARG((widget, event, params, num_params),
	Widget    widget
ARGSEP  XEvent    *event
ARGSEP  String    *params
ARGSEP  Cardinal  *num_params
)
{
#ifdef DEBUGEVENTLOOP
  fprintf(stderr, "action_unmap(0x%lx)\n", (long)XtWindow(widget));
#endif /* DEBUGEVENTLOOP */
  sound_pause(1);
  display.mapped = 0;
  global.pause = global.state < 6;
  map_start = timer_set((unsigned long)0, TIMING_PAUSE) == TIMING_ON;
  set_key_default();
  return;
}
/*}}}*/
/*{{{  int main(argc, argv)*/
void default_bindings(KeySym *keys)
{
  const KeySym defaults[KEYS]={XK_Up,XK_Down,XK_Left,XK_Right,XK_space,XK_p,XK_q,XK_i,XK_k};
  memcpy(keys,defaults,sizeof(defaults));
}

const char *portable_key_name(KeySym key) { return key_name(key); }

extern int main
FUNCARG((argc, argv),
	int     argc
ARGSEP  char CONST **argv
)
{
  Atom wm_delete_window;

  init_scores_early();

  myname = *argv ? *argv : "Xmris";
  open_toolkit(argc, (String *)argv);
  init_scores();
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
  /*{{{  high scores?*/
  if(data.scores)
    {
      list_scores();
      return 0;
    }
  /*}}}*/
#ifndef NDEBUG
  XSetErrorHandler(error_handler);
#endif /* NDEBUG */
  XSetIOErrorHandler(xio_error);
  create_boards();
  create_widget();
  XtAppAddActions(display.context, actions, XtNumber(actions));
  XtOverrideTranslations(display.garden,
      XtParseTranslationTable(garden_translations));
  XtOverrideTranslations(display.form,
      XtParseTranslationTable(form_translations));
  XtOverrideTranslations(display.toplevel,
      XtParseTranslationTable(toplevel_translations));
  XtRealizeWidget(display.toplevel);
  wm_delete_window = XInternAtom(XtDisplay(display.toplevel), "WM_DELETE_WINDOW",
				 False);
  (void)XSetWMProtocols(XtDisplay(display.toplevel), XtWindow(display.toplevel),
			&wm_delete_window, 1);
#ifdef DEBUGEVENTLOOP
  printf("Toplevel is 0x%lx\n", (long)XtWindow(display.toplevel));
  printf("Garden is 0x%lx\n", (long)XtWindow(display.garden));
#endif /* DEBUGEVENTLOOP */
  display.window = XtWindow(display.garden);
  if(!configure_presentation(data.pixel_scale))
    XtError("explicit pixel scaling requires X Render 0.6 on the selected visual");
  if(data.sound)
    sound_open();
  timer_open();
  while(!global.quit)
    {
      color_cycle();
      while(!global.quit && !(display.mapped && global.state == MODE_KEY_DEF && global.key) && (XtAppPending(display.context) || !display.mapped ||
	  global.pressed & 1 << KEY_PAUSE ||
	  (!global.key && global.state == MODE_KEY_DEF)))
	{
#ifdef DEBUGEVENTLOOP
	  XEvent event;

	  XtAppNextEvent(display.context, &event);
	  fprintf(stderr, "Event %lu, Window 0x%lx\n",
	      (long)event.xany.type, (long)event.xany.window);
	  XtDispatchEvent(&event);
#else
	  XtAppProcessEvent(display.context, XtIMAll);
#endif /* DEBUGEVENTLOOP */
	}
      if(global.quit) break;
      if(global.pressed & 1 << KEY_ICONIZE)
	{
	  XIconifyWindow(display.display, XtWindow(display.toplevel),
	    display.screen);
	  global.pressed ^= 1 << KEY_ICONIZE;
	}
      else
	{
	  game_tick();
	  timer_wait();
	}
      assert(monster.normals < 20);
    }
  timer_close();
  sound_close();
  cleanup_scores();
  return 0;
}
/*}}}*/
/*{{{  void set_key_default()*/
VOIDFUNC set_key_default FUNCARGVOID
{
  if(display.repeat != AutoRepeatModeOff)
    {
      XKeyboardControl control;

      control.auto_repeat_mode = display.repeat;
      XChangeKeyboardControl(display.display, KBAutoRepeatMode, &control);
    }
  return;
}
/*}}}*/
/*{{{  void set_key_single()*/
VOIDFUNC set_key_single FUNCARGVOID
{
  if(display.repeat != AutoRepeatModeOff)
    {
      XKeyboardControl control;

      control.auto_repeat_mode = AutoRepeatModeOff;
      XChangeKeyboardControl(display.display, KBAutoRepeatMode, &control);
    }
  return;
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
