#include "cpu.h"

MOS6502::MOS6502() {
    // Setup registers, memory, and cycles counter
    registers["A"] = 0x0u;  // accumulator
    registers["X"] = 0x0u;  // x register
    registers["Y"] = 0x0u;  // y register
    registers["P"] = 0x0u;  // status
    registers["SP"] = 0x0u; // stack pointer
    pc = 0x0u;              // program counter
    memory = (uint8_t*) malloc(CPU_MEM_SIZE * sizeof(uint8_t));
    cyclesRemaining = 0;
}

void MOS6502::cycle() {
    // Each instruction takes a different number of cycles to execute and other hardware
    // expect results completed after that many cycles, therefore, we can execute on the first
    // cycle and then wait until the final cycle has completed to execute the next instruction.

    if (cyclesRemaining == 0) {
        // Read next inst
        uint8_t opcode = memory[pc];
        // Increment pc
        pc++;
        // Calculate number of required cycles for inst
        cyclesRemaining = oplist[opcode].cycles;
        // Calculate number of additional required cycles for addressing mode and opcode
        pageBoundaryCrossed = false;
        cyclesRemaining += (this->*oplist[opcode].addrmode)();
        cyclesRemaining += (this->*oplist[opcode].execute)();
    }

    cyclesRemaining--;
}

void MOS6502::reset() {
    // TODO: Reset CPU state
}

void MOS6502::irq() {
    // TODO: Handle interrupts
}

void MOS6502::nmi() {
    // TODO: Handle non-maskable interrupts
}

uint8_t MOS6502::fetch() {
    // Read from memory at the absolute address unless in implicit addr mode
    // in which case we return the data directly from the byte at pc
    if (!(oplist[opcode].addrmode == &MOS6502::IMP))
        fetched = readMem(addr_abs);
    return fetched;
}

uint8_t MOS6502::readMem(uint16_t addr) {
    return nes->readMem(addr);
}

void MOS6502::writeMem(uint16_t addr, uint8_t val) {
    nes->writeMem(addr, val);
}

uint8_t MOS6502::getFlag(STATUSFLAGS flag) {
    return (uint8_t)(registers["P"] & flag);
}

void MOS6502::setFlag(STATUSFLAGS flag, bool val) {
    if (val)
        registers["P"] |= flag;
    else
        registers["P"] &= ~flag;
}

// Instruction function declarations
uint8_t MOS6502::ADC() {
    // Fetch neccessary data
    fetch();

    // Calculate sum
    uint16_t sum = (uint16_t) registers["A"] + (uint16_t) fetched + (uint16_t) getFlag(C);

    // Set flags
    setFlag(C, sum > 255);              // set carry bit if sum larger than 2^8 - 1
    setFlag(Z, (sum & 0x00FF) == 0);    // set zero bit if sum = 0
    setFlag(N, sum & 0x80);             // negative bit is set to most significant bit
    setFlag(O, (~((uint16_t)registers["A"] ^ (uint16_t)fetched)
                & ((uint16_t)registers["A"] ^ (uint16_t)sum)) & 0x0080);
                                        // overflow bit is set based on most sig. bit of formula
    
    // Convert sum to 8 bits and store in accumulator
    registers["A"] = sum & 0x00FF;

    // Can take an additional cycle
    return pageBoundaryCrossed ? 1u : 0u;
}

uint8_t MOS6502::AND() {
    // Fetch neccessary data
    fetch();

    // Compute bitwise and
    registers["A"] &= fetched;

    // Set flags
    setFlag(Z, (registers["A"] & 0x00FF) == 0);   // set zero bit if res = 0
    setFlag(N, registers["A"] & 0x80);            // negative bit is set to most significant bit

    // Can take an additional cycle
    return pageBoundaryCrossed ? 1u : 0u;
}

