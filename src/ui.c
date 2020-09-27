
#include "ui.h"

#include "msemu.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

// Main window
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
#define LOGICAL_WIDTH  640
#define LOGICAL_HEIGHT 480

// Splashscreen
SDL_Surface* splashscreen_surface = NULL;
SDL_Texture* splashscreen_tex = NULL;
SDL_Rect splashscreen_srcRect = { 0, 0, 0, 0 };
SDL_Rect splashscreen_dstRect = { 0, 0, 0, 0 };
TTF_Font* font = NULL;
SDL_Color font_color = { 0xff, 0xff, 0x00 }; /* Yellow */
int splashscreen_show = 0;

// LCD
SDL_Surface* lcd_surface = NULL;
SDL_Texture* lcd_tex = NULL;
SDL_Rect lcd_srcRect = { 0, 0, 320, 240 };
SDL_Rect lcd_dstRect = { 0, 0, LOGICAL_WIDTH, LOGICAL_HEIGHT };


// This table translates PC scancodes to the Mailstation key matrix
int32_t keyTranslateTable[10][8] = {
	{ SDLK_HOME, SDLK_END, 0, SDLK_F1, SDLK_F2, SDLK_F3, SDLK_F4, SDLK_F5 },
	{ 0, 0, 0, SDLK_AT, 0, 0, 0, SDLK_PAGEUP },
	{ SDLK_BACKQUOTE, SDLK_1, SDLK_2, SDLK_3, SDLK_4, SDLK_5, SDLK_6, SDLK_7 },
	{ SDLK_8, SDLK_9, SDLK_0, SDLK_MINUS, SDLK_EQUALS, SDLK_BACKSPACE, SDLK_BACKSLASH, SDLK_PAGEDOWN },
	{ SDLK_TAB, SDLK_q, SDLK_w, SDLK_e, SDLK_r, SDLK_t, SDLK_y, SDLK_u },
	{ SDLK_i, SDLK_o, SDLK_p, SDLK_LEFTBRACKET, SDLK_RIGHTBRACKET, SDLK_SEMICOLON, SDLK_QUOTE, SDLK_RETURN },
	{ SDLK_CAPSLOCK, SDLK_a, SDLK_s, SDLK_d, SDLK_f, SDLK_g, SDLK_h, SDLK_j },
	{ SDLK_k, SDLK_l, SDLK_COMMA, SDLK_PERIOD, SDLK_SLASH, SDLK_UP, SDLK_DOWN, SDLK_RIGHT },
	{ SDLK_LSHIFT, SDLK_z, SDLK_x, SDLK_c, SDLK_v, SDLK_b, SDLK_n, SDLK_m },
	{ SDLK_LCTRL, 0, 0, SDLK_SPACE, 0, 0, SDLK_RSHIFT, SDLK_LEFT }
};

/* XXX: This needs rework still*/
void ui_init(uint32_t* ms_lcd_buffer)
{
	/* Initialize SDL & SDL_TTF */
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("Failed to initialize SDL: %s\n", SDL_GetError());
		abort();
	}

	if (TTF_Init() != 0) {
		printf("Failed to initialize SDL_TTF: %s\n", TTF_GetError());
		abort();
	}

	/* Create/configure window & renderer */
	window = SDL_CreateWindow(
		"MailStation Emulator",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		LOGICAL_WIDTH, LOGICAL_HEIGHT, SDL_WINDOW_RESIZABLE);

	renderer = SDL_CreateRenderer(window, -1, 0);
	SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xff);

	// This allows us to assume the window size is 320x240,
	// but SDL will scale/letterbox it to whatever size the window is.
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
	SDL_RenderSetLogicalSize(renderer, LOGICAL_WIDTH, LOGICAL_HEIGHT);

	/* Load font */
	font = TTF_OpenFont("fonts/kongtext.ttf", 16);
	if (!font) {
		printf("Failed to load font: %s\n", TTF_GetError());
		abort();
	}

	/* Prepare the splashscreen surface */
	/* TODO: I don't like this in here, the version is a whole project
	 * identifier and buried in UI seems silly. Need to dig out a not
	 * terrible way to do this with CMake.
	 */
	splashscreen_surface = TTF_RenderText_Blended_Wrapped(
		font, "Mailstation Emulator v0.2\n\n" \
			"F12 to Start", font_color, LOGICAL_WIDTH);
	if (!splashscreen_surface) {
		printf("Error creating splashscreen surface: %s\n", TTF_GetError());
		abort();
	}

	splashscreen_tex = SDL_CreateTextureFromSurface(renderer, splashscreen_surface);
	if (!splashscreen_tex) {
		printf("Error creating splashscreen texture: %s\n", SDL_GetError());
		abort();
	}

	splashscreen_srcRect.w = splashscreen_surface->w;
	splashscreen_srcRect.h = splashscreen_surface->h;
	splashscreen_dstRect.w = splashscreen_surface->w;
	splashscreen_dstRect.h = splashscreen_surface->h;

	/* Prepare the MailStation LCD surface */
	lcd_surface = SDL_CreateRGBSurfaceFrom(ms_lcd_buffer, 320, 240, 32, 1280, 0,0,0,0);
	if (!lcd_surface) {
		printf("Error creating LCD surface: %s\n", SDL_GetError());
		abort();
	}

	lcd_tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 320, 240);
	if (!lcd_tex) {
		printf("Error creating LCD texture: %s\n", SDL_GetError());
		abort();
	}
}

