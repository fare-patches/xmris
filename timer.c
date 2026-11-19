/* Copyright (C) 1993, 1992 Nathan Sidwell */
/* RCS $Id: timer.c 1.2 Tue, 16 Mar 1999 11:28:16 +0000 nathan $ */
/*{{{  the problem with real time under unix*/
/*
 * Getting accurate, stable timing is difficult. An auto repeating
 * interrupt cannot be used, because that has rude behaviour, if your
 * slightly too slow. So we must fire off the next IRQ ourselves,
 * preferably inside the interrupt routine. But, there will be some
 * slippage, due to the time for the kernel to deliver the interrupt, and
 * us to set off the next one.
 * A clever way of getting accurate average behaviour is to keep a
 * record of how long the previous IRQ actually took, and use that to
 * generate a correction factor, to be used for the next IRQ request.
 * But, that doesn't converge, because it's fighting the granularity of the
 * kernel's preemptive scheduler. For instance, if we want a rate of
 * 37mS, but the scheduling is in 10mS chunks, we'll be (at least)
 * 3mS too slow, we adjust the delta to be -3 (34mS), and next time we're
 * also 3mS too slow, so we adjust it again to -6 (31mS), and the same
 * happens, so its now a delta of -9 (28mS). This time we're
 * 7mS too fast so the delta changes again to -2 (35mS). As you can
 * see, the delta keeps changing. On average the timing should be ok, but
 * some frames will be too fast and some too slow, which gives
 * jerky animation.
 * Another thing which exacerbates this is that the IRQ is set in uS, but
 * the time can only be go to mS accuracy.
 * So you have a choice, smooth animation, but not quite accurate, or
 * jerky animation, but accurate on average. I've elected for smooth
 * animation.
 *
 * Now to complicate things further. The elapsed game time is now
 * kept, on a per screen basis. Because the game can be paused and there
 * are the history screens, the clock cannot be simply used. I could count
 * animation frames and scale by the interrupt time, but this will always
 * underestimate (due to the slipage above), and may be completely wrong
 * because the interrupts are being missed (but see below).
 * Most systems will have a gettimeofday which gives usec precision of the
 * clock (but not usec accuracy wrt real time, but should be good enough
 * relative to itself). I use this, stopping and starting at appropriate
 * points, to get the elapsed time.
 *
 * In order to get meaningful score comparisons between different
 * hardware, I measure the percentage of missed frames. If this gets too
 * large, I use the error factor to increase the frame time, and
 * scale the scores downwards appropriately. The actual frame time
 * should converge asymtotically to the minimum that the hardware
 * can support, if the hardware is not fast enough for the game.
 *
 * And now to complicate things further still, some OS's round the
 * alarm time downwards, resulting in the signal arriving earlier than
 * expected. To cope with these, I have added the busywait code.
 * During the score scaling code, the mean frame time is examined, and
 * if too short, the busy wait code is activated. This code can be
 * forced by using the busywait flag.
 */
/*}}}*/
#include "xmris.h"
#include <sys/time.h>
#include <signal.h>
#ifdef SVR4
#ifndef SYSV
#define SYSV
#endif
#endif
#define BUSYWAIT
/*{{{  signal_hold, signal_release & signal_pause*/
/* signal_hold blocks the supplied signal and returns a value to be used by
 *    signal_pause and signal_release
 * signal_pause uses the value supplied to return to the signal mask inuse
 *    before the preceeding signal_hold, waits for a signal, then
 *    reblocks the signal
 * signal_release returns the block mask to what it was before signal_hold
 */
#if defined POSIX || defined linux
#  define MASK sigset_t
#  define signal_hold(signal, maskp) \
    {\
      MASK temp; \
      sigemptyset(&temp); \
      sigaddset(&temp, signal); \
      sigprocmask(SIG_BLOCK, &temp, (maskp)); \
    }
