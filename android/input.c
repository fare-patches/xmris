#include "xmris.h"
#include "input.h"

/* The indices of the four movement bindings are up, down, left, right. */
static int direction = -1, steering, confirming_quit;
static SDL_FingerID finger;
static SDL_FPoint anchor, pending_start;
static SDL_FingerID pending_finger;
static int pending;
/* Action strips distinguish a tap from the start of a swipe. */
static struct strip_touch {
  int active;
  SDL_FingerID finger;
  SDL_FPoint start;
} strips[2];
static SDL_FingerID newest_touch;
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
  if (global.state == MODE_KEY_DEF) {
    if (column == 1) rect.h /= 2;
    rect.y += row*rect.h;
  } else {
    rect.y += row*layout.row;
    rect.h = row == 3 ? rect.h-3*layout.row : row == 0 ? 2*layout.row : layout.row;
  }
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

static const struct { const char *label; KeySym key; int row, column; } menu[] = {
  {"Back", SDLK_ESCAPE, 0, 0}, {"Up", SDLK_UP, 0, 1},
  {"Down", SDLK_DOWN, 1, 1}, {"Select", SDLK_RETURN, 0, 2}
};

void phone_draw(SDL_Renderer *r)
{
  static const char *const labels[] = {"Up","Down","Left","Right"};
  struct layout layout = phone_layout(r);
  if (global.state == MODE_KEY_DEF) {
    for (int i=0; i<4; i++) button(r,layout,menu[i].row,menu[i].column,3,menu[i].label,0);
    return;
  }
  if (game_is_paused()) {
    button(r,layout,0,0,1,"Resume",0);
    if (confirming_quit)
      button(r,layout,2,0,1,"Confirm end game",0);
    button(r,layout,3,0,1,confirming_quit ? "Cancel" : "End game...",0);
    return;
  }
  button(r,layout,0,0,1,direction < 0 ? "Swipe to steer" : labels[direction],direction >= 0);
  button(r,layout,2,0,1,in_attract_mode ? "Start" : "Throw",0);
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
  if (!game_is_paused()) confirming_quit = 0;
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
  steering = pending = confirming_quit = 0;
  strips[0].active = strips[1].active = 0;
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
  struct layout layout = phone_layout(SDL_GetRenderer(window));
  SDL_FRect controls = layout.controls;
  SDL_FPoint point = {x,y};
  int inside = SDL_PointInRectFloat(&point,&controls);
  float threshold = controls.w/32;
  float fraction = inside ? (x-controls.x)/controls.w : 0;
  int row = inside ? SDL_min(3,(int)((y-controls.y)/layout.row)) : -1;
  int column = SDL_min(2,(int)(3*fraction));
  for (int i=0;i<2;i++) {
    struct strip_touch *touch = &strips[i];
    if (!touch->active || touch->finger != f->fingerID) continue;
    float distance = SDL_max(SDL_fabsf(x-touch->start.x), SDL_fabsf(y-touch->start.y));
    if (event->type == SDL_EVENT_FINGER_MOTION && distance < threshold) return;
    touch->active = 0;
    if (event->type == SDL_EVENT_FINGER_UP && distance < threshold && row == i+2 &&
        !game_is_paused() && global.state != MODE_KEY_DEF) {
      if (i == 0) tap(data.keysyms[KEY_THROW]);
      else if (in_attract_mode) tap(data.keysyms[KEY_KEYBOARD]);
      else { steer(-1); steering=0; global.pause=1; }
    } else if (event->type == SDL_EVENT_FINGER_MOTION && newest_touch == f->fingerID &&
               !game_is_paused() && global.state != MODE_KEY_DEF) {
      steering=1; finger=f->fingerID; anchor=touch->start;
    }
    break;
  }
  if (event->type == SDL_EVENT_FINGER_UP || event->type == SDL_EVENT_FINGER_CANCELED) {
    if (pending && pending_finger == f->fingerID) pending = 0;
    if (steering && finger == f->fingerID) {
      if (pending && event->type == SDL_EVENT_FINGER_UP) {
        finger = pending_finger; anchor = pending_start; pending = 0;
      } else { steer(-1); steering=0; }
    }
    return;
  }
  if (event->type == SDL_EVENT_FINGER_DOWN) {
    if (global.state == MODE_KEY_DEF) {
      if (inside) {
        int lower = y >= controls.y + controls.h/2;
        int menu_row = column == 1 && lower;
        for (int i=0; i<4; i++)
          if (menu[i].column == column && menu[i].row == menu_row)
            game_keypress(menu[i].key);
      }
      return;
    }
    if (game_is_paused()) {
      if (row == 3) {
        confirming_quit = !confirming_quit;
        refresh_window();
      } else if (row == 2 && confirming_quit) {
        tap(data.keysyms[KEY_QUIT]);
      } else {
        game_wake();
      }
      return;
    }
    newest_touch = f->fingerID;
    pending = 0;
    if (row >= 2) {
      strips[row-2] = (struct strip_touch){1, f->fingerID, point};
      return;
    }
    if (steering) {
      pending = 1; pending_finger = f->fingerID; pending_start = point;
      return;
    }
    steering = 1;
    finger = f->fingerID;
    anchor = point;
  } else if (!game_is_paused() && steering) {
    if (pending && pending_finger == f->fingerID) {
      if (SDL_max(SDL_fabsf(x-pending_start.x),SDL_fabsf(y-pending_start.y)) < threshold)
        return;
      finger = pending_finger; anchor = pending_start; pending = 0;
    }
    if (finger != f->fingerID) return;
    float dx = x-anchor.x, dy = y-anchor.y;
    float ax = SDL_fabsf(dx), ay = SDL_fabsf(dy);
    /* Track forward travel continuously so a turn starts at the thumb tip. */
    if ((direction == RIGHT && dx > 0) || (direction == LEFT && dx < 0)) {
      anchor.x = x;
      if (ax > 2*ay) { anchor.y = y; ay = 0; }
      ax = 0;
    } else if ((direction == DOWN && dy > 0) || (direction == UP && dy < 0)) {
      anchor.y = y;
      if (ay > 2*ax) { anchor.x = x; ax = 0; }
      ay = 0;
    }
    if (SDL_max(ax,ay) < threshold) return;
    /* Keep a short ambiguous diagonal quiet, but resolve a deliberate stroke. */
    if (SDL_max(ax,ay) < 1.5f*threshold && ax < ay*1.2f && ay < ax*1.2f) return;
    if (ax >= ay) steer(dx < 0 ? LEFT : RIGHT);
    else steer(dy < 0 ? UP : DOWN);
    anchor = point;
  }
}
