
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
    ram.storeByte(Address(0x00, 0x8000), 0x60);

    Log::vrb(LOG_TAG).str("basic").hex(basic.readByte(Address(0x00,0xa000))).hex(basic.readByte(Address(0x00,0xa001))).show();

for (int i=0;i<10;i++) {
        Log::vrb(LOG_TAG).str("basic").hex(kernal.readByte(Address(0x00, 0xE419+i))).show();
}



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

    while (!breakPointHit) {
        switch (cpu.getProgramAddress().getOffset()) {
                case 0xff90:
                cpu.setProgramAddress(Address(0x0,0x8000));
                break;
                case 0xff99:  // MEMTOP
                cpu.setXL( 0x1C00 & 0xFF);
                cpu.setYL(  0x1C00 >> 8);
//                debugger.dumpCpu();
                cpu.setProgramAddress(Address(0x0,0x8000));
                break;
                case 0xff9c:  // MEMBOT
                cpu.setXL( 0x0313 & 0xFF);
                cpu.setYL(  0x0313 >> 8);
//                debugger.dumpCpu();
                cpu.setProgramAddress(Address(0x0,0x8000));
                break;
                case 0xffcc:  // CLRCHN
                char ch;
                 Log::vrb(LOG_TAG).str("CLRCHN").hex(cpu.getA()).show();
                cpu.setProgramAddress(Address(0x0,0x8000));
                break;
                case 0xffcf:  // CHRIN
                 Log::vrb(LOG_TAG).str("CHRIN").hex(cpu.getA()).show();
                char ci;
                std::cin >> ci;
                cpu.setA(ci);
                cpu.setProgramAddress(Address(0x0,0x8000));
                break;
                case 0xffd2:  // CHROUT
                 //Log::vrb(LOG_TAG).str("CHROUT").hex(cpu.getA()).show();
                 char s[2] ;
                 s[0] = cpu.getA() & 0xff;
                 s[1] = 0;
                std::cout << s;
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
/*
Cpu65816Debugger: $00:$b91d | $69 ADC #$01
Cpu65816Debugger: $00:$b91f | $06 ASL $70                       [Direct Page]
RAM: read$0$70 $0
RAM: store$0$70 $0
Cpu65816Debugger: $00:$b921 | $26 ROL $65                       [Direct Page]
RAM: read$0$65 $0
RAM: store$0$65 $0
Cpu65816Debugger: $00:$b923 | $26 ROL $64                       [Direct Page]
RAM: read$0$64 $0
RAM: store$0$64 $0
Cpu65816Debugger: $00:$b925 | $26 ROL $63                       [Direct Page]
RAM: read$0$63 $d6
RAM: store$0$63 $ac
Cpu65816Debugger: $00:$b927 | $26 ROL $62                       [Direct Page]
RAM: read$0$62 $31
RAM: store$0$62 $63

69 01 06 70 26 65 26 64 26 83 26 62 10 f2

*/

/*
#define RAM_BOT 0x0313 // everything above is unused by BASIC
#define RAM_TOP sizeof(ram)

unsigned char ram[0x1C00];

int main2(void)
{

    reset6502();
    // cbmbasic assumes the upper few bytes of the stack are unused
    sp -= 4;


    while (1) {
        
        if (pc == 0) {
            // If we hit this, it's probably because of an
            // unimplemented KERNAL call. Turn on the #if 0
            // above to find out what was called.
            while(1); // PANIC
        }

        bool ret = false;
        if (pc == 0xff90) {
            ret = true;
        } else if (pc == 0xff99) { // MEMTOP
            x = RAM_TOP & 0xFF;
            y = RAM_TOP >> 8;
            ret = true;
        } else if (pc == 0xff9c) { // MEMBOT
            x = RAM_BOT & 0xFF;
            y = RAM_BOT >> 8;
            ret = true;
        } else if (pc == 0xffcc) { // CLRCHN
            ret = true;
        } else if (pc == 0xffcf) { // CHRIN
      //      a = input_get(); // may block
a = 0;
            // echo
            char s[2];
            s[0] = a;
            s[1] = 0;

            //usb_puts(s);

            if (a == '\n') {
                a = '\r';
            }
            status &= 0xfe;
            ret = true;
        } else if (pc == 0xffd2) { // CHROUT
     
     
            status &= 0xfe; // C = 0
            ret = true;
        } else if (pc == 0xffe1) { // STOP
            status &= 0xfd; // Z = 0
            ret = true;
        } else if (pc == 0xffe7) { // CLALL
            ret = true;
        }
        
        if (ret) {
            pc = (ram[0x100+sp+1] | ram[0x100+sp+2] << 8) + 1;
            sp += 2;
            continue;
        }

        step6502();
    }
}

uint8_t read6502(uint16_t address)
{
    if (address >= 0xa000 && address <= 0xbfff) {
        return basic_a000[address - 0xa000];
    } else if (address >= 0xe000 && address <= 0xe4b6) {
        return basic_e000[address - 0xe000];
    } else if (address >= 0xfffc && address <= 0xfffd) {
        return basic_a000[address - 0xfffc]; // map reset vector to BASIC start vector
    } else if (address < sizeof(ram)) {
        return ram[address];
    } else {
        return 0;
    }
}

void write6502(uint16_t address, uint8_t value)
{
    if (address < sizeof(ram)) {
        ram[address] = value;
    }
}
*/