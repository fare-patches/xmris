#include "xmris.h"
#include "sound.h"
#if defined(__ANDROID__) || defined(SDL_PLATFORM_IOS)
#include <SDL3/SDL_main.h>
#include "android/input.h"
#endif
#ifdef _WIN32
#include <locale.h>
#else
#include <signal.h>
#endif

extern SDL_Window *sdl_video_open(int, int, SDL_RendererLogicalPresentation);

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
/* Use X keysym spellings for the common cross-backend controls. */
static const struct { KeySym key; const char *name; } key_names[] = {
  {SDLK_SPACE,"space"}, {SDLK_RETURN,"Return"}, {SDLK_ESCAPE,"Escape"},
  {SDLK_LSHIFT,"Shift_L"}, {SDLK_RSHIFT,"Shift_R"},
  {SDLK_LCTRL,"Control_L"}, {SDLK_RCTRL,"Control_R"},
  {SDLK_LALT,"Alt_L"}, {SDLK_RALT,"Alt_R"},
  {SDLK_EXCLAIM,"exclam"},
  {SDLK_DBLAPOSTROPHE,"quotedbl"},
  {SDLK_HASH,"numbersign"},
  {SDLK_DOLLAR,"dollar"},
  {SDLK_PERCENT,"percent"},
  {SDLK_AMPERSAND,"ampersand"},
  {SDLK_APOSTROPHE,"apostrophe"},
  {SDLK_LEFTPAREN,"parenleft"},
  {SDLK_RIGHTPAREN,"parenright"},
  {SDLK_ASTERISK,"asterisk"},
  {SDLK_PLUS,"plus"},
  {SDLK_COMMA,"comma"},
  {SDLK_MINUS,"minus"},
  {SDLK_PERIOD,"period"},
  {SDLK_SLASH,"slash"},
  {SDLK_COLON,"colon"},
  {SDLK_SEMICOLON,"semicolon"},
  {SDLK_LESS,"less"},
  {SDLK_EQUALS,"equal"},
  {SDLK_GREATER,"greater"},
  {SDLK_QUESTION,"question"},
  {SDLK_AT,"at"},
  {SDLK_LEFTBRACKET,"bracketleft"},
  {SDLK_BACKSLASH,"backslash"},
  {SDLK_RIGHTBRACKET,"bracketright"},
  {SDLK_CARET,"asciicircum"},
  {SDLK_UNDERSCORE,"underscore"},
  {SDLK_GRAVE,"grave"},
  {SDLK_LEFTBRACE,"braceleft"},
  {SDLK_PIPE,"bar"},
  {SDLK_RIGHTBRACE,"braceright"},
  {SDLK_TILDE,"asciitilde"},
  {SDLK_KP_DIVIDE,"KP_Divide"},
  {SDLK_KP_MULTIPLY,"KP_Multiply"},
  {SDLK_KP_MINUS,"KP_Subtract"},
  {SDLK_KP_PLUS,"KP_Add"},
  {SDLK_KP_ENTER,"KP_Enter"},
  {SDLK_KP_PERIOD,"KP_Decimal"},
  {SDLK_KP_EQUALS,"KP_Equal"},
  {SDLK_KP_COMMA,"KP_Separator"},
  {SDLK_PAGEUP,"Prior"},
  {SDLK_PAGEDOWN,"Next"},
  {SDLK_BACKSPACE,"BackSpace"},
  {SDLK_PRINTSCREEN,"Print"},
  {SDLK_CAPSLOCK,"Caps_Lock"},
  {SDLK_NUMLOCKCLEAR,"Num_Lock"},
  {SDLK_SCROLLLOCK,"Scroll_Lock"},
  {SDLK_LGUI,"Super_L"},
  {SDLK_RGUI,"Super_R"},
  {SDLK_APPLICATION,"Menu"},
  {SDLK_KP_0,"KP_0"},
  {SDLK_KP_1,"KP_1"},
  {SDLK_KP_2,"KP_2"},
  {SDLK_KP_3,"KP_3"},
  {SDLK_KP_4,"KP_4"},
  {SDLK_KP_5,"KP_5"},
  {SDLK_KP_6,"KP_6"},
  {SDLK_KP_7,"KP_7"},
  {SDLK_KP_8,"KP_8"},
  {SDLK_KP_9,"KP_9"},
  /* Read keypad navigation names saved by older X11 versions. */
  {SDLK_KP_0,"KP_Insert"},
  {SDLK_KP_1,"KP_End"},
  {SDLK_KP_2,"KP_Down"},
  {SDLK_KP_3,"KP_Next"},
  {SDLK_KP_4,"KP_Left"},
  {SDLK_KP_5,"KP_Begin"},
  {SDLK_KP_6,"KP_Right"},
  {SDLK_KP_7,"KP_Home"},
  {SDLK_KP_8,"KP_Up"},
  {SDLK_KP_9,"KP_Prior"},
  {SDLK_KP_PERIOD,"KP_Delete"}
};
void default_bindings(KeySym *keys)
{
  const KeySym defaults[KEYS]={SDLK_UP,SDLK_DOWN,SDLK_LEFT,SDLK_RIGHT,SDLK_SPACE,SDLK_P,SDLK_Q,SDLK_I,SDLK_K};
  memcpy(keys,defaults,sizeof(defaults));
}

