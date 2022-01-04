#include "ppu.h"

PPU::PPU() {
    this->memory = (uint8_t*) malloc(PPU_MEM_SIZE * sizeof(uint8_t));
}
