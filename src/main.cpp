
#include "Log.hpp"
#include "ram.hpp"
#include "video.hpp"
#include "vic.hpp"
#include "spi.hpp"
#include "spidebug.hpp"
#include "../native/loader/loader.h"

#include <fstream>
#include <iostream>
#include <iterator>

#include "gfx.h"

#include <lib65816/include/Cpu65816.hpp>
#include <lib65816/include/Cpu65816Debugger.hpp>
#include <lib65816/include/Interrupt.hpp>
#include <lib65816/include/SystemBus.hpp>

using namespace std;

int load(Ram *ram, char *filename);

#define LOG_TAG "MAIN"

#define BASIC_START 0x0800
#define BASIC_END 0x8000

NativeModeInterrupts nativeInterrupts{
    .coProcessorEnable = 0x0000,
    .brk = 0x0000,
    .abort = 0x0000,
    .nonMaskableInterrupt = 0x0000,
    .reset = 0x0000,
    .interruptRequest = 0x0000,

};
// basic start vector = 0xe394;
EmulationModeInterrupts emulationInterrupts{.coProcessorEnable = 0x0000,
                                            .unused = 0x0000,
                                            .abort = 0x0000,
                                            .nonMaskableInterrupt = 0xFFFA,
                                            .reset = 0xffa0,
                                            .brkIrq = 0xFFFE};

int main(int argc, char **argv) {
  Log::vrb(LOG_TAG).str("+++ 65816 sbc emulator +++").show();

  
  Vic vic = Vic (Address(0x00, 0xFF40));
  SPI spi = SPI (Address(0x00, 0xFF80));


  Ram ram = Ram(0x2);
  // Loader is in RAM just like the real hardware handles this
  for (int i=0;i<0x60;i++) {
          ram.storeByte(Address(0,0xffa0+i), loader[i]);
  }

  Video video = Video(&ram);
  video.update();

  SPIDebug debug = SPIDebug(&video);
  spi.setDevice(1,&debug);



  SystemBus systemBus = SystemBus();
  systemBus.registerDevice(&video);
//  systemBus.registerDevice(&vic);
  systemBus.registerDevice(&spi);
  systemBus.registerDevice(&ram);

  spi.setDevice(3,video.getKeyboard());


  Cpu65816 cpu(systemBus, &emulationInterrupts, &nativeInterrupts);
  Cpu65816Debugger debugger(cpu);
  debugger.setBreakPoint(Address(0x00, 0xff80));
  debugger.doBeforeStep([]() {});
  debugger.doAfterStep([]() {});

  vic.setCpu( &cpu);

  debugger.dumpCpu();

  bool breakPointHit = false;
  debugger.onBreakPoint([&breakPointHit]() { breakPointHit = true; });

  int ix = 0;
  int cnt = 0;
  
  while ((!breakPointHit) && (!video.closed())) {
 

    if (cnt++ % 5000 == 0) {
      video.poll();
      video.update();
    }
 
      debugger.step();
  }

  debugger.dumpCpu();


   for (int i = 0; i < 32; i++) {
    Log::vrb(LOG_TAG)
        .str("ram")
        .hex(ram.readByte(Address(0x00, 0x80 + i)))
        .show();
  }
// SDL_Delay(20000);

  Log::vrb(LOG_TAG).str("+++ Program completed +++").show();
}

int start_address(unsigned char *buffer) {
  int l = *buffer++;
  int h = *buffer++;
  return h << 8 | l;
}

void list_cbm_prg(Ram *ram, unsigned char *buffer) {
  int start = start_address(buffer);
  buffer += 2;
  ram->storeByte(Address(0x00, 0xc1), (start >> 8) & 0xff);
  ram->storeByte(Address(0x00, 0xc2), start & 0xff);

  int j = 100;
  while (1) {
    int next = start_address(buffer);
    if (next == 0) {
      return;
    }
    printf("start %x \n", start);
    printf("next %x \n", next);
    ram->storeByte(Address(0x00, 0xae), (next >> 8) & 0xff);
    ram->storeByte(Address(0x00, 0xaf), next & 0xff);

    ram->storeByte(Address(0x00, 0xc3), (next >> 8) & 0xff);
    ram->storeByte(Address(0x00, 0xc4), next & 0xff);

    while (start < next) {
      ram->storeByte(Address(0x00, start), *buffer++);
      start++;
    }
    start = next;
    if (--j < 0) {
      //  exit(1);
    }
  }
}

int load(Ram *ram, char *filename) {

  Log::vrb("mem ")
      .hex(0xc1)
      .sp()
      .hex(ram->readByte(Address(0x00, 0xc1)))
      .show();
  Log::vrb("mem ")
      .hex(0xc2)
      .sp()
      .hex(ram->readByte(Address(0x00, 0xc2)))
      .show();
  Log::vrb("mem ")
      .hex(0xc3)
      .sp()
      .hex(ram->readByte(Address(0x00, 0xc3)))
      .show();
  Log::vrb("mem ")
      .hex(0xc4)
      .sp()
      .hex(ram->readByte(Address(0x00, 0xc4)))
      .show();
  ifstream is;
  is.open(filename, ios::in | ios::binary);
  if (!is.is_open()) {
    cout << "Unable to open file" << endl;
    return false;
  }

  is.seekg(0, ios::end);
  int size = is.tellg();
  char *content = new char[size];
  is.seekg(0, ios::beg);
  is.read(content, size);
  is.close();
  list_cbm_prg(ram, (unsigned char *)content);

  Log::vrb("mem ")
      .hex(0xc1)
      .sp()
      .hex(ram->readByte(Address(0x00, 0xc1)))
      .show();
  Log::vrb("mem ")
      .hex(0xc2)
      .sp()
      .hex(ram->readByte(Address(0x00, 0xc2)))
      .show();
  Log::vrb("mem ")
      .hex(0xc3)
      .sp()
      .hex(ram->readByte(Address(0x00, 0xc3)))
      .show();
  Log::vrb("mem ")
      .hex(0xc4)
      .sp()
      .hex(ram->readByte(Address(0x00, 0xc4)))
      .show();

  Log::vrb("mem ")
      .hex(0xae)
      .sp()
      .hex(ram->readByte(Address(0x00, 0xae)))
      .show();
  Log::vrb("mem ")
      .hex(0xaf)
      .sp()
      .hex(ram->readByte(Address(0x00, 0xaf)))
      .show();

  ram->storeByte(Address(0x00, 0x90), 0);

  for (int i = 0; i < 100; i++) {
    Log::vrb("mem ")
        .hex(0x400 + i)
        .sp()
        .hex(ram->readByte(Address(0x00, 0x400 + i)))
        .show();
  }

  //  exit(1);
  delete[] content;
  return true;
}