void ui_splashscreen_show()
{
	printf("Splashscreen ON\n");
	splashscreen_show = 1;
}

void ui_splashscreen_hide()
{
	printf("Splashscreen OFF\n");
	splashscreen_show = 0;
}

void ui_update_lcd()
{
	if (SDL_UpdateTexture(lcd_tex, &lcd_srcRect, lcd_surface->pixels, lcd_surface->pitch) != 0)  {
		printf("Failed to update LCD: %s\n", SDL_GetError());
	}
}

void ui_render()
{
	SDL_RenderClear(renderer);

	if (!splashscreen_show) {
		// Render LCD
		SDL_RenderCopy(
			renderer, lcd_tex,
			&lcd_srcRect, &lcd_dstRect);
	}

	// We're rendering back to front, so we always check
	// this last so the splash screen always appears on top.
	if (splashscreen_show) {
		SDL_RenderCopy(
			renderer, splashscreen_tex,
			&splashscreen_srcRect, &splashscreen_dstRect);
	}

	SDL_RenderPresent(renderer);
}

/* Translate real input keys to MS keyboard matrix
 *
 * The lookup matrix is currently [10][8], we just walk the whole thing like
 * one long buffer and do the math later to figure out exactly what bit was
 * pressed.
 *
 * TODO: Would it make sense to rework keyTranslateTable to a single buffer
 * anyway? Right now the declaration looks crowded and would need some rework
 * already.
 */
void generateKeyboardMatrix(ms_ctx* ms, int scancode, int eventtype)
{
	uint32_t i = 0;
	int32_t *keytbl_ptr = &keyTranslateTable[0][0];

	for (i = 0; i < (sizeof(keyTranslateTable)/sizeof(int32_t)); i++) {
		if (scancode == *(keytbl_ptr + i)) {
			/* Couldn't avoid the magic numbers below. As noted,
			 * kTT array is [10][8], directly mapping the MS matrix
			 * of 10 bytes to represent the whole keyboard. Divide
			 * by 8 to get the uint8_t the scancode falls in, and mod
			 * 8 to get the bit in that uint8_t that matches the code.
			 */
			if (eventtype == SDL_KEYDOWN) {
				ms->key_matrix[i/8] &= ~((uint8_t)1 << (i%8));
			} else {
				ms->key_matrix[i/8] |= ((uint8_t)1 << (i%8));
			}
		}
	}
}

void ui_process_events(ms_ctx *ms)
{
	SDL_Event event;

	/* XXX: All of this needs to be reworked to be far more
	 * efficient.
	 */
	// Check SDL events
	while (SDL_PollEvent(&event))
	{
		/* Exit if SDL quits, or Escape key was pushed */
		if ((event.type == SDL_QUIT) ||
		  ((event.type == SDL_KEYDOWN) &&
			(event.key.keysym.sym == SDLK_ESCAPE))) {
			//exitemu = 1;
		}

		/* Emulate power button with F12 key */
		if (event.key.keysym.sym == SDLK_F12)
		{
			if (event.type == SDL_KEYDOWN)
			{
				ms->power_button = 1;
				if (ms->power_state == MS_POWERSTATE_OFF)
				{
					printf("POWER ON\n");
					ui_splashscreen_hide();
					//resetMailstation(ms);
				}
			} else {
				ms->power_button = 0;
			}
		}

		/* Handle other input events */
		if ((event.type == SDL_KEYDOWN) ||
		  (event.type == SDL_KEYUP)) {
			/* Keys pressed whie right ctrl is held */
			if (event.key.keysym.mod & KMOD_RCTRL)
			{
				if (event.type == SDL_KEYDOWN)
				{
					switch (event.key.keysym.sym) {
					  /* Reset whole system */
					  case SDLK_r:
						if (ms->power_state == MS_POWERSTATE_ON)
						  //resetMailstation(ms);
						break;
					  default:
						break;
					}
				}
			} else {
				/* Proces the key for the MS */
				generateKeyboardMatrix(ms, event.key.keysym.sym, event.type);
			}
		}
	}
}
