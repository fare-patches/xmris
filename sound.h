#ifndef XMRIS_SOUND_H
#define XMRIS_SOUND_H

enum sound_effect {
  FX_THROW, FX_HIT, FX_CRUSH, FX_RETURN, FX_PRIZE, FX_DEATH,
  FX_START, FX_CLEAR, FX_HISTORY, FX_LIFE
};
enum music_theme { MUSIC_OFF, MUSIC_GARDEN, MUSIC_PRIZE, MUSIC_CHASE };

void sound_open(void);
void sound_cherry(unsigned count);
void sound_play(enum sound_effect effect);
void sound_music(enum music_theme theme);
void sound_pause(int paused);
void sound_close(void);

#endif
