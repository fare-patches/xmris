#include "xmris.h"
#include "sound.h"
#include <signal.h>

extern SDL_Window *sdl_video_open(int, int);

void fatal_error(const char *format,...)
{
  va_list args;
  fprintf(stderr,"%s: ",myname);
  va_start(args,format);
  vfprintf(stderr,format,args);
  va_end(args);
  fputc('\n',stderr);
  exit(1);
}

const char *key_name(KeySym key) { return SDL_GetKeyName(key); }
void set_key_default(void) {}
void set_key_single(void) {}
void color_cycle(void) {}
void color_set(int type) { (void)type; }

char *garden_resource(const char *name, const char *suffix)
{
  const char *base = SDL_GetBasePath();
  if (!base) return NULL;
  char *path = malloc(strlen(base)+strlen(name)+(suffix?strlen(suffix):0)+1);
  if (path) {
    sprintf(path,"%s%s%s",base,name,suffix?suffix:"");
    if (access(path,R_OK)) { free(path); return NULL; }
  }
  return path;
}

static void handle_event(SDL_Window *window, const SDL_Event *event)
{
  static int resume_timer;
  switch (event->type) {
    case SDL_EVENT_QUIT:
    case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
      global.quit=1;
      break;
    case SDL_EVENT_KEY_DOWN:
      if (event->key.repeat) break;
      if (event->key.key==SDLK_F11)
        SDL_SetWindowFullscreen(window,!(SDL_GetWindowFlags(window)&SDL_WINDOW_FULLSCREEN));
      else game_keypress(event->key.key);
      break;
    case SDL_EVENT_KEY_UP:
      game_keyrelease(event->key.key);
      break;
    case SDL_EVENT_WINDOW_FOCUS_LOST:
      global.pressed=0;
      global.pause=global.state<6;
      break;
    case SDL_EVENT_WINDOW_MINIMIZED:
      if (display.mapped) {
        resume_timer=timer_set(0,TIMING_PAUSE)==TIMING_ON;
        sound_pause(1);
        display.mapped=0;
      }
      break;
    case SDL_EVENT_WINDOW_RESTORED:
      if (resume_timer) timer_set(0,TIMING_ON);
      resume_timer=0;
      sound_pause(0);
      display.mapped=1;
      /* fall through */
    case SDL_EVENT_WINDOW_EXPOSED:
    case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
      refresh_window();
      video_sync();
      break;
  }
}

int main(int argc,char **argv)
{
  const KeySym keys[KEYS]={SDLK_UP,SDLK_DOWN,SDLK_LEFT,SDLK_RIGHT,SDLK_SPACE,
      SDLK_P,SDLK_Q,SDLK_I,SDLK_K};
  int scale=0, fullscreen=0;
  SDL_Window *window;
  sigset_t alarm, previous;
  myname=argv[0];
  init_scores_early();
  memcpy(data.keysyms,keys,sizeof(keys));
  data.music=data.sound=True;
  data.diamond_mask=1;
  for (int i=1;i<argc;++i) {
    if (!strcmp(argv[i],"-fullscreen")) fullscreen=1;
    else if (!strcmp(argv[i],"-nosound")) data.sound=False;
    else if (!strcmp(argv[i],"-nomusic")) data.music=False;
    else if (!strcmp(argv[i],"-random")) data.random=True;
    else if (!strcmp(argv[i],"-gardens") && i+1<argc) data.boards=argv[++i];
    else if (!strcmp(argv[i],"-scale") && i+1<argc) {
      char *end;
      const char *value=argv[++i];
      if (!strcmp(value,"auto")) scale=0;
      else {
        long factor=strtol(value,&end,10);
        if (*end || factor<1 || factor>32767/WINDOW_HEIGHT) fatal_error("invalid scale %s",value);
        scale=factor;
      }
    } else {
      fprintf(stderr,"Usage: %s [-fullscreen] [-scale auto|N] [-nosound] [-nomusic] [-random] [-gardens FILE]\n",myname);
      return strcmp(argv[i],"-help")!=0;
    }
  }
  init_scores();
  create_boards();
  /* Worker threads must leave SIGALRM to the shared game timer. */
  sigemptyset(&alarm);
  sigaddset(&alarm,SIGALRM);
  sigprocmask(SIG_BLOCK,&alarm,&previous);
  if (!SDL_Init(SDL_INIT_VIDEO)) fatal_error("%s",SDL_GetError());
  if (!scale) {
    SDL_Rect bounds;
    scale=1;
    if (SDL_GetDisplayUsableBounds(SDL_GetPrimaryDisplay(),&bounds))
      scale=SDL_max(1,SDL_min(bounds.w/WINDOW_WIDTH,bounds.h/WINDOW_HEIGHT));
  }
  window=sdl_video_open(scale,fullscreen);
  if (data.sound) sound_open();
  sigprocmask(SIG_SETMASK,&previous,NULL);
  display.mapped=1;
  timer_open();
  while (!global.quit) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) handle_event(window,&event);
    if (global.quit) break;
    if (!display.mapped || (global.pressed & (1<<KEY_PAUSE)) ||
        (global.state==MODE_KEY_DEF && !global.key)) {
      if (SDL_WaitEvent(&event)) handle_event(window,&event);
      continue;
    }
    if (global.pressed & (1<<KEY_ICONIZE)) {
      global.pressed &= ~(1<<KEY_ICONIZE);
      SDL_MinimizeWindow(window);
    } else {
      game_tick();
      video_sync();
      timer_wait();
    }
  }
  timer_close();
  sound_close();
  cleanup_scores();
  SDL_Quit();
  return 0;
}
