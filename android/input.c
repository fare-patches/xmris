#include "xmris.h"
#include "input.h"

/* The indices of the four movement bindings are up, down, left, right. */
static int direction = -1, steering;
static SDL_FingerID finger;
static SDL_FPoint anchor;
static KeySym release_key;

enum direction_index { UP, DOWN, LEFT, RIGHT };

/* Work in drawable pixels. Android can change the usable area while playing. */
struct layout { SDL_FRect board, controls; float row; };

static struct layout phone_layout(SDL_Renderer *r)
{
  SDL_Rect area = {0};
  SDL_GetRenderSafeArea(r,&area);
  float scale = SDL_min((float)area.w/WINDOW_WIDTH,
      (float)area.h*2/(3*WINDOW_HEIGHT));
  float w = WINDOW_WIDTH*scale, h = WINDOW_HEIGHT*scale;
  SDL_FRect board = {area.x+(area.w-w)/2,area.y,w,h};
  SDL_FRect controls = {area.x,area.y+h,area.w,area.h-h};
  float toolbar = SDL_min(48*scale,controls.h/4);
  return (struct layout){board,controls,(controls.h-toolbar)/3};
}

SDL_FRect phone_board(SDL_Renderer *r) { return phone_layout(r).board; }

static void button(SDL_Renderer *r, struct layout layout, int row, int column,
    int count, const char *label, int active)
{
  SDL_FRect rect = layout.controls;
  rect.w /= count;
  rect.x += column*rect.w;
  rect.y += row*layout.row;
  if (global.state != MODE_KEY_DEF)
    rect.h = row == 3 ? rect.h-3*layout.row : row == 1 ? 2*layout.row : layout.row;
  rect.x++; rect.y++; rect.w-=2; rect.h-=2;
  SDL_SetRenderDrawColor(r, active ? 40 : 32, active ? 104 : 47, active ? 115 : 59, 255);
  SDL_RenderFillRect(r, &rect);
  SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
  float sx, sy;
  SDL_GetRenderScale(r,&sx,&sy);
  float scale = SDL_max(1, SDL_floorf(SDL_min(rect.h/15,rect.w/(8*(strlen(label)+1)))));
  SDL_SetRenderScale(r,sx*scale,sy*scale);
  SDL_RenderDebugText(r,(rect.x+(rect.w-strlen(label)*8*scale)/2)/scale,
      (rect.y+(rect.h-8*scale)/2)/scale,label);
  SDL_SetRenderScale(r,sx,sy);
}

void phone_draw(SDL_Renderer *r)
{
  static const char *const labels[] = {"Up","Down","Left","Right"};
  static const char *const menu[] = {"Up", "Down", "Select", "Back"};
  struct layout layout = phone_layout(r);
  if (global.state == MODE_KEY_DEF) {
    for (int i=0; i<4; i++) button(r,layout,0,i,4,menu[i],0);
    return;
  }
  button(r,layout,0,0,3,in_attract_mode ? "Start" : "Throw",0);
  button(r,layout,0,1,3,"Swipe",0);
  button(r,layout,0,2,3,in_attract_mode ? "Start" : "Throw",0);
  button(r,layout,1,0,1,direction < 0 ? "Swipe to steer" : labels[direction],direction >= 0);
  button(r,layout,3,0,1,in_attract_mode ? "Options" : "Pause",0);
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
  if (event->type == SDL_EVENT_FINGER_UP || event->type == SDL_EVENT_FINGER_CANCELED) {
    if (steering && finger == f->fingerID) { steer(-1); steering=0; }
    return;
  }
  int w,h;
  float x,y;
  if (!SDL_GetWindowSize(window,&w,&h) ||
      !SDL_RenderCoordinatesFromWindow(SDL_GetRenderer(window),f->x*w,f->y*h,&x,&y)) return;
  struct layout layout = phone_layout(SDL_GetRenderer(window));
  SDL_FRect controls = layout.controls;
  SDL_FPoint point = {x,y};
  int inside = SDL_PointInRectFloat(&point,&controls);
  int board = SDL_PointInRectFloat(&point,&layout.board);
  float fraction = inside ? (x-controls.x)/controls.w : 0;
  int row = inside ? SDL_min(3,(int)((y-controls.y)/layout.row)) : -1;
  int column = SDL_min(2,(int)(3*fraction));
  if (event->type == SDL_EVENT_FINGER_DOWN) {
    if (!inside && !board) return;
    if (global.state == MODE_KEY_DEF) {
      const KeySym menu[] = {SDLK_UP,SDLK_DOWN,SDLK_RETURN,SDLK_ESCAPE};
      if (inside) game_keypress(menu[SDL_min(3,(int)(4*fraction))]);
      return;
    }
    if (row == 3) {
      if (in_attract_mode) tap(data.keysyms[KEY_KEYBOARD]);
      else global.pause = 1;
      return;
    }
    game_wake();
    if ((board && !inside) || (row == 0 && column != 1)) {
      tap(data.keysyms[KEY_THROW]);
      return;
    }
    /* The newest direction owns steering, including its eventual release. */
    steering = 1;
    finger = f->fingerID;
    anchor = point;
    steer(-1);
  } else if (steering && finger == f->fingerID) {
    float dx = x-anchor.x, dy = y-anchor.y;
    float ax = SDL_fabsf(dx), ay = SDL_fabsf(dy);
    float threshold = controls.w/32;
    if (SDL_max(ax,ay) < threshold) return;
    if (ax > ay*1.2f) steer(dx < 0 ? LEFT : RIGHT);
    else if (ay > ax*1.2f) steer(dy < 0 ? UP : DOWN);
    else return;
    anchor = point;
  }
}
