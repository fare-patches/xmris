/* Copyright (C) 1993 Nathan Sidwell */
/* RCS $Id: xmred.h,v 4.27 1994/01/06 10:20:24 nathan stable $ */
#define XMRED 1
#define BUTTONS         5
#include "common.h"
#define COPYRIGHT "Copyright (C) 1994,1993 Nathan Sidwell"
/*{{{  defines*/
#define UNDO_DEPTH  8
#define ICON_WIDTH  (CELLS_ACROSS * 3 - 1)
#define ICON_HEIGHT ((CELLS_DOWN + 1) * 3 - 1)
/*{{{  descriptor numbers*/
#define DESCRIPTOR_NONE     -1
#define DESCRIPTOR_GARDEN   0
#define DESCRIPTOR_INCLUDE  1
#define DESCRIPTOR_COMMENT  2
/*}}}*/
#define SOURCE_UNIQUE   -2
#define SOURCE_BUFFER   -1
/*{{{  dialog numbers*/
#define DIALOG_FILENAME     0
#define DIALOG_NOT_SAVED    1
#define DIALOG_FILE_ERROR   2
#define DIALOG_LOAD_ERROR   3
#define DIALOG_COMMENT      4
#define DIALOG_INCLUDE      5
#define DIALOG_FAILED       6
#define DIALOGS             7
/*}}}*/
/*{{{  dialog options*/
#define DIALOG_NONE      0
#define DIALOG_OK        (1 << 0)
#define DIALOG_RETRY     (1 << 1)
#define DIALOG_CONTINUE  (1 << 2)
#define DIALOG_DEFAULT   (1 << 3)
#define DIALOG_CANCEL    (1 << 4)
#define DIALOG_ABORT     (1 << 5)
#define DIALOG_CLEAR     (1 << 6)
#define DIALOG_AGREE    (DIALOG_OK | DIALOG_RETRY | DIALOG_CONTINUE)
#define DIALOG_DISAGREE (DIALOG_CANCEL | DIALOG_ABORT)
/*}}}*/
/*{{{  changed flags*/
#define CHANGED_GARDEN  (1 << DESCRIPTOR_GARDEN)
#define CHANGED_INCLUDE (1 << DESCRIPTOR_INCLUDE)
#define CHANGED_COMMENT (1 << DESCRIPTOR_COMMENT)
#define CHANGED_ALL     (1 << 3)
#define CHANGED_BUFFER  (1 << 4)
#define CHANGED_ANY     ((1 << 5) - 1)
/*}}}*/
/*{{{  counts*/
#define COUNT_APPLES    0
#define COUNT_CHERRY    4
#define COUNT_RANDOM    5
#define COUNT_SPACES    6
#define COUNT_FALL      7
#define COUNT_PLAYER    8
#define COUNT_DEN       9
#define COUNTS          10
/*}}}*/
/*{{{  modes*/
#define MODE_COMBINED 0
#define MODE_SEPARATE 1
#define MODE_RANDOM   2
#define MODES         3
/*}}}*/
/*{{{  options*/
#define OPTION_APPLES   0
#define OPTION_RANDOM   1
#define OPTION_CHERRY   2
#define OPTION_PATH     3
#define OPTION_PLAYER   4
#define OPTION_DEN      5
#define OPTIONS         6
/*}}}*/
/*}}}*/
/*{{{  structs*/
/*{{{  typedef struct Descriptor*/
typedef struct Descriptor
{
  Pixmap  pixmap;   /* display pixmap */
  int     type;     /* type of descriptor */
  char    *comment;
  BOARD   *board;
} DESCRIPTOR;
/*}}}*/
/*{{{  typedef struct Gizmo*/
typedef struct Gizmo
{
  char  CONST *name;    /* widget name */
  int       parent;     /* parent's gizmo number */
  WidgetClass CONST *class; /* class of widget */
  Arg       *args;      /* creation arguments */
  Cardinal  num_args;   /* number of arguments */
  Widget    widget;     /* the widget created */
  unsigned  align[2];   /* its attachments */
} GIZMO;
/*}}}*/
/*}}}*/
/*{{{  state*/
EXTERN struct
{
  int       button[BUTTONS];  /* button options */
  int       apple;            /* apple */
  int       color;            /* color */
  int       fill;             /* fill */
  DESCRIPTOR *edit;           /* board being edited */
  int       source;           /* source of garden */
  unsigned  change;           /* change mask */
  unsigned  mode;             /* display mode */
  unsigned  counts[COUNTS];   /* apple and cherry counts */
} state;
/*}}}*/
EXTERN unsigned changed_flag;
extern BOARD initial_board[1];
/*{{{  garden widget*/
extern WidgetClass gardenWidgetClass;
typedef struct _GardenClassRec *GardenWidgetClass;
typedef struct _GardenRec *GardenWidget;
/*}}}*/
/*{{{  prototypes*/
/*{{{  all*/
extern VOIDFUNC all_garden_comment PROTOARG((DESCRIPTOR *, unsigned));
extern unsigned check_saved PROTOARG((unsigned));
extern unsigned check_exists PROTOARG((char CONST *));
extern VOIDFUNC free_descriptors PROTOARG((VOIDARG));
extern VOIDFUNC free_dup PROTOARG((char **, char CONST *));
extern VOIDFUNC install_all PROTOARG((Widget));
extern char CONST *load_boards PROTOARG((char CONST *, unsigned));
extern VOIDFUNC new_descriptors PROTOARG((VOIDARG));
extern VOIDFUNC repaint_garden_icon PROTOARG((VOIDARG));
extern char CONST *save_boards PROTOARG((char CONST *));
extern DESCRIPTOR *unique_garden PROTOARG((VOIDARG));
/*}}}*/
/*{{{  control*/
extern VOIDFUNC adjust_count PROTOARG((unsigned, int));
extern VOIDFUNC install_control PROTOARG((Widget));
extern VOIDFUNC set_garden PROTOARG((DESCRIPTOR *, int, unsigned));
extern VOIDFUNC set_garden_source PROTOARG((int, unsigned));
extern VOIDFUNC update_garden PROTOARG((VOIDARG));
/*}}}*/
/*{{{  dialogs*/
extern VOIDFUNC dialog_append PROTOARG((unsigned, char CONST *));
extern VOIDFUNC dialog_popup
    PROTOARG((unsigned, char CONST *, char CONST *, Widget, XtGrabKind));
