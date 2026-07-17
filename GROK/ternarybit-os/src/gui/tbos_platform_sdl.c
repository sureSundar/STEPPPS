/**
 * @file tbos_platform_sdl.c
 * @brief TBOS SDL2 Platform Adapter
 *
 * Provides window management, input handling, and display for desktop platforms.
 *
 * @version 1.0
 * @date 2026-07-16
 */

#ifdef TBOS_USE_SDL2

#include "tbos_platform.h"
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <string.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * PLATFORM STRUCTURE
 * ═══════════════════════════════════════════════════════════════════════════ */

struct tbos_platform {
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    tbos_pixbuf_t* buffer;

    tbos_mouse_t mouse;
    tbos_keyboard_t keyboard;

    bool should_close;
    uint64_t start_time;
    int target_fps;
    uint32_t frame_delay;

    int scale;
    int logical_width;
    int logical_height;
};

/* ═══════════════════════════════════════════════════════════════════════════
 * LIFECYCLE
 * ═══════════════════════════════════════════════════════════════════════════ */

tbos_platform_t* tbos_platform_init(const tbos_window_config_t* config) {
    if (!config) return NULL;

    tbos_platform_t* platform = calloc(1, sizeof(tbos_platform_t));
    if (!platform) return NULL;

    /* Initialize SDL */
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        free(platform);
        return NULL;
    }

    platform->scale = config->scale > 0 ? config->scale : 1;
    platform->logical_width = config->width;
    platform->logical_height = config->height;

    int window_width = config->width * platform->scale;
    int window_height = config->height * platform->scale;

    /* Create window */
    Uint32 window_flags = SDL_WINDOW_SHOWN;
    if (config->fullscreen) window_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    if (config->resizable) window_flags |= SDL_WINDOW_RESIZABLE;
    if (config->borderless) window_flags |= SDL_WINDOW_BORDERLESS;

    platform->window = SDL_CreateWindow(
        config->title ? config->title : "TernaryBit OS",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        window_width, window_height,
        window_flags
    );

    if (!platform->window) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        free(platform);
        return NULL;
    }

    /* Create renderer */
    platform->renderer = SDL_CreateRenderer(
        platform->window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    if (!platform->renderer) {
        /* Fallback to software renderer */
        platform->renderer = SDL_CreateRenderer(platform->window, -1, SDL_RENDERER_SOFTWARE);
    }

    if (!platform->renderer) {
        fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(platform->window);
        SDL_Quit();
        free(platform);
        return NULL;
    }

    /* Set logical size for scaling */
    SDL_RenderSetLogicalSize(platform->renderer, config->width, config->height);

    /* Create texture for pixel buffer */
    platform->texture = SDL_CreateTexture(
        platform->renderer,
        SDL_PIXELFORMAT_RGBA32,
        SDL_TEXTUREACCESS_STREAMING,
        config->width, config->height
    );

    if (!platform->texture) {
        fprintf(stderr, "SDL_CreateTexture failed: %s\n", SDL_GetError());
        SDL_DestroyRenderer(platform->renderer);
        SDL_DestroyWindow(platform->window);
        SDL_Quit();
        free(platform);
        return NULL;
    }

    /* Create pixel buffer */
    platform->buffer = tbos_pixbuf_create(config->width, config->height,
                                           TBOS_PIXEL_RGBA32);
    if (!platform->buffer) {
        SDL_DestroyTexture(platform->texture);
        SDL_DestroyRenderer(platform->renderer);
        SDL_DestroyWindow(platform->window);
        SDL_Quit();
        free(platform);
        return NULL;
    }

    platform->start_time = SDL_GetTicks64();
    platform->target_fps = 60;
    platform->frame_delay = 1000 / 60;

    return platform;
}

void tbos_platform_shutdown(tbos_platform_t* platform) {
    if (!platform) return;

    tbos_pixbuf_free(platform->buffer);
    SDL_DestroyTexture(platform->texture);
    SDL_DestroyRenderer(platform->renderer);
    SDL_DestroyWindow(platform->window);
    SDL_Quit();
    free(platform);
}

tbos_pixbuf_t* tbos_platform_get_buffer(tbos_platform_t* platform) {
    return platform ? platform->buffer : NULL;
}

void tbos_platform_present(tbos_platform_t* platform) {
    if (!platform) return;

    /* Update texture from pixel buffer */
    SDL_UpdateTexture(platform->texture, NULL,
                      platform->buffer->data, platform->buffer->stride);

    /* Clear and render */
    SDL_RenderClear(platform->renderer);
    SDL_RenderCopy(platform->renderer, platform->texture, NULL, NULL);
    SDL_RenderPresent(platform->renderer);
}

