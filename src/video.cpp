#include <SDL2/SDL.h>
#include <iostream>
#include <stdio.h>

#include "Log.hpp"
#include "video.hpp"

#include "charset.h"

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 400
#define ERROR_SDL_INIT 100

Video::Video(Address adr) {
  startAdr = adr;
  cx = 0;
  cy = 0;
  for (unsigned char i = 32; i < 127; i++) {
    vbuffer[i - 32] = i;
  }
  if (SDL_Init(SDL_INIT_VIDEO) == 0) {
    window = SDL_CreateWindow("Console", SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
                              SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

    if (window != NULL) {
      renderer = SDL_CreateRenderer(window, -1, 0);
      texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                                  SDL_TEXTUREACCESS_STATIC, SCREEN_WIDTH,
                                  SCREEN_HEIGHT);
      pixels = new Uint32[SCREEN_WIDTH * SCREEN_HEIGHT];

    } else {
      Log::vrb("Video").str("cant open window").show();
    }
  } else {
    Log::vrb("Video").str("cant init").show();
  }
}

Video::~Video() {
  if (window != NULL) {
    SDL_DestroyWindow(window);
  }
  if (pixels != NULL) {
    delete[] pixels;
  }
  if (texture != NULL) {
    SDL_DestroyTexture(texture);
  }
  if (renderer != NULL) {
    SDL_DestroyRenderer(renderer);
  }

  SDL_Quit();
}

void Video::update() {
  SDL_Event event;
  for (int i = 0; i < 2000; i++) {
    // color[i] >> 4, color[i] & 0x0f
    drawChar(i % 80, i / 80, vbuffer[i], 0xFFFFFFFF, 0xFF);
  }
  SDL_UpdateTexture(texture, NULL, pixels, SCREEN_WIDTH * sizeof(Uint32));
  SDL_PollEvent(&event);

  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, texture, NULL, NULL);
  SDL_RenderPresent(renderer);
}

void Video::drawChar(int x, int y, unsigned char c, Uint32 color,
                     Uint32 bgcolor) {

  unsigned char chOut = c;
  if ((c > 63) && (c < 96)) {
    chOut = c - 64;
  }
  if (!upperCase) {
    chOut += 64;
  }

  for (int i = 0; i < charHeight; i++) {
    unsigned char v = charset[(chOut)*charWidth + i];
    // Log::vrb("Video").str("draw ").hex(renderCh).str(" ofs ").hex(ofs).str("
    // value ").hex(v).show();

    for (int p = charWidth - 1; p > 0; p--) {
      Uint32 col = bgcolor;
      if (v & (1 << p)) {
        col = color;
      }
      int pixelOfs = (y * charHeight * renderHeight * SCREEN_WIDTH +
                      (i * renderHeight * SCREEN_WIDTH)) +
                     (x * charWidth + (charHeight - 1 - p));
      // Log::vrb("Video").str("pixelOfs ").hex(pixelOfs).str (" col
      // ").hex(col).show();
      for (int r = 0; r < renderHeight; r++) {
        pixels[pixelOfs] = col;
        pixelOfs += SCREEN_WIDTH;
      }
    }
  }
}

void Video::poll(void) {
  SDL_Event event;
  SDL_PollEvent(&event);
   if (event.type == SDL_WINDOWEVENT) {
        switch (event.window.event) {
        case SDL_WINDOWEVENT_CLOSE:
            SDL_Log("Window %d closed", event.window.windowID);
            isClosed = true;
            break;
        }
    }
}

unsigned char Video::chrin(void) {
  SDL_Event event;
  SDL_PollEvent(&event);
  if (event.type == SDL_KEYDOWN) {
    printf("key %d %ld\n", event.key.keysym.sym, sizeof(event.key.keysym.sym));
    chrout(event.key.keysym.sym - 32);
    return event.key.keysym.sym - 32;
  }

    if (event.type == SDL_WINDOWEVENT) {
        switch (event.window.event) {
        case SDL_WINDOWEVENT_CLOSE:
            SDL_Log("Window %d closed", event.window.windowID);
            isClosed = true;
            break;
        }
    }

  return 0;
}

void Video::chrout(unsigned char a) {
  switch (a) {
  case 147: // clear
    memset(&vbuffer, 32, sizeof(vbuffer));
    Log::vrb("Video").str("clear ").show();
    cx = 0;
    cy = 0;
    break;
  case 14:
    toggleCase();
    break;
  case 10: // line feed
    cy++;
    break;
  case 13: // return
    cy++;
    cx = 0;
    break;
  default:
    vbuffer[cy * 80 + cx] = a;
    cx++;
  }
  if (cx > 80) {
    cy++;
    cx = 0;
  }
  if (cy > 24) {
    cy = 24;
    for (int i = 0; i < 24; i++) {
      memcpy(&vbuffer[i * 80], &vbuffer[(i + 1) * 80], 80);
    }
    memset(&vbuffer[24 * 80], 32, 80);
  }
  update();
}

void Video::storeByte(const Address &address, uint8_t value) {
  // do nothing
}

uint8_t Video::readByte(const Address &address) {
  Log::vrb("Video")
      .str("read")
      .hex(address.getBank())
      .hex(address.getOffset())
      .show();

  return 0;
}

bool Video::decodeAddress(const Address &in, Address &out) {
  if ((in.getBank() == startAdr.getBank()) &&
      (in.getOffset() > startAdr.getOffset()) &&
      (in.getOffset() < startAdr.getOffset() + 2000)) {
    Log::vrb("Video")
        .str("decodeAddress")
        .hex(in.getBank())
        .hex(in.getOffset())
        .show();
    out = in;

    return true;
  }

  return false;
}
