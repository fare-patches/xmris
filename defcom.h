/* Copyright (C) 1993, 1992 Nathan Sidwell */
/* this file is common to xmris and xmred, included from def*.h */
/* RCS $Id: defcom.h 1.2 Tue, 16 Mar 1999 11:28:16 +0000 nathan $ */
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#define XtRGender       "Gender"
#define COLOR_PLANES 3  /* max color planes for sprite generation */
/*{{{  some macros*/
#define RGB2X(r, g, b)  (int)(((long)(b) - (long)(r)) * (56756 / 2) / 65536)
#define RGB2Y(r, g, b)  (int)(((long)(g) - ((long)(r) + (long)(b)) / 2) / 2)
#define RGB2H(r, g, b)  (int)(((long)(g) * 4 + (long)(r) * 3 + (long)(b)) / 8)
/*}}}*/
/*{{{  structs*/
/*{{{  typedef struct Help*/
typedef struct Help
/* command line argument specifier */
{
  char CONST *help;     /* help message */
  char CONST *arg;      /* arg text */
  char CONST *resource; /* resource name */
  char CONST *option;   /* option type */
} HELP;
/*}}}*/
/*{{{  typedef struct Context*/
typedef struct Context
/* GC definer */
{
  int       function;         /* graphics function */
  unsigned long CONST *fgp;   /* foreground color pointer */
  unsigned long CONST *bgp;   /* background color pointer */
} CONTEXT;
/*}}}*/
/*{{{  typedef struct Sprite_Def*/
typedef struct Sprite_Def
/* sprite definition */
{
  unsigned char *bitmap;  /* image bitmap */
  unsigned  planes;       /* number of planes in bitmap */
  unsigned long *colors[2];    /* colors arrays */
  SIZE      size;         /* size of sprite */
  SIZE      expected;     /* expected size */
  unsigned  copy;         /* generated from this sprite */
  unsigned  flags;        /* reflection mode, or border bits */
} SPRITE_DEF;
#define REFLECT_ALIAS       0 /* alias */
#define REFLECT_VERTICAL    1 /* vertical axis reflect */
#define REFLECT_HORIZONTAL  2 /* horizontal axis reflect */
#define REFLECT_DIAGONAL    3 /* 45' axis reflect */
#define BORDER_HALO_MASK          1  /* halo */
#define BORDER_MONOSWAP_EDGE_MASK 2  /* -mono -swap outer white */
#define BORDER_WHITE_EDGE_MASK    4  /* color outer white */
/*}}}*/
/*{{{  typedef struct Color_Def*/
typedef struct Color_Def
{
  char CONST *name;       /* my name for the color */
  unsigned  type;         /* type of color
			   * bit0-3, mris, mris.swap, msit, msit.swap
			   * bit4 - background
			   * bit5 - dynamic
			   */
  char CONST *source[4];  /* possible names for the color */
  unsigned alloc;         /* 
			   * 0 - unallocated
			   * 1 - allocated
			   * 2 - copy
			   * 3 - missed
			   * 4 - unrequired
			   * 5 - info
			   */
  int       coord[3];     /* position in colorspace */
  struct Color_Def  *nearest;    /* nearest allocated color */
  unsigned long distance; /* distance to nearest */
} COLOR_DEF;
/*}}}*/
/*{{{  typedef struct Visual_Class*/
typedef struct Visual_Class
{
  char CONST *name;       /* class name */
  int       class;        /* visual class */
} VISUAL_CLASS;
/*}}}*/
/*}}}*/
extern unsigned long color_zero;
extern unsigned long color_one;
extern Boolean  default_gender;
/*{{{  create tables*/
extern CommandOptionDesc command_options[];
extern XrmOptionDescRec options[];
extern XtResource resources[];
extern COLOR_DEF color_names[];
extern CONTEXT gcsdefine[GCS];
extern HELP CONST help[];
extern SPRITE_DEF CONST she_nadger[];
extern SPRITE_DEF CONST icons[];
extern SPRITE_DEF sprites_def[SPRITES];
extern SPRITE_DEF fills_def[FILLS];
extern VISUAL_CLASS CONST visual_class[];
/*}}}*/
