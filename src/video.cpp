#include <SDL2/SDL.h>
#include <iostream>
#include <stdio.h>

#include "Log.hpp"
#include "video.hpp"

#include "charset.h"


Video::Video(Ram *ram) {
  this->ram = ram;
  cx = 0;
  cy = 0;
  for (unsigned char i = 32; i < 255; i++) {
    vbuffer[i - 32] = i;
    cbuffer[i] = 1;
  }
    for (int i = 255; i < 512; i++) {
    vbuffer[i ] = i%2 == 0 ?0xa0:0x20;
    cbuffer[i] = 1;
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
  for (int i = 0; i < SCREEN_MEM_SIZE; i++) {
    // color[i] >> 4, color[i] & 0x0f
    int color = colortable [cbuffer[i] & 0x0f];
    drawChar(i % WIDTH, i / WIDTH, vbuffer[i], color, colortable[6]);
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
   Uint32 tmp;
  if (chOut>128) { // inverse
     chOut = chOut - 64;
     tmp = color,
     color= bgcolor;
     bgcolor = tmp;
  }
// Log::vrb("Video").str("draw ").hex(c).sp().hex(chOut).show();
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
  if (SDL_PollEvent(&event)) {
  if (event.type == SDL_KEYUP) {
    unsigned char c = ram->readByte(Address(0x00, 0xc6));
    printf("key %d %ld : %d\n", event.key.keysym.sym, sizeof(event.key.keysym.sym), c);
        if (c <10) {
          unsigned char k= event.key.keysym.sym;
          if (k >=96 ) {
            k = k -32;
          }
        ram->storeByte(Address(0,0x277+c), k);
        ram->storeByte(Address(0,0xc6), c+1);
         Log::vrb("Video")
        .str("key !").show();

        }
  }

   if (event.type == SDL_WINDOWEVENT) {
        switch (event.window.event) {
        case SDL_WINDOWEVENT_CLOSE:
            SDL_Log("Window %d closed", event.window.windowID);
            isClosed = true;
            break;
        }
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
    vbuffer[cy * WIDTH + cx] = a;
    cx++;
  }
  if (cx > WIDTH) {
    cy++;
    cx = 0;
  }
  if (cy > HEIGHT-1) {
    cy = HEIGHT-1;
    for (int i = 0; i < HEIGHT-1; i++) {
      memcpy(&vbuffer[i * WIDTH], &vbuffer[(i + 1) * WIDTH], WIDTH);
    }
    memset(&vbuffer[(HEIGHT-1) * WIDTH], 32, WIDTH);
  }
  update();
}

void Video::storeByte(const Address &address, uint8_t value) {
  /*
  Log::vrb("VIC")
      .str("memwrite ")
      .hex(address.getBank())
      .hex(address.getOffset())
      .sp()
      .hex(value)
      .show();
      */
      if (address.getOffset() > startAdr.getOffset() + SCREEN_MEM_SIZE ) {
      cbuffer[address.getOffset()-colorAdr.getOffset()] = value;

      } else {
    // do nothing
      vbuffer[address.getOffset()-startAdr.getOffset()] = value;

      }

}

uint8_t Video::readByte(const Address &address) {
  /*
  
  Log::vrb("VIC")
      .str("memread ")
      .hex(address.getBank())
      .hex(address.getOffset())
      .show();
      */
      if (address.getOffset() > startAdr.getOffset() + SCREEN_MEM_SIZE ) {
  return cbuffer[address.getOffset()-colorAdr.getOffset()];
      } else {

  return vbuffer[address.getOffset()-startAdr.getOffset()];
      }

}

bool Video::decodeAddress(const Address &in, Address &out) {
  if ((in.getBank() == startAdr.getBank()) &&
      (in.getOffset() >= startAdr.getOffset()) &&
      (in.getOffset() < startAdr.getOffset() + SCREEN_MEM_SIZE)) {
        /*
    Log::vrb("Video")
        .str("decodeAddress vid")
        .hex(in.getBank())
        .hex(in.getOffset())
        .show();
        */
    out = in;

    return true;
  }

  if ((in.getBank() == colorAdr.getBank()) &&
      (in.getOffset() >= colorAdr.getOffset()) &&
      (in.getOffset() < colorAdr.getOffset() + SCREEN_MEM_SIZE)) {
        /*
    Log::vrb("Video")
        .str("decodeAddress color")
        .hex(in.getBank())
        .hex(in.getOffset())
        .show();
        */
    out = in;
    return true;
  }

  return false;
}
