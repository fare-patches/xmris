/* Copyright (C) 1992 Nathan Sidwell */
/* RCS $Id: scram.c,v 4.7 1993/11/09 16:27:47 nathan stable $ */
/*{{{  about me*/
/* a simple program to compress the monochrome, mask & color sprites
 * together into one set of planes. This is done by color separation
 * overlay. Not particularly well written or documented, but then
 * its not supposed to be a general tool.
 * If anybody wants to make it more general, they are welcome
 * args
 * -n invert noswap
 * +n add edge to noswap
 * -s swap is ~noswap
 * +s swap is same as noswap
 * -m mask is solid (and therefore not present)
 * -c insert noswap as first color plane
 * -0 no monochrome
 * -[1-9] no monochrome, set color planes
 */
/*}}}*/
#include "ansiknr.h"
/*{{{  includes*/
#include <assert.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#ifdef TRANSPUTER
#include <iocntrl.h>
#else
#include <unistd.h>
#endif /* TRANSPUTER */
/*}}}*/
#define SUFFIXMONO  ".bw"
#define SUFFIXCOLOR ".rgb"
#define MAX_PLANES 5
#define MAX_COLORS (1 << MAX_PLANES)
/*{{{  typedef struct Coord*/
typedef struct Coord
/* general coordinate store */
{
  int       x;
  int       y;
} COORD;
/*}}}*/
/*{{{  typedef struct Size*/
typedef struct Size
/* general size store */
{
  unsigned  x;
  unsigned  y;
} SIZE;
/*}}}*/
/*{{{  typedef struct Bitmap*/
typedef struct Bitmap
{
  char      *bits;    /* bit pattern */
  SIZE      size;     /* size in pixels */
  COORD     hot;      /* hot spot */
  unsigned  scan;     /* scan line */
} BITMAP;
/*}}}*/
static BITMAP source;
static BITMAP current;
static BITMAP scratch;
static SIZE size;
static BITMAP color_bitmap;
static int color_planes = 0;
static unsigned long mono_noswap;
static unsigned long mono_swap;
static BITMAP bitmaps[MAX_COLORS];
static char *noswap_colors[MAX_COLORS];
static char *swap_colors[MAX_COLORS];
static int colors;
static char *filename;
static int error = 0;
#define GC_COPY 0
#define GC_AND  1
#define GC_XOR  2
#define GC_OR   3
#define GC_CLEAR 4 
/*{{{  void *smalloc(size)*/
static VOID *smalloc
FUNCARG((size),
	size_t    size
)
{
  VOID      *ptr;
  
  ptr = malloc(size);
  if(!ptr)
    {
      fprintf(stderr, "Malloc failed\n");
      exit(1);
    }
  return ptr;
}
/*}}}*/
/*{{{  unsigned read_bitmap(name, bitmap)*/
static unsigned read_bitmap
FUNCARG((name, bitmap),
	char CONST *name      /* name of file */
ARGSEP  BITMAP    *bitmap     /* bitmap ptr */
)
  /* state 0    new block
   * state 1    comment
   * state 2    {
   * state 3    reading
   * state 4    done
   */
{
  unsigned  error;
  FILE      *stream;
  unsigned  state;
  char      line[128];
  SIZE      place;
  unsigned  extra;
  
  stream = fopen(name, "r");
  if(!stream)
    return 1;
  error = 0;
  extra = 0;
  bitmap->size.x = bitmap->size.y = bitmap->scan = 0;
  bitmap->hot.x = bitmap->hot.y = -1;
  bitmap->bits = NULL;
  state = 0;
  while(fgets(line, sizeof(line), stream))
    {
      char      *ptr;
      
      ptr = line;
      while(*ptr == ' ' || *ptr == '\t')
	ptr++;
      if(*ptr == '\n')
	continue;
      /*{{{  new state?*/
      if(state == 0)
	{
	  if(ptr[0] == '/' && ptr[1] == '*')
	    {
	      ptr += 2;
	      state = 1;
	    }
	  else if(!strncmp(ptr, "#define", 7))
	    /*{{{  define*/
	    {
	      /* some poor saps don't have offsetof */
	      /*{{{  static char CONST *table[] =*/
	      static char CONST *table[] =
	      {
		"width", "height", "x_hot", "y_hot"
	      };
	      /*}}}*/
	      size_t    length;
	      unsigned  ix;
	      char CONST **tptr;
	     
	      ptr += 8;
	      while(*ptr == ' ' || *ptr == '\t')
		ptr++;
	      for(length = 0; ptr[length] != ' ' && ptr[length] != '\t' &&
		  ptr[length]; length++)
		/* EMPTY */;
	      for(tptr = table, ix = 4; ix--; tptr++)
		{
		  size_t    len;
		  
		  len = strlen(*tptr);
		  if(length >= len && !strncmp(&ptr[length - len], *tptr, len))
		    {
		      int     value;
		      
		      ptr += length;
		      while(*ptr == ' ' || *ptr == '\t')
			ptr++;
		      value = strtol(ptr, &ptr, 0);
		      switch(ix)
		      {
			case 3:
			  bitmap->size.x = value;
			  break;
			case 2:
			  bitmap->size.y = value;
			  break;
			case 1:
			  bitmap->hot.x = value;
			  break;
			case 0:
			  bitmap->hot.y = value;
			  break;
		      }
		      bitmap->scan = (bitmap->size.x + 7) / 8;
		      break;
		    }
		}
	    }
	    /*}}}*/
	  else
	    {
	      char      *bptr;
	      
	      bptr = strchr(ptr, '[');
	      if(bptr && bptr - ptr > 3 && !strncmp(bptr - 4, "bits", 4) &&
		  bptr[1] == ']')
		{
		  state = 2;
		  ptr = bptr;
		}
	    }
	}
      /*}}}*/
      /*{{{  in comment?*/
      if(state == 1)
	{
	  while(*ptr && state)
	    {
	      while(*ptr && *ptr != '*')
		ptr++;
	      if(*ptr)
		{
		  if(ptr[1] == '/')
		    {
		      ptr += 2;
		      state = 0;
		      break;
		    }
		  else
		    ptr++;
		}
	    }
	}
      /*}}}*/
      /*{{{  {?*/
      if(state == 2)
	{
	  ptr = strchr(ptr, '{');
	  if(ptr)
	    {
	      int       count;
	      
	      state = 3;
	      ptr++;
	      if(!bitmap->size.x || !bitmap->size.y)
		{
		  error = 1;
		  break;
		}
	      count = bitmap->scan * bitmap->size.y;
	      bitmap->bits = smalloc(count);
	      place.x = place.y = 0;
	      for(; count--;)
		bitmap->bits[count] = 0;
	    }
	}
      /*}}}*/
      /*{{{  reading?*/
      if(state == 3)
	{
	  while(state != 4 && *ptr && *ptr != '\n')
	    {
	      while(*ptr == ' ' || *ptr == '\t')
		ptr++;
	      if(*ptr == '}')
		state = 4;
	      else
		/*{{{  item*/
		{
		  unsigned  val;
		  
		  val = strtol(ptr, &ptr, 0);
		  if(*ptr == ',')
		    ptr++;
		  if(place.y != bitmap->size.y)
		    {
		      bitmap->bits[place.y * bitmap->scan + place.x] = val;
		      place.x++;
		      if(place.x == bitmap->scan)
			{
			  place.x = 0;
			  place.y++;
			}
		    }
		  else if(!extra)
		    {
		      fprintf(stderr, "Warning: Extra on end of bitmap '%s'\n",
			name);
		      extra = 1;
		    }
		}
		/*}}}*/
	    }
	}
      /*}}}*/
    }
  error |= ferror(stream) || !bitmap->bits;
  fclose(stream);
  return error;
}
/*}}}*/
/*{{{  void malloc_bitmap(bitmap, x, y)*/
static VOIDFUNC malloc_bitmap
FUNCARG((bitmap, x, y),
	BITMAP    *bitmap
ARGSEP  unsigned  x
ARGSEP  unsigned  y
)
{
  unsigned    ix;
  
  bitmap->size.x = x;
  bitmap->size.y = y;
  bitmap->hot.x = bitmap->hot.y = -1;
  bitmap->scan = (x + 7) / 8;
  ix = bitmap->scan * y;
  bitmap->bits = smalloc(ix);
  while(ix--)
    bitmap->bits[ix] = 0;
  return;
}
/*}}}*/
/*{{{  unsigned is_blank(bitmap)*/
static unsigned is_blank
FUNCARG((bitmap),
	BITMAP    *bitmap
)
{
  unsigned  ix;
  unsigned  bits;
  
  bits = 0;
  for(ix = bitmap->scan * bitmap->size.y; ix--;)
    bits |= bitmap->bits[ix] & 0xFF;
  return !bits;
}
/*}}}*/
/*{{{  void fill_area(bitmap, gc, x, y, w, h)*/
static VOIDFUNC fill_area
FUNCARG((bitmap, gc, x, y, w, h),
	BITMAP    *bitmap
ARGSEP  unsigned  gc
ARGSEP  int       x
ARGSEP  int       y
ARGSEP  unsigned  w
ARGSEP  unsigned  h
)
{
  unsigned  ix;
  unsigned  iy;
  int       count;
  
  assert(!(x & 7) && w && h);
  assert(x >= 0 && y >= 0 && x + w <= bitmap->size.x &&
      y + h <= bitmap->size.y);
  for(iy = 0; h--; iy++)
    {
      char    *bptr;
      
      bptr = &bitmap->bits[iy * bitmap->scan];
      for(ix = x / 8, count = w; count > 0; ix++, count -= 8)
	{
	  unsigned  mask;
	  
	  mask = count > 7 ? 0xFF : 0xFF >> (8 - count);
	  switch(gc)
	  {
	    case GC_COPY:
	      bptr[ix] |= mask;
	      break;
	    case GC_CLEAR:
	      bptr[ix] &= ~mask;
	      break;
	    case GC_XOR:
	      bptr[ix] ^= mask;
	      break;
	    case GC_OR:
	      bptr[ix] |= mask;
	      break;
	  }
	}
    }
  return;
}
/*}}}*/
/*{{{  void copy_area(source, dest, gc, sx, sy, w, h, dx, dy)*/
static VOIDFUNC copy_area
FUNCARG((source, dest, gc, sx, sy, w, h, dx, dy),
	BITMAP    *source
ARGSEP  BITMAP    *dest
ARGSEP  unsigned  gc
ARGSEP  int       sx
ARGSEP  int       sy
ARGSEP  unsigned  w
ARGSEP  unsigned  h
ARGSEP  int       dx
ARGSEP  int       dy
)
{
  unsigned  ix;
  unsigned  iy;
  char      *sptr;
  char      *dptr;
  char      line[16];
  int       count;
  
  assert(sx >= 0 && sy >= 0 && sx + w <= source->size.x &&
      sy + h <= source->size.y);
  assert(dx >= 0 && dy >= 0 && dx + w <= dest->size.x &&
      dy + h <= dest->size.y);
  assert(w < sizeof(line) * 8);
  for(iy = h; iy--;)
    {
      sptr = &source->bits[(sy + iy) * source->scan + sx / 8];
      /*{{{  set source pointer*/
      if(sx & 7)
	{
	  for(ix = 0, count = w; count > 0; ix++, count -= 8)
	    {
	      line[ix] = (sptr[ix] & 0xFF) >> (sx & 7);
	      if(ix)
		line[ix - 1] |= (sptr[ix] & 0xFF) << (8 - (sx & 7));
	      else
		count += (sx & 7);
	    }
	  sptr = line;
	}
      /*}}}*/
      dptr = &dest->bits[(dy + iy) * dest->scan + dx / 8];
      for(ix = 0, count = w; count > 0; ix++)
	{
	  unsigned  mask;
	  unsigned  shift;
	  
	  shift = dx & 7;
	  mask = (0xFF << shift) & 0xFF;
	  if(count < 8 - shift)
	    mask &= 0xFF >> ((8 - shift) - count);
	  /*{{{  first half*/
	  switch(gc)
	  {
	    case GC_COPY:
	      dptr[ix] &= ~mask;
	      dptr[ix] |= ((sptr[ix] & 0xFF) << shift) & mask;
	      break;
	    case GC_AND:
	      dptr[ix] &= (((sptr[ix] & 0xFF) << shift) & mask) | ~mask;
	      break;
	    case GC_OR:
	      dptr[ix] |= ((sptr[ix] & 0xFF) << shift) & mask;
	      break;
	    case GC_XOR:
	      dptr[ix] ^= ((sptr[ix] & 0xFF) << shift) & mask;
	      break;
	  }
	  /*}}}*/
	  count -= 8 - shift;
	  if(shift && count > 0)
	    {
	      mask = count >= 8 ? 0xFF : 0xFF >> (8 - count);
	      mask >>= 8 - shift;
	      /*{{{  second half*/
	      switch(gc)
	      {
		case GC_COPY:
		  dptr[ix + 1] &= ~mask;
		  dptr[ix + 1] |= ((sptr[ix] & 0xFF) >> (8 - shift)) & mask;
		  break;
		case GC_AND:
		  dptr[ix + 1] &= (((sptr[ix] & 0xFF) >> (8 - shift)) &
		      mask) | ~mask;
		  break;
		case GC_OR:
		  dptr[ix + 1] |= ((sptr[ix] & 0xFF) >> (8 - shift)) & mask;
		  break;
		case GC_XOR:
		  dptr[ix + 1] ^= ((sptr[ix] & 0xFF) >> (8 - shift)) & mask;
		  break;
	      }
	      /*}}}*/
	      count -= shift;
	    }
	}
    }
  return;
}
/*}}}*/
/*{{{  void draw_rectangle(bitmap, gc, x, y, w, h)*/
static VOIDFUNC draw_rectangle
FUNCARG((bitmap, gc, x, y, w, h),
	BITMAP    *bitmap
ARGSEP  unsigned  gc
ARGSEP  int       x
ARGSEP  int       y
ARGSEP  unsigned  w
ARGSEP  unsigned  h
)
{
  unsigned  ix;
  unsigned  iy;
  int       count;
  char      *bptr;
  unsigned  mask;
  
  assert(w & h && !(x & 7));
  assert(x >= 0 && y >= 0 && x + w < bitmap->size.x && y + h < bitmap->size.y);
  bptr = &bitmap->bits[y * bitmap->scan];
  /*{{{  top and bottom edges*/
  for(ix = x / 8, count = w + 1; count > 0; ix++, count -= 8)
    {
      mask = count > 7 ? 0xFF : 0xFF >> (8 - count);
      /*{{{  edge*/
      switch(gc)
      {
	case GC_COPY:
	case GC_OR:
	  bptr[ix] |= mask;
	  bptr[ix + h * bitmap->scan] |= mask;
	  break;
	case GC_CLEAR:
	  bptr[ix] &= ~mask;
	  bptr[ix + h * bitmap->scan] &= ~mask;
	  break;
	case GC_XOR:
	  bptr[ix] ^= mask;
	  bptr[ix + h * bitmap->scan] ^= mask;
	  break;
	case GC_AND:
	  break;
      }
      /*}}}*/
    }
  /*}}}*/
  /*{{{  left and right edges*/
  for(iy = y + 1, count = h - 2; count > 0; iy++, count--)
    {
      bptr = &bitmap->bits[iy * bitmap->scan + x / 8];
      mask = 1 << (x & 7);
      /*{{{  left edge*/
      switch(gc)
      {
	case GC_OR:
	case GC_COPY:
	  *bptr |= mask;
	  break;
	case GC_XOR:
	  *bptr ^= mask;
	  break;
	case GC_AND:
	  break;
	case GC_CLEAR:
	  *bptr &= ~mask;
	  break;
      }
      /*}}}*/
      bptr = &bitmap->bits[iy * bitmap->scan + (x + w) / 8];
      mask = 1 << ((x + w) & 7);
      /*{{{  right edge*/
      switch(gc)
      {
	case GC_OR:
	case GC_COPY:
	  *bptr |= mask;
	  break;
	case GC_XOR:
	  *bptr ^= mask;
	  break;
	case GC_AND:
	  break;
	case GC_CLEAR:
	  *bptr &= ~mask;
	  break;
      }
      /*}}}*/
    }
  /*}}}*/
  return;
}
/*}}}*/
/*{{{  void make_color_pixmaps(char **name, BITMAP *bitmaps, char CONST *title)*/
static VOIDFUNC make_color_pixmaps
FUNCARG((name, bitmaps, title),
	char      **name
ARGSEP  BITMAP    *bitmaps
ARGSEP  char CONST *title
)
{
  unsigned ix;
  
  for(ix = 0; name[ix]; ix++)
    if(ix >= (1 << color_planes))
      fill_area(&bitmaps[ix], GC_CLEAR, 0, 0, size.x, size.y);
    else
      {
	unsigned bit;
	
	copy_area(&source, &bitmaps[ix], GC_COPY, 0, 0, size.x, size.y, 0, 0);
	for(bit = 0; bit != color_planes; bit++)
	  if((1 << bit) & ix)
	    {
	      copy_area(&color_bitmap, &bitmaps[ix], GC_AND,
		0, (int)(size.y * bit), size.x, size.y, 0, 0);
	    }
	  else
	    {
	      copy_area(&color_bitmap, &current, GC_COPY,
		0, (int)(size.y * bit), size.x, size.y, 0, 0);
	      fill_area(&current, GC_XOR, 0, 0, size.x, size.y);
	      copy_area(&current, &bitmaps[ix], GC_AND,
		  0, 0, size.x, size.y, 0, 0);
	    }
      }
  copy_area(&source, &current, GC_COPY, 0, 0, size.x, size.y, 0, 0);
  for(ix = 0; name[ix]; ix++)
    copy_area(&bitmaps[ix], &current, GC_XOR, 0, 0, size.x, size.y, 0, 0);
  if(!is_blank(&current))
    {
      unsigned  x, y;
      
      error = 1;
      fprintf(stderr, "%s: Error on %s at", filename, title);
      for(y = 0; y != size.y; y++)
	for(x = 0; x != size.x; x++)
	  if(current.bits[y * current.scan + x / 8] & 1 << (x & 7))
	    fprintf(stderr, " (%d, %d)", x, y);
      fprintf(stderr, "\n");
    }
  return;
}
/*}}}*/
/*{{{  void add_edge(unsigned ix, BITMAP *bitmaps)*/
static VOIDFUNC add_edge
FUNCARG((ix, bitmaps),
	unsigned ix
ARGSEP  BITMAP *bitmaps
)
{
  copy_area(&source, &current, GC_COPY, 0, 0, size.x, size.y, 0, 0);
  copy_area(&source, &current, GC_AND, 0, 0, size.x, size.y - 1, 0, 1);
  copy_area(&source, &current, GC_AND, 0, 1, size.x, size.y - 1, 0, 0);
  copy_area(&source, &current, GC_AND, 0, 0, size.x - 1, size.y, 1, 0);
  copy_area(&source, &current, GC_AND, 1, 0, size.x - 1, size.y, 0, 0);
  draw_rectangle(&current, GC_CLEAR, 0, 0, size.x - 1, size.y - 1);
  copy_area(&source, &current, GC_XOR, 0, 0, size.x, size.y, 0, 0);
  copy_area(&current, &bitmaps[ix], GC_COPY, 0, 0, size.x, size.y, 0, 0);
  fill_area(&current, GC_XOR, 0, 0, size.x, size.y);
  while(ix--)
    copy_area(&current, &bitmaps[ix], GC_AND, 0, 0, size.x, size.y, 0, 0);
  return;
}
/*}}}*/
/*{{{  void compress_colors(char **name, BITMAP *bitmaps, char CONST *title)*/
static VOIDFUNC compress_colors
FUNCARG((name, bitmaps, title),
	char **name
ARGSEP  BITMAP *bitmaps
ARGSEP  char CONST *title
)
{
  unsigned  ix, j;
  
  for(ix = 0; name[ix + 1]; ix++)
    {
      for(j = ix + 1; name[j]; j++)
	if(!strcmp(name[ix], name[j]))
	  {
	    copy_area(&bitmaps[j], &bitmaps[ix], GC_OR,
		0, 0, size.x, size.y, 0, 0);
	    fill_area(&bitmaps[j], GC_CLEAR, 0, 0, size.x, size.y);
	    name[j] = (char *)"-";
	  }
    }
  for(ix = 0; name[ix]; ix++)
    {
      if(is_blank(&bitmaps[ix]))
	{
	  BITMAP old;
	  
	  if(strcmp(name[ix], "-"))
	    fprintf(stderr, "Warning: %s color '%s' has no pixels\n",
		title, name[ix]);
	  memcpy(&old, &bitmaps[ix], sizeof(BITMAP));
	  for(j = ix; name[j + 1]; j++)
	    {
	      name[j] = name[j+1];
	      memcpy(&bitmaps[j], &bitmaps[j+1], sizeof(BITMAP));
	    }
	  name[j] = NULL;
	  memcpy(&bitmaps[j], &old, sizeof(BITMAP));
	  ix--;
	}
      else if(!strcmp(name[ix], "-"))
      {
	unsigned  x, y;
	
	error = 1;
	fprintf(stderr, "%s: Void color on %s at", filename, title);
	for(y = 0; y != size.y; y++)
	  for(x = 0; x != size.x; x++)
	    if(bitmaps[ix].bits[y * bitmaps[ix].scan + x / 8] & (1 << x & 7))
	      fprintf(stderr, " (%d, %d)", x, y);
	fprintf(stderr, "\n");
      }
    }
  return;
}
/*}}}*/
/*{{{  void merge_mono()*/
static VOIDFUNC merge_mono FUNCARGVOID
{
  unsigned ix;

  for(ix = 1; ix != colors; ix++)
    {
      if(is_blank(&source))
	break;
      copy_area(&source, &current, GC_COPY, 0, 0, size.x, size.y, 0, 0);
      copy_area(&bitmaps[ix], &current, GC_AND, 0, 0, size.x, size.y, 0, 0);
      if(!is_blank(&current))
	{
	  mono_swap |= (unsigned long)1 << ix;
	  copy_area(&current, &scratch, GC_COPY,
	      0, 0, size.x, size.y, 0, 0);
	  copy_area(&bitmaps[ix], &scratch, GC_XOR,
	      0, 0, size.x, size.y, 0, 0);
	  if(!is_blank(&scratch))
	    {
	      copy_area(&bitmaps[ix], &bitmaps[colors], GC_COPY,
		  0, 0, size.x, size.y, 0, 0);
	      copy_area(&current, &bitmaps[colors], GC_XOR,
		  0, 0, size.x, size.y, 0, 0);
	      noswap_colors[colors] = noswap_colors[ix];
	      swap_colors[colors] = swap_colors[ix];
	      mono_noswap = mono_noswap |
		  (((mono_noswap >> ix) & 1) << colors);
	      copy_area(&current, &bitmaps[ix], GC_COPY,
		  0, 0, size.x, size.y, 0, 0);
	      colors++;
	    }
	  copy_area(&bitmaps[ix], &source, GC_XOR,
	      0, 0, size.x, size.y, 0, 0);
	}
    }
  return;
}
/*}}}*/
/*{{{  int main(int argc, char **argv)*/
int main
FUNCARG((argc, argv),
	int argc
ARGSEP  char **argv
)
{
  BITMAP    mono_bitmap;
  BITMAP    mono_source;
  int       planes;
  char      *edge = NULL;
  char      *noswap[MAX_COLORS + 1];
  char      *swap[MAX_COLORS + 1];
  BITMAP    noswap_bitmaps[MAX_COLORS];
  BITMAP    swap_bitmaps[MAX_COLORS];
  int       status;
  unsigned  ix;
  BITMAP    output_bitmap;
  unsigned  noswap_edge = 0;
  unsigned  noswap_invert = 0;
  unsigned  swap_invert = 0;
  unsigned  swap_copy = 0;
  unsigned  no_mono = 0;
  unsigned  mono_planes;
  unsigned  color_copy = 0;
  unsigned  solid_mask = 0;
  
  /*{{{  slurp up the arguments*/
  {
    filename = argv[1];
    if(argc > 2)
      {
	argv += 2;
	argc -= 2;
	for(; argc; argc--, argv++)
	  {
	    if(argv[0][0] == '+')
	      {
		if(argv[0][2])
		  error = 1;
		else if(argv[0][1] == 's')
		  swap_copy = 1;
		else if(argv[0][1] == 'n')
		  noswap_edge = 1;
		else
		  error = 1;
	      }
	    else if(argv[0][0] == '-')
	      {
		if(!argv[0][1])
		  break;
		else if(argv[0][2])
		  error = 1;
		else if(argv[0][1] == 's')
		  swap_invert = 1;
		else if(argv[0][1] == 'n')
		  noswap_invert = 1;
		else if(argv[0][1] == 'c')
		  color_copy = 1;
		else if(argv[0][1] == 'm')
		  solid_mask = 1;
		else if(argv[0][1] == '0')
		  no_mono = 1;
		else if(isdigit(argv[0][1]))
		  {
		    no_mono = 1;
		    color_planes = argv[0][1] - '0';
		  }
		else
		  error = 1;
	      }
	    else
	      break;
	    if(error)
	      {
		fprintf(stderr, "'%s' not a flag\n", argv[0]);
		argc = 0;
		break;
	      }
	  }
      }
    for(ix = 0; argc && strcmp(*argv, "+"); argc--, argv++)
      noswap[ix++] = *argv;
    if(argc)
      argv++, argc--;
    while(ix != MAX_COLORS + 1)
      noswap[ix++] = NULL;
    for(ix = 0; argc && strcmp(*argv, "+"); argc--, argv++)
      swap[ix++] = *argv;
    if(argc)
      argv++, argc--;
    while(ix != MAX_COLORS + 1)
      swap[ix++] = NULL;
    if(argc)
      {
	edge = *argv;
	argv++, argc--;
      }
    if(argc || !filename || (no_mono &&
	(noswap_edge || noswap_invert || swap_invert || swap_copy)))
      {
	fprintf(stderr,
	  "Usage: scram <name> [-<n>|[+n][+s][-n][-s]] <noswap> + <swap> + [<edge>]\n");
	return 1;
      }
  }
  /*}}}*/
  if(no_mono)
    mono_planes = 1;
  else
    {
      mono_planes = noswap_edge | noswap_invert | swap_copy | swap_invert ?
	  2 : 3;
      if(solid_mask)
	mono_planes--;
    }
  /*{{{  read files*/
  {
    char *name;
    
    name = smalloc(strlen(filename) + 7);
    strcpy(name, filename);
    strcat(name, SUFFIXMONO);
    status = read_bitmap(name, &mono_source);
    size.x = mono_source.size.x;
    size.y = mono_source.size.y;
    if(status || size.y % mono_planes)
      {
	fprintf(stderr, "Cannot read monochrome '%s'\n", name);
	return 1;
      }
    size.y /= mono_planes;
    strcpy(name, filename);
    strcat(name, SUFFIXCOLOR);
    status = read_bitmap(name, &color_bitmap);
    if(status)
      {
	fprintf(stderr, "Cannot read colour '%s'\n", name);
	return 1;
      }
    if(color_planes)
      size.y = color_bitmap.size.y / color_planes;
    else
      color_planes = color_bitmap.size.y / size.y;
    if(color_bitmap.size.y % size.y || color_bitmap.size.x != size.x)
      {
	fprintf(stderr, "Bitmaps are incompatible sizes\n");
	return 1;
      }
    free(name);
  }
  /*}}}*/
  /*{{{  alloc some bitmaps*/
  {
    malloc_bitmap(&source, size.x, size.y);
    malloc_bitmap(&current, size.x, size.y);
    malloc_bitmap(&scratch, size.x, size.y);
    malloc_bitmap(&mono_bitmap, size.x, size.y * 2);
  }
  /*}}}*/
  /*{{{  solid mask?*/
  if(solid_mask)
    {
      BITMAP  temp;
      
      if(!no_mono)
	mono_planes++;
      malloc_bitmap(&temp, size.x, size.y * mono_planes);
      copy_area(&mono_source, &temp, GC_COPY, 0, 0,
	  size.x, size.y * (mono_planes - 1), 0, size.y);
      fill_area(&temp, GC_COPY, 0, 0, size.x, size.y);
      memcpy(&temp.hot, &mono_source.hot, sizeof(COORD));
      memcpy(&mono_source, &temp, sizeof(BITMAP));
    }
  /*}}}*/
  /*{{{  copy color?*/
  if(color_copy)
    {
      BITMAP temp;
      
      color_planes++;
      malloc_bitmap(&temp, size.x, size.y * color_planes);
      copy_area(&color_bitmap, &temp, GC_COPY, 0, 0,
	  size.x, size.y * (color_planes - 1), 0, size.y);
      copy_area(&mono_source, &temp, GC_COPY,
	  0, size.y, size.x, size.y, 0, 0);
      memcpy(&color_bitmap, &temp, sizeof(BITMAP));
    }
  /*}}}*/
  /*{{{  alloc a pile of bitmaps*/
  {
    for(ix = 0; ix < MAX_COLORS; ix++)
      {
	malloc_bitmap(&bitmaps[ix], size.x, size.y);
	fill_area(&bitmaps[ix], GC_CLEAR, 0, 0, size.x, size.y);
      }
    for(ix = 0; ix < MAX_COLORS; ix++)
      {
	malloc_bitmap(&noswap_bitmaps[ix], size.x, size.y);
	fill_area(&noswap_bitmaps[ix], GC_CLEAR, 0, 0, size.x, size.y);
      }
    for(ix = 0; ix < MAX_COLORS; ix++)
      {
	malloc_bitmap(&swap_bitmaps[ix], size.x, size.y);
	fill_area(&swap_bitmaps[ix], GC_CLEAR, 0, 0, size.x, size.y);
      }
  }
  /*}}}*/
  colors = !no_mono;
  if(!no_mono)
    {
      /*{{{  fiddle mono noswap*/
      {
	copy_area(&mono_source, &source, GC_COPY,
	    0, (int)size.y, size.x, size.y, 0, 0);
	if(noswap_invert)
	  fill_area(&source, GC_XOR, 0, 0, size.x, size.y);
	if(noswap_edge)
	  {
	    copy_area(&mono_source, &current, GC_COPY, 0, 0,
		size.x, size.y, 0, 0);
	    copy_area(&mono_source, &current, GC_AND, 0, 0,
		size.x, size.y - 1, 0, 1);
	    copy_area(&mono_source, &current, GC_AND, 0, 1,
		size.x, size.y - 1, 0, 0);
	    copy_area(&mono_source, &current, GC_AND, 0, 0,
		size.x - 1, size.y, 1, 0);
	    copy_area(&mono_source, &current, GC_AND, 1, 0,
		size.x - 1, size.y, 0, 0);
	    draw_rectangle(&current, GC_CLEAR, 0, 0, size.x - 1, size.y - 1);
	    fill_area(&current, GC_XOR, 0, 0, size.x, size.y);
	    copy_area(&current, &source, GC_OR, 0, 0, size.x, size.y, 0, 0);
	  }
	copy_area(&mono_source, &source, GC_AND, 0, 0, size.x, size.y, 0, 0);
	copy_area(&source, &mono_bitmap, GC_COPY,
	    0, 0, size.x, size.y, 0, 0);
      }
      /*}}}*/
      /*{{{  fiddle mono swap*/
      {
	copy_area(&mono_source, &source, GC_COPY,
	    0, (int)(size.y * (mono_planes - 1)), size.x, size.y, 0, 0);
	if(swap_invert)
	  fill_area(&source, GC_XOR, 0, 0, size.x, size.y);
	copy_area(&mono_source, &source, GC_AND,
	    0, 0, size.x, size.y, 0, 0);
	copy_area(&source, &mono_bitmap, GC_COPY,
	    0, 0, size.x, size.y, 0, (int)size.y);
      }
      /*}}}*/
    }
  copy_area(&mono_source, &source, GC_COPY, 0, 0, size.x, size.y, 0, 0);
  make_color_pixmaps(noswap, noswap_bitmaps, "Noswap");
  if(edge)
    {
      for(ix = 0; noswap[ix]; ix++)
	/*EMPTY*/;
      add_edge(ix, noswap_bitmaps);
      noswap[ix] = edge;
    }
  copy_area(&mono_source, &source, GC_COPY, 0, 0, size.x, size.y, 0, 0);
  make_color_pixmaps(swap, swap_bitmaps, "Swap");
  compress_colors(noswap, noswap_bitmaps, "Noswap");
  compress_colors(swap, swap_bitmaps, "Swap");
  /*{{{  merge the noswap colors*/
  {
    for(ix = 0; noswap[ix]; ix++)
      {
	copy_area(&noswap_bitmaps[ix], &bitmaps[ix + colors], GC_COPY,
	  0, 0, size.x, size.y, 0, 0);
	noswap_colors[ix + colors] = noswap[ix];
      }
    colors = ix + colors;
  }
  /*}}}*/
  /*{{{  merge the swap colors*/
  for(ix = 0; swap[ix]; ix++)
    {
      unsigned j;
      
      for(j = 0; j != colors; j++)
	{
	  if(is_blank(&swap_bitmaps[ix]))
	    break;
	  copy_area(&swap_bitmaps[ix], &current, GC_COPY,
	      0, 0, size.x, size.y, 0, 0);
	  copy_area(&bitmaps[j], &current, GC_AND,
	      0, 0, size.x, size.y, 0, 0);
	  if(!is_blank(&current))
	    {
	      swap_colors[j] = swap[ix];
	      copy_area(&current, &scratch, GC_COPY,
		  0, 0, size.x, size.y, 0, 0);
	      copy_area(&bitmaps[j], &scratch, GC_XOR,
		  0, 0, size.x, size.y, 0, 0);
	      if(!is_blank(&scratch))
		{
		  copy_area(&bitmaps[j], &bitmaps[colors], GC_COPY,
		      0, 0, size.x, size.y, 0, 0);
		  copy_area(&current, &bitmaps[colors], GC_XOR,
		      0, 0, size.x, size.y, 0, 0);
		  noswap_colors[colors] = noswap_colors[j];
		  copy_area(&current, &bitmaps[j], GC_COPY,
		      0, 0, size.x, size.y, 0, 0);
		  colors++;
		}
	      copy_area(&bitmaps[j], &swap_bitmaps[ix], GC_XOR,
		  0, 0, size.x, size.y, 0, 0);
	    }
	}
    }
  /*}}}*/
  if(!no_mono)
    {
      /*{{{  merge the noswap mono*/
      {
	copy_area(&mono_bitmap, &source, GC_COPY, 0, 0, size.x, size.y, 0, 0);
	merge_mono();
	mono_noswap = mono_swap;
	mono_swap = 0;
      }
      /*}}}*/
      /*{{{  merge the swap mono*/
      {
	copy_area(&mono_bitmap, &source, GC_COPY,
	    0, (int)size.y, size.x, size.y, 0, 0);
	merge_mono();
      }
      /*}}}*/
      /*{{{  add mono color masks*/
      {
	char st[20];
	
	sprintf(st, "0x%lx", mono_noswap);
	noswap_colors[0] = strcpy(smalloc(strlen(st) + 1), st);
	sprintf(st, "0x%lx", mono_swap);
	swap_colors[0] = strcpy(smalloc(strlen(st) + 1), st);
      }
      /*}}}*/
    }
  /*{{{  create the output bitmap*/
  {
    unsigned mask;
    
    for(mask = 1, planes = 0; mask < colors; mask <<= 1)
      planes++;
    fprintf(stderr, "Creating %s.%s with %d colors on %d planes\n", filename,
	no_mono ? "packed" : "h", colors, planes);
    malloc_bitmap(&output_bitmap, size.x, size.y * planes);
    fill_area(&output_bitmap, GC_CLEAR, 0, 0, size.x, size.y * planes);
    for(ix = 0; ix != colors; ix++)
      {
	int bit;
	
	for(bit = 0; bit != planes; bit++)
	  if(ix & (1 << bit))
	    copy_area(&bitmaps[ix], &output_bitmap, GC_OR,
		0, 0, size.x, size.y, 0, (int)(bit * size.y));
      }
  }
  /*}}}*/
  /*{{{  write the file*/
  {
    char *name;
    FILE  *stream;
    char *ptr;
    unsigned  x, y;
    unsigned  count;
    unsigned  ix;
    
    name = smalloc(strlen(filename) + 8);
    strcpy(name, filename);
    strcat(name, no_mono ? ".packed" : ".h");
    ptr = strrchr(filename, '/');
    if(!ptr)
      ptr = filename;
    else
      ptr++;
    stream = fopen(name, "w");
    if(!stream)
      {
	fprintf(stderr, "Cannot write file\n");
	return 1;
      }
    fprintf(stream, "#define %s_width %d\n", ptr, (int)size.x);
    fprintf(stream, "#define %s_height %d\n", ptr, (int)(size.y * planes));
    if(mono_source.hot.x >= 0)
      {
	fprintf(stream, "#define %s_x_hot %d\n", ptr, mono_source.hot.x);
	fprintf(stream, "#define %s_y_hot %d\n", ptr, mono_source.hot.y);
      }
    if(!no_mono)
      fprintf(stream, "#define %s_depth %d\n", ptr, (int)planes);
    fprintf(stream, "static unsigned char %s_bits[] =\n{\n ", ptr);
    count = 10;
    for(y = 0; y < output_bitmap.size.y; y++)
      {
	for(x = 0; x < (output_bitmap.size.x + 7) / 8; x++)
	  {
	    if(!count--)
	      {
		count = 9;
		fputs("\n ", stream);
	      }
	    fprintf(stream, " 0x%02x,", ((size.x - x * 8) > 7 ?
		0xFF : 0xFF >> (8 - (size.x - x * 8))) &
		output_bitmap.bits[y * output_bitmap.scan + x]);
	  }
      }
    fprintf(stream, "\n};\n");
    if(no_mono)
      {
	fputs("  ", stdout);
	for(ix = 0; ix != colors; ix++)
	  fprintf(stdout, "%s ", noswap_colors[ix]);
	fputs("+ \\\n", stdout);
	fputs("  ", stdout);
	for(ix = 0; ix != colors; ix++)
	  fprintf(stdout, "%s ", swap_colors[ix]);
	fputs("+\n", stdout);
      }
    else
      {
	fprintf(stream, "static unsigned long %s_noswap[] =\n{\n", ptr);
	for(ix = 0; ix != colors; ix++)
	  fprintf(stream, "  %s,\n", noswap_colors[ix]);
	while(ix++ < 1 << planes)
	  fprintf(stream, "  ~(unsigned long)0,\n");
	fprintf(stream, "};\n");
	fprintf(stream, "static unsigned long %s_swap[] =\n{\n", ptr);
	for(ix = 0; ix != colors; ix++)
	  fprintf(stream, "  %s,\n", swap_colors[ix]);
	while(ix++ < 1 << planes)
	  fprintf(stream, "  ~(unsigned long)0,\n");
	fprintf(stream, "};\n");
      }
    if(ferror(stream))
      {
	fprintf(stderr, "Error writing file");
	error = 1;
      }
    fclose(stream);
    if(error)
      unlink(name);
    free(name);
  }
  /*}}}*/
  return error;
}
/*}}}*/