const char *portable_key_name(KeySym key)
{
  static char ascii[2];
  if (key >= 'a' && key <= 'z') { ascii[0]=key; return ascii; }
  for (unsigned i=0;i<XtNumber(key_names);++i)
    if (key_names[i].key==key) return key_names[i].name;
  return key_name(key);
}

static void load_keys(void)
{
  const char *const actions[]={"up","down","left","right","throw","pause","quit","iconize","keyboard"};
  for (unsigned i=0;i<KEYS;++i) {
    const char *name=option_default(actions[i]);
    if (!name) continue;
    KeySym key=SDL_GetKeyFromName(name);
    for (unsigned j=0;j<XtNumber(key_names);++j)
      if (!strcmp(key_names[j].name,name)) key=key_names[j].key;
    if (key) data.keysyms[i]=key;
  }
}
void set_key_default(void) {}
void set_key_single(void) {}
void color_cycle(void) {}
void color_set(int type) { (void)type; }

char *garden_resource(const char *name, const char *suffix)
{
  const char *base = SDL_GetBasePath();
  if (!base) return NULL;
  /* Support a source checkout and the installed bin/ and share/ layout. */
  const char *dirs[] = {"", "../share/xmris/"};
  for (unsigned i = 0; i < XtNumber(dirs); ++i) {
    char *path = malloc(strlen(base) + strlen(dirs[i]) + strlen(name) +
                        (suffix ? strlen(suffix) : 0) + 1);
    if (!path) return NULL;
    sprintf(path, "%s%s%s%s", base, dirs[i], name, suffix ? suffix : "");
    if (!access(path, R_OK)) return path;
    free(path);
  }
  return NULL;
}

/* Use SDL's mappings for gamepads and conventional axes for arcade sticks. */
static SDL_Gamepad *pad;
static SDL_Joystick *stick;
static SDL_JoystickID controller_id;
static KeySym held_move, held_fire, held_pause;
static int axes[2], hat, last_axis;

static void controller_key(KeySym *held, KeySym next)
{
  if (*held == next) return;
  if (*held) game_keyrelease(*held);
  *held = next;
  if (next) game_keypress(next);
}

static void reset_controller(void)
{
  controller_key(&held_move,0);
  controller_key(&held_fire,0);
  controller_key(&held_pause,0);
  axes[0] = axes[1] = hat = last_axis = 0;
}

static void open_controller(SDL_JoystickID id)
{
  if (controller_id) return;
  if (SDL_IsGamepad(id)) pad = SDL_OpenGamepad(id);
  if (!pad) stick = SDL_OpenJoystick(id);
  if (pad || stick) controller_id = id;
}

static void close_controller(void)
{
  reset_controller();
  if (pad) SDL_CloseGamepad(pad);
  if (stick) SDL_CloseJoystick(stick);
  pad = NULL;
  stick = NULL;
  controller_id = 0;
}

