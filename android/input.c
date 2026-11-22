#include "xmris.h"
#include "input.h"

/* The indices of the four movement bindings are up, down, left, right. */
enum input_style { ZONES, TILT, FLICK };
static enum input_style style;
static const char *const styles[] = {"Zones", "Tilt", "Flick"};
static int direction = -1, steering, calibrated, flicked, zone_touch;
static SDL_FingerID finger;
static KeySym release_key;
static float origin_x, origin_y, neutral_x, neutral_y;
static SDL_Sensor *sensor;

const char *phone_style_name(void) { return styles[style]; }

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
  if (style == ZONES) {
    for (int row=0; row<3; row++)
      for (int col=0; col<3; col++) {
        int action = pad[row][col];
        button(r,row,col,3,action == THROW && in_attract_mode ? "Start" : labels[action],
            action == direction);
      }
  } else {
    button(r,0,0,1,style == TILT ? "Tap to center tilt" : "Flick to steer / tap to stop",0);
    button(r,1,0,1,in_attract_mode ? "Start" : "Throw",0);
  }
  char mode[20];
  snprintf(mode,sizeof(mode),"Mode: %s",phone_style_name());
  button(r,3,0,2,mode,0);
  button(r,3,1,2,in_attract_mode ? "Options" : "Pause",0);
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
  steering = calibrated = 0;
  phone_after_tick();
  if (sensor) {
    SDL_CloseSensor(sensor);
    SDL_QuitSubSystem(SDL_INIT_SENSOR);
    sensor = NULL;
  }
}

static int open_tilt(void)
{
  if (!SDL_InitSubSystem(SDL_INIT_SENSOR)) return 0;
  int count;
  SDL_SensorID *ids = SDL_GetSensors(&count);
  for (int i=0; ids && i<count && !sensor; i++)
    if (SDL_GetSensorTypeForID(ids[i]) == SDL_SENSOR_ACCEL)
      sensor = SDL_OpenSensor(ids[i]);
  SDL_free(ids);
  if (!sensor) SDL_QuitSubSystem(SDL_INIT_SENSOR);
  return sensor != NULL;
}

void phone_init(void)
{
  const char *saved = option_default("inputStyle");
  if (saved && !strcmp(saved,"Flick")) style = FLICK;
  if (saved && !strcmp(saved,"Tilt") && open_tilt()) style = TILT;
}

static void cycle_style(void)
{
  phone_reset();
  if (style == ZONES) style = open_tilt() ? TILT : FLICK;
  else style = style == TILT ? FLICK : ZONES;
  options_save(4);
  refresh_window();
}

static void vector(float x, float y, float dead, int sliding)
{
  float ax=SDL_fabsf(x), ay=SDL_fabsf(y);
  if (SDL_max(ax,ay) < dead) {
    if (!sliding) steer(-1);
  } else {
    if (ax > ay*1.12f) steer(x < 0 ? 2 : 3);
    else if (ay > ax*1.12f) steer(y < 0 ? 0 : 1);
    else return;
    if (sliding) flicked = 1;
  }
}

void phone_event(SDL_Window *window, const SDL_Event *event)
{
  if (event->type == SDL_EVENT_WINDOW_FOCUS_LOST) { phone_reset(); return; }
  if (event->type == SDL_EVENT_WINDOW_FOCUS_GAINED && style == TILT && !sensor) {
    if (!open_tilt()) style = ZONES;
  }
  if (event->type == SDL_EVENT_SENSOR_UPDATE && sensor &&
      event->sensor.which == SDL_GetSensorID(sensor) && global.state != MODE_KEY_DEF) {
    const float *v = event->sensor.data;
    if (!calibrated) { neutral_x=v[0]; neutral_y=v[1]; calibrated=1; }
    vector(neutral_x-v[0],v[1]-neutral_y,1.2f,0);
    return;
  }
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
      if (x < WINDOW_WIDTH/2) cycle_style();
      else if (in_attract_mode) tap(data.keysyms[KEY_KEYBOARD]);
      else global.pause = 1;
      return;
    }
    int action = -1;
    if (style == ZONES && row >= 0)
      action = pad[row][(int)(3*x/WINDOW_WIDTH)];
    if (action == THROW || (style != ZONES && row == 1)) {
      tap(data.keysyms[KEY_THROW]);
      return;
    }
    if (style != ZONES && row == 2) return;
    if (action == STOP) { steer(-1); steering=0; return; }
    if (row == 0 && style == TILT) { calibrated=0; steer(-1); return; }
    if (row < 0 && (style != FLICK || x >= WINDOW_WIDTH/2)) {
      tap(data.keysyms[KEY_THROW]);
      return;
    }
    if (steering) return;
    steering=1; finger=f->fingerID; origin_x=x; origin_y=y; flicked=0;
    zone_touch = style == ZONES;
    if (zone_touch) steer(action);
  } else if (steering && finger == f->fingerID) {
    if (zone_touch) {
      int next = -1;
      if (event->type == SDL_EVENT_FINGER_MOTION && inside && row >= 0 && row < 3) {
        int action = pad[row][(int)(3*x/WINDOW_WIDTH)];
        if (action < STOP) next = action;
      }
      steer(next);
    }
    else if (event->type == SDL_EVENT_FINGER_CANCELED) steer(-1);
    else {
      vector(x-origin_x,y-origin_y,12,1);
      if (event->type == SDL_EVENT_FINGER_UP && !flicked) steer(-1);
    }
    if (event->type != SDL_EVENT_FINGER_MOTION) steering=0;
  }
}