uint8_t MOS6502::ASL() {
    // Fetch neccessary data
    if (oplist[opcode].addrmode == &MOS6502::IMP){
        fetched = registers["A"];
    } else {
        fetch();
    }

    // Compute shift
    uint16_t shifted = ((uint16_t) fetched) << 1;

    // Set flags
    setFlag(C, (shifted & 0xFF00) > 0);
    setFlag(Z, (shifted & 0x00ff) == 0x00);
    setFlag(N, shifted & 0x80);

    if (oplist[opcode].addrmode == &MOS6502::IMP) {
        registers["A"] = shifted & 0x00FF;
    } else {
        writeMem(addr_abs, shifted & 0x00FF);
    }
    
    return 0u;
}

uint8_t MOS6502::BCC() {
    if (!getFlag(C)) {
        pc = addr_abs;

        // Can take an additional cycle
        return pageBoundaryCrossed ? 2u : 1u;
    }

    return 0u;
}

uint8_t MOS6502::BCS() {
    if (!getFlag(C)) {
        pc = addr_abs;

        // Can take an additional cycle
        return pageBoundaryCrossed ? 2u : 1u;
    }

    return 0u;
}

uint8_t MOS6502::BEQ() {
    if (getFlag(Z)) {
        pc = addr_abs;

        // Can take an additional cycle
        return pageBoundaryCrossed ? 2u : 1u;
    }
    
    return 0u;
}
uint8_t MOS6502::BIT() {
    // Fetch neccessary data
    fetch();
    
    uint16_t res = registers["A"] & fetched;

    // Set flags
    setFlag(Z, (res & 0x00FF) == 0x00);
    setFlag(N, fetched & N);
    setFlag(O, fetched & O);

    return 0u;
}

uint8_t MOS6502::BMI() {
    if (getFlag(N)) {
        pc = addr_abs;

        // Can take an additional cycle
        return pageBoundaryCrossed ? 2u : 1u;
    }

    return 0u;
}

uint8_t MOS6502::BNE() {
    if (!getFlag(Z)) {
        pc = addr_abs;

        // Can take an additional cycle
        return pageBoundaryCrossed ? 2u : 1u;
    }
    
    return 0u;
}

uint8_t MOS6502::BPL() {
    if (!getFlag(N)) {
        pc = addr_abs;

        // Can take an additional cycle
        return pageBoundaryCrossed ? 2u : 1u;
    }
    
    return 0u;
}

uint8_t MOS6502::BRK() {
    pc++;

    setFlag(I, 1);
    writeMem(0x0100 + registers["SP"], (pc >> 8) & 0x00FF);
    registers["SP"]--;
    writeMem(0x0100 + registers["SP"], pc & 0x00FF);
    registers["SP"]--;

    setFlag(B, 1);
    writeMem(0x0100 + registers["SP"], registers["P"]);
    registers["SP"]--;
    setFlag(B, 0);

    pc = (uint16_t) readMem(0xFFFE) | ((uint16_t) readMem(0xFFFF) << 8);

    return 0u;
}

uint8_t MOS6502::BVC() {
    if (!getFlag(O)) {
        pc = addr_abs;

        // Can take an additional cycle
        return pageBoundaryCrossed ? 2u : 1u;
    }
    
    return 0u;
}

uint8_t MOS6502::BVS() {
    if (getFlag(O)) {
        pc = addr_abs;

        // Can take an additional cycle
        return pageBoundaryCrossed ? 2u : 1u;
    }
    
    return 0u;
}

uint8_t MOS6502::CLC() {
    setFlag(C, 0);

    return 0u;
}

uint8_t MOS6502::CLD() {
    setFlag(D, 0);

    return 0u;
}

uint8_t MOS6502::CLI() {
    setFlag(I, 0);

    return 0u;
}

uint8_t MOS6502::CLV() {
    setFlag(O, 0);

    return 0u;
}
uint8_t MOS6502::CMP() {
    // Fetch neccessary data
    fetch();

    uint16_t res = registers["A"] - fetched;
    setFlag(Z, (res & 0x00FF) == 0);        // set zero bit if res = 0
    setFlag(N, res & 0x80);                 // negative bit is set to most significant bit
    setFlag(C, registers["A"] >= fetched);  // set carry if accumulator is bigger than the data from memory

    // Can take an additional cycle
    return pageBoundaryCrossed ? 1u : 0u;
}

