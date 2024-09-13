
#include "simlcd.h"

SDL_Event windowEvent;

simlcd_buffer_t simlcd_init(uint16_t height,uint16_t width,uint8_t scale)
{
  simlcd_buffer_t buf;
  buf.h=height;
  buf.w=width;
  buf.wx=50;
  buf.wy=50;
  buf.wu=false;
  buf.scale=scale+1;
  buf.buf=(uint32_t*)calloc(sizeof(uint32_t),buf.h*buf.w);
  buf.displayed=false;
  return buf;
}

void simlcd_deinit(simlcd_buffer_t *buf)
{
  if(buf->displayed)SDL_DestroyWindow(buf->window);
  buf->displayed=false;
  free(buf->buf);
}

void simlcd_set_color(simlcd_buffer_t *buf,uint8_t r,uint8_t g,uint8_t b)
{
  buf->selected_color=(uint32_t)((r<<16)|(g<<8)|b);
  // SDL_SetRenderDrawColor(renderer,r,g,b,SDL_ALPHA_OPAQUE);
}

void simlcd_draw_point(simlcd_buffer_t *buf,int x,int y)
{
  buf->buf[(y*buf->w)+x]=buf->selected_color;
}

uint32_t simlcd_get_point(simlcd_buffer_t *buf,int x,int y)
{
  return buf->buf[(y*buf->w)+x];
}

void simlcd_draw_rect(simlcd_buffer_t *buf,int x,int y,int w,int h)
{
  static SDL_Rect rect;
  static int i,j;

  rect.x=x;
  rect.y=y;

  rect.h=h;
  rect.w=w;

  for(i=rect.x;i<rect.x+rect.w;i++)
      for(j=rect.y;j<rect.y+rect.h;j++)
        buf->buf[(j*buf->w)+i]=buf->selected_color;

}

void simlcd_display(simlcd_buffer_t *buf)
{
  int i,j;
  uint32_t color;
  SDL_Rect rect;

  if(buf->wu)
  {
    if(buf->displayed)SDL_DestroyWindow(buf->window);
    buf->displayed=false;
    buf->wu=false;
  }

  if(buf->displayed==false)
  {
    // SDL_CreateWindowAndRenderer(WIDTH*SCALE,HEIGHT*SCALE,0,&window,&renderer);
    SDL_Init(SDL_INIT_EVERYTHING);
    buf->window=SDL_CreateWindow("",buf->wx,buf->wy,buf->w*buf->scale,buf->h*buf->scale,SDL_WINDOW_BORDERLESS|SDL_WINDOW_ALWAYS_ON_TOP|SDL_WINDOW_SKIP_TASKBAR);
    buf->renderer=SDL_CreateRenderer(buf->window,-1,SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    // wxp+=(buf->w*buf->scale);
  }

  SDL_RenderClear(buf->renderer);
  for(i=0;i<buf->h;i++)
  {
    for(j=0;j<buf->w;j++)
    {
      color=buf->buf[(i*buf->w)+j];
      SDL_SetRenderDrawColor(buf->renderer,(color>>16)&0xff,(color>>8)&0xff,(color>>0)&0xff,SDL_ALPHA_OPAQUE);

      // SDL_RenderDrawPoint(renderer, j, i);
      rect.x=j*buf->scale;
      rect.y=i*buf->scale;
      rect.h=buf->scale;
      rect.w=buf->scale;
      SDL_RenderFillRect(buf->renderer,&rect);
    }
  }
  SDL_RenderPresent(buf->renderer);
  buf->displayed=true;
}

void simlcd_delay(uint32_t ms)
{
  SDL_Delay(ms);
}

#ifdef SIMLCD_USE_TOUCH
int simlcd_send_touch(uint32_t x,uint32_t y,uint16_t event)
{
  static uint32_t ox,oy;
  x=x/LCD_BUFFER.scale;
  y=y/LCD_BUFFER.scale;
  if(x>=LCD_BUFFER.w||x<0||y>=LCD_BUFFER.h||y<0) return 0;
  return simlcd_touch_event(x,y,event);
}
#endif

void simlcd_play()
{
  int ret=0;
  while(true)
  {
    if(SDL_PollEvent(&windowEvent))
    {
      if(windowEvent.window.event == SDL_WINDOWEVENT_CLOSE)
      {
        ret=-1;
      }
      else if(windowEvent.type==SDL_QUIT)
      {
        ret=-1;
      }
      else if( windowEvent.type == SDL_KEYDOWN )
      {
        ret=loop((int)windowEvent.key.keysym.scancode);
      }
      else if(windowEvent.type>=SDL_MOUSEMOTION && windowEvent.type<=SDL_MOUSEWHEEL)
      {
        #ifdef SIMLCD_USE_TOUCH
        ret=simlcd_send_touch(windowEvent.button.x,windowEvent.button.y,(uint16_t)windowEvent.type);
        #endif
      }
    }
    else ret=loop((int)SDL_SCANCODE_UNKNOWN);

    if(ret!=0)break;
  }

  simlcd_deinit(&LCD_BUFFER);
  simlcd_exit();
  SDL_Quit();
}

void simlcd_msaa(simlcd_buffer_t *in,simlcd_buffer_t *out,int msaaX)
{
  uint32_t color;
  uint32_t i,j,x,y;
  int r,g,b;

  out->scale=in->scale;

  out->wx=in->wx;
  out->wy=in->wy;

  if(out->h!=(in->h/msaaX))
  {
    out->wu=true;
  }
  out->h=(in->h/msaaX);

  if(out->w!=(in->w/msaaX))
  {
    out->wu=true;
  }
  out->w=(in->w/msaaX);

  if(out->displayed==true || out->wu==true)free(out->buf);
  out->buf=(uint32_t*)calloc(sizeof(uint32_t),out->h*out->w);

  for(y=0;y<(out->h);y++)
    for(x=0;x<(out->w);x++)
    {
      r=0,g=0,b=0;
      for(i=0;i<msaaX;i++)
        for(j=0;j<msaaX;j++)
        {
          if((y>=((out->h)-1) && j>0));
          else
          color=in->buf[(((y*msaaX)+j)*(in->w))+((x*msaaX)+i)];
          b+=((color>>0)&0xff);
          g+=((color>>8)&0xff);
          r+=((color>>16)&0xff);
        }
      color=((uint32_t)((float)r/(msaaX*msaaX))<<16)|((uint32_t)((float)g/(msaaX*msaaX))<<8)|\
            ((uint32_t)((float)b/(msaaX*msaaX))<<0);

      out->buf[(y*(out->w))+x]=color;
    }
}

void simlcd_div(simlcd_buffer_t *in,simlcd_buffer_t *out,int n)
{
  uint32_t color;
  uint32_t x,y;

  out->scale=in->scale;
  out->h=(in->h/n);
  out->w=(in->w/n);

  free(out->buf);
  out->buf=(uint32_t*)calloc(sizeof(uint32_t),out->h*out->w);

  for(y=0;y<(out->h);y++)
    for(x=0;x<(out->w);x++)
    {
      color=in->buf[(((y*n))*(in->w))+((x*n))];
      out->buf[(y*(out->w))+x]=color;
    }
}
