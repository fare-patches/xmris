/* Copyright (C) 1993, 1992 Nathan Sidwell */
/* RCS $Id: makecom.c,v 4.23 1994/02/08 18:52:34 nathan stable $ */
/* common make stuff for xmris and xmred, 
 * included from makemris and makemred
 */
/*{{{  interesting color stuff*/
/*
 * color is spelt as it is, 'cos its just too complicated spelling it as
 * it should be, when X doesn't.
 * The colour allocation is done by first parsing all the colour names
 * to RGB values (using a type convertor from the resources). Each RGB
 * value is converted to a 3space coordinate in a colour 'cone' (the
 * colour circle, (well, hexagon in this case), and a height,
 * corresponding to brightness). With that we can work out the
 * perceived distances between different colours.
 * These are then allocated in turn, by picking the unallocated colour
 * farthest from the allocated colours. If the allocation fails, we pick
 * the allocated colour which is nearest the desired colour. This way
 * we try to get the most diverse set of colours possible with the
 * colormap that's being used. Remember, black and white have already
 * been allocated so we don't need to allocate those, and it gives us
 * a starting ix in the allocation loop.
 * The board background colours only need allocating, if it is a read
 * only visual (Direct Colour or something). For writable visuals,
 * we allocate two colorcells and set these as required, allowing
 * dynamic colour changes, and saving entries in the colormap.
 * You can see the order in which this is done by giving the -colours arg.
 * There is a bug (IMHO) in the way that X parses hex color names to XColor.
 * #FA5 is parsed as #F000A0005000, not #FFFFAAAA5555 as I would have
 * expected it to be. That's why I've specified colours accurately.
 * When the color #F000A0005000 is allocated, the rgb values are set,
 * as I expected, to #F0F0A0A05050, on an 8 bit visual. There doesn't
 * seem to be a way to get X to tell you what colour it would
 * allocate on a colormap, assuming that there's a space.
 */
/*}}}*/
/*{{{  static tables*/
/*{{{  static char CONST *names[] =*/
static char CONST *names[] =
{
  "xmris",
  "xmsit"
};
/*}}}*/
/*}}}*/
/*{{{  prototypes*/
static Boolean convert_string2color PROTOARG((Display *, XrmValue *,
    Cardinal *, XrmValue *, XrmValue *, XtPointer *));
static Boolean convert_string2gender PROTOARG((Display *, XrmValue *,
    Cardinal *, XrmValue *, XrmValue *, XtPointer *));
static VOIDFUNC create_resources PROTOARG((Widget));
static VOIDFUNC extract_options
    PROTOARG((int *, String *, CommandOptionDesc CONST *));
static VOIDFUNC gettoplevelresources PROTOARG((VOIDARG));
static VOIDFUNC list_color_resource
    PROTOARG((unsigned, char CONST *, char CONST *));
static VOIDFUNC make_color_image
    PROTOARG((SPRITE *, SPRITE_DEF *, Window, unsigned, Pixmap));
static VOIDFUNC make_mono_image
    PROTOARG((SPRITE *, SPRITE_DEF *, Window, unsigned, Pixmap));
static VOIDFUNC make_mask_image
    PROTOARG((SPRITE *, SPRITE_DEF *, Window, unsigned, Pixmap));