uint8_t MOS6502::CPX() {
    // Fetch neccessary data
    fetch();

    // Perform subtraction and set flags
    uint16_t res = (uint16_t) registers["X"] - (uint16_t) fetched;
    setFlag(Z, (res & 0x00FF) == 0);        // set zero bit if res = 0
    setFlag(N, res & 0x80);                 // negative bit is set to most significant bit
    setFlag(C, registers["X"] >= fetched);  // set carry if x is bigger than the data from memory

    return 0u;
}

uint8_t MOS6502::CPY() {
    // Fetch neccessary data
    fetch();

    // Perform subtraction and set flags
    uint16_t res = (uint16_t) registers["Y"] - (uint16_t) fetched;
    setFlag(Z, (res & 0x00FF) == 0);        // set zero bit if res = 0
    setFlag(N, res & 0x80);                 // negative bit is set to most significant bit
    setFlag(C, registers["Y" ] >= fetched); // set carry if y is bigger than the data from memory

    return 0u;
}

uint8_t MOS6502::DEC() {
    // Read from mem, decrement, and write back to mem
    uint8_t res = readMem(addr_abs);
    res--;
    writeMem(addr_abs, res);

    // Set flags
    setFlag(Z, (registers["A"] & 0x00FF) == 0);   // set zero bit if res = 0
    setFlag(N, registers["A"] & 0x80);            // negative bit is set to most significant bit

    return 0u;
}

uint8_t MOS6502::DEX() {
    // Decrement X
    registers["X"]--;

    // Set flags
    setFlag(Z, (registers["X"] & 0x00FF) == 0);   // set zero bit if res = 0
    setFlag(N, registers["X"] & 0x80);            // negative bit is set to most significant bit

    return 0u;
}

uint8_t MOS6502::DEY() {
    // Decrement Y
    registers["Y"]--;

    // Set flags
    setFlag(Z, (registers["Y"] & 0x00FF) == 0);   // set zero bit if res = 0
    setFlag(N, registers["Y"] & 0x80);            // negative bit is set to most significant bit

    return 0u;
}

uint8_t MOS6502::EOR() {
    // Fetch neccessary data
    fetch();

    // Compute bitwise and
    registers["A"] ^= fetched;

    // Set flags
    setFlag(Z, (registers["A"] & 0x00FF) == 0);   // set zero bit if res = 0
    setFlag(N, registers["A"] & 0x80);            // negative bit is set to most significant bit

    // Can take an additional cycle
    return pageBoundaryCrossed ? 1u : 0u;
}

uint8_t MOS6502::INC() {
    // Read from mem, increment, and write back to mem
    uint8_t res = readMem(addr_abs);
    res++;
    writeMem(addr_abs, res);

    // Set flags
    setFlag(Z, (registers["A"] & 0x00FF) == 0);   // set zero bit if res = 0
    setFlag(N, registers["A"] & 0x80);            // negative bit is set to most significant bit

    return 0u;
}

uint8_t MOS6502::INX() {
    // Increment X
    registers["X"]++;

    // Set flags
    setFlag(Z, (registers["X"] & 0x00FF) == 0);   // set zero bit if res = 0
    setFlag(N, registers["X"] & 0x80);            // negative bit is set to most significant bit

    return 0u;
}

uint8_t MOS6502::INY() {
    // Increment Y
    registers["Y"]++;

    // Set flags
    setFlag(Z, (registers["Y"] & 0x00FF) == 0);   // set zero bit if res = 0
    setFlag(N, registers["Y"] & 0x80);            // negative bit is set to most significant bit

    return 0u;
}

uint8_t MOS6502::JMP() {
    // Set next to the address we're jumping to
    pc = addr_abs;

    return 0u;
}

uint8_t MOS6502::JSR() {
    // Push pc to stack
    pc--;
    writeMem(0x0100 + registers["SP"], (pc >> 8) & 0x00FF);
    registers["SP"]--;
    writeMem(0x0100 + registers["SP"], pc & 0xFF00);
    registers["SP"]--;

    pc = addr_abs;

    return 0u;
}

