 
 #include "spi.hpp"
 #include "spidebug.hpp"

  SPIDebug::SPIDebug(Video *video) {
      this->video = video;
  }
  SPIDebug::~SPIDebug() {}

   void SPIDebug::select(void) { 
     Log::vrb("SPIDebug").str("select ").show();
    }
   void SPIDebug::unselect(void) { 
     Log::vrb("SPIDebug").str("unselect ").show();
     }

   uint8_t SPIDebug::read(void) {
        Log::vrb("SPIDebug").str("read: ").hex(startAdr).str(": ").show();
    
    return 0;
  }
   void SPIDebug::write(uint8_t value) {
         Log::vrb("SPIDebug").str("write: ").hex(value).show();
      video->chrout(value);

  }
