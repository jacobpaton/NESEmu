#include "ppu.h"

class PPU {
    public:
        PPU() {
            this->memory = (int8_t*) malloc(PPU_MEM_SIZE * sizeof(int8_t));
        }

    private:
        int8_t* memory;
};
