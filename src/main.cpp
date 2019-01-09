
#include "Log.hpp"
#include "basic.hpp"
#include "ram.hpp"
#include "rom.hpp"
#include "video.hpp"
#include "vic.hpp"

#include <fstream>
#include <iostream>
#include <iterator>

#include "gfx.h"
#include "kernel.h"

#include <lib65816/include/Cpu65816.hpp>
#include <lib65816/include/Cpu65816Debugger.hpp>
#include <lib65816/include/Interrupt.hpp>
#include <lib65816/include/SystemBus.hpp>

using namespace std;

int load(Ram *ram, char *filename);

#define LOG_TAG "MAIN"

#define BASIC_START 0x0400
#define BASIC_END 0xA000

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
                                            .reset = 0xFCE2,
                                            .brkIrq = 0xFFFE};

int main(int argc, char **argv) {
  Log::vrb(LOG_TAG).str("+++ Lib65816 Sample Programs +++").show();

  Video video = Video();
  video.update();

  Rom basic =
      Rom(Address(0x00, 0xa000), (uint8_t *)&basic_a000, basic_a000_size);
  //  Rom kernal =
  //     Rom(Address(0x00, 0xe000), (uint8_t *)&basic_e000, basic_e000_size);

  Rom kernal =  Rom(Address(0x00, 0xe000), (uint8_t *)&kernal_64_bin, kernal_64_bin_len);

  Vic vic = Vic (Address(0x00, 0xD000));

  Ram ram = Ram(0x2);
  // ret used return from interferred calls
  ram.storeByte(Address(0x00, 0x8000), 0x60);

  Log::vrb(LOG_TAG)
      .str("basic")
      .hex(basic.readByte(Address(0x00, 0xa000)))
      .hex(basic.readByte(Address(0x00, 0xa001)))
      .show();

  for (int i = 0; i < 10; i++) {
    Log::vrb(LOG_TAG)
        .str("basic")
        .hex(kernal.readByte(Address(0x00, 0xE419 + i)))
        .show();
  }

  SystemBus systemBus = SystemBus();
  systemBus.registerDevice(&video);
  systemBus.registerDevice(&vic);
  systemBus.registerDevice(&basic);
  systemBus.registerDevice(&kernal);
  systemBus.registerDevice(&ram);

  Cpu65816 cpu(systemBus, &emulationInterrupts, &nativeInterrupts);
  Cpu65816Debugger debugger(cpu);
  debugger.setBreakPoint(Address(0x00, 0x0003));
  debugger.doBeforeStep([]() {});
  debugger.doAfterStep([]() {});
  // make basic happy
  cpu.getStack()->push8Bit(0);
  cpu.getStack()->push8Bit(0);
  cpu.getStack()->push8Bit(0);
  cpu.getStack()->push8Bit(0);

  vic.setCpu( &cpu);

  debugger.dumpCpu();

  bool breakPointHit = false;
  debugger.onBreakPoint([&breakPointHit]() { breakPointHit = true; });

  //      char buffer[] = "10 PRINT \"HELLO WORLD \";\r20 GOTO 10\rSAVE
  //      \"HELLO\"\r";
  char buffer[] = "LOAD \"*\"\rLIST\r";
  //  char buffer[]="SAVE \"HELLO\"\r";
  int ix = 0;
  int cnt = 0;
  
  while ((!breakPointHit) && (!video.closed())) {
    if (cnt++ % 5000 == 0) {
      video.poll();
      video.update();
    }
            /*
    switch (cpu.getProgramAddress().getOffset()) {
    case 0xffd8:
      for (int i = 0; i < 100; i++) {
        Log::vrb("mem ")
            .hex(BASIC_START + i)
            .sp()
            .hex(ram.readByte(Address(0x00, BASIC_START + i)))
            .show();
      }
      break;
    case 0xff90:
    case 0xffbd: // setnam - set filename parameters
    case 0xffba: // setfls // set file parameters
    case 0xFFF0: // save cursor
      cpu.setProgramAddress(Address(0x0, 0x8000));
      break;
    case 0xffb7: // status
      cpu.setA(0);
      cpu.setProgramAddress(Address(0x0, 0x8000));
      break;
    case 0xffd5: // load
      debugger.dumpCpu();
      load(&ram, "pet clock");
      // SDL_Delay(20000);
      cpu.getCpuStatus()->clearCarryFlag();

      cpu.setProgramAddress(Address(0x0, 0x8000));
      break;
    case 0xff99: // MEMTOP
      cpu.setXL(BASIC_END & 0xFF);
      cpu.setYL(BASIC_END >> 8);
      //                debugger.dumpCpu();
      cpu.setProgramAddress(Address(0x0, 0x8000));
      break;
    case 0xff9c: // MEMBOT
      cpu.setXL(BASIC_START & 0xFF);
      cpu.setYL(BASIC_START >> 8);
      //                debugger.dumpCpu();
      cpu.setProgramAddress(Address(0x0, 0x8000));
      break;
    case 0xffcc: // CLRCHN
      char ch;

      Log::vrb(LOG_TAG).str("CLRCHN").hex(cpu.getA()).show();
      cpu.setProgramAddress(Address(0x0, 0x8000));
      break;
    case 0xffcf: // CHRIN

      Log::vrb(LOG_TAG).str("*******CHRIN").hex(cpu.getA()).show();
      unsigned char ci;
      ci = video.chrin();
      cpu.setA(0);
      if (ix > sizeof(buffer)) {
        cpu.setX(0);
      } else {
        cpu.setA(buffer[ix++]);
      }
      cpu.getCpuStatus()->clearCarryFlag();

      cpu.setProgramAddress(Address(0x0, 0x8000));
      printf("CHRIN ");

      for (int i = 0; i < 40; i++) {
        printf("%02x ", ram.readByte(Address(0x00, 0x400 + i)));
      }
      printf("\n");
      break;
    case 0xffd2: // CHROUT
      //                Log::vrb(LOG_TAG).str("CHROUT").hex(cpu.getA()).show();
      char s[2];
      s[0] = cpu.getA() & 0xff;
      s[1] = 0;
      video.chrout(cpu.getA() & 0xff);
      cpu.getCpuStatus()->clearCarryFlag();
      cpu.setProgramAddress(Address(0x0, 0x8000));
      break;
    case 0xffe1: // STOP
      cpu.getCpuStatus()->clearZeroFlag();
      cpu.setProgramAddress(Address(0x0, 0x8000));
      break;
    case 0xffe7: // STOP
                 //                cpu.getCpuStatus()->clearZeroFlag();
      cpu.setProgramAddress(Address(0x0, 0x8000));
      break;
    }
    */
  if (ix) {
   //SDL_Delay(100);

  }
    debugger.step();
  }

  debugger.dumpCpu();


   for (int i = 0; i < 32; i++) {
    Log::vrb(LOG_TAG)
        .str("ram")
        .hex(ram.readByte(Address(0x00, 0x300 + i)))
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