static void controller_move(void)
{
  static const KeySym arrows[] = {SDLK_UP,SDLK_DOWN,SDLK_LEFT,SDLK_RIGHT};
  int x = !!(hat & SDL_HAT_RIGHT) - !!(hat & SDL_HAT_LEFT);
  int y = !!(hat & SDL_HAT_DOWN) - !!(hat & SDL_HAT_UP);
  if (!hat) {
    x = axes[0];
    y = axes[1];
    if (SDL_abs(x) < 12000) x = 0;
    if (SDL_abs(y) < 12000) y = 0;
  }
  int dir = -1;
  if (x && (SDL_abs(x)>SDL_abs(y) || (SDL_abs(x)==SDL_abs(y) && last_axis==0)))
    dir = x<0 ? 2 : 3;
  else if (y) dir = y<0 ? 0 : 1;
  KeySym key = 0;
  if (dir >= 0) key = global.state == MODE_KEY_DEF ? arrows[dir] : data.keysyms[dir];
  controller_key(&held_move,key);
}

static void controller_button(int button, int down)
{
  if (pad) {
    static const int bits[] = {SDL_HAT_UP, SDL_HAT_DOWN, SDL_HAT_LEFT, SDL_HAT_RIGHT};
    if (button >= SDL_GAMEPAD_BUTTON_DPAD_UP && button <= SDL_GAMEPAD_BUTTON_DPAD_RIGHT) {
      int bit = bits[button-SDL_GAMEPAD_BUTTON_DPAD_UP];
      if (down) { hat |= bit; last_axis = !(bit & (SDL_HAT_LEFT|SDL_HAT_RIGHT)); }
      else hat &= ~bit;
      controller_move();
      return;
    }
  }
  if (button == (pad ? SDL_GAMEPAD_BUTTON_SOUTH : 0))
    controller_key(&held_fire,down ? (global.state==MODE_KEY_DEF ? SDLK_RETURN : data.keysyms[KEY_THROW]) : 0);
  if (button == (pad ? SDL_GAMEPAD_BUTTON_START : 1))
    controller_key(&held_pause,down ? (global.state==MODE_KEY_DEF ? SDLK_ESCAPE :
        in_attract_mode ? data.keysyms[KEY_KEYBOARD] : data.keysyms[KEY_PAUSE]) : 0);
}

static void controller_event(const SDL_Event *e)
{
  switch (e->type) {
    case SDL_EVENT_JOYSTICK_ADDED: open_controller(e->jdevice.which); break;
    case SDL_EVENT_JOYSTICK_REMOVED:
      if (e->jdevice.which == controller_id) {
        close_controller();
        int count;
        SDL_JoystickID *ids = SDL_GetJoysticks(&count);
        for (int i=0; ids && i<count && !controller_id; i++) open_controller(ids[i]);
        SDL_free(ids);
      }
      break;
    case SDL_EVENT_GAMEPAD_AXIS_MOTION:
    case SDL_EVENT_JOYSTICK_AXIS_MOTION: {
      int mapped = e->type == SDL_EVENT_GAMEPAD_AXIS_MOTION;
      if (mapped != !!pad || (mapped ? e->gaxis.which : e->jaxis.which) != controller_id) break;
      int axis = mapped ? e->gaxis.axis : e->jaxis.axis;
      if (axis>1) break;
      axes[axis] = mapped ? e->gaxis.value : e->jaxis.value;
      last_axis = axis;
      controller_move();
      break;
    }
    case SDL_EVENT_JOYSTICK_HAT_MOTION:
      if (stick && e->jhat.which==controller_id && !e->jhat.hat) {
        hat=e->jhat.value;
        controller_move();
      }
      break;
    case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
    case SDL_EVENT_GAMEPAD_BUTTON_UP:
      if (pad && e->gbutton.which==controller_id) controller_button(e->gbutton.button,e->gbutton.down);
      break;
    case SDL_EVENT_JOYSTICK_BUTTON_DOWN:
    case SDL_EVENT_JOYSTICK_BUTTON_UP:
      if (stick && e->jbutton.which==controller_id) controller_button(e->jbutton.button,e->jbutton.down);
      break;
    case SDL_EVENT_WINDOW_FOCUS_LOST: reset_controller(); break;
  }
}

