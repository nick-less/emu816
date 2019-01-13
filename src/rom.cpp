#include "rom.hpp"
#include "Log.hpp"

Rom::Rom(Address adr, uint8_t* memPtr, int size) {
    mRom = memPtr;
    startAdr = adr;
    rSize = size;

            Log::vrb("ROM").str("at ").hex(startAdr.getOffset()).sp().str("size :").hex(size).sp().hex((long)memPtr).show();

}

Rom::~Rom() {
}

void Rom::storeByte(const Address &address, uint8_t value) {
    // do nothing
}

uint8_t Rom::readByte(const Address &address) {
    
    //Log::vrb("ROM").str("read").hex(address.getBank()).hex(address.getOffset()).sp().hex(address.getOffset()-startAdr.getOffset()).str(": ").hex(mRom[address.getOffset()-startAdr.getOffset()]).show();

        

    return mRom[address.getOffset()-startAdr.getOffset()];
}

bool Rom::decodeAddress(const Address &in, Address &out) {

    if ((in.getBank() == startAdr.getBank()) && (in.getOffset()>=startAdr.getOffset()) && (in.getOffset() < startAdr.getOffset()+rSize)) {
        // Log::vrb("ROM").str("decodeAddress").hex(in.getBank()).hex(in.getOffset()).show();
        out = in;
    
        return true;
    }

    return false;
}
