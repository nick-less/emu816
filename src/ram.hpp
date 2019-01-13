#ifndef RAM_H
#define RAM_H

#include <lib65816/include/SystemBusDevice.hpp>

class Ram : public SystemBusDevice {
private:
    uint8_t *mRam;

public:

    Ram(uint8_t);
    ~Ram();

    void storeByte(const Address &, uint8_t);
    uint8_t readByte(const Address &);
    bool decodeAddress(const Address &, Address &);
};


#endif