#define PHONE_ROW 80
#define PHONE_TOOLBAR 48
#define PHONE_BAR_HEIGHT (3 * PHONE_ROW + PHONE_TOOLBAR)
void phone_draw(SDL_Renderer *renderer);
void phone_event(SDL_Window *window, const SDL_Event *event);
void phone_after_tick(void);
void phone_reset(void);
void phone_init(void);
const char *phone_style_name(void);
