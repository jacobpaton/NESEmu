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
        cyclesRemaining += (*(oplist[opcode].addrmode))();
        cyclesRemaining += (*(oplist[opcode].execute))();
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

uint8_t fetch() {
    if (!(oplist[opcode].addrmode == &MOS6502::IMP))
        fetched = readMem(addr_abs);
    return fetched;
}

uint8_t MOS6502::readMem(uint16_t addr) {
    return nes->readMem(addr);
}

void MOS6502::writeMem(uint16_t addr, uint8_t val) {
    return nes->writeMem(addr, val);
}

uint8_t MOS6502::getFlag(STATUSFLAGS flag) {
    return (uint8_t)(registers["P"] & flag);
}

uint8_t MOS6502::setFlag(STATUSFLAGS flag, bool val) {
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
    setFlag(Z, (sum & 0x00FF) == 0);   // set zero bit if sum = 0
    setFlag(N, sum & 0x80);            // negative bit is set to most significant bit

    // Can take an additional cycle
    return pageBoundaryCrossed ? 1u : 0u;
}

uint8_t MOS6502::ASL() {
    // Fetch neccessary data
    fetch();

    // Compute shift
    uint16_t shifted = ((uint16_t) fetched) << 1;

    // Set flags
    setFlag(C, (shifted & 0xFF00) > 0);
    setFlag(Z, (shifted & 0x00ff) == 0x00);
    setFlag(N, shift & 0x80);

    if (oplist[opcode].addrmode == &MOS6052::IMP)
        registers["A"] = shifted & 0x00FF;
    else
        writeMem(addr_abs, shifted & 0x00FF);

    return 0u;
}

uint8_t MOS6502::BCC() {return 0x0;}
uint8_t MOS6502::BCS() {return 0x0;}
uint8_t MOS6502::BEQ() {return 0x0;}
uint8_t MOS6502::BIT() {return 0x0;}
uint8_t MOS6502::BMI() {return 0x0;}
uint8_t MOS6502::BNE() {return 0x0;}
uint8_t MOS6502::BPL() {return 0x0;}
uint8_t MOS6502::BRK() {return 0x0;}
uint8_t MOS6502::BVC() {return 0x0;}
uint8_t MOS6502::BVS() {return 0x0;}
uint8_t MOS6502::CLC() {return 0x0;}
uint8_t MOS6502::CLD() {return 0x0;}
uint8_t MOS6502::CLI() {return 0x0;}
uint8_t MOS6502::CLV() {return 0x0;}
uint8_t MOS6502::CMP() {return 0x0;}
uint8_t MOS6502::CPX() {return 0x0;}
uint8_t MOS6502::CPY() {return 0x0;}
uint8_t MOS6502::DEC() {return 0x0;}
uint8_t MOS6502::DEX() {return 0x0;}
uint8_t MOS6502::DEY() {return 0x0;}
uint8_t MOS6502::EOR() {return 0x0;}
uint8_t MOS6502::INC() {return 0x0;}
uint8_t MOS6502::INX() {return 0x0;}
uint8_t MOS6502::INY() {return 0x0;}
uint8_t MOS6502::JMP() {return 0x0;}
uint8_t MOS6502::JSR() {return 0x0;}
uint8_t MOS6502::LDA() {return 0x0;}
uint8_t MOS6502::LDX() {return 0x0;}
uint8_t MOS6502::LDY() {return 0x0;}
uint8_t MOS6502::LSR() {return 0x0;}
uint8_t MOS6502::NOP() {return 0x0;}
uint8_t MOS6502::ORA() {return 0x0;}
uint8_t MOS6502::PHA() {return 0x0;}
uint8_t MOS6502::PHP() {return 0x0;}
uint8_t MOS6502::PLA() {return 0x0;}
uint8_t MOS6502::PLP() {return 0x0;}	
uint8_t MOS6502::ROL() {return 0x0;}
uint8_t MOS6502::ROR() {return 0x0;}	
uint8_t MOS6502::RTI() {return 0x0;}	
uint8_t MOS6502::RTS() {return 0x0;}	
uint8_t MOS6502::SBC() {return 0x0;}
uint8_t MOS6502::SEC() {return 0x0;}	
uint8_t MOS6502::SED() {return 0x0;}	
uint8_t MOS6502::SEI() {return 0x0;}	
uint8_t MOS6502::STA() {return 0x0;}
uint8_t MOS6502::STX() {return 0x0;}	
uint8_t MOS6502::STY() {return 0x0;}	
uint8_t MOS6502::TAX() {return 0x0;}	
uint8_t MOS6502::TAY() {return 0x0;}
uint8_t MOS6502::TSX() {return 0x0;}	
uint8_t MOS6502::TXA() {return 0x0;}	
uint8_t MOS6502::TXS() {return 0x0;}	
uint8_t MOS6502::TYA() {return 0x0;}

// Catch all for illegal opcodes which have not been implemented
uint8_t MOS6502::ILL() {return 0x0;}

// Addressing modes
uint8_t MOS6502::IMP() {return 0x0;}	
uint8_t MOS6502::IMM() {return 0x0;}
uint8_t MOS6502::ZP0() {return 0x0;}	
uint8_t MOS6502::ZPX() {return 0x0;}
uint8_t MOS6502::ZPY() {return 0x0;}	
uint8_t MOS6502::REL() {return 0x0;}
uint8_t MOS6502::ABS() {return 0x0;}	
uint8_t MOS6502::ABX() {return 0x0;}
uint8_t MOS6502::ABY() {return 0x0;}	
uint8_t MOS6502::IND() {return 0x0;}
uint8_t MOS6502::IZX() {return 0x0;} 
uint8_t MOS6502::IZY() {return 0x0;}
