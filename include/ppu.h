#include <stdlib.h>

#ifndef PPU_H
#define PPU_MEM_SIZE 256

class PPU {
    public:
        PPU();

    private:
        uint8_t* memory;
};
#endif
