/* Copyright (C) 1993 Nathan Sidwell */
/* RCS $Id: ansiknr.h,v 4.5 1994/01/25 18:33:03 nathan Stable $ */
/* macros to make both ANSI & K&R compilers happy */
/* uses FUNCPROTO like Xfuncproto.h does, so you can tailor things */
/*{{{  prototypes*/
#if __STDC__ || FUNCPROTO & 1
# define PROTOARG(list) list
# define VOIDFUNC void
# define VOID void
# define FUNCARG(list, defs) (defs)
# define FUNCARGVOID ()
# define ARGSEP ,
# define VOIDARG void
#else
# define PROTOARG(list) ()
# if STRICTKNR /* most knr compilers now understand void */
#  define VOIDFUNC int
#  define VOID char
# else
#  define VOIDFUNC void
#  define VOID void
# endif /* NOVOID */
# define FUNCARG(list, defs) list defs;
# define FUNCARGVOID ()
# define ARGSEP  ;
# define VOIDARG 
#endif
/*}}}*/
/*{{{  varargs prototypes*/
#if __STDC__ || FUNCPROTO & 2
# define VARARG ...
# define VARARGDEF ...
# define FUNCVARARG(list, defs) (defs, ...)
# define VARARGSET(args, last) va_start(args, last)
# define PROTOVARARG(list) PROTOARG(list)
#else
# define VARARG int
# define VARARGDEF va_alist
# define FUNCVARARG(list, defs) list defs; va_dcl
# define VARARGSET(args, last) va_start(args)
# define PROTOVARARG(list) ()
#endif
/*}}}*/
/*{{{  nested prototypes*/
#if __STDC__ || FUNCPROTO & 8
# define NESTEDPROTOARG(list) list
#else
# define NESTEDPROTOARG(list) ()
#endif
/*}}}*/
/*{{{  const, volatile*/
#undef CONST
#undef VOLATILE
#if __STDC__ || FUNCPROTO & 4
# define CONST const
# define VOLATILE volatile
#else
# define CONST /* blank */
# define VOLATILE /* blank */
#endif
/*}}}*/
#if __STDC__
# include <stdarg.h>
#else /* !__STDC__ */
# include <varargs.h>
#endif /* __STDC__ */
