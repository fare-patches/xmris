#include "sound.h"
#include <SDL2/SDL_audio.h>
#include <SDL2/SDL.h>
#include <signal.h>
#include <stdio.h>

enum { SAMPLE_RATE = 24000 };
static SDL_AudioDeviceID device;

#include <stdint.h>
#include "music-score.h"

struct playback {
  const struct tune *tune;
  unsigned row, ticks, clock;
  uint32_t phase[4];
};
static struct playback music, fanfare, apple_fall, apple_impact;
static const struct tune *encore;
static struct playback letter, drones, gem, effect, cherry;

static void begin_tune(struct playback *player, const struct tune *tune)
{
  struct playback initial = {0};
  initial.tune = tune;
  *player = initial;
}

static int tune_sample(struct playback *player)
{
  /* The score uses PSG attenuation steps of 2 dB. */
  static const int gain[] = {1450, 1152, 915, 727, 577, 459, 364, 289, 230, 183, 145, 115, 91, 73, 58};
  const struct chord *row;
  int sum = 0;
  unsigned i;
  if (!player->tune)
    return 0;
  row = &player->tune->rows[player->row];
  for (i = 0; i < 4; ++i) {
    unsigned note = row->voice[i][0];
    if (note) {
      sum += player->phase[i] < UINT32_C(0x80000000)
          ? gain[row->voice[i][1]] : -gain[row->voice[i][1]];
      player->phase[i] += pitch_step[note];
    }
  }
  /* 59.943 video ticks per second, independent of callback size. */
  player->clock += 59943;
  if (player->clock >= SAMPLE_RATE * 1000) {
    player->clock -= SAMPLE_RATE * 1000;
    if (++player->ticks == row->ticks) {
      player->ticks = 0;
      if (++player->row == player->tune->count) {
        player->row = player->tune->repeat;
        if (player->row == player->tune->count)
          player->tune = NULL;
      }
    }
  }
  return sum;
}

static void mix_audio(void *unused, Uint8 *stream, int bytes)
{
  Sint16 *out = (Sint16 *)stream;
  int i;
  (void)unused;
  for (i = 0; i < bytes / (int)sizeof(*out); i++)
    {
      int sample = tune_sample(&music);
      if (letter.tune) sample = tune_sample(&letter);
      if (drones.tune) sample = tune_sample(&drones);
      if (gem.tune) sample = tune_sample(&gem);
      if (fanfare.tune) {
        sample = tune_sample(&fanfare);
        if (!fanfare.tune && encore) {
          begin_tune(&fanfare, encore);
          encore = NULL;
        }
      }
      out[i] = sample + tune_sample(&apple_fall) + tune_sample(&apple_impact)
          + tune_sample(&effect) + tune_sample(&cherry);
    }
}

/* All apples share the arcade fall channel. A fresh drop retriggers it;
 * a landing silences it only when there are no other apples in flight. */
void sound_apples(int started, int falling, int crushed)
{
  if (!device || (!started && falling && !crushed))
    return;
  SDL_LockAudioDevice(device);
  if (started || !falling)
    begin_tune(&apple_fall, falling ? &fall : NULL);
  if (crushed)
    begin_tune(&apple_impact, &sparkle);
  SDL_UnlockAudioDevice(device);
}

void sound_play(enum sound_effect cue)
{
  static const struct tune *const scores[] = {
    &bounce, &burst, &returning, &center_bonus, &apple_break, &chew, &credit,
    &death_song, &opening, &victory, &interlude, &extra_life, &diamond_show, &game_over, &game_over
  };
  if (!device)
    return;
  SDL_LockAudioDevice(device);
  if (cue >= FX_DEATH) {
    encore = cue == FX_HIGH_SCORE ? &initials : NULL;
    begin_tune(&apple_fall, NULL);
    if (cue == FX_START) begin_tune(&apple_impact, NULL);
    begin_tune(&fanfare, scores[cue]);
  } else {
    begin_tune(&effect, scores[cue]);
  }
  SDL_UnlockAudioDevice(device);
}

void sound_music(unsigned flags)
{
  static unsigned current;
  unsigned changed = flags ^ current;
  if (!device || !changed)
    return;
  SDL_LockAudioDevice(device);
  if (changed & (MUSIC_GARDEN | MUSIC_CHASE))
    begin_tune(&music, flags & MUSIC_CHASE ? &chase : flags & MUSIC_GARDEN ? &garden : NULL);
  if (changed & MUSIC_EXTRA) begin_tune(&letter, flags & MUSIC_EXTRA ? &prize : NULL);
  if (changed & MUSIC_DRONES) begin_tune(&drones, flags & MUSIC_DRONES ? &minions : NULL);
  if (changed & MUSIC_DIAMOND) begin_tune(&gem, flags & MUSIC_DIAMOND ? &diamond_song : NULL);
  current = flags;
  SDL_UnlockAudioDevice(device);
}

void sound_cherry(unsigned count)
{
  static const struct tune *const notes[] = {
    &cherry1, &cherry2, &cherry3, &cherry4, &cherry5, &cherry6, &cherry7, &cherry8
  };
  if (!device || count < 1 || count > 8)
    return;
  SDL_LockAudioDevice(device);
  begin_tune(&cherry, notes[count - 1]);
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