uint8_t MOS6502::LDA() {
    // Fetch neccessary data
    fetch();

    // Write fetched data to accumulator
    registers["A"] = fetched;

    // Set flags
    setFlag(Z, (registers["A"] & 0x00FF) == 0);   // set zero bit if res = 0
    setFlag(N, registers["A"] & 0x80);            // negative bit is set to most significant bit

    // Can take an additional cycle
    return pageBoundaryCrossed ? 1u : 0u;
}

uint8_t MOS6502::LDX() { 
    // Fetch neccessary data
    fetch();

    // Write fetched data to X register
    registers["X"] = fetched;

    // Set flags
    setFlag(Z, (registers["X"] & 0x00FF) == 0);   // set zero bit if res = 0
    setFlag(N, registers["X"] & 0x80);            // negative bit is set to most significant bit

    // Can take an additional cycle
    return pageBoundaryCrossed ? 1u : 0u;
}

uint8_t MOS6502::LDY() {
    // Fetch neccessary data
    fetch();

    // Write fetched data to X register
    registers["Y"] = fetched;

    // Set flags
    setFlag(Z, (registers["Y"] & 0x00FF) == 0);   // set zero bit if res = 0
    setFlag(N, registers["Y"] & 0x80);            // negative bit is set to most significant bit

    // Can take an additional cycle
    return pageBoundaryCrossed ? 1u : 0u;
}

uint8_t MOS6502::LSR() {
    // Fetch neccessary data
    if (oplist[opcode].addrmode == &MOS6502::IMP){
        fetched = registers["A"];
    } else {
        fetch();
    }

    // Set carry to bit shifted out
    setFlag(C, fetched & 0x01);

    // Compute shift
    uint16_t shifted = ((uint16_t) fetched) >> 1;

    // Set flags
    setFlag(Z, (shifted & 0x00ff) == 0x00);
    setFlag(N, shifted & 0x80);

    if (oplist[opcode].addrmode == &MOS6502::IMP) {
        registers["A"] = shifted & 0x00FF;
    } else {
        writeMem(addr_abs, shifted & 0x00FF);
    }

    return 0u;
}

uint8_t MOS6502::NOP() {
    return 0u;
}

uint8_t MOS6502::ORA() {
    // Fetch neccessary data
    fetch();

    // Compute bitwise and
    registers["A"] |= fetched;

    // Set flags
    setFlag(Z, (registers["A"] & 0x00FF) == 0);   // set zero bit if res = 0
    setFlag(N, registers["A"] & 0x80);            // negative bit is set to most significant bit

    // Can take an additional cycle
    return pageBoundaryCrossed ? 1u : 0u;
}

uint8_t MOS6502::PHA() {
    writeMem(0x0100 + registers["SP"], registers["A"]);
    registers["SP"]--;

    return 0u;
}

uint8_t MOS6502::PHP() {
    writeMem(0x0100 + registers["SP"], registers["P"] | B | U);
    setFlag(B, 0);
    setFlag(U, 0);
    registers["SP"]--;

    return 0u;
}

uint8_t MOS6502::PLA() {
    registers["SP"]++;
    readMem(0x0100 + registers["SP"]);
    setFlag(Z, registers["A"] ? 0u : 1u);
    setFlag(N, registers["A"] & 0x80);

    return 0u;
}

uint8_t MOS6502::PLP() {
    registers["SP"]++;
    registers["P"] = readMem(0x0100 + registers["SP"]);
    setFlag(U, 1);

    return 0u;
}

uint8_t MOS6502::ROL() {
    if (oplist[opcode].addrmode == &MOS6502::IMP) {
        fetched = registers["A"];
    } else {
        fetch();
    }

    uint8_t prevCarry = getFlag(C);

    // Store MSB of fetched in carry
    setFlag(C, (fetched & 0x80) == 0x80);
    
    fetched <<= 1u;

    // Store old carry at LSB
    if (prevCarry) {
        fetched |= 0x1u;
    }

    setFlag(Z, fetched ? 0u : 1u);
    setFlag(N, fetched & 0x80);            // negative bit is set to most significant bit

    if (oplist[opcode].addrmode == &MOS6502::IMP) {
        registers["A"] = fetched;
    } else {
        writeMem(addr_abs, fetched);
    }

    return 0u;
}

