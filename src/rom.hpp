#ifndef ROM_H
#define ROM_H

#include <lib65816/include/SystemBusDevice.hpp>

class Rom : public SystemBusDevice {
private:
    uint8_t *mRom;
    Address startAdr;
    int rSize;

public:

    Rom(Address adr, uint8_t *, int size);
    ~Rom();

    void storeByte(const Address &, uint8_t);
    uint8_t readByte(const Address &);
    bool decodeAddress(const Address &, Address &);
};


#endif 
