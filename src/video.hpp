#ifndef VIDEO_H
#define VIDEO_H

#include <lib65816/include/SystemBusDevice.hpp>

class Video : public SystemBusDevice {
private:
    Address startAdr;

public:

    Video(Address adr);
    ~Video();

    void storeByte(const Address &, uint8_t);
    uint8_t readByte(const Address &);
    bool decodeAddress(const Address &, Address &);
};


#endif 
