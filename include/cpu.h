/*
 * This emulates the MOS6502 variant used in the NES. The MOS6502 was fairly popular (also used in Apple I, Apple II, Atari, Commodore 64, BBC Micro, etc) so there are a few details worth noting. The NES chip ran at 1.79MHz, with Binary Coded Decimal mode disabled. Despite the 16-bit (64KB) address space, the NES only had 2KB of RAM leave anything above 0x0800 to the PPU, APU, input devices, etc.
 *
 */

#ifndef CPU_H
#define CPU_H
#define CPU_MEM_SIZE 2048

#include <cstdlib>
#include <cstdint>
#include <map>
#include <string>
#include <iostream>

using std::int8_t;
using std::map;
using std::malloc;
using std::string;

class MOS6502 {
    public:
        MOS6502();
        void cycle();

    private:
        map<string, int8_t> registers;
        int16_t PC;
        int8_t* memory;
};
#endif