bool tbos_platform_should_close(tbos_platform_t* platform) {
    return platform ? platform->should_close : true;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * INPUT HANDLING
 * ═══════════════════════════════════════════════════════════════════════════ */

static int sdl_key_to_tbos(SDL_Keycode key) {
    switch (key) {
        case SDLK_UP:       return KEY_UP;
        case SDLK_DOWN:     return KEY_DOWN;
        case SDLK_LEFT:     return KEY_LEFT;
        case SDLK_RIGHT:    return KEY_RIGHT;
        case SDLK_HOME:     return KEY_HOME;
        case SDLK_END:      return KEY_END;
        case SDLK_DELETE:   return KEY_DELETE;
        case SDLK_PAGEUP:   return KEY_PAGE_UP;
        case SDLK_PAGEDOWN: return KEY_PAGE_DOWN;
        case SDLK_INSERT:   return KEY_INSERT;
        case SDLK_BACKSPACE:return KEY_BACKSPACE;
        case SDLK_TAB:      return KEY_TAB;
        case SDLK_RETURN:   return KEY_ENTER;
        case SDLK_ESCAPE:   return KEY_ESCAPE;
        default:
            if (key >= SDLK_a && key <= SDLK_z) {
                return key - SDLK_a + 'a';
            }
            if (key >= SDLK_0 && key <= SDLK_9) {
                return key - SDLK_0 + '0';
            }
            return key < 128 ? key : KEY_UNKNOWN;
    }
}

void tbos_platform_poll_events(tbos_platform_t* platform) {
    if (!platform) return;

    /* Reset single-frame states */
    platform->mouse.left_clicked = false;
    platform->mouse.right_clicked = false;
    platform->mouse.scroll_delta = 0;
    platform->keyboard.key_pressed = false;
    platform->keyboard.text_len = 0;

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                platform->should_close = true;
                break;

            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
                    platform->should_close = true;
                }
                break;

            case SDL_MOUSEMOTION:
                platform->mouse.x = event.motion.x;
                platform->mouse.y = event.motion.y;
                break;

            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    platform->mouse.left_down = true;
                    platform->mouse.left_clicked = true;
                } else if (event.button.button == SDL_BUTTON_RIGHT) {
                    platform->mouse.right_down = true;
                    platform->mouse.right_clicked = true;
                }
                break;

            case SDL_MOUSEBUTTONUP:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    platform->mouse.left_down = false;
                } else if (event.button.button == SDL_BUTTON_RIGHT) {
                    platform->mouse.right_down = false;
                }
                break;

            case SDL_MOUSEWHEEL:
                platform->mouse.scroll_delta = event.wheel.y;
                break;

            case SDL_KEYDOWN:
                platform->keyboard.key = sdl_key_to_tbos(event.key.keysym.sym);
                platform->keyboard.key_down = true;
                platform->keyboard.key_pressed = true;
                platform->keyboard.ctrl = (event.key.keysym.mod & KMOD_CTRL) != 0;
                platform->keyboard.shift = (event.key.keysym.mod & KMOD_SHIFT) != 0;
                platform->keyboard.alt = (event.key.keysym.mod & KMOD_ALT) != 0;
                break;

            case SDL_KEYUP:
                platform->keyboard.key_down = false;
                break;

            case SDL_TEXTINPUT:
                strncpy(platform->keyboard.text_input, event.text.text,
                        sizeof(platform->keyboard.text_input) - 1);
                platform->keyboard.text_len = strlen(platform->keyboard.text_input);
                break;
        }
    }
}

void tbos_platform_get_mouse(tbos_platform_t* platform, tbos_mouse_t* mouse) {
    if (platform && mouse) {
        *mouse = platform->mouse;
    }
}

void tbos_platform_get_keyboard(tbos_platform_t* platform, tbos_keyboard_t* keyboard) {
    if (platform && keyboard) {
        *keyboard = platform->keyboard;
    }
}

