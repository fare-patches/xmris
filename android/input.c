#include "xmris.h"
#include "input.h"

/* The indices of the four movement bindings are up, down, left, right. */
static int direction = -1, steering;
static SDL_FingerID finger;
static KeySym release_key;

enum pad_action { UP, DOWN, LEFT, RIGHT, STOP, THROW };
static const enum pad_action pad[3][3] = {
  {THROW, UP, THROW}, {LEFT, STOP, RIGHT}, {STOP, DOWN, STOP}
};

static void button(SDL_Renderer *r, int row, int column, int count, const char *label, int active)
{
  float width = (float)WINDOW_WIDTH/count;
  int height = row == 3 ? PHONE_TOOLBAR : PHONE_ROW;
  SDL_FRect rect = {column*width+1, WINDOW_HEIGHT+row*PHONE_ROW+1, width-2, height-2};
  SDL_SetRenderDrawColor(r, active ? 40 : 32, active ? 104 : 47, active ? 115 : 59, 255);
  SDL_RenderFillRect(r, &rect);
  SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
  float sx, sy;
  SDL_GetRenderScale(r,&sx,&sy);
  float scale = strlen(label)*16+8 <= rect.w ? 2 : 1;
  SDL_SetRenderScale(r,sx*scale,sy*scale);
  SDL_RenderDebugText(r,(rect.x+(rect.w-strlen(label)*8*scale)/2)/scale,
      (rect.y+(rect.h-8*scale)/2)/scale,label);
  SDL_SetRenderScale(r,sx,sy);
}

void phone_draw(SDL_Renderer *r)
{
  static const char *const labels[] = {"Up","Down","Left","Right","Stop","Throw"};
  static const char *const menu[] = {"Up", "Down", "Select", "Back"};
  if (global.state == MODE_KEY_DEF) {
    for (int i=0; i<4; i++) button(r,0,i,4,menu[i],0);
    return;
  }
  for (int row=0; row<3; row++)
    for (int col=0; col<3; col++) {
      int action = pad[row][col];
      button(r,row,col,3,action == THROW && in_attract_mode ? "Start" : labels[action],
          action == direction);
    }
  button(r,3,0,1,in_attract_mode ? "Options" : "Pause",0);
}

static void steer(int next)
{
  if (next == direction) return;
  if (direction >= 0) game_keyrelease(data.keysyms[direction]);
  direction = next;
  if (next >= 0) game_keypress(data.keysyms[next]);
}

void phone_after_tick(void)
{
  if (release_key) {
    game_keyrelease(release_key);
    release_key = 0;
  }
}

static void tap(KeySym key)
{
  phone_after_tick();
  release_key = key;
  game_keypress(key);
}

void phone_reset(void)
{
  steer(-1);
  steering = 0;
  phone_after_tick();
}

void phone_event(SDL_Window *window, const SDL_Event *event)
{
  if (event->type == SDL_EVENT_WINDOW_FOCUS_LOST) { phone_reset(); return; }
  if (event->type != SDL_EVENT_FINGER_DOWN && event->type != SDL_EVENT_FINGER_UP &&
      event->type != SDL_EVENT_FINGER_MOTION && event->type != SDL_EVENT_FINGER_CANCELED) return;
  const SDL_TouchFingerEvent *f = &event->tfinger;
  int w,h;
  float x,y;
  if (!SDL_GetWindowSize(window,&w,&h) ||
      !SDL_RenderCoordinatesFromWindow(SDL_GetRenderer(window),f->x*w,f->y*h,&x,&y)) return;
  int inside = x>=0 && x<WINDOW_WIDTH && y>=0 && y<WINDOW_HEIGHT+PHONE_BAR_HEIGHT;
  int row = y < WINDOW_HEIGHT ? -1 : (int)((y-WINDOW_HEIGHT)/PHONE_ROW);
  int col = (int)(4*x/WINDOW_WIDTH);
  if (event->type == SDL_EVENT_FINGER_DOWN) {
    if (!inside) return;
    if (global.state == MODE_KEY_DEF) {
      const KeySym menu[] = {SDLK_UP,SDLK_DOWN,SDLK_RETURN,SDLK_ESCAPE};
      if (row == 0) game_keypress(menu[col]);
      return;
    }
    if (row == 3) {
      if (in_attract_mode) tap(data.keysyms[KEY_KEYBOARD]);
      else global.pause = 1;
      return;
    }
    int action = row < 0 ? THROW : pad[row][(int)(3*x/WINDOW_WIDTH)];
    if (action == THROW) {
      tap(data.keysyms[KEY_THROW]);
      return;
    }
    if (steering && action != STOP) return;
    steering = 1;
    finger = f->fingerID;
    steer(action == STOP ? -1 : action);
  } else if (steering && finger == f->fingerID) {
    int next = -1;
    if (event->type == SDL_EVENT_FINGER_MOTION && inside && row >= 0 && row < 3) {
      int action = pad[row][(int)(3*x/WINDOW_WIDTH)];
      if (action < STOP) next = action;
    }
    steer(next);
    if (event->type != SDL_EVENT_FINGER_MOTION) steering=0;
  }
}
