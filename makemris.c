/* Copyright (C) 1993, 1992 Nathan Sidwell */
/* RCS $Id: makemris.c 1.2 Tue, 16 Mar 1999 11:28:16 +0000 nathan $ */
/*{{{  includes*/
#include "xmris.h"
#include "defmris.h"
#include <X11/IntrinsicP.h> /* XtConfigureWidget */
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Paned.h>
#include <X11/Xaw/Simple.h>
#include "makecom.c"
/*}}}*/
/*{{{  tables*/
/*{{{  static color*/
static struct
{
  unsigned  backgrounds;      /* normal background */
  unsigned  current[2][3];    /* current colors */
  int       state;            /* type of colors */
  unsigned  busy;             /* not arrived at target */
  unsigned  whizz;            /* whizz value */
  unsigned  count;            /* delay count */
} color;    /* color information */
/*}}}*/
/*{{{  static Arg toplevel_args[] =*/
static Arg toplevel_args[] =
{
  {XtNinput, True},
};
/*}}}*/
/*}}}*/
/*{{{  prototypes*/
static Boolean convert_string2keysym PROTOARG((Display *, XrmValue *,
    Cardinal *, XrmValue *, XrmValue *, XtPointer *));
/*}}}*/
/*{{{  Boolean convert_string2keysym(display, args, num_args, from, to, data)*/
static Boolean convert_string2keysym
/* ARGSUSED */
FUNCARG((display, args, num_args, from, to, data),
	Display   *display
ARGSEP  XrmValue  *args
ARGSEP  Cardinal  *num_args
ARGSEP  XrmValue  *from
ARGSEP  XrmValue  *to
ARGSEP  XtPointer *data
)
/*
 * converts a key name string to a key code
 */
{
  static KeySym keysym;
  
  if(to->size < sizeof(KeySym))
    {
      to->size = sizeof(KeySym);
      return False;
    }
  keysym = XStringToKeysym((char CONST *)from->addr);
  if(keysym != NoSymbol)
    {
      to->size = sizeof(KeySym);
      if(to->addr)
	*(KeySym *)to->addr = keysym;
      else
	to->addr = (XtPointer)&keysym;
      return True;
    }
  else
    {
      XtDisplayStringConversionWarning(display, from->addr, XtRKeySym);
      return False;
    }
}
/*}}}*/
/* Zero requests automatic sizing; the renderer always has a positive scale. */
static int fixed_scale;

static void center_garden(Widget form, XtPointer closure, XEvent *event,
    Boolean *dispatch)
{
  Dimension width, height, border;
  int scale = fixed_scale;
  int spare_x, spare_y, changed;

  if(event->type != ConfigureNotify && event->type != MapNotify)
    return;
  XtVaGetValues(form, XtNwidth, &width, XtNheight, &height, (String)NULL);
  XtVaGetValues(display.garden, XtNborderWidth, &border, (String)NULL);
  if(!scale)
    {
      int across = ((int)width - 2 * border) / WINDOW_WIDTH;
      int down = ((int)height - 2 * border) / WINDOW_HEIGHT;
      scale = across < down ? across : down;
      if(scale < 1)
        scale = 1;
    }
  if(scale != data.pixel_scale && !configure_presentation(scale))
    scale = 1;
  changed = scale != data.pixel_scale;
  data.pixel_scale = scale;
  spare_x = (int)width - WINDOW_WIDTH * scale - 2 * border;
  spare_y = (int)height - WINDOW_HEIGHT * scale - 2 * border;
  XtConfigureWidget(display.garden, spare_x > 0 ? spare_x / 2 : 0,
      spare_y > 0 ? spare_y / 2 : 0, WINDOW_WIDTH * scale,
      WINDOW_HEIGHT * scale, border);
  if(changed && display.mapped)
    refresh_window();
}

