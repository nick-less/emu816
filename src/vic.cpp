
/**
 * emulate enough hardware to make c64 kernel and basic run
 * 
 * looks like we only need the timerA1 
 * 
 */

#include "vic.hpp"
#include "Log.hpp"

Vic::Vic(Address adr) { startAdr = adr; }

Vic::~Vic() {}

void Vic::storeByte(const Address &address, uint8_t value) {

  

  switch (address.getOffset()) {
  case 0xDC04:
    timer1ALatch = (timer1ALatch & 0xff00) | value;
    break;
  case 0xDC05:
    timer1ALatch = (timer1ALatch & 0xff) | ((int)value << 8);
    break;
  case 0xDC06:
    timer1BLatch = (timer1BLatch & 0xff00) | value;
    break;
  case 0xDC07:
    timer1BLatch = (timer1BLatch & 0xff) | ((int)value << 8);
    break;
  case 0xDC0E:
    if (value == 0x11) {
      timer1A = timer1ALatch;

      Log::vrb("VIC").str("latching A ").hex(timer1ALatch).show();
      timer1Aenable = true;
      ciaIntr = false;
    }
    break;
  case 0xDC0F:
    if (value == 0x11) {
      timer1B = timer1BLatch;

      Log::vrb("VIC").str("latching B ").hex(timer1ALatch).show();
      timer1Benable = true;
    }
    break;
  case 0xDD0D:
    break;
  case 0xD019:
    break;
  case 0xD01a:
    break;
  case 0xDc00:
    break;
   default:
   Log::vrb("VIC")
      .str("set: ")
      .hex(address.getOffset())
      .str(": ")
      .hex(value)
      .show();
  }
}

void Vic::setCpu(Cpu65816 *cpu) { this->cpu = cpu; }

uint8_t Vic::readByte(const Address &address) { 
  
     switch (address.getOffset()) {
         case 0xdc0d: 
//              Log::vrb("VIC").str("clear ieq ").show();
              timer1A = timer1ALatch;

             cpu->setIRQPin(false);
             return 0x81;
        case 0xdc01:
            break;
        default:
           Log::vrb("VIC")
      .str("read: ")
      .hex(address.getOffset())
      .show();
         
     }

    return 0x00; 
    }

bool Vic::decodeAddress(const Address &in, Address &out) {
    if ((timer1Aenable) && (prescale++ % 10 ==0)) {
        timer1A--;

        if (timer1A == 0) {
            //Log::vrb("VIC").str("IRQ ").hex(timer1A).str (cpu->getCpuStatus()->interruptDisableFlag() ?" true ": " false").show();

            // cpu->setIRQPin(true);
//           timer1Aenable = false;
        }
    }

  // cpu->setIRQPin(true);
  if ((in.getOffset() >= 0xdc00) && (in.getOffset() < 0xdc10)) {
    out = in;
    return true;
  }
  if ((in.getOffset() >= 0xdd00) && (in.getOffset() < 0xdd10)) {
    out = in;
    return true;
  }


  if ((in.getBank() == startAdr.getBank()) &&
      (in.getOffset() > startAdr.getOffset()) &&
      (in.getOffset() < startAdr.getOffset() + size)) {
    out = in;
    return true;
  }

  return false;
}