uint8_t MOS6502::ROR() {
    if (oplist[opcode].addrmode == &MOS6502::IMP) {
        fetched = registers["A"];
    } else {
        fetch();
    }

    uint8_t prevCarry = getFlag(C);

    // Store MSB of fetched in carry
    setFlag(C, (fetched & 0x01) == 0x01);
    
    fetched >>= 1u;

    // Store old carry at LSB
    if (prevCarry) {
        fetched |= 0x80u;
    }

    setFlag(Z, fetched ? 0u : 1u);
    setFlag(N, fetched & 0x80);            // negative bit is set to most significant bit

    if (oplist[opcode].addrmode == &MOS6502::IMP) {
        registers["A"] = fetched;
    } else {
        writeMem(addr_abs, fetched);
    }

    return 0u;
}

uint8_t MOS6502::RTI() {
    // Pop status from stack
    registers["SP"]++;
    registers["P"] = readMem(0x0100 + registers["SP"]);
    registers["P"] &= ~B;
    registers["P"] &= ~U;

    // Pop return address from stack (and shift + or to compose it)
    registers["SP"]++;
    pc = (uint16_t) readMem(0x0100 + registers["SP"]);
    registers["SP"]++;
    pc |= (uint16_t) readMem(0x0100 + registers["SP"]) << 8;

    return 0u;
}

uint8_t MOS6502::RTS() {
    // Pop return address from stack (and shift + or to compose it)
    registers["SP"]++;
    pc = (uint16_t) readMem(0x0100 + registers["SP"]);
    registers["SP"]++;
    pc |= (uint16_t) readMem(0x0100 + registers["SP"]) << 8;

    return 0u;
}

uint8_t MOS6502::SBC() {
    // Fetch neccessary data
    fetch();

    // 2s complement subtraction
    uint16_t complement = ((uint16_t) fetched) ^ 0x00FF;
    uint16_t res = (uint16_t) registers["A"] + complement + (uint16_t) getFlag(C);
    setFlag(C, res & 0xFF00);
    setFlag(Z, ((res & 0x00FF) == 0));
    setFlag(O, (res ^ (uint16_t) registers["A"]) & (res ^ complement) & 0x0080);
    setFlag(N, res & 0x0080);
    registers["A"] = res & 0x00FF;

    // Can take an additional cycle
    return pageBoundaryCrossed ? 1u : 0u;
}

uint8_t MOS6502::SEC() {
    setFlag(C, 1);

    return 0u;
}

uint8_t MOS6502::SED() {
    setFlag(D, 1);

    return 0u;
}

uint8_t MOS6502::SEI() {
    setFlag(I, 1);

    return 0u;
}

uint8_t MOS6502::STA() {
    // Write A register contents to given address
    writeMem(addr_abs, registers["A"]);

    return 0u;
}

uint8_t MOS6502::STX() {
    // Write X register contents to given address
    writeMem(addr_abs, registers["X"]);

    return 0u;
}

uint8_t MOS6502::STY() {
    // Write Y register contents to given address
    writeMem(addr_abs, registers["Y"]);

    return 0u;
}

uint8_t MOS6502::TAX() {
    // Copy A to X
    registers["X"] = registers["A"];

    // Set flags
    setFlag(Z, (registers["X"] & 0x00FF) == 0);   // set zero bit if res = 0
    setFlag(N, registers["X"] & 0x80);            // negative bit is set to most significant bit

    return 0u;
}

uint8_t MOS6502::TAY() {
    // Copy A to Y
    registers["Y"] = registers["A"];

    // Set flags
    setFlag(Z, (registers["Y"] & 0x00FF) == 0);   // set zero bit if res = 0
    setFlag(N, registers["Y"] & 0x80);            // negative bit is set to most significant bit

    return 0u;
}

uint8_t MOS6502::TSX() {
    // Copy A to X
    registers["X"] = registers["SP"];

    // Set flags
    setFlag(Z, (registers["X"] & 0x00FF) == 0);   // set zero bit if res = 0
    setFlag(N, registers["X"] & 0x80);            // negative bit is set to most significant bit

    return 0u;
}

