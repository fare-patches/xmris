#ifndef XMRIS_SOUND_H
#define XMRIS_SOUND_H

enum sound_effect {
  FX_BOUNCE, FX_HIT, FX_RETURN, FX_PRIZE, FX_BREAK, FX_CHEW, FX_CREDIT, FX_DEATH,
  FX_START, FX_CLEAR, FX_HISTORY, FX_LIFE, FX_DIAMOND, FX_GAME_OVER, FX_HIGH_SCORE
};
enum music_theme { MUSIC_OFF = 0, MUSIC_GARDEN = 1, MUSIC_CHASE = 2, MUSIC_EXTRA = 4,
  MUSIC_DRONES = 8, MUSIC_DIAMOND = 16 };

int sound_effects_enabled(void);
void sound_apples(int started, int falling, int crushed);
void sound_open(void);
void sound_cherry(unsigned count);
void sound_play(enum sound_effect effect);
void sound_music(unsigned flags);
void sound_pause(int paused);
void sound_close(void);

#endif