static void handle_event(SDL_Window *window, const SDL_Event *event)
{
  static int resume_timer;
  controller_event(event);
#if defined(__ANDROID__) || defined(SDL_PLATFORM_IOS)
  phone_event(window,event);
#endif
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
  int scale=0, fullscreen=0;
  SDL_RendererLogicalPresentation mode = SDL_LOGICAL_PRESENTATION_INTEGER_SCALE;
  SDL_Window *window;
#if !defined(_WIN32) && !defined(__ANDROID__) && !defined(SDL_PLATFORM_APPLE)
  sigset_t alarm, previous;
#endif
#ifdef _WIN32
  setlocale(LC_CTYPE, ".UTF8");
#endif
#if defined(__ANDROID__) || defined(SDL_PLATFORM_IOS)
  fullscreen=1;
  SDL_SetHint(SDL_HINT_ORIENTATIONS, "Portrait");
  mode=SDL_LOGICAL_PRESENTATION_LETTERBOX;
#endif
  myname=argv[0];
  init_scores_early();
  default_bindings(data.keysyms);
  data.music=data.sound=True;
  options_read();
  const char *difficulty=option_default("diamondDifficulty");
  data.diamond_mask=difficulty && !strcmp(difficulty,"classic") ? 0 :
      difficulty && !strcmp(difficulty,"hard") ? 7 : 1;
  data.effects=!option_default("effects") || strcmp(option_default("effects"),"false")!=0;
  data.music=!option_default("music") || strcmp(option_default("music"),"false")!=0;
  for (int i=1;i<argc;++i) {
    if (!strcmp(argv[i],"-fullscreen")) fullscreen=1;
    else if (!strcmp(argv[i],"-nosound")) data.sound=False;
    else if (!strcmp(argv[i],"-music")) data.music=True;
    else if (!strcmp(argv[i],"-nomusic")) data.music=False;
    else if (!strcmp(argv[i],"-random")) data.random=True;
    else if (!strcmp(argv[i],"-gardens") && i+1<argc) data.boards=argv[++i];
    else if (!strcmp(argv[i],"-scale") && i+1<argc) {
      char *end;
      const char *value=argv[++i];
      mode = SDL_LOGICAL_PRESENTATION_INTEGER_SCALE;
      if (!strcmp(value,"fit")) {
        scale = 0;
        mode = SDL_LOGICAL_PRESENTATION_LETTERBOX;
      } else if (!strcmp(value,"auto")) scale=0;
      else {
        long factor=strtol(value,&end,10);
        if (*end || factor<1 || factor>32767/WINDOW_HEIGHT) fatal_error("invalid scale %s",value);
        scale=factor;
      }
    } else {
      fprintf(stderr,"Usage: %s [-fullscreen] [-scale auto|fit|N] [-nosound] [-music|-nomusic] [-random] [-gardens FILE]\n",myname);
      return strcmp(argv[i],"-help")!=0;
    }
  }
  init_scores();
  create_boards();
#if !defined(_WIN32) && !defined(__ANDROID__) && !defined(SDL_PLATFORM_APPLE)
  /* Worker threads must leave SIGALRM to the shared game timer. */
  sigemptyset(&alarm);
  sigaddset(&alarm,SIGALRM);
  sigprocmask(SIG_BLOCK,&alarm,&previous);
#endif
  SDL_SetAppMetadata("xmris", XMRISVERSION, NULL);
  if (!SDL_Init(SDL_INIT_VIDEO)) fatal_error("%s",SDL_GetError());
  SDL_InitSubSystem(SDL_INIT_GAMEPAD);
  /* Named keys are resolved using the active SDL keyboard layout. */
  load_keys();
  if (!scale) {
    SDL_Rect bounds;
    scale=1;
    if (SDL_GetDisplayUsableBounds(SDL_GetPrimaryDisplay(),&bounds))
      scale=SDL_max(1,SDL_min(bounds.w/WINDOW_WIDTH,bounds.h/WINDOW_HEIGHT));
  }
  window=sdl_video_open(scale,fullscreen,mode);
  if (data.sound) sound_open();
#if !defined(_WIN32) && !defined(__ANDROID__) && !defined(SDL_PLATFORM_APPLE)
  sigprocmask(SIG_SETMASK,&previous,NULL);
#endif
  display.mapped=1;
  timer_open();
  while (!global.quit) {
    SDL_Event event;
    while (!(display.mapped && global.state==MODE_KEY_DEF && global.key) && SDL_PollEvent(&event)) handle_event(window,&event);
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
#if defined(__ANDROID__) || defined(SDL_PLATFORM_IOS)
      phone_after_tick();
#endif
      video_sync();
      timer_wait();
    }
  }
  timer_close();
  sound_close();
  cleanup_scores();
#if defined(__ANDROID__) || defined(SDL_PLATFORM_IOS)
  phone_reset();
#endif
  close_controller();
  SDL_Quit();
  return 0;
}
