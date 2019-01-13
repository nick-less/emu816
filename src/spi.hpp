
#ifndef LIB65816_SPI_H
#define LIB65816_SPI_H

#include <lib65816/include/SystemBusDevice.hpp>
#include <lib65816/include/Cpu65816.hpp>

#define SLAVE_0 0x01
#define SLAVE_1 0x02
#define SLAVE_2 0x04
#define SLAVE_3 0x08

class SPIDevice {
    public:
    virtual void select(void) = 0;
    virtual void unselect(void) = 0;
    virtual uint8_t read(void) = 0;
    virtual void write(uint8_t value) = 0;
};

class SPI : public SystemBusDevice {
private:
    Address startAdr;
    int size=4;
    Cpu65816 *cpu;


    uint8_t spi_data;
    uint8_t spi_status;
    uint8_t spi_divisor;
    uint8_t spi_select;
    SPIDevice *devices [4];

public:

    SPI(Address adr);
    ~SPI();
    void setCpu(Cpu65816 *cpu);

    void storeByte(const Address &, uint8_t);
    uint8_t readByte(const Address &);
    bool decodeAddress(const Address &, Address &);
};


#endif