uint8_t MOS6502::TXA() {
    // Copy X to A
    registers["A"] = registers["X"];

    // Set flags
    setFlag(Z, (registers["A"] & 0x00FF) == 0);   // set zero bit if res = 0
    setFlag(N, registers["A"] & 0x80);            // negative bit is set to most significant bit

    return 0u;
}

uint8_t MOS6502::TXS() {
    // Copy A to X
    registers["SP"] = registers["X"];

    return 0u;
}

uint8_t MOS6502::TYA() {
    // Copy Y to A
    registers["A"] = registers["Y"];

    // Set flags
    setFlag(Z, (registers["A"] & 0x00FF) == 0);   // set zero bit if res = 0
    setFlag(N, registers["A"] & 0x80);            // negative bit is set to most significant bit

    return 0u;
}

// Addressing modes
uint8_t MOS6502::IMP() {
    fetched = registers["A"];

    return 0u;
}

uint8_t MOS6502::IMM() {
    addr_abs = pc;
    pc++;

    return 0u;
}

uint8_t MOS6502::ZP0() {
    addr_abs = readMem(pc);
    pc++;
    addr_abs &= 0x00FF;

    return 0u;
}

uint8_t MOS6502::ZPX() {
    addr_abs = readMem(pc) + registers["X"];
    pc++;
    addr_abs &= 0x00FF;

    return 0u;
}

uint8_t MOS6502::ZPY() {
    addr_abs = readMem(pc) + registers["Y"];
    pc++;
    addr_abs &= 0x00FF;

    return 0u;
}

uint8_t MOS6502::REL() {
    addr_abs = readMem(pc);
    pc++;
    if (addr_abs & 0x80)
        addr_abs |= 0xFF00;

    return 0u;
}

uint8_t MOS6502::ABS() {
    uint16_t lo = readMem(pc);
    pc++;
    uint16_t hi = readMem(pc);
    pc++;

    addr_abs = (hi << 8) | lo;

    return 0u;
}

uint8_t MOS6502::ABX() {
    uint16_t lo = readMem(pc);
    pc++;
    uint16_t hi = readMem(pc);
    pc++;

    addr_abs = (hi << 8) | lo;
    addr_abs += registers["X"];

    if ((addr_abs & 0xFF00) != (hi << 8))
        return 1u;

    return 0u;
}

uint8_t MOS6502::ABY() {
    uint16_t lo = readMem(pc);
    pc++;
    uint16_t hi = readMem(pc);
    pc++;
    
    addr_abs = (hi << 8) | lo;
    addr_abs += registers["Y"];

    if ((addr_abs & 0xFF00) != (hi << 8))
        return 1u;

    return 0u;
}

uint8_t MOS6502::IND() {
    uint16_t ptr_lo = readMem(pc);
    pc++;
    uint16_t ptr_hi = readMem(pc);
    pc++;

    uint16_t ptr = (ptr_hi << 8) | ptr_lo;

    if (ptr_lo == 0x00FF) // Simulate page boundary hardware bug
        addr_abs = (readMem(ptr & 0xFF00) << 8) | readMem(ptr + 0);
    else
        addr_abs = (readMem(ptr + 1) << 8) | readMem(ptr + 0);

    return 0u;
}

uint8_t MOS6502::IZX() {
    uint16_t t = readMem(pc);
    pc++;
    
    uint16_t lo = readMem((uint16_t)(t + (uint16_t)registers["X"]) & 0x00FF);
    uint16_t hi = readMem((uint16_t)(t + (uint16_t)registers["Y"] + 1) & 0x00FF);
    
    addr_abs = (hi << 8) | lo;

    return 0u;
}

uint8_t MOS6502::IZY() {
    uint16_t t = readMem(pc);
    pc++;

    uint16_t lo = readMem(t & 0x00FF);
    uint16_t hi = readMem((t + 1) & 0x00FF);

    addr_abs = (hi << 8) | lo;
    addr_abs += registers["Y"];

    if ((addr_abs & 0xFF00) != (hi << 8))
        return 1u;

    return 0u;
}