#  define signal_release(maskp) sigprocmask(SIG_SETMASK, maskp, (MASK *)NULL)
#  ifdef linux
#    define signal_pause(maskp) sigsuspend(maskp)
#  else
#    define signal_pause(maskp) sigpause(*(maskp))
#  endif
#else
#  ifdef __hpux /* hpux is a weird mixture of BSD & SYSV */
/* don't know if this is right */
#    define MASK    int
#    define signal_hold(signal, maskp) \
      (*(maskp) = sigblock(sigmask(signal)))
#    define signal_release(maskp) sigsetmask(*(maskp))
#    define signal_pause(maskp) sigpause(*(maskp))
#  else
#    ifdef SYSV
/* the signals are already masks, use the supplied signal number
 * for the returned mask, to toggle the blocked mask.
 * sigpause does not automatically block the signal again, so that must be
 * done.
 */
#      define MASK    int
#      define signal_hold(signal, maskp) \
	(*(maskp) = ((void)sighold(signal), signal))
#      define signal_release(maskp) sigrelse(*(maskp))
#      define signal_pause(maskp) (sigpause(*(maskp)), sighold(*(maskp)))
#      define USESIGSET
#    else
/* signals are bit numbers, so we must construct the bit mask.
 * hold returns the previous mask, so we can use that for pause and release
 * sigpause reblocks the signal after being signalled, so no need to
 * reblock it
 */
#      define MASK    int
#      define signal_hold(signal, maskp) \
	(*(maskp) = sigblock(sigmask(signal)))
#      define signal_release(maskp) sigsetmask(*(maskp))
#      define signal_pause(maskp) sigpause(*(maskp))
#    endif /* SYSV */
#  endif /* __hpux */
#endif /* POSIX */
/*}}}*/
/*{{{  get current time*/
/* TICKTIME specifies how many microseconds in each timer tick
 * tick_t is the typedef of the timer return value
 * BUSYWAIT is set if the timer is precise enough for busywait code
 * to work
 */
# ifdef USETIME
#   define gettick(ptr) (*(ptr) = time((time_t *)NULL))
#   define TICKTIME (unsigned long)1000000
#   define tickdelta(later, earlier) (unsigned long)((later) - (earlier))
#   define tickadd(time, interval) ((time) + (interval))
#   define tickafter(later, earlier) \
      (tickdelta(later, earlier) < ~(~(unsigned long)0 >> 1))
    typedef time_t tick_t;
#   undef BUSYWAIT
# else
/* say whether gettimeofday needs a timezone argument
 * as far as I know only SYSV doesn't -- this needs checking
 */
#ifdef POSIX
#   define TIMEZONE struct timezone
#else
#   ifdef __hpux
#     define TIMEZONE struct timezone
#   else
#     ifdef SYSV
#       define TIMEZONE VOID
#     else
#       define TIMEZONE struct timezone
#     endif /* SYSV */
#   endif /* __hpux */
#endif /* POSIX */
#   define TICKTIME (unsigned long)1000
#   define gettick(ptr) \
      {\
	struct timeval timeofday; \
	gettimeofday(&timeofday, (TIMEZONE *)NULL); \
	*(ptr) = (unsigned long)timeofday.tv_sec * (1000000 / TICKTIME) + \
	    (unsigned long)timeofday.tv_usec / (1000000 / TICKTIME); \
      }
#   define tickdelta(later, earlier) ((later) - (earlier))
#   define tickadd(time, interval) ((time) + (interval))
#   define tickafter(later, earlier) \
      (tickdelta(later, earlier) < ~(~(unsigned long)0 >> 1))
    typedef unsigned long tick_t;
