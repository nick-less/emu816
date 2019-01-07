#ifndef VIDEO_H
#define VIDEO_H

#include <SDL2/SDL.h>
#include <lib65816/include/SystemBusDevice.hpp>

class Video : public SystemBusDevice {
private:
    Address startAdr;
    unsigned char vbuffer[2000];
    unsigned char cbuffer[2000];
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

public:

    Video(Address adr);
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