/*{{{  void create_widget()*/
extern VOIDFUNC create_widget FUNCARGVOID
/*
 * Create the graphics contexts, sprites and colours and stuff.
 * sets visual and stuff for the toplevel widget
 */
{
  if(strcmp(data.scale_option, "auto") != 0)
    {
      char *end;
      long value = strtol(data.scale_option, &end, 10);
      if(*end || value < 1 || value > 32767 / WINDOW_HEIGHT)
        XtError("-scale requires auto or a positive integer fitting X11 coordinates");
      fixed_scale = (int)value;
    }
  data.pixel_scale = fixed_scale ? fixed_scale : 1;
  display.form = XtVaCreateManagedWidget("form", formWidgetClass,
      display.toplevel, XtNdefaultDistance, (XtArgVal)0, (String)NULL);
  display.garden = XtVaCreateManagedWidget(&names[data.gender != False][1],
      simpleWidgetClass, display.form, XtNwidth, (XtArgVal)(WINDOW_WIDTH * data.pixel_scale),
      XtNheight, (XtArgVal)(WINDOW_HEIGHT * data.pixel_scale),
      XtNleft, XawChainLeft, XtNright, XawChainLeft,
      XtNtop, XawChainTop, XtNbottom, XawChainTop, (String)NULL);
  XtAddEventHandler(display.form, StructureNotifyMask, False, center_garden, NULL);
  create_resources(display.garden);
  assert(!(VEL_X_FAST * FAST_STEPS * 2 % VEL_X));
  assert(!(VEL_Y_FAST * FAST_STEPS * 2 % VEL_Y));
  return;
}
/*}}}*/
/*{{{  void color_cycle()*/
extern VOIDFUNC color_cycle FUNCARGVOID
/* 
 * Moves the background colors towards their targets. Called during
 * each animation cycle, and programmed by color_set.
 */
{
  if(color.busy)
    {
      assert(display.dynamic);
      if(!color.count--)
	{
	  unsigned  target[3];
	  unsigned  ix;
	  
	  color.count = WHIZZ_CYCLES - 1;
	  color.busy = 0;
	  for(ix = 2; ix--;)
	    {
	      /*{{{  set target*/
	      if(color.state == BACKGROUND_WHIZZ)
		/*{{{  set whizz target*/
		{
		  unsigned  cycle, offset;
		  unsigned  a, b, c;
		
		  color.busy = 1;
		  offset = color.whizz % WHIZZ_STEPS * WHIZZ_SIZE;
		  cycle = color.whizz / WHIZZ_STEPS;
		  if(cycle & 1)
		    a = 0xFFFF - offset, b = 0xFFFF, c = 0;
		  else
		    a = 0xFFFF, b = offset, c = 0;
		  if(cycle / 2 == 1)
		    c = b, b = a, a = 0;
		  else if(cycle / 2 == 2)
		    c = a, a = b, b = 0;
		  target[0] = a;
		  target[1] = b;
		  target[2] = c;
		  if(ix)
		    for(cycle = 3; cycle--;)
		      target[cycle] ^= 0xFFFF;
		}
		/*}}}*/
	      else
		/*{{{  set drone or normal target*/
		{
		  XColor    *sptr;
		  
		  sptr = &colors[backgrounds[color.state ==
		      BACKGROUND_DRONES ? BACKGROUNDS :
		      color.backgrounds][ix]];
		  target[0] = sptr->red;
		  target[1] = sptr->green;
		  target[2] = sptr->blue;
		}
		/*}}}*/
	      /*}}}*/
	      /*{{{  move*/
	      {
		unsigned  gun;
		unsigned  *tptr, *cptr;
		
		for(tptr = target, cptr = color.current[ix], gun = 3;
		    gun--; tptr++, cptr++)
		  if(*tptr > *cptr)
		    {
		      if(*tptr - *cptr <= WHIZZ_SIZE)
			*cptr = *tptr;
		      else
			{
			  *cptr += WHIZZ_SIZE;
			  color.busy = 1;
			}
		    }
		  else if(*tptr < *cptr)
		    {
		      if(*cptr - *tptr <= WHIZZ_SIZE)
			*cptr = *tptr;
		      else
			{
			  *cptr -= WHIZZ_SIZE;
			  color.busy = 1;
			}
		    }
	      }
	      /*}}}*/
	      /*{{{  store*/
	      {
		XColor    *cptr;
		unsigned  *sptr;
		
		sptr = color.current[ix];
		cptr = &colors[COLOR_DYNAMIC + ix];
		cptr->red = sptr[0];
		cptr->green = sptr[1];
		cptr->blue = sptr[2];
		cptr->flags = DoRed | DoGreen | DoBlue;
		XStoreColor(display.display, display.colormap, cptr);
	      }
	      /*}}}*/
	    }
	  color.whizz = (color.whizz + 1) % (6 * WHIZZ_STEPS);
	}
    }
  return;
}
/*}}}*/
/*{{{  void color_set(type)*/
extern VOIDFUNC color_set
FUNCARG((type),
	int       type
)
/*
 * Set the board background colours to be what they ought.
 * either one of the BACKGROUND_ specials, or a background index.
 * color_cycle actually sets the color for us.
 */
{
  if(display.dynamic)
    {
      color.count = 0;
      if(type < 0 || type >= BACKGROUNDS)
	color.state = type;
      else
	{
	  color.state = BACKGROUND_NORMAL;
	  color.backgrounds = type;
	}
      color.busy = 1;
    }
  return;
}
/*}}}*/
/*{{{  void open_toolkit(arc, argv)*/
extern VOIDFUNC open_toolkit
FUNCARG((argc, argv),
	int     argc
ARGSEP  String  *argv
)
{
#if !__STDC__
  /*{{{  K&R string fixups*/
  {
    char      *ptr;
    
    ptr = XtMalloc(strlen(XMRISVERSION) + strlen(DATE) +
	strlen(title_text[0].text));
    sprintf(ptr, title_text[0].text, XMRISVERSION, DATE);
    title_text[0].text = ptr;
  }
  /*}}}*/
#endif /* __STDC__ */
  extract_options(&argc, argv, command_options);
  default_gender = GAME_GENDER;
  /*{{{  determine gender of the game*/
  if(*argv)
    {
      size_t    length;
      unsigned  ix;
      char CONST *ptr;
	
      for(ptr = *argv + strlen(*argv); ptr != *argv && *ptr != '/'; ptr--)
	/* EMPTY */;
      if(*ptr == '/')
	ptr++;
      for(length = 0; ptr[length] && ptr[length] != '.'; length++)
	/*  EMPTY */;
      for(ix = 2; ix--;)
	{
	  size_t    other;
	  
	  other = strlen(&names[ix][1]);
	  if(length >= other &&
	      !strncmp(&ptr[length - other], &names[ix][1], other))
	    default_gender = ix;
	}
    }
  /*}}}*/
  if(data.help == False)
    /*{{{  more setup*/
    {
      if(data.nodisplay == False)
	{
	  XrmOptionDescRec CONST *ptr;
	  const char *myclass = default_gender ? "Xmsit" : "Xmris";
    
	  /* as options is an extern of unknown size, I can't use
	   * XtNumber() to get its size, so I've done it this way
	   */
	  for(ptr = options; *(char CONST **)ptr; ptr++)
	    /* EMPTY */;
	  display.toplevel = XtAppInitialize(&display.context, myclass,
	      options, (Cardinal)(ptr - options),
	      &argc, (String *)argv,
	      (String *)NULL, toplevel_args, XtNumber(toplevel_args));
	}
      if(argc > 1)
	/*{{{  error*/
	{
	  unsigned  ix;
	  
	  data.help = True;
	  data.colors = False;
	  fprintf(stderr, "%s: Unknown option%s:", argv[0],
	      argc > 2 ? "s" : "");
	  for(ix = 1; ix != argc; ix++)
	    fprintf(stderr, "%s %s", ix == 1 ? "" : ",", argv[ix]);
	  fputc('\n', stderr);
	}
	/*}}}*/
      else if(data.nodisplay != False)
	{
	  if(data.scores == False && !data.expire &&
	      !data.remove && !data.format)
	    fatal_error("-nodisplay must be accompanied by -scores, -expire, -remove or -format");
	}
      else
	/*{{{  some more setup*/
	{
	  XtResource CONST *ptr;
	  
	  XtAppSetTypeConverter(display.context, XtRString, XtRKeySym,
	      convert_string2keysym, (XtConvertArgRec *)NULL, 0, XtCacheNone,
	      (void (*)PROTOARG((XtAppContext, XrmValue *, XtPointer,
		  XrmValue *, Cardinal *)))NULL);
	  XtAppSetTypeConverter(display.context, XtRString, XtRGender,
	      convert_string2gender, (XtConvertArgRec *)NULL, 0, XtCacheNone,
	      (void (*)PROTOARG((XtAppContext, XrmValue *, XtPointer,
		  XrmValue *, Cardinal *)))NULL);
	  display.display = XtDisplay(display.toplevel);
	  for(ptr = resources; *(char CONST **)ptr; ptr++)
	    /* EMPTY */;
          options_read();
          for (XtResource *r = resources; r->resource_name; ++r) {
            const char *saved = option_default(r->resource_name);
            if (saved) {
              r->default_type = XtRString;
              r->default_addr = (XtPointer)saved;
            }
          }
	  XtGetApplicationResources(display.toplevel, (XtPointer)&data,
	      resources, (Cardinal)(ptr - resources), (Arg *)NULL, 0);
          if(!strcmp(data.diamond_difficulty, "classic"))
            data.diamond_mask = 0;
          else if(!strcmp(data.diamond_difficulty, "hard"))
            data.diamond_mask = 7;
          else
            {
              data.diamond_mask = 1;
              if(strcmp(data.diamond_difficulty, "easy"))
                XtAppWarning(display.context, "unknown diamondDifficulty; using easy");
            }
	  gettoplevelresources();
	  XtVaSetValues(display.toplevel,
	      XtNtitle, (XtArgVal)names[data.gender != False],
	      XtNiconName, (XtArgVal)names[data.gender != False],
	      NULL);
	  {
	    XKeyboardState keyboard;
	    
	    XGetKeyboardControl(display.display, &keyboard);
	    display.repeat = keyboard.global_auto_repeat;
	  }
	}
	/*}}}*/
    }
    /*}}}*/
  return;
}
/*}}}*/