void tbos_platform_update_gui(tbos_platform_t* platform, tbos_gui_t* gui) {
    if (!platform || !gui) return;

    gui->mouse = platform->mouse;
    gui->keyboard = platform->keyboard;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * WINDOW CONTROL
 * ═══════════════════════════════════════════════════════════════════════════ */

void tbos_platform_set_title(tbos_platform_t* platform, const char* title) {
    if (platform && platform->window) {
        SDL_SetWindowTitle(platform->window, title);
    }
}

void tbos_platform_set_size(tbos_platform_t* platform, int width, int height) {
    if (platform && platform->window) {
        SDL_SetWindowSize(platform->window, width * platform->scale,
                          height * platform->scale);
        SDL_RenderSetLogicalSize(platform->renderer, width, height);
    }
}

void tbos_platform_get_size(tbos_platform_t* platform, int* width, int* height) {
    if (platform && platform->window) {
        SDL_GetWindowSize(platform->window, width, height);
        if (width) *width /= platform->scale;
        if (height) *height /= platform->scale;
    }
}

void tbos_platform_set_fullscreen(tbos_platform_t* platform, bool fullscreen) {
    if (platform && platform->window) {
        SDL_SetWindowFullscreen(platform->window,
            fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
    }
}

void tbos_platform_minimize(tbos_platform_t* platform) {
    if (platform && platform->window) {
        SDL_MinimizeWindow(platform->window);
    }
}

void tbos_platform_maximize(tbos_platform_t* platform) {
    if (platform && platform->window) {
        SDL_MaximizeWindow(platform->window);
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * TIMING
 * ═══════════════════════════════════════════════════════════════════════════ */

uint64_t tbos_platform_get_time(tbos_platform_t* platform) {
    if (!platform) return 0;
    return SDL_GetTicks64() - platform->start_time;
}

void tbos_platform_sleep(tbos_platform_t* platform, uint32_t ms) {
    (void)platform;
    SDL_Delay(ms);
}

void tbos_platform_set_target_fps(tbos_platform_t* platform, int fps) {
    if (platform && fps > 0) {
        platform->target_fps = fps;
        platform->frame_delay = 1000 / fps;
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * CLIPBOARD
 * ═══════════════════════════════════════════════════════════════════════════ */

void tbos_platform_set_clipboard(tbos_platform_t* platform, const char* text) {
    (void)platform;
    SDL_SetClipboardText(text);
}

char* tbos_platform_get_clipboard(tbos_platform_t* platform) {
    (void)platform;
    return SDL_GetClipboardText();
}

/* ═══════════════════════════════════════════════════════════════════════════
 * CURSOR
 * ═══════════════════════════════════════════════════════════════════════════ */

void tbos_platform_set_cursor(tbos_platform_t* platform, tbos_cursor_type_t cursor) {
    if (!platform) return;

    SDL_SystemCursor sdl_cursor;
    switch (cursor) {
        case TBOS_CURSOR_ARROW:      sdl_cursor = SDL_SYSTEM_CURSOR_ARROW; break;
        case TBOS_CURSOR_IBEAM:      sdl_cursor = SDL_SYSTEM_CURSOR_IBEAM; break;
        case TBOS_CURSOR_HAND:       sdl_cursor = SDL_SYSTEM_CURSOR_HAND; break;
        case TBOS_CURSOR_RESIZE_H:   sdl_cursor = SDL_SYSTEM_CURSOR_SIZEWE; break;
        case TBOS_CURSOR_RESIZE_V:   sdl_cursor = SDL_SYSTEM_CURSOR_SIZENS; break;
        case TBOS_CURSOR_RESIZE_DIAG:sdl_cursor = SDL_SYSTEM_CURSOR_SIZENWSE; break;
        case TBOS_CURSOR_MOVE:       sdl_cursor = SDL_SYSTEM_CURSOR_SIZEALL; break;
        case TBOS_CURSOR_WAIT:       sdl_cursor = SDL_SYSTEM_CURSOR_WAIT; break;
        case TBOS_CURSOR_HIDDEN:
            SDL_ShowCursor(SDL_DISABLE);
            return;
        default:
            sdl_cursor = SDL_SYSTEM_CURSOR_ARROW;
    }

    SDL_ShowCursor(SDL_ENABLE);
    SDL_Cursor* c = SDL_CreateSystemCursor(sdl_cursor);
    if (c) {
        SDL_SetCursor(c);
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * APPLICATION LOOP
 * ═══════════════════════════════════════════════════════════════════════════ */

int tbos_app_run(const tbos_window_config_t* config,
                 const tbos_app_callbacks_t* callbacks) {
    if (!config || !callbacks) return -1;

    tbos_platform_t* platform = tbos_platform_init(config);
    if (!platform) return -1;

    tbos_pixbuf_t* buffer = tbos_platform_get_buffer(platform);

    tbos_gui_t gui;
    tbos_gui_init(&gui, buffer);
    tbos_gui_style_default(&gui);

    /* Initialize app */
    if (callbacks->init) {
        callbacks->init(&gui, callbacks->userdata);
    }

    uint64_t last_time = tbos_platform_get_time(platform);

    /* Main loop */
    while (!tbos_platform_should_close(platform)) {
        uint64_t now = tbos_platform_get_time(platform);
        float dt = (now - last_time) / 1000.0f;
        last_time = now;

        /* Poll input */
        tbos_platform_poll_events(platform);
        tbos_platform_update_gui(platform, &gui);

        /* Begin frame */
        tbos_gui_begin(&gui);

        /* Update */
        if (callbacks->update) {
            callbacks->update(&gui, dt, callbacks->userdata);
        }

        /* Render */
        if (callbacks->render) {
            callbacks->render(&gui, callbacks->userdata);
        }

        /* End frame */
        tbos_gui_end(&gui);

        /* Present */
        tbos_platform_present(platform);

        /* Frame limiting */
        uint64_t frame_time = tbos_platform_get_time(platform) - now;
        if (frame_time < platform->frame_delay) {
            tbos_platform_sleep(platform, platform->frame_delay - frame_time);
        }
    }

    /* Shutdown */
    if (callbacks->shutdown) {
        callbacks->shutdown(callbacks->userdata);
    }

    tbos_gui_free(&gui);
    tbos_platform_shutdown(platform);

    return 0;
}

#endif /* TBOS_USE_SDL2 */
