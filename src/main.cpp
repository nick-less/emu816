

#include "Log.hpp"
#include "ram.hpp"
#include "rom.hpp"
#include "basic.hpp"

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
        .reset = 0x0000,
        .brkIrq = 0x0000
};

int main(int argc, char **argv) {
    Log::vrb(LOG_TAG).str("+++ Lib65816 Sample Programs +++").show();

    Rom basic = Rom(Address(0x00, 0xa000),  (uint8_t*)&basic_a000, 0x3fff);
    Rom kernal = Rom(Address(0x00, 0xe000), (uint8_t*)&basic_e000 , 0x1fff);

    Ram ram = Ram(0x2);
    ram.storeByte(Address(0x00, 0x0000), 0x18);
    ram.storeByte(Address(0x00, 0x0001), 0xFB);
    ram.storeByte(Address(0x00, 0x0002), 0xA9);
    ram.storeByte(Address(0x00, 0x0003), 0x65);
    ram.storeByte(Address(0x00, 0x0004), 0x12);

    Log::vrb(LOG_TAG).str("Reset").hex(kernal.readByte(Address(0x00,0xFFFC))).hex(kernal.readByte(Address(0x00,0xFFFD))).show();


    SystemBus systemBus = SystemBus();
    systemBus.registerDevice(&ram);
    systemBus.registerDevice(&basic);
    systemBus.registerDevice(&kernal);

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
        debugger.step();
    }

    debugger.dumpCpu();

    Log::vrb(LOG_TAG).str("+++ Program completed +++").show();
}