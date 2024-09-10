#ifndef __simlcd_h__
#define __simlcd_h__

#include <stdint.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL_syswm.h>

#define SIMLCD_USE_TOUCH


typedef struct{
  bool displayed;
  uint32_t h,w;
  uint32_t * buf;
  uint8_t scale;
  uint32_t selected_color;
  SDL_Window *window;
  SDL_Renderer *renderer;
}simlcd_buffer_t;

extern simlcd_buffer_t LCD_BUFFER;

#ifdef __cplusplus
extern "C" {
#endif

void      simlcd_set_color(simlcd_buffer_t *buf,uint8_t r,uint8_t g,uint8_t b);
void      simlcd_draw_point(simlcd_buffer_t *buf,int x,int y);
uint32_t  simlcd_get_point(simlcd_buffer_t *buf,int x,int y);
void      simlcd_draw_rect(simlcd_buffer_t *buf,int x,int y,int w,int h);
void      simlcd_delay(uint32_t ms);

simlcd_buffer_t simlcd_init(uint16_t height,uint16_t width,uint8_t scale);
void      simlcd_deinit(simlcd_buffer_t *buf);

void      simlcd_display(simlcd_buffer_t *buf);

void      simlcd_play();

extern int loop(int key);
extern void simlcd_exit();

#ifdef SIMLCD_USE_TOUCH
// Event: SDL_MOUSEMOTION , SDL_MOUSEBUTTONDOWN , SDL_MOUSEBUTTONUP , SDL_MOUSEWHEEL
extern int simlcd_touch_event(uint32_t x,uint32_t y,uint16_t event);
#endif

void simlcd_msaa(simlcd_buffer_t *in,simlcd_buffer_t *out,int msaaX);
void simlcd_div(simlcd_buffer_t *in,simlcd_buffer_t *out,int n);

#ifdef __cplusplus
}
#endif

#endif
