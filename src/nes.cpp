#include "nes.h"

NES::NES(const char* romFileName) {
    // Setup hardware - the NES used a modified version of the MOS6502, a PPU (Picture
    // Processing Unit), APU (Audio Processing Unit), and a variety of mappers that were
    // hosted on cartridge. See the respective header files for details.
    // Hardware will be added as implemented
    MOS6502 tempCPU;
    cpu = &tempCPU;
   
    // Read rom file
    ifstream romFile(romFileName, std::ios::binary);
    if (romFile.is_open()) {
        Cartridge cartridge;

        char buf[HEADER_SIZE];
        romFile.read(buf, HEADER_SIZE);
        cartridge.header = reinterpret_cast<ROMHeader*>(buf);

        if (cartridge.header->flags6 & 0x4) {
            char trainerBuf[TRAINER_SIZE];
            romFile.read(trainerBuf, TRAINER_SIZE);
        }

        uint32_t prgSize = cartridge.header->prgSize * 16384;
        cartridge.prgROM.resize(prgSize);
        romFile.read(reinterpret_cast<char*>(cartridge.prgROM.data()), prgSize);

        uint32_t chrSize = cartridge.header->chrSize * 8192;

        if (chrSize > 0) {
            cartridge.chrROM.resize(chrSize);
            romFile.read(reinterpret_cast<char*>(cartridge.chrROM.data()), chrSize);
        }

        romFile.close();
    }
}

void NES::run() {
    cpu->nes = this;
    bool quit = false;
    int cycleCount = 0;

    // Main loop
    while (!quit) {
        // Each master clock cycle should be one ppu clock cycle and every 3rd master clock
        // cycle should be one cpu clock cycle as the ppu's frequency is 3x faster.

        // ppu.cycle();
        if (cycleCount % 3 == 0)
            cpu->cycle();

        cycleCount++;
    }
}

uint8_t NES::readMem(uint16_t addr) {
    return 0x0u;
}

void NES::writeMem(uint16_t addr, uint8_t val) {
    cpu->memory[addr] = val;
}
