//
// Created by Francesco Rigoni on 05/03/2018.
//

#include "ram.hpp"
#include "Log.hpp"

Ram::Ram(uint8_t banks) {
    mRam = new uint8_t[banks * BANK_SIZE_BYTES];
    memset(mRam, 0, banks * BANK_SIZE_BYTES);
}

Ram::~Ram() {
    delete[] mRam;
}

void Ram::storeByte(const Address &address, uint8_t value) {
      //  Log::vrb("RAM").str("store").hex(address.getBank()).hex(address.getOffset(),4).sp().hex(value).show();

    mRam[address.getOffset()] = value;
}

uint8_t Ram::readByte(const Address &address) {
//            Log::vrb("RAM").str("read").hex(address.getBank()).hex(address.getOffset(),4).sp().hex(mRam[address.getOffset()]).show();

    return mRam[address.getOffset()];
}

bool Ram::decodeAddress(const Address &in, Address &out) {
    out = in;
    return true;
}
