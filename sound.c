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
  const unsigned char *lengths; /* steps per note, or NULL for one each */
};

struct voice {
  const struct pattern *pattern;
  unsigned index, sample;
  double phase;
};

#define SCORE(notes, ms) {notes, sizeof(notes) / sizeof(notes[0]), ms, 0}

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
  {NULL, 0, 0, 0}, SCORE(garden, 155), SCORE(prize, 115), SCORE(chase, 100)
};

static const unsigned short throw_notes[] = {880, 659, 440};
static const unsigned short hit[] = {784, 392, 196, 98};
static const unsigned short crush[] = {165, 98, 73, 55};
static const unsigned short recovered[] = {587, 1175};
static const unsigned short bonus[] = {523, 659, 784, 1047};
static const unsigned short death[] = {494, 466, 392, 311, 247, 196, 131};
/* G4 C5 C5 C5 | F4 B4 B4 B4 | G4 F4 E4 D4 | C4 (held). */
enum { INTRO_STEP_MS = 110 };
static const unsigned short start[] = {
  392, 523, 523, 523, 349, 494, 494, 494, 392, 349, 330, 294, 262
};
/* A quiet accompaniment in C, G7, and C. */
static const unsigned short intro_bass[] = {131, 196, 98, 147, 98, 196, 131};
static const struct pattern bass_line = SCORE(intro_bass, 2 * INTRO_STEP_MS);
static const unsigned char start_lengths[] = {1,1,1,1, 1,1,1,1, 1,1,1,1, 2};
/* sol Do Mi Sol-ol mi Sol-ol-ol */
static const unsigned short clear[] = {392, 523, 659, 784, 659, 784};
static const unsigned char clear_lengths[] = {1, 1, 1, 2, 1, 3};
static const unsigned short history[] = {392, 523, 659, 784, 0, 698, 880, 1047, 0, 784, 1047};
static const unsigned short life[] = {659, 784, 988, 1319};
static const struct pattern cues[] = {
  SCORE(throw_notes, 24), SCORE(hit, 35), SCORE(crush, 55), SCORE(recovered, 55),
  SCORE(bonus, 65), SCORE(death, 110), {start, sizeof(start) / sizeof(start[0]), INTRO_STEP_MS, start_lengths},
  {clear, sizeof(clear) / sizeof(clear[0]), INTRO_STEP_MS, clear_lengths},
  SCORE(history, 105), SCORE(life, 90)
};
static const unsigned short scale[] = {523, 587, 659, 698, 784, 880, 988, 1047};
static struct pattern cherry_note = {NULL, 1, 125, 0};
static struct voice melody, cherry, effect, jingle, bass;

static int next_sample(struct voice *v, int volume, int loop)
{
  const struct pattern *p = v->pattern;
  double triangle, envelope;
  unsigned duration, hz;
  int value;

  if (!p)
    return 0;
  duration = p->step_ms * (SAMPLE_RATE / 1000);
  if (p->lengths)
    duration *= p->lengths[v->index];
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
    {
      int background = bass.pattern ? next_sample(&bass, 2100, 0)
                                    : next_sample(&melody, 2200, 1);
      out[i] = background + next_sample(&cherry, 7500, 0)
          + next_sample(&effect, 5800, 0) + next_sample(&jingle, 6200, 0);
    }
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
  if (cue >= FX_DEATH)
    start_voice(&bass, cue == FX_START ? &bass_line : NULL);
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
