#ifndef VIDEO_H
#define VIDEO_H

#include <SDL2/SDL.h>
#include <lib65816/include/SystemBusDevice.hpp>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 400
#define ERROR_SDL_INIT 100

#define WIDTH 40
#define HEIGHT 25
#define SCREEN_MEM_SIZE (WIDTH * HEIGHT)

#define VIDEO_ADDR 0x0400
#define COLOR_ADDR 0xD800

class Video : public SystemBusDevice {
private:
    Address startAdr = Address(0x00, VIDEO_ADDR);
    Address colorAdr = Address(0x00, COLOR_ADDR);
    unsigned char vbuffer[SCREEN_MEM_SIZE];
    unsigned char cbuffer[SCREEN_MEM_SIZE];
    int charHeight = 8;
    int charWidth = 8;
    int renderHeight = 2;
    int cx, cy;
    bool upperCase = true;
    bool isClosed = false;

    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    SDL_Surface *screenSurface = NULL;
    SDL_Texture * texture = NULL;
    Uint32 * pixels = NULL;

    Uint32 colortable[16] = {
        0x00000000,
        0x00FFFFFF,
        0x0068372B,
        0x0070A4B2,
        0x006F3D86,
        0x00588D43,
        0x00352879,
        0x00B8C76F,
        0x006F4F25,
        0x00433900,
        0x009A6759,
        0x00444444,
        0x006C6C6C,
        0x009AD284,
        0x006C5EB5,
        0x00959595
    };


public:

    Video();
    ~Video();

    void poll(void);

    bool closed(void) { return isClosed;};

    void toggleCase() {upperCase = !upperCase;}
    void update();
    void drawChar(int x, int y, unsigned char c, Uint32 color, Uint32 bgcolor);

    void chrout(unsigned char a);
    unsigned char chrin(void);

    void storeByte(const Address &, uint8_t);
    uint8_t readByte(const Address &);
    bool decodeAddress(const Address &, Address &);
};


#endif 
