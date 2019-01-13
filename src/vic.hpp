#ifndef LIB65816_VIC_H
#define LIB65816_VIC_H

#include <lib65816/include/SystemBusDevice.hpp>
#include <lib65816/include/Cpu65816.hpp>

class Vic : public SystemBusDevice {
private:
    Address startAdr;
    int size=49;
    Cpu65816 *cpu;
    bool ciaIntr = false;
    bool vicIntr = false;
    int timer1A;
    int timer1B;
    int timer1Aenable = false;
    int timer1Benable = false;
    int timer1ALatch;
    int timer1BLatch;
    int timer2A;
    int timer2B;
    int timer2Aenable = false;
    int timer2Benable = false;
    int timer2ALatch;
    int timer2BLatch;

    int prescale =0;


public:

    Vic(Address adr);
    ~Vic();
    void setCpu(Cpu65816 *cpu);

    void storeByte(const Address &, uint8_t);
    uint8_t readByte(const Address &);
    bool decodeAddress(const Address &, Address &);
};


#endif