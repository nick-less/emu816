//
// Created by Francesco Rigoni on 05/03/2018.
//

#include "rom.hpp"
#include "Log.hpp"

Rom::Rom(Address adr, uint8_t* memPtr, int size) {
    mRom = memPtr;
    startAdr = adr;
    rSize = size;
}

Rom::~Rom() {
}

void Rom::storeByte(const Address &address, uint8_t value) {
    // do nothing
}

uint8_t Rom::readByte(const Address &address) {
            Log::vrb("ROM").str("read").hex(address.getBank()).hex(address.getOffset()).show();

    return mRom[address.getOffset()];
}

bool Rom::decodeAddress(const Address &in, Address &out) {
    if ((in.getBank() == startAdr.getBank()) && (in.getOffset()>startAdr.getOffset()) && (in.getOffset() < startAdr.getOffset()+rSize)) {
        Log::vrb("ROM").str("decodeAddress").hex(in.getBank()).hex(in.getOffset()).show();
        out = in;
    
        return true;
    }

    return false;
}