extern unsigned dialog_wait
    PROTOARG((unsigned, char CONST *, char CONST *, char CONST *, char CONST **));
extern VOIDFUNC install_dialogs PROTOARG((Widget));
/*}}}*/
/*{{{  garden*/
extern VOIDFUNC install_garden PROTOARG((Widget));
extern VOIDFUNC paint_garden_icon PROTOARG((DESCRIPTOR *));
extern VOIDFUNC paint_garden_image PROTOARG((VOIDARG));
extern VOIDFUNC paint_garden_source PROTOARG((VOIDARG));
/*}}}*/
/*{{{  makemred*/
extern VOIDFUNC create_gizmos PROTOARG((Widget, GIZMO *, Cardinal));
extern VOIDFUNC create_widget PROTOARG((VOIDARG));
#ifndef NDEBUG
extern int error_handler PROTOARG((Display *, XErrorEvent *));
#endif /* NDEBUG */
extern VOIDFUNC fatal_error PROTOVARARG((char CONST *, VARARG));
extern VOIDFUNC list_help PROTOARG((char CONST *));
extern VOIDFUNC nadger_widget_colors PROTOARG((Widget, WidgetClass));
extern VOIDFUNC open_toolkit PROTOARG((int, String *));
/*}}}*/
/*{{{  menubar*/
extern VOIDFUNC install_menubar PROTOARG((Widget));
extern VOIDFUNC menu_garden PROTOARG((char CONST *));
extern VOIDFUNC menu_total PROTOARG((unsigned));
extern VOIDFUNC reset_garden_stack PROTOARG((VOIDARG));
extern VOIDFUNC save_garden PROTOARG((VOIDARG));
/*}}}*/
/*{{{  xmred*/
extern size_t itoa PROTOARG((char *, unsigned long, unsigned));
/*}}}*/
/*}}}*/