/*}}}*/
/*{{{  Boolean convert_string2color(display, args, num_args, from, to, data)*/
static Boolean convert_string2color
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
 * converts a string to a colordef
 */
{
  static XColor result;
  Status    status;
  
  if(to->size < sizeof(XColor))
    {
      to->size = sizeof(XColor);
      return False;
    }
  status = XParseColor(display, *(Colormap *)args[1].addr,
      (char CONST *)from->addr, to->addr ? (XColor *)to->addr : &result);
  if(status)
    {
      to->size = sizeof(XColor);
      if(to->addr)
	((XColor *)to->addr)->flags = DoRed | DoGreen | DoBlue;
      else
	{
	  result.flags = DoRed | DoGreen | DoBlue;
	  to->addr = (XtPointer)&result;
	}
      return True;
    }
  else
    {
      XtDisplayStringConversionWarning(display, from->addr, XtRColor);
      return False;
    }
}
/*}}}*/
/*{{{  Boolean convert_string2gender(display, args, num_args, from, to, data)*/
static Boolean convert_string2gender
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
 * converts a string to a gender boolean (my new type XtRGender)
 */
{
  static char CONST *genders[] =
    {"he", "she", "male", "female", "mris", "msit", "boy", "girl", NULL};
  char CONST **ptr;
  static Boolean  result;
  
  if(to->size < sizeof(Boolean))
    {
      to->size = sizeof(Boolean);
      return False;
    }
  for(ptr = genders; *ptr; ptr++)
    if(!strcmp(*ptr, (char CONST *)from->addr))
      {
	to->size = sizeof(Boolean);
	result = (ptr - genders) & 1 ? True : False;
	if(to->addr)
	   *(Boolean *)to->addr = result;
	else
	  to->addr = (XtPointer)&result;
	return True;
      }
  XtDisplayStringConversionWarning(display, from->addr, XtRGender);
  return False;
}
/*}}}*/
/*{{{  void create_resources(widget)*/
static VOIDFUNC create_resources
FUNCARG((widget),
	Widget  widget
)
/*
 * Create the graphics contexts, sprites and colours and stuff.
 * sets visual and stuff for the toplevel widget
 */
{
  Window    root;
  unsigned  depth;
  XVisualInfo visualinfo;

  /*{{{  get visual info*/
  {
    XVisualInfo *list;
    int       found;
      
    visualinfo.visualid = XVisualIDFromVisual(display.visual);
    list = XGetVisualInfo(display.display, VisualIDMask, &visualinfo, &found);
    while(found--)
      if(list[found].visual == display.visual)
	{
	  memcpy(&visualinfo, &list[found], sizeof(XVisualInfo));
	  break;
	}
    XFree((VOID *)list);
  }
  /*}}}*/
  depth = display.depth;
  if(visualinfo.colormap_size < 16)
    data.mono = True;
  root = RootWindow(display.display, display.screen);
  if(XDefaultVisual(display.display, display.screen) != display.visual &&
      data.private == False)
    fprintf(stderr, "Warning:Non-default visual with shared colormap\n");
  /*{{{  show visual class?*/
  if(data.colors != False)
    {
      VISUAL_CLASS CONST *vptr;
      
      for(vptr = visual_class; vptr->name; vptr++)
	if(vptr->class == visualinfo.class)
	  break;
      fprintf(stdout, "Using %s visual with %s colormap of %d entries\n",
	  vptr->name ? vptr->name : vptr->class & 1 ?
	  "UnknownDynamic" : "UnknownStatic",
	  data.private != False ? "private" : "shared",
	  visualinfo.colormap_size);
    }
  /*}}}*/
  /*{{{  make black and white*/
  {
    Status  status;
    XColor  color;
    
    color.red = color.green = color.blue = 0xFFFF;
    color.flags = DoRed | DoGreen | DoBlue;
    status = XAllocColor(display.display, display.colormap, &color);
    if(status)
      {
	display.white = color.pixel;
	color.red = color.green = color.blue = 0x0000;
	color.flags = DoRed | DoGreen | DoBlue;
	status = XAllocColor(display.display, display.colormap, &color);
	display.black = color.pixel;
      }
    if(!status)
      fatal_error("Cannot get hold of black and white pixels");
    if(data.colors != False)
      {
	fprintf(stdout, "Color white pixel %lu\n", display.white);
	fprintf(stdout, "Color black pixel %lu\n", display.black);
      }
  }
  /*}}}*/
  XtAppSetTypeConverter(display.context, XtRString, XtRColor,
      convert_string2color, (XtConvertArgList)colorConvertArgs, 2,
      XtCacheNone, (void (*)PROTOARG((XtAppContext, XrmValue *, XtPointer,
	  XrmValue *, Cardinal *)))NULL);
  color_one = ((unsigned long)1 << depth) - (unsigned long)1;
  if(data.mono == False)
    {
      unsigned  ix;
      COLOR_DEF *nptr;
      XColor    *cptr;
      unsigned  count;
      unsigned  new;
      unsigned  share;
      
      /*{{{  get resources*/
      {
	XtResource *resources;
	size_t    length;
	char      *text;
	char      *tptr;
	XtResource *rptr;
	unsigned  source;
	unsigned  mask;
      
	resources = (XtResource *)XtMalloc(COLORS * sizeof(XtResource));
	length = COLORS;
	for(nptr = color_names, count = COLORS; count--; nptr++)
	  length += strlen(nptr->name);
	text = XtMalloc(length);
	colors[COLOR_WHITE].flags = DoRed | DoGreen | DoBlue;
	colors[COLOR_WHITE].red = 0xFF00;
	colors[COLOR_WHITE].green = 0xFF00;
	colors[COLOR_WHITE].blue = 0xFF00;
	colors[COLOR_BLACK].flags = DoRed | DoGreen | DoBlue;
	colors[COLOR_BLACK].red = 0;
	colors[COLOR_BLACK].green = 0;
	colors[COLOR_BLACK].blue = 0;
	source = (data.gender != False ? 2 : 0) +
	    (data.swap != False ? 1 : 0);
	mask = (2 << source) - 1;
	if(source == 2)
	  mask ^= 2;
	/*{{{  build the resource table*/
	{
	  for(nptr = color_names, rptr = resources, tptr = text, ix = 0;
	      ix != COLORS; ix++, nptr++)
	    {
	      char CONST **sptr;
	      unsigned  bit;
	      
	      rptr->resource_name = tptr;
	      strcpy(tptr, nptr->name);
	      if(isupper(*tptr))
		*tptr = lowercase(*tptr);
	      tptr += strlen(tptr) + 1;
	      rptr->resource_class = (String)nptr->name;
	      rptr->resource_type = XtRColor;
	      rptr->resource_size = sizeof(XColor);
	      rptr->resource_offset = sizeof(XColor) * ix;
	      rptr->default_type = XtRString;
	      for(sptr = &nptr->source[source], bit = 1 << source;
		  (!*sptr || !(bit & mask)) && bit; bit >>= 1, sptr--)
		/* EMPTY */;
	      if(bit)
		rptr++->default_addr = (XtPointer)*sptr;
	    }
	  assert(tptr - text == length);
	}
	/*}}}*/
	if(data.swap != False)
	  XtGetSubresources(widget, (XtPointer)colors, "swap", "Swap",
	      resources, rptr - resources, NULL, 0);
	else
	  XtGetApplicationResources(widget, colors,
	      resources, rptr - resources, NULL, 0);
	XtFree(text);
	XtFree((char *)resources);
      }
      /*}}}*/
      display.dynamic = 0;
#ifndef XMRED
      /*{{{  dynamic colors*/
      if(data.nodynamic == False && visualinfo.class & 1)
	{
	  unsigned long pixels[2];
	  unsigned long planes;
	  
	  if(XAllocColorCells (display.display, display.colormap, False,
	      &planes, 0, pixels, 2))
	    {
	      display.dynamic = 1;
	      colors[COLOR_DYNAMIC].pixel = pixels[0];
	      colors[COLOR_DYNAMIC + 1].pixel = pixels[1];
	      if(data.colors != False)
		fprintf(stdout, "Dynamic: background %lu, foreground %lu\n",
		    pixels[0], pixels[1]);
	    }
	}
      /*}}}*/
#endif /* XMRED */
      count = new = 0;
      /*{{{  parse all the colors*/
      {
	unsigned  mask;
	
	mask = (data.gender != False ? 4 : 1) << (data.swap != False);
	for(cptr = colors, nptr = color_names, ix = COLORS;
	    ix--; nptr++, cptr++)
	  if(cptr->flags)
	    {
	      nptr->coord[0] = RGB2X(cptr->red, cptr->green, cptr->blue);
	      nptr->coord[1] = RGB2Y(cptr->red, cptr->green, cptr->blue);
	      nptr->coord[2] = RGB2H(cptr->red, cptr->green, cptr->blue);
	      nptr->distance = 0;
	      nptr->nearest = NULL;
	      if(nptr->type & mask)
		{
		  if(nptr->type & 16 && display.dynamic)
		    nptr->alloc = 5;
		  else if(nptr->type & 32 && !display.dynamic)
		    nptr->alloc = 4;
		  else
		    {
		      nptr->alloc = 0;
		      count++;
		    }
		}
	      else
		nptr->alloc = 4;
	    }
	  else if(nptr->type & mask)
	    fatal_error("Have no color for %s", nptr->name);
      }
      /*}}}*/
      share = 2;
      /*{{{  allocate them in optimum order*/
      for(colors[COLOR_WHITE].pixel = display.white,
	  nptr = &color_names[COLOR_WHITE], nptr->alloc = 1;
	  --count;)
	{
	  if(nptr->alloc == 1)
	    /*{{{  work out distance of just allocated color*/
	    {
	      unsigned  ix;
	      COLOR_DEF *optr;
		  
	      for(optr = color_names, ix = COLORS; ix--; optr++)
		if(!optr->alloc)
		  {
		    unsigned long distance;
		    unsigned  ix;
		    
		    distance = 0;
		    for(ix = 3; ix--;)
		      /*
		       * must be very careful about type promotion and
		       * overflow here
		       */
		      {
			unsigned  delta;
			
			delta = nptr->coord[ix] < optr->coord[ix] ?
			    optr->coord[ix] - nptr->coord[ix] :
			    nptr->coord[ix] - optr->coord[ix];
			distance += ((unsigned long)delta *
			    (unsigned long)delta) / 4;
		      }
		    if(distance < optr->distance || !optr->nearest)
		      {
			optr->distance = distance;
			optr->nearest = nptr;
		      }
		  }
	    }
	    /*}}}*/
	  if(nptr == &color_names[COLOR_WHITE])
	    {
	      nptr = &color_names[COLOR_BLACK];
	      nptr->alloc = 1;
	      colors[COLOR_BLACK].pixel = display.black;
	    }
	  else
	    /*{{{  allocate the farthest one*/
	    {
	      unsigned long distance;
	      COLOR_DEF *optr;
	      Status    status;
	      unsigned  ix;
	      XColor    *fptr;
	    
	      nptr = NULL;
	      distance = 0;
	      for(optr = color_names, ix = COLORS; ix--; optr++)
		if(!optr->alloc && distance <= optr->distance)
		  {
		    nptr = optr;
		    distance = nptr->distance;
		  }
	      assert(nptr && nptr->nearest);
	      cptr = &colors[nptr - color_names];
	      fptr = &colors[nptr->nearest - color_names];
	      if(distance)
		/*{{{  different*/
		{
		  if(data.colors != False)
		    fprintf(stdout,
			"Color %s:#%04X%04X%04X near %s:#%04X%04X%04X (%lu)",
			nptr->name, cptr->red, cptr->green, cptr->blue,
			nptr->nearest->name,
			fptr->red, fptr->green, fptr->blue, distance);
		  if(data.distinct)
		    status = XAllocColor(display.display, display.colormap,
		      cptr);
		  else
		    status = 0;
		}
		/*}}}*/
	      else
		/*{{{  same*/
		{
		  if(data.colors != False)
		    fprintf(stdout,
			"Color %s:#%04X%04X%04X at %s:#%04X%04X%04X",
			nptr->name, cptr->red, cptr->green, cptr->blue,
			nptr->nearest->name,
			fptr->red, fptr->green, fptr->blue);
		  status = 0;
		}
		/*}}}*/
	      /*{{{  fixup*/
	      if(status)
		{
		  new++;
		  nptr->alloc = 1;
		  distance = 0;
		  data.distinct--;
		}
	      else
		{
		  nptr->alloc = 2 + !!distance;
		  cptr->pixel = fptr->pixel;
		  share++;
		}
	      /*}}}*/
	      if(data.colors != False)
		fprintf(stdout, nptr->alloc == 1 ?
		    " pixel %lu\n" : " sharing %lu\n", cptr->pixel);
	      /*{{{  error message?*/
	      if(distance)
		{
		  fprintf(stderr,
		      "No color %s:#%04X%04X%04X used %s:#%04X%04X%04X\n",
		      nptr->name, cptr->red, cptr->green, cptr->blue,
		      nptr->nearest->name,
		      fptr->red, fptr->green, fptr->blue);
		}
	      /*}}}*/
	    }
	    /*}}}*/
	}
      /*}}}*/
      if(data.colors != False)
	fprintf(stdout, "%u new colours and %u shared\n", new, share);
      for(nptr = color_names, count = COLORS; count--; nptr++)
	assert(nptr->alloc);
    }
  /*{{{  swap b & w?*/
  if(data.swap != False)
    {
      unsigned long temp;
    
      temp = display.black;
      display.black = display.white;
      display.white = temp;
    }
  /*}}}*/
  if(data.mono == False)
    {
      display.white = colors[COLOR_BACKGROUND].pixel;
      display.black = colors[COLOR_FOREGROUND].pixel;
      display.border = colors[COLOR_BORDER].pixel;
    }
  else
    display.border = display.black;
  /*{{{  nadger created widgets*/
  {
    Widget    root;
    Widget    parent;
    
    for(root = widget; (parent = XtParent(root)); root = parent)
      /* EMPTY */;
    nadger_widget_colors(root, (WidgetClass)NULL);
  }
  /*}}}*/
  display.xor = display.black ^ display.white;
  /*{{{  set foreground & background types*/
  {
    unsigned long set;
    unsigned long clear;
    unsigned  ix;
    
    set = color_zero;
    clear = color_one;
    /*{{{  set on and off*/
    if(data.mono != False)
      {
	set |= display.white;
	clear &= display.white;
	set |= display.black;
	clear &= display.black;
      }
    else
      {
	for(ix = COLORS; ix--;)
	  {
	    set |= colors[ix].pixel;
	    clear &= colors[ix].pixel;
	  }
      }
    /*}}}*/
    display.background = display.white == clear ? COLOUR_ZERO :
	display.white == set ? COLOUR_ONE : COLOUR_WEIRD;
    display.foreground = display.black == clear ? COLOUR_ZERO :
	display.black == set ? COLOUR_ONE : COLOUR_WEIRD;
  }
  /*}}}*/
  display.copy = XCreatePixmap(display.display, root,
      WINDOW_WIDTH, WINDOW_HEIGHT, depth);
#ifndef XMRED
  display.back = XCreatePixmap(display.display, root,
      WINDOW_WIDTH, WINDOW_HEIGHT, depth);
#endif /* XMRED */
  /*{{{  create graphics contexts*/
  {
    XGCValues gcv;
    CONTEXT   *cptr;
    unsigned  count;
	
    gcv.font       = data.font;
    gcv.graphics_exposures = False;
    gcv.line_width = 1;
    gcv.cap_style = CapNotLast;
    for(cptr = gcsdefine, count = 0; count != GCS; count++, cptr++)
      {
	gcv.function = cptr->function;
	gcv.foreground = *cptr->fgp;
	gcv.background = *cptr->bgp;
	GCN(count) = XCreateGC(display.display, display.copy,
	    GCForeground | GCBackground | GCFunction | GCFont |
	    GCGraphicsExposures | GCCapStyle | GCLineWidth, &gcv);
      }
  }
  /*}}}*/
  XFillRectangle(display.display, display.copy, GCN(GC_CLEAR),
      0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
  {
    SPRITE_DEF CONST *sptr;
    int       nsizes;
    XIconSize *sizes;
    Status    status;
    SIZE      size;
    
#ifndef XMRED
    sptr = &icons[data.gender != False];
#else
    sptr = &icons[0];
#endif /* XMRED */
    status = XGetIconSizes(display.display, root, &sizes, &nsizes);
    if(status)
      /*{{{  find best size*/
      {
	int       ix;
	SIZE      undersize;
	int       found;
	
	size.x = size.y = 128;
	undersize.x = undersize.y = 1;
	found = 0;
	for(ix = nsizes; ix--; sizes++)
	  {
	    SIZE      this;
	    
	    if(sptr->size.x < sizes->min_width)
	      this.x = sizes->min_width;
	    else if(sptr->size.x > sizes->max_width)
	      this.x = sizes->max_width;
	    else
	      this.x = (sptr->size.x - sizes->min_width +
		  sizes->width_inc - 1) / sizes->width_inc *
		  sizes->width_inc + sizes->min_width;
	    if(sptr->size.y < sizes->min_height)
	      this.y = sizes->min_height;
	    else if(sptr->size.y > sizes->max_height)
	      this.y = sizes->max_height;
	    else
	      this.y = (sptr->size.y - sizes->min_height +
		  sizes->height_inc - 1) / sizes->height_inc *
		  sizes->height_inc + sizes->min_height;
	    if(this.x < sptr->size.x || this.y < sptr->size.y)
	      {
		if(this.x * this.y > undersize.x * undersize.y)
		  {
		    undersize.x = this.x;
		    undersize.y = this.y;
		  }
	      }
	    else if(this.x * this.y < size.x * size.y)
	      {
		found = 1;
		size.x = this.x;
		size.y = this.y;
	      }
	  }
	if(!found)
	  {
	    size.x = undersize.x;
	    size.y = undersize.y;
	  }
      }
      /*}}}*/
    else
      {
	size.x = sptr->size.x;
	size.y = sptr->size.y;
      }
    display.icon = XCreateBitmapFromData(display.display, root,
	(char CONST *)sptr->bitmap, sptr->size.x, sptr->size.y);
    /*{{{  create correct sized icon*/
    if(size.x != sptr->size.x || size.y != sptr->size.y)
      {
	Pixmap    temp;
	GC        gc;
	XGCValues gcv;
	
	temp = XCreatePixmap(display.display, root, size.x, size.y, 1);
	gc = XCreateGC(display.display, temp, 0, &gcv);
	XSetFunction(display.display, gc, GXclear);
	XFillRectangle(display.display, temp, gc, 0, 0, size.x, size.y);
	XSetFunction(display.display, gc, GXcopy);
	XCopyArea(display.display, display.icon, temp, gc, 0, 0,
	    sptr->size.x, sptr->size.y,
	    (size.x - sptr->size.x) / 2, (size.y - sptr->size.y) / 2);
	XFreeGC(display.display, gc);
	XFreePixmap(display.display, display.icon);
	display.icon = temp;
      }
    /*}}}*/
    XtVaSetValues(display.toplevel, XtNiconPixmap, (XtArgVal)display.icon,
	NULL);
  }
  /*{{{  get a font*/
  {
    char CONST *string = "09\'\"";
    int       direction;
    XCharStruct chars;
	
    XQueryTextExtents(display.display, data.font, string, 4,
	    &direction, &font.ascent, &font.descent, &chars);
    font.width = chars.width / 4;
    font.center = (font.ascent - font.descent) / 2;
    font.digitup = chars.ascent;
    font.digitdown = chars.descent;
  }
  /*}}}*/
  if(data.gender != False)
    {
      SPRITE_DEF *dptr;
      SPRITE_DEF CONST *sptr;
      
      for(sptr = she_nadger; sptr->copy; sptr++)
	{
	  dptr = &sprites_def[sptr->copy];
	  dptr->bitmap = sptr->bitmap;
	  dptr->planes = sptr->bitmap ? sptr->planes : 0;
	  dptr->flags = sptr->flags;
	  dptr->copy = sptr->bitmap ? 0 : sptr->planes;
	  memcpy(&dptr->size, &sptr->size, sizeof(sptr->size));
	  memcpy(dptr->colors, sptr->colors, sizeof(sptr->colors));
	}
#ifndef XMRED
      memcpy(ball_hold, she_hold, sizeof(ball_hold));
      lettering[0] = letter_msit;
#endif /* XMRED */
    }
  /*{{{  create sprites*/
  {
    unsigned  i;
    SPRITE    *dptr;
    SPRITE_DEF *sptr;
    unsigned  unknown;
    unsigned  changed;
	
    /*{{{  generate all the ones from bitmaps*/
    for(i = 0, sptr = sprites_def, dptr = sprites;
	i != SPRITES; i++, sptr++, dptr++)
      {
	/* check that its the size we expected */
	if(sptr->bitmap)
	  {
	    Pixmap    bitmap;
	    
	    assert(!sptr->expected.x || sptr->expected.x == sptr->size.x);
	    assert(!sptr->expected.y || sptr->expected.y * sptr->planes == sptr->size.y);
	    assert(sptr->size.x && sptr->size.y && !sptr->copy);
	    dptr->size.x = sptr->size.x;
	    dptr->size.y = sptr->size.y / sptr->planes;
	    bitmap = XCreatePixmapFromBitmapData(display.display,
		root, (char *)sptr->bitmap,
		sptr->size.x, sptr->size.y, color_one, color_zero, depth);
	    dptr->image = XCreatePixmap(display.display, root,
		dptr->size.x, dptr->size.y, depth);
	    dptr->mask = XCreatePixmap(display.display, root,
		dptr->size.x, dptr->size.y, depth);
	    if(!sptr->colors[0])
	      XFillRectangle(display.display, dptr->image, GCN(GC_CLEAR),
		  0, 0, sptr->size.x, sptr->size.y);
	    else if(data.mono != False)
	      make_mono_image(dptr, sptr, root, depth, bitmap);
	    else
	      make_color_image(dptr, sptr, root, depth, bitmap);
	    make_mask_image(dptr, sptr, root, depth, bitmap);
	    XFreePixmap(display.display, bitmap);
	  }
      }
    /*}}}*/
    do
      {
	unknown = SPRITES;
	changed = 0;
	/*{{{  do what copies we can*/
	for(sptr = sprites_def, dptr = sprites, i = SPRITES;
	    i--; sptr++, dptr++)
	  if(!sptr->copy)
	    {
	      /*
	       * Sun's assert macro is broken, so I have to
	       * put it in a scope
	       */
	      assert(sptr->bitmap);
	    }
	  else if(!dptr->mask && !dptr->image)
	    {
	      SPRITE    *optr;
	
	      optr = &sprites[sptr->copy];
	      if(!optr->size.x || !optr->size.y)
		unknown = i;
	      else
		{
		  changed = 1;
		  dptr->size.x = optr->size.x;
		  dptr->size.y = optr->size.y;
		  switch(sptr->flags)
		  {
		    /*{{{  case REFLECT_ALIAS:*/
		    case REFLECT_ALIAS:
		      dptr->mask = optr->mask;
		      dptr->image = optr->image;
		      break;
		    /*}}}*/
		    /*{{{  case REFLECT_VERTICAL:*/
		    case REFLECT_VERTICAL:
		    {
		      int       i;
		    
		      dptr->mask = XCreatePixmap(display.display, root,
			  dptr->size.x, dptr->size.y, depth);
		      dptr->image = XCreatePixmap(display.display, root,
			  dptr->size.x, dptr->size.y, depth);
		      for(i = dptr->size.x; i--;)
			{
			  XCopyArea(display.display, optr->mask, dptr->mask,
			      GCN(GC_COPY), i, 0, 1, dptr->size.y,
			      (int)dptr->size.x - i - 1, 0);
			  XCopyArea(display.display, optr->image, dptr->image,
			      GCN(GC_COPY), i, 0, 1, dptr->size.y,
			      (int)dptr->size.x - i - 1, 0);
			}
		      break;
		    }
		    /*}}}*/
		    /*{{{  case REFLECT_HORIZONTAL:*/
		    case REFLECT_HORIZONTAL:
		    {
		      int       i;
		    
		      dptr->mask = XCreatePixmap(display.display, root,
			  dptr->size.x, dptr->size.y, depth);
		      dptr->image = XCreatePixmap(display.display, root,
			  dptr->size.x, dptr->size.y, depth);
		      for(i = dptr->size.y; i--;)
			{
			  XCopyArea(display.display, optr->mask, dptr->mask,
			      GCN(GC_COPY), 0, i, dptr->size.x, 1,
			      0, (int)dptr->size.y - i - 1);
			  XCopyArea(display.display, optr->image, dptr->image,
			      GCN(GC_COPY), 0, i, dptr->size.x, 1,
			      0, (int)dptr->size.y - i - 1);
			}
		      break;
		    }
		    /*}}}*/
		    /*{{{  case REFLECT_DIAGONAL:*/
		    case REFLECT_DIAGONAL:
		    {
		      int       x, y;
		    
		      dptr->size.x = optr->size.y;
		      dptr->size.y = optr->size.x;
		      dptr->mask = XCreatePixmap(display.display, root,
			  dptr->size.x, dptr->size.y, depth);
		      dptr->image = XCreatePixmap(display.display, root,
			  dptr->size.x, dptr->size.y, depth);
		      for(x = optr->size.x; x--;)
			for(y = optr->size.y; y--;)
			  {
			    XCopyArea(display.display, optr->mask, dptr->mask,
				GCN(GC_COPY), x, y, 1, 1, y, x);
			    XCopyArea(display.display,
				optr->image, dptr->image,
				GCN(GC_COPY), x, y, 1, 1, y, x);
			  }
		      break;
		    }
		    /*}}}*/
		    /*{{{  default*/
		    default:
		      assert(0);
		    /*}}}*/
		  }
		}
	    }
	/*}}}*/
      }
    while(unknown != SPRITES && changed);
    assert(unknown == SPRITES); /* check no circular definitions */
  }
  /*}}}*/
#ifndef XMRED
  /*{{{  digits special*/
  {
    Pixmap    bitmap;
    SPRITE    *dptr;
    
    dptr = &sprites[SPRITE_DIGITS];
    bitmap = XCreatePixmap(display.display, root,
	dptr->size.x, dptr->size.y, depth);
    XFillRectangle(display.display, bitmap, GCN(GC_CLEAR),
	0, 0, dptr->size.x, dptr->size.y);
    XCopyArea(display.display, dptr->mask, bitmap, GCN(GC_MASK),
	0, 0, dptr->size.x, dptr->size.y, 0, 0);
    XCopyArea(display.display, bitmap, dptr->image, GCN(GC_OR),
	0, 0, dptr->size.x, dptr->size.y, 0, 0);
    XFreePixmap(display.display, bitmap);
  }
  /*}}}*/
#else
  /*{{{  visual special*/
  if(data.mono == False && visualinfo.class != StaticGray &&
      visualinfo.class != GrayScale)
  {
    int       count;
    COLOR_DEF *cptr;
    SPRITE    *dptr;
    
    for(cptr = &color_names[COLOR_BACKGROUNDS], count = BACKGROUNDS * 2;
	count--; cptr++)
      if(cptr->alloc == 3)
	break;
    if(count < 0)
      for(dptr = &sprites[SPRITE_COLORS], count = BACKGROUNDS; count--; dptr++)
	{
	  XFreePixmap(display.display, dptr->mask);
	  dptr->mask = 0;
	  XFreePixmap(display.display, dptr->image);
	  dptr->image = 0;
	}
  }
  /*}}}*/
  /*{{{  sprite build*/
  {
    unsigned  ix;
    SPRITE    *sptr;
    SPRITE    *aptr;
    
    for(sptr = &sprites[SPRITE_APPLES], ix = 0; ix != 5; sptr++, ix++)
      {
	unsigned  count;
	
	for(count = 0; count != 4; count++)
	  {
	    aptr = &sprites[SPRITE_SMALL_APPLE + (count != ix)];
	    XCopyArea(display.display, aptr->mask, sptr->mask, GCN(GC_COPY),
		0, 0, aptr->size.x, aptr->size.y,
		(count % 2) * aptr->size.x, (count / 2) * aptr->size.y);
	    XCopyArea(display.display, aptr->image, sptr->image, GCN(GC_COPY),
		0, 0, aptr->size.x, aptr->size.y,
		(count % 2) * aptr->size.x, (count / 2) * aptr->size.y);
	  }
      }
    sptr = &sprites[SPRITE_APPLE_DROP];
    aptr = &sprites[SPRITE_BIG_APPLE];
    XCopyArea(display.display, aptr[1].mask, sptr->mask, GCN(GC_COPY),
	0, 0, sptr->size.x, sptr->size.y, 0, 0);
    XCopyArea(display.display, aptr[1].image, sptr->image, GCN(GC_COPY),
	0, 0, sptr->size.x, sptr->size.y, 0, 0);
    XCopyArea(display.display, aptr[3].mask, sptr->mask, GCN(GC_OR),
	0, 0, sptr->size.x, sptr->size.y, 0, 0);
    XCopyArea(display.display, aptr[3].image, sptr->image, GCN(GC_OR),
	0, 0, sptr->size.x, sptr->size.y, 0, 0);
  }
  /*}}}*/
#endif /* XMRED */
#ifndef NDEBUG
  /* check we've used all the colors we allocated */
  if(data.mono == False)
    {
      unsigned  ix;
      COLOR_DEF *cptr;
      
      color_names[COLOR_FOREGROUND].name = NULL;
      color_names[COLOR_BACKGROUND].name = NULL;
      color_names[COLOR_BORDER].name = NULL;
#ifndef XMRED
      color_names[COLOR_DYNAMIC].name = NULL;
      color_names[COLOR_DYNAMIC + 1].name = NULL;
#endif /* XMRED */
      for(cptr = color_names, ix = COLORS; ix--; cptr++)
	{
	  /* again some broken compilers can't cope with asserts
	   * over several lines */
	  int     check;
	  
	  check = cptr->alloc > 3 || !cptr->name ||
	      (cptr->type & 16 && !display.dynamic);
	  assert(check);
	}
    }
#endif /* NDEBUG */
  if(display.background == COLOUR_ZERO)
    {
      SPRITE  *sptr;
      unsigned ix;
      
      for(sptr = &sprites[SPRITE_CENTER_BASE], ix = 4; ix--; sptr++)
	XCopyArea(display.display, sptr->image, sptr->mask, GCN(GC_MASK),
	    0, 0, sptr->size.x, sptr->size.y, 0, 0);
    }
  /*{{{  create fills*/
  {
    unsigned  i;
    SPRITE    *dptr;
    SPRITE_DEF *sptr;
	
    for(i = 0, sptr = fills_def, dptr = fills;
	i != FILLS; i++, sptr++, dptr++)
      {
	int       x, y;
	char      c;
	unsigned char *ptr;
	
	assert(sptr->size.x && sptr->size.y);
	dptr->size.x = sptr->size.x;
	dptr->size.y = sptr->size.y;
	if(data.mono)
	  {
	    if(data.swap != False)
	      for(ptr = sptr->bitmap,
		  x = (sptr->size.x + 7) / 8 * sptr->size.y; x--; ptr++)
		*ptr ^= 0xFF;
	    for(ptr = sptr->bitmap, y = sptr->size.y; y--;)
	      for(c = 0x55 << (y & 1), x = (sptr->size.x + 7) / 8; x--; ptr++)
		*ptr |= c;
	  }
	dptr->mask = XCreateBitmapFromData(display.display, root,
	    (char CONST *)sptr->bitmap, sptr->size.x, sptr->size.y);
      }
  }
  /*}}}*/
#ifndef XMRED
  /*{{{  create score pixmaps*/
  {
    unsigned  i;
	
    for(i = BOARD_SCORES; i--;)
      {
	update.score.list[i].image = XCreatePixmap(display.display,
	    root, DIGIT_WIDTH * 4, DIGIT_HEIGHT, depth);
	update.score.list[i].mask = XCreatePixmap(display.display,
	    root, DIGIT_WIDTH * 4, DIGIT_HEIGHT, depth);
      }
  }
  /*}}}*/
#endif /* XMRED */
  return;
}
/*}}}*/
#ifndef NDEBUG
/*{{{  int error_handler(display, event)*/
extern int error_handler
FUNCARG((dpy, event),
	Display *dpy
ARGSEP  XErrorEvent *event
)
{
  char  msg[80];
  
  XGetErrorText(dpy, event->error_code, msg, 80);
  fprintf(stderr, "X Error: %s\n", msg);
  fprintf(stderr, "  Major opcode: %d\n", event->request_code);
  fprintf(stderr, "  Minor opcode: %d\n", event->minor_code);
  fprintf(stderr, "  Resource id: 0x%lx\n", (long)event->resourceid);
  fprintf(stderr, "  Serial number: %ld\n", (long)event->serial);
#ifdef DEBUGEVENTLOOP
  while(QLength(display.display))
    {
      XEvent  buffered;
      
      XNextEvent(display.display, &buffered);
      fprintf(stderr, "0x%lx type %d\n", buffered.xany.window, buffered.type);
    }
#endif /* DEBUGEVENTLOOP */
  fflush(stderr);
  abort();
  return 0;
}
/*}}}*/
#endif /* NDEBUG */
/*{{{  void extract_options(argcp, argv, options)*/
static VOIDFUNC extract_options
FUNCARG((argcp, argv, options),
	int       *argcp
ARGSEP  String    *argv
ARGSEP  CommandOptionDesc CONST *options
)
{
  CommandOptionDesc CONST *ptr;
  unsigned  ix;
  unsigned  copy;
  
  for(ptr = options; ptr->option; ptr++)
    if(ptr->optional)
      *(char **)((char *)&data + ptr->offset) = NULL;
    else
      *(Boolean *)((char *)&data + ptr->offset) = False;
  for(ix = copy = 1; ix != *argcp; ix++)
    {
      size_t    length;
      
      length = strlen(argv[ix]);
      for(ptr = options; ptr->option; ptr++)
	{
	  if(!strncmp(argv[ix], ptr->option, length))
	    {
	      if(ptr->optional)
		{
		  if(ix < *argcp - 1)
		    { 
		      *(char **)((char *)&data + ptr->offset) =
			  argv[ix + 1];
		      ix++;
		    }
		  else
		    {
		      data.help = True;
		      data.colors = False;
		      fprintf(stderr, "Argument expected for %s\n",
			  ptr->option);
		    }
		}
	      else
		*(Boolean *)((char *)&data + ptr->offset) = True;
	      break;
	    }
	}
      if(!ptr->option)
	argv[copy++] = argv[ix];
    }
  *argcp = copy;
  return;
}
/*}}}*/
/*{{{  void gettoplevelresources()*/
static VOIDFUNC gettoplevelresources FUNCARGVOID
/* 
 * get toplevel resources and
 * create a private colormap, if required
 * must be done before creating other widgets, in order
 * to get the visual inheritance correct
 */
{
  Screen    *screenptr;

  XtVaGetValues(display.toplevel, XtNvisual, (XtArgVal)&display.visual,
      XtNdepth, (XtArgVal)&display.depth, XtNscreen, (XtArgVal)&screenptr, 
      XtNcolormap, (XtArgVal)&display.colormap, NULL);
  display.screen = XScreenNumberOfScreen(screenptr);
  if(!display.visual)
    display.visual = DefaultVisualOfScreen(screenptr);
  if(data.private != False)
    {
      display.colormap = XCreateColormap(display.display,
	  RootWindowOfScreen(screenptr), display.visual, AllocNone);
      XtVaSetValues(display.toplevel,
	  XtNcolormap, (XtArgVal)display.colormap, NULL);
    }
  return;
}
/*}}}*/
/*{{{  void fatal_error(text, ...)*/
extern VOIDFUNC fatal_error
FUNCVARARG((text, VARARGDEF),
	char CONST *text
)
{
  va_list   args;
  
  fputs(myname, stderr);
  fputc(':', stderr);
  VARARGSET(args, text);
  vfprintf(stderr, text, args);
  fputc('\n', stderr);
/*  destroy_widget();
  close_toolkit();*/
  exit(1);
  return;
}
/*}}}*/
/*{{{  void list_color_resource(mask, resource, value)*/
static VOIDFUNC list_color_resource
FUNCARG((mask, resource, value),
	unsigned  mask
ARGSEP  char CONST *resource
ARGSEP  char CONST *value
)
{
  unsigned  len;
  
  assert(mask && mask != 6 && mask != 9);
  if(((mask + 1) | mask) == 0xF)
    mask = 15;
  fputs("Xmris", stderr);
  len = 5;
  if(mask == 15 || mask == 5)
    {
      fputc('*', stderr);
      len++;
    }
  else
    {
      if(!(mask & 3))
	{
	  fputs("*msit", stderr);
	  len += 5;
	}
      else if(!(mask & 12))
	{
	  fputs("*mris", stderr);
	  len += 5;
	}
      if(!(mask & 10))
	{
	  fputc('.', stderr);
	  len++;
	}
      else if(mask == 12 || mask == 3)
	{
	  fputc('*', stderr);
	  len++;
	}
      else if(mask == 10)
	{
	  fputs("*swap.", stderr);
	  len += 6;
	}
      else
	{
	  fputs(".swap.", stderr);
	  len += 6;
	}
    }
  len += strlen(resource);
  fprintf(stderr, "%s:%*s%s\n", resource,
      len < 32 ? (int)(32 - len) : 1, "", value);
  return;
}
/*}}}*/
/*{{{  void list_help(name)*/
extern VOIDFUNC list_help
FUNCARG((name),
	char CONST *name
)
{
  if(data.colors != False)
    /*{{{  color resources*/
    {
      COLOR_DEF CONST *cptr;
      unsigned  count;
      
      fprintf(stderr, "!Colour resources\n");
      fprintf(stderr, "!Xmris*{mris,msit}.{swap}.<name>:<color>\n");
      for(cptr = &color_names[4], count = COLORS - 4; count--; cptr++)
	{
	  unsigned  mask;
	  char CONST *values[4];
	  unsigned  point;
	  
	  mask = cptr->type & 15;
	  memcpy(values, cptr->source, sizeof(values));
	  if(!values[1])
	    values[1] = values[0];
	  if(!values[3])
	    values[3] = values[2];
	  if(!values[3])
	    values[3] = values[1];
	  if(!values[2])
	    values[2] = values[0];
	  for(point = 0; mask && point < 4; point++)
	    {
	      unsigned  ix;
	      unsigned  select;
	      
	      for(select = 0, ix = 4; ix--;)
		if(!strcmp(values[ix], values[point]))
		  select |= 1 << ix;
	      select &= mask;
	      if(select)
		list_color_resource(select, cptr->name, values[point]);
	      mask ^= select;
	    }
	}
    }
    /*}}}*/
  else
    {
      HELP CONST *hptr;
      
      fprintf(stderr, "%s [-option ...] [-toolkitoption ...]\n", name);
      fprintf(stderr, "%s %s %s %s\n",
#ifndef XMRED
	  names[0], XMRISVERSION,
#else
	  "xmred", XMREDVERSION,
#endif /* XMRED */
	  DATE, COPYRIGHT);
      fprintf(stderr, " Option    Resource         Argument\n");
      for(hptr = help; hptr->help; hptr++)
	fprintf(stderr, "%-10s %-16s %-16s %s\n",
	    hptr->arg, hptr->resource, hptr->option, hptr->help);
    }
  return;
}
/*}}}*/
/*{{{  void make_color_image(dptr, sptr, root, depth, bitmap)*/
static VOIDFUNC make_color_image
FUNCARG((dptr, sptr, root, depth, bitmap),
	SPRITE    *dptr
ARGSEP  SPRITE_DEF *sptr
ARGSEP  Window    root
ARGSEP  unsigned  depth
ARGSEP  Pixmap    bitmap
)
/*
 * The colour sprite image is generated from the bitmap, which
 * is treated as a set of n planes vertically alligned.
 * The allows 2^n different colours per sprite (including the mask). The
 * mapping from the sprite planes to the colour map is
 * controlled by the sprite's colour table, which pointes the game's
 * colour table, which contains the pixel value to use.
 * We go through the bit planes, once for each colour, generating colour
 * stencils which we or together for the image. Rather like colour
 * printing really.
 */
{
  Pixmap    temp;
  unsigned  ix;
  
  temp = XCreatePixmap(display.display, root,
      dptr->size.x, dptr->size.y, depth);
  XSetForeground(display.display, GCN(GC_BOARD), color_zero);
  XFillRectangle(display.display, dptr->image, GCN(GC_BOARD),
      0, 0, dptr->size.x, dptr->size.y);
  for(ix = 1 << sptr->planes; --ix;)
    {
      unsigned long color;
      
      color = sptr->colors[data.swap != False][ix];
      if(color != ~(unsigned long)0)
	{
	  unsigned  count;
	  
	  {
	    /* stupid compilers with broken stringizizing
	     * and stupid compilers which don't understand \
	     * outside of #define */
	    unsigned  alloc;
	    
	    alloc = color_names[(unsigned)color].alloc;
	    assert(alloc == 1 || alloc == 2 || alloc == 3);
	  }
#ifndef NDEBUG
	  color_names[(unsigned)color].name = NULL;
#endif /* NDEBUG */
	  XSetForeground(display.display, GCN(GC_BOARD),
	      colors[(unsigned)color].pixel);
	  XFillRectangle(display.display, temp, GCN(GC_BOARD),
	      0, 0, dptr->size.x, dptr->size.y);
	  for(count = sptr->planes; count--;)
	    XCopyArea(display.display, bitmap, temp,
		GCN(ix & (1 << count) ? GC_AND : GC_MASK),
		0, (int)(count * dptr->size.y),
		dptr->size.x, dptr->size.y, 0, 0);
	  XCopyArea(display.display, temp, dptr->image, GCN(GC_OR),
	      0, 0, dptr->size.x, dptr->size.y, 0, 0);
	}
    }
  XFreePixmap(display.display, temp);
  return;
}
/*}}}*/
/*{{{  void make_mask_image(dptr, sptr, root, depth, bitmap)*/
static VOIDFUNC make_mask_image
FUNCARG((dptr, sptr, root, depth, bitmap),
	SPRITE  *dptr
ARGSEP  SPRITE_DEF *sptr
ARGSEP  Window    root
ARGSEP  unsigned  depth
ARGSEP  Pixmap    bitmap
)
/*
 * the mask is made from logical color ~0, ie or all the bitmap
 * planes together, and that's the mask.
 * the mask halo is generated from the mask by oring the mask ontop
 * of itself, offset by the 4 adjacent pixels. This grows the mask too.
 * the halo is the xor of the grown mask and the original mask.
 */
{
  unsigned  ix;
  
  XSetForeground(display.display, GCN(GC_BOARD), color_zero);
  XFillRectangle(display.display, dptr->mask, GCN(GC_BOARD),
      0, 0, dptr->size.x, dptr->size.y);
  for(ix = sptr->planes; ix--;)
    XCopyArea(display.display, bitmap, dptr->mask, GCN(GC_OR),
	0, (int)(dptr->size.y * ix), dptr->size.x, dptr->size.y, 0, 0);
  XCopyArea(display.display,
      dptr->mask, dptr->image, GCN(GC_AND),
      0, 0, dptr->size.x, dptr->size.y, 0, 0);
  if(sptr->flags & BORDER_HALO_MASK)
    /*{{{  make the halo*/
    {
      Pixmap    halo;
      
      halo = XCreatePixmap(display.display, root,
	  dptr->size.x, dptr->size.y, depth);
      XFillRectangle(display.display, halo, GCN(GC_BOARD),
	  0, 0, dptr->size.x, dptr->size.y);
      XCopyArea(display.display, dptr->mask, halo, GCN(GC_OR),
	  0, 0, dptr->size.x, dptr->size.y, 0, 1);
      XCopyArea(display.display, dptr->mask, halo, GCN(GC_OR),
	  0, 0, dptr->size.x, dptr->size.y, 0, -1);
      XCopyArea(display.display, dptr->mask, halo, GCN(GC_OR),
	  0, 0, dptr->size.x, dptr->size.y, 1, 0);
      XCopyArea(display.display, dptr->mask, halo, GCN(GC_OR),
	  0, 0, dptr->size.x, dptr->size.y, -1, 0);
      XCopyArea(display.display, dptr->mask, halo, GCN(GC_MASK),
	  0, 0, dptr->size.x, dptr->size.y, 0, 0);
      XCopyArea(display.display, halo, dptr->mask, GCN(GC_OR),
	  0, 0, dptr->size.x, dptr->size.y, 0, 0);
      XSetForeground(display.display, GCN(GC_BOARD),
	  data.swap != False && sptr->flags & (data.mono != False ?
	  BORDER_MONOSWAP_EDGE_MASK : BORDER_WHITE_EDGE_MASK) ?
	  display.black : display.white);
      XSetFunction(display.display, GCN(GC_BOARD), GXand);
      XFillRectangle(display.display, halo, GCN(GC_BOARD),
	  0, 0, dptr->size.x, dptr->size.y);
      XSetFunction(display.display, GCN(GC_BOARD), GXcopy);
      XCopyArea(display.display, halo, dptr->image, GCN(GC_OR),
	  0, 0, dptr->size.x, dptr->size.y, 0, 0);
      XFreePixmap(display.display, halo);
    }
    /*}}}*/
  return;
}
/*}}}*/
/*{{{  void make_mono_image(dptr, sptr, root, depth, bitmap)*/
static VOIDFUNC make_mono_image
FUNCARG((dptr, sptr, root, depth, bitmap),
	SPRITE  *dptr
ARGSEP  SPRITE_DEF *sptr
ARGSEP  Window    root
ARGSEP  unsigned  depth
ARGSEP  Pixmap    bitmap
)
/*
 * As with the colour sprite image, the monochrome is generated from the
 * same bitmap and logical colors, but these either map to black or white
 * only.
 */
{
  Pixmap    temp;
  unsigned long colors;
  unsigned  ix;
  
  temp = XCreatePixmap(display.display, root,
      dptr->size.x, dptr->size.y, depth);
  XSetForeground(display.display, GCN(GC_BOARD), color_zero);
  XFillRectangle(display.display, dptr->image, GCN(GC_BOARD),
      0, 0, dptr->size.x, dptr->size.y);
  colors = sptr->colors[data.swap != False][0];
  if(data.swap != False)
    colors = ~colors;
  for(ix = 1 << sptr->planes; ix; ix--)
    {
      unsigned  count;
      
      if(sptr->colors[data.swap != False][ix] != ~(unsigned long)0)
	{
	  XSetForeground(display.display, GCN(GC_BOARD),
	      colors & (unsigned long)1 << ix ?
	      display.black : display.white);
	  XFillRectangle(display.display, temp, GCN(GC_BOARD),
	      0, 0, dptr->size.x, dptr->size.y);
	  for(count = sptr->planes; count--;)
	    XCopyArea(display.display, bitmap, temp,
		GCN(ix & (1 << count) ? GC_AND : GC_MASK),
		0, (int)(count * dptr->size.y),
		dptr->size.x, dptr->size.y, 0, 0);
	  XCopyArea(display.display, temp, dptr->image, GCN(GC_OR),
	     0, 0, dptr->size.x, dptr->size.y, 0, 0);
	}
    }
  XFreePixmap(display.display, temp);
  return;
}
/*}}}*/
/*{{{  void nadger_widget_colors(root, class)*/
extern VOIDFUNC nadger_widget_colors
FUNCARG((root, class),
	Widget    root
ARGSEP  WidgetClass class
)
/* traverse the widget tree and set the foreground, background and border
 * colors.
 * This has to be done, so that the swap attribute works as expected.
 * Without it, swap doesn't affect some widget's colors, which is wrong.
 */
{
  /*{{{  typedef struct Stack*/
  typedef struct Stack
  {
    Widget  *children;
    Cardinal count;
  } STACK;
  /*}}}*/
  /*{{{  static Arg nadger[] =*/
  static Arg nadger[] =
  {
    {XtNforeground},
    {XtNbackground},
    {XtNborderColor},
    {XtNinternalBorderColor},
  };
  /*}}}*/
  /*{{{  static Arg values[] =*/
  static Arg values[] =
  {
    {XtNchildren},
    {XtNnumChildren},
  };
  /*}}}*/
  STACK     stack[8];
  STACK     *sptr;
  
  nadger[0].value = (XtArgVal)display.black;
  nadger[1].value = (XtArgVal)display.white;
  nadger[2].value = (XtArgVal)display.border;
  nadger[3].value = (XtArgVal)display.border;
  /* Ha, work this one out then! */
  for(sptr = stack; 1; root = *(sptr->children++), sptr++)
    {
      if(!class || XtIsSubclass(root, class) != False)
	XtSetValues(root, nadger, XtNumber(nadger));
      if(XtIsComposite(root))
	{
	  values[0].value = (XtArgVal)&sptr->children;
	  values[1].value = (XtArgVal)&sptr->count;
	  sptr->count = 0;
	  XtGetValues(root, values, XtNumber(values));
	}
      else if(sptr != stack)
	sptr--;
      while(sptr != stack && !sptr->count)
	sptr--;
      if(!sptr->count--)
	break;
    }
  return;
}
/*}}}*/
