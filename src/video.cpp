#include <SDL2/SDL.h>
#include <stdio.h>

#include "video.hpp"
#include "Log.hpp"

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

Video::Video(Address adr) {
    startAdr = adr;
}

Video::~Video() {
}

void Video::storeByte(const Address &address, uint8_t value) {
    // do nothing
}

uint8_t Video::readByte(const Address &address) {
            Log::vrb("Video").str("read").hex(address.getBank()).hex(address.getOffset()).show();

    return 0;
}

bool Video::decodeAddress(const Address &in, Address &out) {
    if ((in.getBank() == startAdr.getBank()) && (in.getOffset()>startAdr.getOffset()) && (in.getOffset() < startAdr.getOffset()+2048)) {
        Log::vrb("Video").str("decodeAddress").hex(in.getBank()).hex(in.getOffset()).show();
        out = in;
    
        return true;
    }

    return false;
}
