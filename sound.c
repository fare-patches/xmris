#include "sound.h"
#include <SDL2/SDL_audio.h>
#include <SDL2/SDL.h>
#include <signal.h>
#include <stdio.h>

enum { SAMPLE_RATE = 24000, NOTE_SAMPLES = SAMPLE_RATE / 8 };
static SDL_AudioDeviceID device;

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

void sound_cherry(unsigned count)
{
  static const double hz[] = {
    523.25, 587.33, 659.26, 698.46, 783.99, 880.00, 987.77, 1046.50
  };
  Sint16 samples[NOTE_SAMPLES];
  double phase = 0.0;
  int i;

  if (!device || count < 1 || count > 8)
    return;
  for (i = 0; i < NOTE_SAMPLES; i++) {
    double triangle = phase < 0.5 ? 4 * phase - 1 : 3 - 4 * phase;
    double envelope = (double)(NOTE_SAMPLES - 1 - i) / NOTE_SAMPLES;
    if (i < 48)
      envelope *= i / 48.0;
    samples[i] = (Sint16)(7500 * triangle * envelope);
    phase += hz[count - 1] / SAMPLE_RATE;
    if (phase >= 1.0)
      phase -= 1.0;
  }
  SDL_ClearQueuedAudio(device);
  SDL_QueueAudio(device, samples, sizeof(samples));
}

void sound_close(void)
{
  if (device) {
    SDL_CloseAudioDevice(device);
    device = 0;
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
  }
}
