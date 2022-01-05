/*
 * Nintendo Entertainment System (NES) emulator
 * Author: Jacob Paton
 */

#ifndef NES_H
#define NES_H

#include <iostream>
#include <vector>
#include <fstream>

#include "cpu.h"
#include "ppu.h"

using std::ifstream;
using std::vector;

class MOS6502;

struct ROMHeader {
    uint8_t string[4];
    uint8_t prgSize;
    uint8_t chrSize;
    uint8_t flags6;
    uint8_t flags7;
    uint8_t flags8;
    uint8_t flags9;
    uint8_t flags10;
    uint8_t padding[5];
};

struct Cartridge {
    struct ROMHeader* header;
    vector<uint8_t> prgROM;
    vector<uint8_t> chrROM;
    char* romFileName;
};

const unsigned int HEADER_SIZE = 16u;
const unsigned int TRAINER_SIZE = 512u;

class NES {
    public:
        NES(const char* romFile);
        void run();
        uint8_t readMem(uint16_t addr);
        void writeMem(uint16_t addr, uint8_t val);
    
    private:
        MOS6502* cpu;
        PPU* ppu;
};

#endif
