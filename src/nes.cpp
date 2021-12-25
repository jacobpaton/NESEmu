#include "nes.h"

int main() {
    // Setup hardware - the NES used a modified version of the MOS6502, a PPU (Picture Processing Unit), APU (Audio Processing Unit), and a variety of mappers that were hosted on cartridge. See the respective header files for details.
    // Hardware will be added as implemented
    MOS6502 cpu;

    bool quit = false;
    int cycleCount = 0;
    int cyclesNeeded;

    // Main loop
    while (!quit) {
        std::cout << "main loop" << std::endl;
        // Complete the next instruction and perform the 
        cyclesNeeded = cpu.execNextInst();
        // ppu.execCycles(cyclesNeeded);
    }

    return 0;
}
