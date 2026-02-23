/* Copyright (C) 1993, 1992 Nathan Sidwell */
/* RCS $Id: makemred.c 1.2 Tue, 16 Mar 1999 11:28:16 +0000 nathan $ */
/*{{{  includes*/
#include "xmred.h"
#include "defmred.h"
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Paned.h>
#include "makecom.c"
#include <X11/IntrinsicP.h>
#include <X11/CoreP.h>
#include "Icon.h"
#include "PixmapList.h"
#ifdef SCROLLBARBUGFIX
#include <X11/Xaw/Scrollbar.h>
#endif
/*}}}*/
/*{{{  define*/
/*{{{  gizmos*/
#define GIZMO_MRED   0
#define GIZMO_PANE   1
#define GIZMO_MENU   2
#define GIZMO_ONE    3
#define GIZMO_ALL    4
#define GIZMO_GARDEN 5
#define GIZMO_PANEL  6
/*}}}*/
/*}}}*/
/*{{{  tables*/
/*{{{  static GIZMO gizmos[] =*/
static GIZMO gizmos[] =
{
  {"mred", -1, &formWidgetClass},
  {"pane", GIZMO_MRED, &panedWidgetClass},
  {"menus", GIZMO_PANE, &panedWidgetClass},
  {"one", GIZMO_PANE, &panedWidgetClass},
  {"all", GIZMO_PANE, &panedWidgetClass},
  {"garden", GIZMO_ONE, &gardenWidgetClass},
  {"panel", GIZMO_ONE, &formWidgetClass},
};
/*}}}*/
/*{{{  static Arg toplevel_args[] =*/
static Arg toplevel_args[] =
{
  {XtNinput, True},
};
/*}}}*/
/*{{{  static Arg highlight_args[] =*/
static Arg highlight_args[] =
{
  {XtNforeground, (XtArgVal)0},
};
/*}}}*/
/*{{{  static Arg color_args[] =*/
static Arg color_args[] =
{
  {XtNforeground, (XtArgVal)0},
  {XtNbackground, (XtArgVal)0},
  {XtNborderColor, (XtArgVal)0},
  {XtNinternalBorderColor, (XtArgVal)0},
};
/*}}}*/
/*{{{  static char CONST *fallback_resources[] =*/
static char CONST *fallback_resources[] =
{
#if __STDC__
"Xmris.Font:" FONT_NAME,
#else
"Xmris.Font:",
#endif
"*TransientShell.allowShellResize:yes",
"*Text.useStringInPlace:no",
"*dialog*resize:yes",
"*load*Text*editType:textAppend",
"*comment*Text.baseTranslations:#override",
"*load*Text.baseTranslations:#override \
<Key>Return:\n",
"*TransientShell.Dialog.Text.baseTranslations:#override \
<Key>Return:set-dialog-button(agree)\n\
Ctrl<Key>g:set-dialog-button(disagree)\n\
<Key>Escape:set-dialog-button(disagree)\n",
"*dialog*ok.label:Ok",
"*dialog*continue.label:Continue",
"*dialog*retry.label:Retry",
"*dialog*cancel.label:Cancel",
"*dialog*clear.label:Clear",
"*dialog*abort.label:Abort",
"*dialog*default.label:Default",
"*altered.title:Altered",
"*showGrip:no",
"*mred.defaultDistance:0",
"*pane.orientation:vertical",
"*menus.orientation:horizontal",
"*menus*allowResize:yes",
"*menus*justify:left",
"*file.label:File",
"*garden.label:Garden",
"*menu.undo.label:Undo",
"*menu.redo.label:Redo",
"*menu.clear.label:Clear",
"*menu.new.label:New",
"*menu.load.label:Load",
"*menu.insert.label:Insert",
"*menu.save.label:Save",
"*menu.saveas.label:Save As",
"*menu.quit.label:Quit",
"*Form.borderWidth:0",
"*Label.borderWidth:0",
"*all.orientation:horizontal",
"*buffer.skipAdjust:yes",
"*buffer.Icon.top:chainTop",
"*buffer.Icon.bottom:chainTop",
"*all.gardens.orientation:vertical",
"*gardens.gardens.min:51",
"*gardens.orientation:horizontal",
"*gardens.scrollbar.orientation:horizontal",
"*gardens.scrollbar.height:12",
"*gardens.scrollbar.thumb:None",
"*gardens.scrollbar.skipAdjust:yes",
"*edit.skipAdjust:yes",
"*edit.Icon.top:chainTop",
"*edit.Icon.bottom:chainTop",
"*edit.include.fromHoriz:comment",
"*edit.garden.fromHoriz:include",
"*one.defaultDistance:0",
"*one.orientation:horizontal",
"*one.garden.skipAdjust:yes",
"*one.panel.attach:garden",
"*panel.defaultDistance:0",
"*panel*Label.internalHeight:0",
"*panel*Label.internalWidth:0",
"*controls.defaultDistance:0",
"*controls*left:chainLeft",
"*controls*right:chainLeft",
"*controls.Form.Icon.top:chainTop",
"*controls.Form.Icon.bottom:chainTop",
"*options.label.label:Options",
"*controls*Icon.fromVert:label",
"*options.random.fromHoriz:apple",
"*options.cherry.fromHoriz:random",
"*options.path.fromHoriz:cherry",
"*options.player.fromHoriz:path",
"*options.den.fromHoriz:player",
"*fills.fromVert:options",
"*fills.label.label:Fills",
"*fills.fill1.fromHoriz:fill0",
"*fills.fill2.fromHoriz:fill1",
"*fills.fill3.fromHoriz:fill2",
"*colors.fromVert:fills",
"*colors.label.label:Colors",
"*colors.color1.fromHoriz:color0",
"*colors.color2.fromHoriz:color1",
"*buttons.fromVert:colors",
"*buttons.label.label:Buttons",
"*buttons.button2.fromHoriz:button1",
"*buttons.button3.fromHoriz:button2",
"*buttons.button4.fromHoriz:button3",
"*buttons.button5.fromHoriz:button4",
"*buttons.button1.option:path",
"*buttons.button2.option:cherry",
"*buttons.button3.option:apple",
"*buttons.button4.option:random",
"*totals.fromVert:controls",
"*totals.left:chainLeft",
"*totals.right:chainLeft",
"*totals.label.label:Totals",
"*totals.label.horizDistance:4",
"*totals.Form.defaultDistance:0",
"*total0.fromVert:label",
"*total0.horizDistance:28",
"*total1.fromVert:total0",
"*total1.horizDistance:28",
"*total2.fromVert:total1",
"*total2.horizDistance:28",
"*total3.fromVert:total2",
"*total3.horizDistance:28",
"*total4.fromVert:label",
"*total4.horizDistance:30",
"*total5.fromVert:total4",
"*total5.horizDistance:30",
"*total6.fromVert:total5",
"*total6.horizDistance:30",
"*total7.fromVert:total6",
"*total7.horizDistance:30",
"*total4.fromHoriz:total0",
"*total5.fromHoriz:total1",
"*total6.fromHoriz:total2",
"*total7.fromHoriz:total3",
"*total8.fromVert:total3",
"*total8.horizDistance:28",
"*total9.fromHoriz:total8",
"*total9.fromVert:total7",
"*total9.horizDistance:30",
"*count.fromHoriz:icon",
"*count.horizDistance:4",
"*totals.total5.icon.fromHoriz:scrollbar",
"*totals.Form.icon.horizDistance:0",
"*total5.scrollbar.height:32",
"*total5.scrollbar.width:12",
"*total5.scrollbar.minimumThumb:1",
"*total5.scrollbar.thumb:None",
"*total5.scrollbar.baseTranslations:#override \
~Shift<KeyPress>space:StartScroll(Forward) NotifyScroll(FullLength) EndScroll()\n\
Shift<KeyPress>space:StartScroll(Backward) NotifyScroll(FullLength) EndScroll()\n",
"*totals.Form.icon.fromHoriz:warning",
"*panel.totals.Form.warning.width:12",
"*panel.totals.Form.warning.internalHeight:5",
"*panel.totals.Form.warning.internalWidth:1",
"*panel.totals.Form.count.height:34",
"*panel.totals.Form.count.width:32",
"*totals.Form.count.resize:no",
"*totals.Form.left:chainLeft",
"*totals.Form.right:chainLeft",
"*totals.Form.top:chainTop",
"*totals.Form.bottom:chainTop",
"*panel.mode.fromVert:totals",
"*mode.fromHoriz:comment",
"*mode.horizDistance:40",
"*mode.Icon.left:chainRight",
"*mode.Icon.right:chainRight",
"*mode.label.left:chainRight",
"*mode.label.right:chainRight",
"*mode.Icon.top:chainTop",
"*mode.Icon.bottom:chainTop",
"*mode.label.top:chainTop",
"*mode.label.bottom:chainTop",
"*mode.label.label:Mode",
"*mode.label.height:32",
"*mode.combined.fromHoriz:label",
"*mode.separate.fromHoriz:combined",
"*mode.random.fromHoriz:separate",
"*mode.left:chainLeft",
"*mode.right:chainLeft",
"*mode.top:chainBottom",
"*mode.bottom:chainBottom",
"*panel.comment.fromVert:totals",
"*comment.vertDistance:4",
"*comment.horizDistance:4",
"*comment.left:chainLeft",
"*comment.right:chainLeft",
"*comment.top:chainBottom",
"*comment.bottom:chainBottom",
NULL
};
/*}}}*/
/*}}}*/
/*{{{  prototypes*/
/*}}}*/
/*{{{  void create_gizmos(root, gizmos, num_gizmos)*/
extern VOIDFUNC create_gizmos
FUNCARG((root, gizmos, num_gizmos),
	Widget    root
ARGSEP  GIZMO     *gizmos
ARGSEP  Cardinal  num_gizmos
)
/*
 * Creates a widget hierarchy according to the gizmo table
 * supplied.
 * The widget creation is done in this complicated fashion, so
 * that the layout of form widget children can be specified in
 * any order, and is not constrained by the order in which the
 * widgets are created.
 * I have also added an 'attach' resource for children of composite
 * widgets.
 */
{
  /*{{{  resources*/
  static char CONST *resources[] =
    {
      XtNfromHoriz,
      XtNfromVert
    };
  /*}}}*/
  GIZMO     *gptr;
  int       parent;
  int       count;
  Widget    parent_widget;
  int       parent_form;
  int       parent_attach;
  int       ix;
  
  count = num_gizmos;
  while(count)
    {
      unsigned  children;
      
      for(gptr = gizmos, ix = num_gizmos; ix--; gptr++)
	if(!gptr->widget &&
	    (gptr->parent < 0 || gizmos[gptr->parent].widget))
	  break;
      assert(ix >= 0);
      parent = gptr->parent;
      parent_widget = parent < 0 ? root : gizmos[parent].widget;
      parent_form = XtClass(parent_widget) == formWidgetClass;
      parent_attach = XtIsComposite(parent_widget) &&
	  !XtIsConstraint(parent_widget);
      children = 0;
      /*{{{  find siblings*/
      for(gptr = gizmos, ix = num_gizmos; ix--; gptr++)
	if(gptr->parent == parent)
	  {
	    if(parent_form || parent_attach)
	      /*{{{  mark dependencies on siblings*/
	      {
		/*{{{  static XtResource formresources[] =*/
		static XtResource formresources[] =
		{
		  {XtNfromHoriz, XtCWidget, XtRString, sizeof(String),
		      0 * sizeof(String), XtRImmediate, NULL},
		  {XtNfromVert, XtCWidget, XtRString, sizeof(String),
		      1 * sizeof(String), XtRImmediate, NULL},
		};
		/*}}}*/
		/*{{{  static XtResource attachresources[] =*/
		static XtResource attachresources[] =
		{
		  {"attach", XtCWidget, XtRString, sizeof(String),
		      0 * sizeof(String), XtRImmediate, NULL},
		};
		/*}}}*/
		unsigned  ix;
		unsigned  child;
		GIZMO     *subptr;
		String    subrecs[2];
		
		subrecs[1] = NULL;
		XtGetSubresources(parent_widget, (XtPointer)subrecs,
		    gptr->name,
		    ((CoreClassRec *)*gptr->class)->core_class.class_name,
		    parent_form ? formresources : attachresources,
		    parent_form ? XtNumber(formresources) :
		      XtNumber(attachresources), NULL, 0);
		child = 0;
		for(subptr = gizmos, ix = num_gizmos; ix--; subptr++)
		  if(subptr->parent == parent)
		    {
		      if(gptr != subptr)
			{
			  unsigned  i;
			  
			  for(i = 2; i--;)
			    if(subrecs[i] &&
				!strcmp(subrecs[i], subptr->name))
			      gptr->align[i] = num_gizmos - ix;
			}
		      child++;
		    }
	      }
	      /*}}}*/
	    children++;
	  }
      /*}}}*/
      count -= children;
      /* create the siblings */
      while(children--)
	{
	  /*{{{  find a child*/
	  {
	    GIZMO     *fptr;
	    
	    fptr = NULL;
	    for(gptr = gizmos, ix = num_gizmos; ix--; gptr++)
	      if(gptr->parent == parent && !gptr->widget)
		{
		  if(!parent_attach || !gptr->align[0] ||
		      gizmos[gptr->align[0] - 1].widget)
		    break;
		  if(!fptr)
		    fptr = gptr;
		}
	    if(ix < 0)
	      gptr = fptr;
	  }
	  /*}}}*/
	  /* create it */
	  {
	    Arg       args[2];
	    unsigned  nargs;
	    unsigned  count;
	    WidgetClass class;
	      
	    nargs = 0;
	    if(parent_form)
	      for(count = 2; count--;)
		if(gptr->align[count])
		  {
		    XtSetArg(args[nargs], (String)resources[count], NULL);
		    nargs++;
		  }
	    for(class = *gptr->class; class;
		class = ((CoreClassPart *)class)->superclass)
	      if(class == shellWidgetClass)
		{
		  gptr->widget = XtCreatePopupShell(gptr->name,
		      *gptr->class, parent_widget, args, nargs);
		  break;
		}
	    if(!class)
	      gptr->widget = XtCreateManagedWidget(gptr->name,
		  *gptr->class, parent_widget, args, nargs);
	    if(gptr->args)
	      XtSetValues(gptr->widget, gptr->args, gptr->num_args);
#ifdef SCROLLBARBUGFIX
	    if(*gptr->class == scrollbarWidgetClass)
	      /* EMPTY */;
	    else
#endif
	    if(display.black || display.white)
	      XtSetValues(gptr->widget, color_args, XtNumber(color_args));
	    if(data.mono == False && (*gptr->class == iconWidgetClass ||
		*gptr->class == pixmapListWidgetClass))
	      XtSetValues(gptr->widget, highlight_args,
		  XtNumber(highlight_args));
	  }
	}
      if(parent_form)
	/*{{{  set args on siblings*/
	for(gptr = gizmos, ix = num_gizmos; ix--; gptr++)
	  if(gptr->parent == parent)
	    {
	      Arg       args[2];
	      unsigned  nargs;
	      unsigned  count;
	      
	      nargs = 0;
	      for(count = 2; count--;)
		if(gptr->align[count])
		  {
		    XtSetArg(args[nargs], (String)resources[count],
			gizmos[gptr->align[count] - 1].widget);
		    nargs++;
		  }
	      XtSetValues(gptr->widget, args, nargs);
	    }
	/*}}}*/
    }
  return;
}
/*}}}*/
/*{{{  void create_widget()*/
extern VOIDFUNC create_widget FUNCARGVOID
/*
 * Create the graphics contexts, sprites and colours and stuff.
 * sets visual and stuff for the toplevel widget
 */
{
  gizmos[GIZMO_GARDEN].name = &names[data.gender != False][1];
  create_gizmos(display.toplevel, gizmos, XtNumber(gizmos));
  create_resources(gizmos[GIZMO_GARDEN].widget);
  color_args[0].value = (XtArgVal)display.black;
  color_args[1].value = (XtArgVal)display.white;
  color_args[2].value = (XtArgVal)display.border;
  color_args[3].value = (XtArgVal)display.border;
  if(data.mono == False)
    highlight_args[0].value = (XtArgVal)colors[COLOR_APPLE_FAINT].pixel;
  install_menubar(gizmos[GIZMO_MENU].widget);
  install_control(gizmos[GIZMO_PANEL].widget);
  install_garden(gizmos[GIZMO_GARDEN].widget);
  install_all(gizmos[GIZMO_ALL].widget);
  install_dialogs(gizmos[GIZMO_MRED].widget);
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
/*{{{  knr?*/
#if !__STDC__
  {
    char      *ptr;
    
    ptr = XtMalloc(strlen(fallback_resources[0]) + strlen(FONT_NAME) + 1);
    strcpy(ptr, fallback_resources[0]);
    strcat(ptr, FONT_NAME);
    fallback_resources[0] = ptr;
  }
#endif
/*}}}*/
  extract_options(&argc, argv, command_options);
  default_gender = GAME_GENDER;
  if(data.help == False)
    {
      XrmOptionDescRec CONST *ptr;
    
      /* as options is an extern of unknown size, I can't use
       * XtNumber() to get its size, so I've done it this way
       */
      for(ptr = options; *(char CONST **)ptr; ptr++)
	/* EMPTY */;
      display.toplevel = XtAppInitialize(&display.context, "Xmris",
	  options, (Cardinal)(ptr - options),
	  (Cardinal *)&argc, (String *)argv,
	  (String *)fallback_resources,
	  toplevel_args, XtNumber(toplevel_args));
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
      else
	{
	  XtResource CONST *ptr;
	  
	  XtAppSetTypeConverter(display.context, XtRString, XtRGender,
	      convert_string2gender, (XtConvertArgRec *)NULL, 0, XtCacheNone,
	      (void (*)PROTOARG((XtAppContext, XrmValue *, XtPointer,
		  XrmValue *, Cardinal *)))NULL);
	  display.display = XtDisplay(display.toplevel);
	  for(ptr = resources; *(char CONST **)ptr; ptr++)
	    /* EMPTY */;
	  XtGetApplicationResources(display.toplevel, (XtPointer)&data,
	      resources, (Cardinal)(ptr - resources), (Arg *)NULL, 0);
#ifndef NDEBUG
	  XSetErrorHandler(error_handler);
#endif /* NDEBUG */
	  gettoplevelresources();
	}
    }
  return;
}
/*}}}*/
