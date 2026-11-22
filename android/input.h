#define PHONE_ROW (WINDOW_WIDTH / 3)
#define PHONE_TOOLBAR 48
#define PHONE_BAR_HEIGHT (3 * PHONE_ROW + PHONE_TOOLBAR)
void phone_draw(SDL_Renderer *renderer);
void phone_event(SDL_Window *window, const SDL_Event *event);
void phone_after_tick(void);
void phone_reset(void);
