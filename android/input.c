#include "xmris.h"
#include "input.h"

/* Direction indices follow default_bindings(). One steering finger can coexist
 * with a throwing finger. Latch taps until the game has sampled a frame. */
static int direction = -1, steering, throw_pending, calibrated;
static SDL_FingerID finger;
static float origin_x, origin_y, neutral_x, neutral_y;
static SDL_Sensor *sensor;

static void steer(int next)
{
  if (next == direction) return;
  if (direction >= 0) game_keyrelease(data.keysyms[direction]);
  direction = next;
  if (next >= 0) game_keypress(data.keysyms[next]);
}

void phone_after_tick(void)
{
  if (throw_pending) {
    game_keyrelease(data.keysyms[KEY_THROW]);
    throw_pending = 0;
  }
}

void phone_reset(void)
{
  steer(-1);
  steering = calibrated = 0;
  phone_after_tick();
  if (sensor) {
    SDL_CloseSensor(sensor);
    SDL_QuitSubSystem(SDL_INIT_SENSOR);
  }
  sensor = NULL;
}

static void toggle_tilt(void)
{
  int count;
  int was_enabled = sensor != NULL;
  phone_reset();
  if (was_enabled || !SDL_InitSubSystem(SDL_INIT_SENSOR)) return;
  SDL_SensorID *ids = SDL_GetSensors(&count);
  for (int i = 0; ids && i < count && !sensor; i++)
    if (SDL_GetSensorTypeForID(ids[i]) == SDL_SENSOR_ACCEL)
      sensor = SDL_OpenSensor(ids[i]);
  SDL_free(ids);
  if (!sensor) SDL_QuitSubSystem(SDL_INIT_SENSOR);
}
static void vector(float x, float y, float dead)
{
  float ax = SDL_fabsf(x), ay = SDL_fabsf(y);
  if (SDL_max(ax,ay) < dead) steer(-1);
  else if (ax > ay * 1.15f) steer(x < 0 ? 2 : 3);
  else if (ay > ax * 1.15f) steer(y < 0 ? 0 : 1);
}

void phone_event(SDL_Window *window, const SDL_Event *event)
{
  const SDL_TouchFingerEvent *f = &event->tfinger;
  int w, h;
  switch (event->type) {
  case SDL_EVENT_FINGER_DOWN:
    if (f->y < 0.10f) toggle_tilt();
    else if (f->x >= 0.5f) {
      game_keypress(data.keysyms[KEY_THROW]);
      throw_pending = 1;
    } else if (!steering && !sensor) {
      steering = 1;
      finger = f->fingerID;
      origin_x = f->x;
      origin_y = f->y;
    }
    break;
  case SDL_EVENT_FINGER_MOTION:
    if (steering && finger == f->fingerID && SDL_GetWindowSize(window,&w,&h))
      vector((f->x-origin_x)*w,(f->y-origin_y)*h,0.025f*w);
    break;
  case SDL_EVENT_FINGER_UP:
  case SDL_EVENT_FINGER_CANCELED:
    if (steering && finger == f->fingerID) {
      steering = 0;
      steer(-1);
    }
    break;
  case SDL_EVENT_SENSOR_UPDATE:
    if (sensor && event->sensor.which == SDL_GetSensorID(sensor)) {
      const float *v = event->sensor.data;
      if (!calibrated) {
        neutral_x = v[0];
        neutral_y = v[1];
        calibrated = 1;
      }
      vector(neutral_x-v[0],v[1]-neutral_y,1.2f);
    }
    break;
  case SDL_EVENT_WINDOW_FOCUS_LOST:
    phone_reset();
    break;
  }
}
