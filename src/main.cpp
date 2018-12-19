
#include <iterator>
#include "Log.hpp"
#include "ram.hpp"
#include "rom.hpp"
#include "basic.hpp"
#include "video.hpp"

#include "gfx.h"

#include <lib65816/include/Interrupt.hpp>
#include <lib65816/include/SystemBus.hpp>
#include <lib65816/include/Cpu65816.hpp>
#include <lib65816/include/Cpu65816Debugger.hpp>

#define LOG_TAG "MAIN"

NativeModeInterrupts nativeInterrupts {
        .coProcessorEnable = 0x0000,
        .brk = 0x0000,
        .abort = 0x0000,
        .nonMaskableInterrupt = 0x0000,
        .reset = 0x0000,
        .interruptRequest = 0x0000,
        
};

EmulationModeInterrupts emulationInterrupts {
        .coProcessorEnable = 0x0000,
        .unused = 0x0000,
        .abort = 0x0000,
        .nonMaskableInterrupt = 0x0000,
        .reset = 0xe394,
        .brkIrq = 0x0000
};


int main(int argc, char **argv) {
    Log::vrb(LOG_TAG).str("+++ Lib65816 Sample Programs +++").show();

    Video video = Video (Address(0x00, 0xd000));
    video.update();


    Rom basic = Rom(Address(0x00, 0xa000),  (uint8_t*)&basic_a000, basic_a000_size);
    Rom kernal = Rom(Address(0x00, 0xe000), (uint8_t*)&basic_e000 , basic_e000_size);

    Ram ram = Ram(0x2);

    // ret used return from interferred calls
    ram.storeByte(Address(0x00, 0x8000), 0x60);

    Log::vrb(LOG_TAG).str("basic").hex(basic.readByte(Address(0x00,0xa000))).hex(basic.readByte(Address(0x00,0xa001))).show();

for (int i=0;i<10;i++) {
        Log::vrb(LOG_TAG).str("basic").hex(kernal.readByte(Address(0x00, 0xE419+i))).show();
}



    SystemBus systemBus = SystemBus();
//    systemBus.registerDevice(&video);
    systemBus.registerDevice(&basic);
    systemBus.registerDevice(&kernal);
    systemBus.registerDevice(&ram);

    Cpu65816 cpu(systemBus, &emulationInterrupts, &nativeInterrupts);
    Cpu65816Debugger debugger(cpu);
    debugger.setBreakPoint(Address(0x00, 0x0005));
    debugger.doBeforeStep([]() {});
    debugger.doAfterStep([]() {});
    // make basic happy
    cpu.getStack()->push8Bit(0);
    cpu.getStack()->push8Bit(0);
    cpu.getStack()->push8Bit(0);
    cpu.getStack()->push8Bit(0);

    debugger.dumpCpu();

    bool breakPointHit = false;
    debugger.onBreakPoint([&breakPointHit]()  {
        breakPointHit = true;
    });

//    char buffer[] = "10 PRINT \"HELLO\"\rLIST\r";
    char buffer[] = "RUN\r";
    int ix=0;
    int loaded = false;

    while (!breakPointHit) {
        switch (cpu.getProgramAddress().getOffset()) {
                case 0xff90:
                cpu.setProgramAddress(Address(0x0,0x8000));
                break;
                case 0xff99:  // MEMTOP
                cpu.setXL( 0x7fff & 0xFF);
                cpu.setYL(  0x7fff >> 8);
//                debugger.dumpCpu();
                cpu.setProgramAddress(Address(0x0,0x8000));
                break;
                case 0xff9c:  // MEMBOT
                cpu.setXL( 0x0401 & 0xFF);
                cpu.setYL(  0x0401 >> 8);
//                debugger.dumpCpu();
                cpu.setProgramAddress(Address(0x0,0x8000));
                break;
                case 0xffcc:  // CLRCHN
                char ch;

                 Log::vrb(LOG_TAG).str("CLRCHN").hex(cpu.getA()).show();
                cpu.setProgramAddress(Address(0x0,0x8000));
                break;
                case 0xffcf:  // CHRIN

                if (!loaded) {
        loaded = true;
                            for (int i=0;i<gfx_prg_len-2;i++) {
      ram.storeByte(Address(0x00, 0x401+i), gfx_prg[i+2]);
    }

}
                 Log::vrb(LOG_TAG).str("*******CHRIN").hex(cpu.getA()).show();
                unsigned char ci;
                ci = video.chrin();
                cpu.setA(0);
                if (ix > sizeof(buffer)) {
                      cpu.setX(0);
                      SDL_Delay(20009);
                } else {
                cpu.setA(buffer[ix++]);
                  
                }
                cpu.getCpuStatus()->clearCarryFlag();

                cpu.setProgramAddress(Address(0x0,0x8000));
                    printf("CHRIN ");

                    for (int i=0;i<40;i++) {
            printf("%02x ", ram.readByte(Address(0x00, 0x400+i)));
    }
    printf("\n");
                break;
                case 0xffd2:  // CHROUT
                Log::vrb(LOG_TAG).str("CHROUT").hex(cpu.getA()).show();

 
                 char s[2] ;
                 s[0] = cpu.getA() & 0xff;
                 s[1] = 0;
                std::cout << s;
                video.chrout(cpu.getA() & 0xff);
                cpu.getCpuStatus()->clearCarryFlag();
                cpu.setProgramAddress(Address(0x0,0x8000));
                break;
                case 0xffe1:  // STOP
                cpu.getCpuStatus()->clearZeroFlag();
                cpu.setProgramAddress(Address(0x0,0x8000));
                break;
                case 0xffe7:  // STOP
//                cpu.getCpuStatus()->clearZeroFlag();
                cpu.setProgramAddress(Address(0x0,0x8000));
                break;
       }
        debugger.step();
    }



    debugger.dumpCpu();

    Log::vrb(LOG_TAG).str("+++ Program completed +++").show();
}
