
#ifndef SPIDEBUG_H
#define SPIDEBUG_H

#include "spi.hpp"
#include "video.hpp"

class SPIDebug : public SPIDevice {
private:
  uint8_t cmd;
  uint32_t cnt;
  uint32_t startAdr;
  uint8_t *content;
  Video *video;

public:
  SPIDebug(Video *video);
  ~SPIDebug() ;

  virtual void select(void);
  virtual void unselect(void);
  virtual uint8_t read(void);
    virtual void write(uint8_t value);
};
#endif 