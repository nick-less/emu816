
/**
 * emulate a SPI65/B CPLD
 *  
 */
#include "spi.hpp"
#include "Log.hpp"
#include <fstream>
#include <iostream>
using namespace std;

#define CMD_NONE 0x00
#define CMD_READ 0x03

 class SPIDummy : public SPIDevice {
 public:
   virtual void select(void) {

   }
    virtual void unselect(void) {

    }
    virtual uint8_t read(void) {
      return 0x42;
    }
    virtual void write(uint8_t value) {

    }
};

/**
  * emulate a spi attached flash 
  */
class SPIFlash : public SPIDevice {
private:
  uint8_t cmd;
  uint32_t cnt;
  uint32_t startAdr;
  uint8_t *content;

public:
  SPIFlash() {
    cmd = CMD_NONE;
    ifstream is;
    is.open("../native/flashImage.bin", ios::in | ios::binary);
    if (is.is_open()) {
      is.seekg(0, ios::end);
      int size = is.tellg();
      content = new uint8_t[size];
      is.seekg(0, ios::beg);
      is.read((char*)content, size);
      is.close();
    }
  }
  ~SPIFlash() {}

  virtual void select(void) { 
     Log::vrb("SPIDevice").str("select ").show();
    cmd = CMD_NONE;
    }
  virtual void unselect(void) { 
     Log::vrb("SPIDevice").str("unselect ").show();
    cmd = CMD_NONE;
     }

  virtual uint8_t read(void) {
    switch (cmd) {
    case CMD_READ:
        uint8_t v = content[startAdr++];
       // Log::vrb("SPIDevice").str("read: ").hex(startAdr).str(": ").hex(v).show();
      return  v ;
    }
  }
  virtual void write(uint8_t value) {
         Log::vrb("SPIDevice").str("write: ").hex(value).show();

    if (cmd == CMD_NONE) {
        cmd = value;
        cnt=0;
        startAdr =0;
               // Log::vrb("SPIDevice").str("cmd READ: ").hex(startAdr).show();

        return;
    }
    if (cmd == CMD_READ) {
      uint32_t v = value;
      if (cnt == 0) {
        v = v << 16;
        startAdr = startAdr | v;
      }
      if (cnt == 1) {
        v = v << 8;
        startAdr = startAdr | v;
      }

      if (cnt == 2) {
        startAdr = startAdr | value;
      }
      cnt++;
    }
  }
};

SPI::SPI(Address adr) { startAdr = adr; 
  devices[0] = new SPIDummy();
  devices[1] = new SPIDummy();
  devices[2] = new SPIFlash();
  devices[3] = new SPIDummy();

}

SPI::~SPI() {}
void SPI::setCpu(Cpu65816 *cpu) { this->cpu = cpu; }

void SPI::storeByte(const Address &address, uint8_t value) {

  Log::vrb("SPI")
      .str("set: ")
      .hex(address.getOffset())
      .str(": ")
      .hex(value)
      .show();

  switch (address.getOffset()-startAdr.getOffset()) {
    case 0:
    switch (spi_select & 0xf) {
      case 1:
        devices[0]->write(value);
        break;
      case 2:
        devices[1]->write(value);
        break;
      case 4:
        devices[2]->write(value);
        break;
      case 8:
        devices[3]->write(value);
        break;
    }
    break;
    case 1:
    spi_status = value;
    break;
    case 2:
    spi_divisor = value;
    break;
    case 3:
    for (int i=0;i<4;i++) {
      if ((spi_select & 0x01 ) && !(value & 0x01)) {
        devices [0]->unselect();
      }
    }
    spi_select = value;
     switch (spi_select & 0xf) {
      case 1:
        devices[0]->select();
        break;
      case 2:
        devices[1]->select();
        break;
      case 4:
        devices[2]->select();
        break;
      case 8:
        devices[3]->select();
        break;
    }
    break;
  }
}


uint8_t SPI::readByte(const Address &address) {
  //Log::vrb("SPI").str("read: ").hex(address.getOffset()).show();
  switch (address.getOffset()-startAdr.getOffset()) {
  case 0:
      switch (spi_select & 0xf) {
      case 1:
        return devices[0]->read();
      case 2:
        return devices[1]->read();
      case 4:
        return devices[2]->read();
      case 8:
        return devices[3]->read();
    }

    return 0;
  case 1:
  case 2:
  case 3:
    return 0;
  }

  return 0x00;
}

bool SPI::decodeAddress(const Address &in, Address &out) {

  if ((in.getBank() == startAdr.getBank()) &&
      (in.getOffset() >= startAdr.getOffset()) &&
      (in.getOffset() < startAdr.getOffset() + size)) {
    out = in;
    return true;
  }

  return false;
}