# endif /* USETIME */
/*}}}*/
/*{{{  timer*/
static struct
{
  VOIDFUNC  (*handler) PROTOARG((int));   /* original handler */
  unsigned  long usec;              /* interval time in usec */
  struct itimerval interval;        /* internal interval time */
  unsigned  VOLATILE elapsed;       /* timer elapsed */
  unsigned  VOLATILE waiting;       /* waiting for the interrupt */
#ifdef BUSYWAIT
  unsigned  busywait;               /* busywaiting is turned on */
  unsigned  VOLATILE restarted;     /* restarted in signal handler */
  tick_t    timeout;                /* timeout */
  tick_t    delay;                  /* interval delay */
#endif /* BUSYWAIT */
  unsigned  state;          /* timing state */
  tick_t    game;           /* start of game tick */
  tick_t    start;          /* timing start */
  tick_t    stop;           /* timing stop */
  unsigned  count;          /* frame count */
  unsigned  missed;         /* missed count */
} timer;
/*}}}*/
#ifndef NDEBUG
/*{{{  debug*/
/*{{{  typedef struct*/
typedef struct
{
  char CONST *text;
  unsigned  elapsed;
  unsigned  waiting;
  unsigned  restarted;
} RECORD;
/*}}}*/
static RECORD list[64];
static unsigned next;
/*}}}*/
#endif
/*{{{  prototypes*/
static VOIDFUNC timer_alarm PROTOARG((int));
#ifndef NDEBUG
static VOIDFUNC timer_debug PROTOARG((char CONST *));
#endif
/*}}}*/
/*{{{  void timer_alarm(sig)*/
static VOIDFUNC timer_alarm
/* ARGSUSED */
FUNCARG((sig),
	int       sig
)
{
  /*
   * Most calls are undefined in a signal handler
   * (only signal, exit, longjump & abort are guaranteed to work)
   * This should work, except on _really_ weird library implementations,
   * because timer.waiting is only true when the main thread is stuck
   * in a wait() call.
   */
#ifndef NDEBUG
  timer_debug("In handler");
#endif
#ifndef USESIGSET
  signal(SIGALRM, timer_alarm);
#endif
  timer.elapsed = !timer.waiting;
  if(timer.waiting)
    {
      timer.waiting = 0;
#ifdef BUSYWAIT
      if(timer.busywait)
	{
	  tick_t  now;
      
	  gettick(&now);
	  if(tickafter(now, timer.timeout))
	    {
	      timer.timeout = tickadd(timer.timeout, timer.delay);
	      timer.restarted = 1;
	      setitimer(ITIMER_REAL, &timer.interval,
		  (struct itimerval *)NULL);
	    }
	}
      else
	{
	  timer.restarted = 1;
#else
	{
#endif
	  setitimer(ITIMER_REAL, &timer.interval,
	     (struct itimerval *)NULL);
	}
    }
  return;
}
/*}}}*/
/*{{{  void timer_close()*/
extern VOIDFUNC timer_close FUNCARGVOID
/*
 * closes the timer stuff
 */
{
  if(data.busywait == False)
    {
      MASK      mask;
      
      signal_hold(SIGALRM, &mask);
      timer.interval.it_value.tv_usec = 0;
      timer.interval.it_interval.tv_usec = 0;
      while(!timer.elapsed)
	signal_pause(&mask);
      signal_release(&mask);
      setitimer(ITIMER_REAL, &timer.interval, (struct itimerval *)NULL);
#ifdef USESIGSET
      sigset(SIGALRM, timer.handler);
#else
      signal(SIGALRM, timer.handler);
#endif /* USESIGSET */
    }
  return;
}
/*}}}*/
#ifndef NDEBUG
/*{{{  static void timer_debug(text)*/
static VOIDFUNC timer_debug
FUNCARG((text),
	char CONST *text
)
{
  RECORD *ptr;
  
  ptr = &list[next];
  if(next++ == 63)
    next = 0;
  ptr->text = text;
  ptr->elapsed = timer.elapsed;
  ptr->waiting = timer.waiting;
  ptr->restarted = timer.restarted;
  return;
}
/*}}}*/
#endif
/*{{{  void timer_open()*/
extern VOIDFUNC timer_open FUNCARGVOID
/*
 * initialize the timer stuff
 * this means installing the alarm signal handler
 * and starting the first tick
 */
{
  if(data.busywait != False)
# ifdef BUSYWAIT
    {
      timer.busywait = 1;
      gettick(&timer.timeout);
    }
# else
    {
      fprintf(stderr, "Busywait code not included.\n");
      data.busywait = False;
    }
# endif /* BUSYWAIT */
  timer.interval.it_interval.tv_sec = 0;
  timer.interval.it_interval.tv_usec = 0;
  timer.interval.it_value.tv_sec = 0;
  timer.interval.it_value.tv_usec = 0;
  if(data.busywait == False)
#ifdef USESIGSET
      timer.handler = sigset(SIGALRM, timer_alarm);
#else
      timer.handler = signal(SIGALRM, timer_alarm);
#endif /* USESIGSET */
  timer.waiting = 0;
  timer.elapsed = 1;
#ifndef NDEBUG
  timer_debug("Init");
#endif
  global.missed = 0;
  global.dilation = FRAME_SCALE;
  global.scale = SCORE_SCALE;
  return;
}
/*}}}*/
/*{{{  int timer_set(tick, state)*/
extern unsigned timer_set
FUNCARG((tick, state),
	unsigned  long tick
ARGSEP  unsigned  state
)
/*
 * sets the timer tick and fiddles the timer state
 * if the tick is zero, then the timer state only is altered
 */
{
  unsigned  value;
  
  if(tick)
    {
      {
	/* stupid compilers with broken stringizizing
	 * and stupid compilers which don't understand \
	 * outside of #define */
	unsigned  t1, t2;
	
	t1 = timer.state == TIMING_OFF || timer.state == TIMING_PAUSE;
	t2 = state == TIMING_OFF || state == TIMING_PAUSE;
	assert(t1 || t2);
      }
      timer.usec = tick;
#ifdef BUSYWAIT
      timer.delay = (tick_t)(tick / TICKTIME);
#endif /* BUSYWAIT */
      timer.interval.it_value.tv_usec = tick;
    }
  value = timer.state;
  switch(state)
  {
    /*{{{  case TIMING_ON:*/
    case TIMING_ON:
    {
      if(timer.state == TIMING_OFF)
	global.msec = 0;
      if(timer.state != TIMING_ON)
	{
	  gettick(&timer.start);
	  if(timer.state == TIMING_OFF)
	    timer.game = timer.start;
	  else
	    {
	      tick_t    now;
	      
	      gettick(&now);
	      timer.game += tickdelta(now, timer.stop);
	    }
	  timer.state = TIMING_ON;
	  timer.count = 0;
	}
      break;
    }
    /*}}}*/
    /*{{{  case TIMING_OFF:*/
    case TIMING_OFF:
    {
      if(timer.state != TIMING_OFF)
	{
	  if(timer.state == TIMING_ON)
	    gettick(&timer.stop);
	  global.msec = tickdelta(timer.stop, timer.game) *
	      TICKTIME / (unsigned long)1000;
	  timer.state = TIMING_OFF;
	}
      break;
    }
    /*}}}*/
    /*{{{  case TIMING_PAUSE:*/
    case TIMING_PAUSE:
    {
      if(timer.state == TIMING_ON)
	{
	  timer.state = TIMING_PAUSE;
	  gettick(&timer.stop);
	}
      break;
    }
    /*}}}*/
    default:
      assert(0);
  }
  return value;
}
/*}}}*/
/*{{{  void timer_wait()*/
extern VOIDFUNC timer_wait FUNCARGVOID
/*
 * waits for the next timer interrupt
 * if this has already gone by, then we immediately return
 * If we arrive here before the interrupt, the interrupt is retriggered
 * in the signal handler (timer_alarm), for minimum slipage. If
 * we're too late, then the interrupt has to be restarted here. (If
 * the signal handler did it, things rapidly get out of hand.)
 *
 * the transputer code is realy simple, and doesn't give ANY slippage
 * provided the system is fast enough. If too slow, slip is inserted
 * as required.
 */
{
  int       point;
  
  {
    MASK      mask;
    
    signal_hold(SIGALRM, &mask);
    if(!timer.elapsed && data.busywait == False)
      {
	point = -1;
#ifndef NDEBUG
	timer_debug("Starting wait");
#endif
	timer.waiting = 1;
	while(timer.waiting)
	  signal_pause(&mask);
#ifndef NDEBUG
	timer_debug("Done wait");
#endif
#ifdef BUSYWAIT
	if(!timer.restarted)
	  point = 0;
	timer.restarted = 0;
#endif /* BUSYWAIT */
      }
    else
      {
	timer.elapsed = 0;
#ifdef BUSYWAIT
	if(timer.busywait)
	  point = 0;
	else
#endif /* BUSYWAIT */
	  {
	    point = 1;
	    setitimer(ITIMER_REAL, &timer.interval, (struct itimerval *)NULL);
	    timer.missed++;
	  }
      }
    signal_release(&mask);
  }
#ifdef BUSYWAIT
  /*{{{  busywait?*/
  if(!point)
    {
      tick_t  now;
      
      gettick(&now);
      if(tickafter(now, timer.timeout))
	{
	  point = 1;
	  timer.missed++;
	  timer.timeout = tickadd(now, timer.delay);
	}
      else
	{
	  while(!tickafter(now, timer.timeout))
	    gettick(&now);
	  timer.timeout = tickadd(timer.timeout, timer.delay);
	  point = -1;
	}
      if(data.busywait == False)
	setitimer(ITIMER_REAL, &timer.interval, (struct itimerval *)NULL);
    }
  /*}}}*/
#endif /* BUSYWAIT */
  if(timer.state != TIMING_ON)
    /* EMPTY */;
  else if(!timer.count)
    {
      timer.missed = 0;
      gettick(&timer.start);
    }
  else if(timer.count == FRAME_RATIO_UPDATE)
    {
      unsigned  dilation;
      int       xold, xnew;
      unsigned long usec;
      
      gettick(&timer.stop);
      usec = (unsigned long)tickdelta(timer.stop, timer.start) * TICKTIME;
      dilation = (unsigned)(usec / FRAME_RATIO_UPDATE *
	  FRAME_SCALE / timer.usec);
#ifdef BUSYWAIT
      if(!timer.busywait && dilation * 100 < FRAME_SCALE * 97)
	{
	  fprintf(stderr, "%s:Timing too quick, using partial busywait.\n",
	      myname);
	  timer.timeout = tickadd(timer.stop, timer.delay);
	  timer.busywait = 1;
	}
#endif /* BUSYWAIT */
      if(dilation <= FRAME_SCALE)
	dilation = FRAME_SCALE;
      else if(timer.missed <= (global.dilation == FRAME_SCALE ?
	  FRAME_MISS_START : FRAME_MISS_STOP))
	dilation = FRAME_SCALE;
      xold = WINDOW_WIDTH - (int)((unsigned long)WINDOW_WIDTH *
	    FRAME_SCALE / global.dilation);
      xnew = WINDOW_WIDTH - (int)((unsigned long)WINDOW_WIDTH *
	    FRAME_SCALE / dilation);
      /*{{{  swap?*/
      if(xold > xnew)
	{
	  int     t;
	  
	  t = xold;
	  xold = xnew;
	  xnew = t;
	}
      /*}}}*/
      present_load(xold, xnew);
      global.dilation = dilation;
      timer.start = timer.stop;
      timer.missed = 0;
      timer.count = 1;
      /*{{{  set score scale*/
      {
	unsigned long scale;
	unsigned  last;
	
	scale = (unsigned long)SCORE_SCALE * SCORE_SCALE *
	    FRAME_SCALE / global.dilation;
	dilation = SCORE_SCALE;
	do
	  {
	    last = dilation;
	    dilation = (unsigned)(((unsigned long)dilation * dilation +
		scale) / dilation / 2);
	  }
	while(dilation != last);
	global.scale = dilation;
      }
      /*}}}*/
    }
  timer.count++;
  /*{{{  plot load point?*/
  if(point < 0)
    {
      if(global.missed)
	{
	  present_load((int)(WINDOW_WIDTH - global.missed), (int)(WINDOW_WIDTH - global.missed));
	  global.missed--;
	}
    }
  else if(point > 0)
    {
      if(global.missed < WINDOW_WIDTH)
	{
	  global.missed++;
	  present_load((int)(WINDOW_WIDTH - global.missed), (int)(WINDOW_WIDTH - global.missed));
	}
    }
  /*}}}*/
  return;
}
/*}}}*/
