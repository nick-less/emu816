
#include <iterator>
#include "Log.hpp"
#include "ram.hpp"
#include "rom.hpp"
#include "basic.hpp"
#include "video.hpp"

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

    Rom basic = Rom(Address(0x00, 0xa000),  (uint8_t*)&basic_a000, basic_a000_size);
    Rom kernal = Rom(Address(0x00, 0xe000), (uint8_t*)&basic_e000 , basic_e000_size);

    Ram ram = Ram(0x2);
    ram.storeByte(Address(0x00, 0x0000), 0x18);
    ram.storeByte(Address(0x00, 0x0001), 0xFB);
    ram.storeByte(Address(0x00, 0x0002), 0xA9);
    ram.storeByte(Address(0x00, 0x0003), 0x65);
    ram.storeByte(Address(0x00, 0x0004), 0x12);

    // ret used return from interferred calls
    ram.storeByte(Address(0x00, 0x0100), 0x60);

    Log::vrb(LOG_TAG).str("basic").hex(basic.readByte(Address(0x00,0xa000))).hex(basic.readByte(Address(0x00,0xa001))).show();


    SystemBus systemBus = SystemBus();
    systemBus.registerDevice(&video);
    systemBus.registerDevice(&basic);
    systemBus.registerDevice(&kernal);
    systemBus.registerDevice(&ram);

    Cpu65816 cpu(systemBus, &emulationInterrupts, &nativeInterrupts);
    Cpu65816Debugger debugger(cpu);
    debugger.setBreakPoint(Address(0x00, 0x0005));
    debugger.doBeforeStep([]() {});
    debugger.doAfterStep([]() {});

    bool breakPointHit = false;
    debugger.onBreakPoint([&breakPointHit]()  {
        breakPointHit = true;
    });

    while (!breakPointHit) {
        switch (cpu.getProgramAddress().getOffset()) {
                case 0xff90:
                cpu.setProgramAddress(Address(0x0,0x100));
                break;
                case 0xff99:  // MEMTOP
                cpu.setXL( 0x1C00 & 0xFF);
                cpu.setYL(  0x1C00 >> 8);
                debugger.dumpCpu();
                cpu.setProgramAddress(Address(0x0,0x100));
                break;
                case 0xff9c:  // MEMBOT
                cpu.setXL( 0x0313 & 0xFF);
                cpu.setYL(  0x0313 >> 8);
                debugger.dumpCpu();
                cpu.setProgramAddress(Address(0x0,0x100));
                break;
                case 0xffcc:  // CLRCHN
                char ch;
                cpu.setProgramAddress(Address(0x0,0x100));
                break;
                case 0xffcf:  // CHRIN
                while (1);
                char ci;
                std::cin >> ci;
                cpu.setA(ci);
                cpu.setProgramAddress(Address(0x0,0x100));
                break;
                case 0xffd2:  // CHROUT
                std::cout << cpu.getA();
                cpu.setProgramAddress(Address(0x0,0x100));
                break;
        }
        debugger.step();
    }

    debugger.dumpCpu();

    Log::vrb(LOG_TAG).str("+++ Program completed +++").show();
}