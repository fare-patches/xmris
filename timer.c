/*{{{  (C) 1992 Nathan Sidwell*/
/*****************************************************************************
			X M R I S V1.01
			---------------
		    (C) 1992 Nathan Sidwell

This program is copyright (C) 1992 Nathan Sidwell. This software and documentation
is in the public domain. Permission is granted to distribute and compile
verbatim copies of this software for non-commercial, non-profit use,
without fee. The software may be modified, provided that both the above copyright
notice and this permission notice appear.

No guarantee is given as to the robustness or suitability of this
software for your computer.

Nathan Sidwell  INMOS UK |                 | nathan@inmos.co.uk       DoD#0390
*****************************************************************************/
/*}}}*/
#include "xmris.h"
#include <time.h>
#include <sys/time.h>
#include <signal.h>
/*{{{  timer*/
static struct
{
  void      (*handler)();   /* original handler */
  unsigned  mask;           /* previous signal mask */
  unsigned  volatile counter;        /* timer counter */
  struct itimerval interval; /* interval time */
} timer;
/*}}}*/
/*{{{  prototypes*/
static void timer_alarm PROTOARGLIST((int));
/*}}}*/
/*{{{  void timer_alarm(sig)*/
static void timer_alarm FUNCARGLIST((sig))
int       sig FUNCARGTERM
{
  timer.counter = 1;
  signal(SIGALRM, timer_alarm);
  return;
}
/*}}}*/
/*{{{  void timer_close()*/
extern void timer_close FUNCARGVOID
/*
 * closes the timer stuff
 */
{
  signal(SIGALRM, timer.handler);
  return;
}
/*}}}*/
/*{{{  void timer_open()*/
extern void timer_open FUNCARGVOID
/*
 * initialize the timer stuff
 * this means installing the alarm signam handler
 */
{
  timer.interval.it_interval.tv_sec = 0;
  timer.interval.it_interval.tv_usec = 0;
  timer.interval.it_value.tv_sec = 0;
  timer.interval.it_value.tv_usec = 0;
  timer.handler = signal(SIGALRM, timer_alarm);
  return;
}
/*}}}*/
/*{{{  void timer_start(tick)*/
extern void timer_start FUNCARGLIST((tick))
unsigned  long tick FUNCARGTERM
/*
 * starts an interval timer
 * which ticks every n microseconds
 */
{
  timer.interval.it_value.tv_usec = tick;
  timer.counter = 1;
  timer_wait();
  return;
}
/*}}}*/
/*{{{  void timer_stop()*/
extern void timer_stop FUNCARGVOID
/*
 * stops the interval timer
 */
{
  timer.interval.it_value.tv_usec = 0;
  timer_wait();
  XDrawLine(display.display, display.window, GCN(GC_BALL),
      WINDOW_WIDTH - global.missed, WINDOW_HEIGHT - 1,
      WINDOW_WIDTH, WINDOW_HEIGHT - 1);
  global.missed = 0;
  return;
}
/*}}}*/
/*{{{  void timer_wait()*/
extern void timer_wait FUNCARGVOID
/*
 * waits for the next timer interrupt
 * if this has already gone by, then we immediatley return
 */
{
  if(!timer.counter)
    {
      int     mask;
      
      mask = sigblock(sigmask(SIGALRM));
      if(!timer.counter)
	{
	  if(global.missed)
	    {
	      XDrawPoint(display.display, display.window, GCN(GC_BALL),
		  WINDOW_WIDTH - global.missed, WINDOW_HEIGHT - 1);
	      global.missed--;
	    }
	  sigpause(0);
	}
      sigsetmask(mask);
    }
  else if(global.missed < WINDOW_WIDTH)
    {
      global.missed++;
      XDrawPoint(display.display, display.window, GCN(GC_BALL),
	  WINDOW_WIDTH - global.missed, WINDOW_HEIGHT - 1);
    }
  timer.counter = 0;
  setitimer(ITIMER_REAL, &timer.interval, (struct itimerval *)NULL);
  return;
}
/*}}}*/
