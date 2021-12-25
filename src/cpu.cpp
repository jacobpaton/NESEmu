#include "cpu.h"

MOS6502::MOS6502() {
    // Setup registers and memory
    registers["A"] = 0;
    registers["X"] = 0;
    registers["Y"] = 0;
    registers["P"] = 0;
    registers["SP"] = 0;
    this->PC = 0;
    this->memory = (int8_t*) malloc(CPU_MEM_SIZE * sizeof(int8_t));

    // Setup instruction to function ptr map

}

void MOS6502::cycle() {
    // Each cpu cycle needs to fetch, decode, and execute
    
}
