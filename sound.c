#include "sound.h"
#include <SDL2/SDL_audio.h>
#include <SDL2/SDL.h>
#include <signal.h>
#include <stdio.h>

enum { SAMPLE_RATE = 24000 };
static SDL_AudioDeviceID device;

/* Each entry is a pitch in Hz. Zero is a rest. Each pattern has its
 * own step length so the music and the short event cues can overlap. */
struct pattern {
  const unsigned short *notes;
  unsigned count, step_ms;
};

struct voice {
  const struct pattern *pattern;
  unsigned index, sample;
  double phase;
};

#define SCORE(notes, ms) {notes, sizeof(notes) / sizeof(notes[0]), ms}

static const unsigned short garden[] = {
  330, 392, 0, 440, 392, 330, 294, 0, 262, 330, 392, 0, 349, 294, 247, 294
};
static const unsigned short prize[] = {
  440, 0, 523, 440, 622, 523, 440, 370, 415, 494, 587, 494, 415, 0, 370, 415
};
static const unsigned short chase[] = {
  294, 349, 440, 294, 330, 392, 494, 330, 349, 440, 523, 440, 392, 330, 294, 247
};
static const struct pattern themes[] = {
  {NULL, 0, 0}, SCORE(garden, 155), SCORE(prize, 115), SCORE(chase, 100)
};

static const unsigned short throw_notes[] = {880, 659, 440};
static const unsigned short hit[] = {784, 392, 196, 98};
static const unsigned short crush[] = {165, 98, 73, 55};
static const unsigned short recovered[] = {587, 1175};
static const unsigned short bonus[] = {523, 659, 784, 1047};
static const unsigned short death[] = {494, 466, 392, 311, 247, 196, 131};
static const unsigned short start[] = {392, 0, 523, 659, 784};
static const unsigned short clear[] = {523, 659, 784, 0, 988, 1047};
static const unsigned short history[] = {392, 523, 659, 784, 0, 698, 880, 1047, 0, 784, 1047};
static const unsigned short life[] = {659, 784, 988, 1319};
static const struct pattern cues[] = {
  SCORE(throw_notes, 24), SCORE(hit, 35), SCORE(crush, 55), SCORE(recovered, 55),
  SCORE(bonus, 65), SCORE(death, 110), SCORE(start, 90), SCORE(clear, 100),
  SCORE(history, 105), SCORE(life, 90)
};
static const unsigned short scale[] = {523, 587, 659, 698, 784, 880, 988, 1047};
static struct pattern cherry_note = {NULL, 1, 125};
static struct voice melody, cherry, effect, jingle;

static int next_sample(struct voice *v, int volume, int loop)
{
  const struct pattern *p = v->pattern;
  double triangle, envelope;
  unsigned duration, hz;
  int value;

  if (!p)
    return 0;
  duration = p->step_ms * (SAMPLE_RATE / 1000);
  hz = p->notes[v->index];
  triangle = v->phase < 0.5 ? 4 * v->phase - 1 : 3 - 4 * v->phase;
  envelope = (double)(duration - 1 - v->sample) / duration;
  if (v->sample < 48)
    envelope *= v->sample / 48.0;
  value = hz ? (int)(volume * triangle * envelope) : 0;
  v->phase += (double)hz / SAMPLE_RATE;
  if (v->phase >= 1)
    v->phase -= 1;
  if (++v->sample == duration) {
    v->sample = 0;
    v->phase = 0;
    if (++v->index == p->count) {
      v->index = 0;
      if (!loop)
        v->pattern = NULL;
    }
  }
  return value;
}

static void mix_audio(void *unused, Uint8 *stream, int bytes)
{
  Sint16 *out = (Sint16 *)stream;
  int i;
  (void)unused;
  for (i = 0; i < bytes / (int)sizeof(*out); i++)
    out[i] = next_sample(&melody, 2200, 1) + next_sample(&cherry, 7500, 0)
        + next_sample(&effect, 5800, 0) + next_sample(&jingle, 6200, 0);
}

static void start_voice(struct voice *v, const struct pattern *p)
{
  v->pattern = p;
  v->sample = v->index = 0;
  v->phase = 0;
}

void sound_play(enum sound_effect cue)
{
  if (!device)
    return;
  SDL_LockAudioDevice(device);
  start_voice(cue >= FX_DEATH ? &jingle : &effect, &cues[cue]);
  SDL_UnlockAudioDevice(device);
}

void sound_music(enum music_theme theme)
{
  static enum music_theme current = MUSIC_OFF;
  if (!device || theme == current)
    return;
  SDL_LockAudioDevice(device);
  start_voice(&melody, theme == MUSIC_OFF ? NULL : &themes[theme]);
  current = theme;
  SDL_UnlockAudioDevice(device);
}

void sound_cherry(unsigned count)
{
  if (!device || count < 1 || count > 8)
    return;
  SDL_LockAudioDevice(device);
  cherry_note.notes = &scale[count - 1];
  start_voice(&cherry, &cherry_note);
  SDL_UnlockAudioDevice(device);
}

void sound_pause(int paused)
{
  if (device)
    SDL_PauseAudioDevice(device, paused);
}

void sound_open(void)
{
  SDL_AudioSpec spec = {0};
  sigset_t alarm, previous;

  /* xmris waits for SIGALRM on the game thread. Audio threads inherit
   * the blocked mask while SDL starts them. */
  sigemptyset(&alarm);
  sigaddset(&alarm, SIGALRM);
  if (sigprocmask(SIG_BLOCK, &alarm, &previous) != 0) {
    perror("xmris: disabling sound");
    return;
  }
  SDL_SetHint(SDL_HINT_NO_SIGNAL_HANDLERS, "1");
  if (SDL_InitSubSystem(SDL_INIT_AUDIO) == 0) {
    spec.callback = mix_audio;
    spec.freq = SAMPLE_RATE;
    spec.format = AUDIO_S16SYS;
    spec.channels = 1;
    spec.samples = 256;
    device = SDL_OpenAudioDevice(NULL, 0, &spec, NULL, 0);
    if (device)
      SDL_PauseAudioDevice(device, 0);
  }
  if (!device) {
    fprintf(stderr, "xmris: disabling sound: %s\n", SDL_GetError());
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
  }
  sigprocmask(SIG_SETMASK, &previous, NULL);
}

void sound_close(void)
{
  if (device) {
    SDL_CloseAudioDevice(device);
    device = 0;
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
  }
}
