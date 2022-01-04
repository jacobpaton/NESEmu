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
#include <vector>

#include "nes.h"

using std::int8_t;
using std::map;
using std::malloc;
using std::string;
using std::vector;

// Enum for quick status flag access
enum STATUSFLAGS {
    C = (1 << 0),   // Carry
    Z = (1 << 1),   // Zero
    I = (1 << 2),   // Interrupt disable
    D = (1 << 3),   // Decimal Mode (N/A for NES)
    B = (1 << 4),   // Break
    U = (1 << 5),   // Unused
    O = (1 << 6),   // Overflow
    N = (1 << 7),   // Negative
};

class NES;

class MOS6502 {
    friend class NES;

    public:
        MOS6502();
        void cycle();                                   // Perform one cycle worth of work
        void reset();                                   // Reset CPU
        void irq();                                     // Interrupt request
        void nmi();                                     // Non-Maskable Interrupt Request

    private:
        NES* nes;                                       // The NES which this CPU is part of
        map<string, int8_t> registers;                  // CPU's registers
        uint16_t pc;                                    // Program Counter
        bool pageBoundaryCrossed;                       // Memory access crosses page boundaries or not
        uint8_t fetched;                                // Byte fetched for data (from addr or next byte) 
        uint8_t opcode;                                 // Instruction loaded from pc
        uint16_t addr_abs;                              // Address holder
        uint16_t addr_rel;                              // Address following a branch
        uint8_t* memory;                                // CPU's RAM
        unsigned int cyclesRemaining;                   // Number of cycles before given inst completes

        uint8_t fetch();                                // Fetch data used by inst from mem or pc+1
        uint8_t readMem(uint16_t addr);                 // Read memory at addr
        uint8_t getFlag(STATUSFLAGS flag);              // Read status flag bit
        uint8_t setFlag(STATUSFLAGS flag, bool val);    // Set status flag bit to val

        // Instruction struct to hold instruction name, function, address mode, and cycles
        struct INSTRUCTION
    	{
	    	std::string name;                           // Human readable instruction name
		    uint8_t     (MOS6502::*execute)();          // Function pointer to execute the instruction
		    uint8_t     (MOS6502::*addrmode)();         // Address mode of the instruction
		    uint8_t     cycles;                         // Number of cycles to complete base instruction
	    };

        // Instruction function declarations
        uint8_t ADC();	uint8_t AND();	uint8_t ASL();	uint8_t BCC();
	    uint8_t BCS();	uint8_t BEQ();	uint8_t BIT();	uint8_t BMI();
    	uint8_t BNE();	uint8_t BPL();	uint8_t BRK();	uint8_t BVC();
    	uint8_t BVS();	uint8_t CLC();	uint8_t CLD();	uint8_t CLI();
    	uint8_t CLV();	uint8_t CMP();	uint8_t CPX();	uint8_t CPY();
    	uint8_t DEC();	uint8_t DEX();	uint8_t DEY();	uint8_t EOR();
    	uint8_t INC();	uint8_t INX();	uint8_t INY();	uint8_t JMP();
    	uint8_t JSR();	uint8_t LDA();	uint8_t LDX();	uint8_t LDY();
    	uint8_t LSR();	uint8_t NOP();	uint8_t ORA();	uint8_t PHA();
    	uint8_t PHP();	uint8_t PLA();	uint8_t PLP();	uint8_t ROL();
    	uint8_t ROR();	uint8_t RTI();	uint8_t RTS();	uint8_t SBC();
    	uint8_t SEC();	uint8_t SED();	uint8_t SEI();	uint8_t STA();
    	uint8_t STX();	uint8_t STY();	uint8_t TAX();	uint8_t TAY();
    	uint8_t TSX();	uint8_t TXA();	uint8_t TXS();	uint8_t TYA();
    	uint8_t ILL();

        // Addressing modes
        uint8_t IMP();	uint8_t IMM();
	    uint8_t ZP0();	uint8_t ZPX();
    	uint8_t ZPY();	uint8_t REL();
    	uint8_t ABS();	uint8_t ABX();
    	uint8_t ABY();	uint8_t IND();
    	uint8_t IZX();	uint8_t IZY();

        // Setup instruction to function ptr map
        const INSTRUCTION oplist[256] = {
            { "BRK", &MOS6502::BRK, &MOS6502::IMM, 7 },{ "ORA", &MOS6502::ORA, &MOS6502::IZX, 6 },
            { "ILL", &MOS6502::ILL, &MOS6502::IMP, 2 },{ "ILL", &MOS6502::ILL, &MOS6502::IMP, 8 },
            { "ILL", &MOS6502::NOP, &MOS6502::IMP, 3 },{ "ORA", &MOS6502::ORA, &MOS6502::ZP0, 3 },
            { "ASL", &MOS6502::ASL, &MOS6502::ZP0, 5 },{ "ILL", &MOS6502::ILL, &MOS6502::IMP, 5 },
            { "PHP", &MOS6502::PHP, &MOS6502::IMP, 3 },{ "ORA", &MOS6502::ORA, &MOS6502::IMM, 2 },
            { "ASL", &MOS6502::ASL, &MOS6502::IMP, 2 },{ "ILL", &MOS6502::ILL, &MOS6502::IMP, 2 },
            { "ILL", &MOS6502::NOP, &MOS6502::IMP, 4 },{ "ORA", &MOS6502::ORA, &MOS6502::ABS, 4 },
            { "ASL", &MOS6502::ASL, &MOS6502::ABS, 6 },{ "ILL", &MOS6502::ILL, &MOS6502::IMP, 6 },
            { "BPL", &MOS6502::BPL, &MOS6502::REL, 2 },{ "ORA", &MOS6502::ORA, &MOS6502::IZY, 5 },
            { "ILL", &MOS6502::ILL, &MOS6502::IMP, 2 },{ "ILL", &MOS6502::ILL, &MOS6502::IMP, 8 },
            { "ILL", &MOS6502::NOP, &MOS6502::IMP, 4 },{ "ORA", &MOS6502::ORA, &MOS6502::ZPX, 4 },
            { "ASL", &MOS6502::ASL, &MOS6502::ZPX, 6 },{ "ILL", &MOS6502::ILL, &MOS6502::IMP, 6 },
            { "CLC", &MOS6502::CLC, &MOS6502::IMP, 2 },{ "ORA", &MOS6502::ORA, &MOS6502::ABY, 4 },
            { "ILL", &MOS6502::NOP, &MOS6502::IMP, 2 },{ "ILL", &MOS6502::ILL, &MOS6502::IMP, 7 },
            { "ILL", &MOS6502::NOP, &MOS6502::IMP, 4 },{ "ORA", &MOS6502::ORA, &MOS6502::ABX, 4 },
            { "ASL", &MOS6502::ASL, &MOS6502::ABX, 7 },{ "ILL", &MOS6502::ILL, &MOS6502::IMP, 7 },
            { "JSR", &MOS6502::JSR, &MOS6502::ABS, 6 },{ "AND", &MOS6502::AND, &MOS6502::IZX, 6 },
            { "ILL", &MOS6502::ILL, &MOS6502::IMP, 2 },{ "ILL", &MOS6502::ILL, &MOS6502::IMP, 8 },
            { "BIT", &MOS6502::BIT, &MOS6502::ZP0, 3 },{ "AND", &MOS6502::AND, &MOS6502::ZP0, 3 },
            { "ROL", &MOS6502::ROL, &MOS6502::ZP0, 5 },{ "ILL", &MOS6502::ILL, &MOS6502::IMP, 5 },
            { "PLP", &MOS6502::PLP, &MOS6502::IMP, 4 },{ "AND", &MOS6502::AND, &MOS6502::IMM, 2 },
            { "ROL", &MOS6502::ROL, &MOS6502::IMP, 2 },{ "ILL", &MOS6502::ILL, &MOS6502::IMP, 2 },
            { "BIT", &MOS6502::BIT, &MOS6502::ABS, 4 },{ "AND", &MOS6502::AND, &MOS6502::ABS, 4 },
            { "ROL", &MOS6502::ROL, &MOS6502::ABS, 6 },{ "ILL", &MOS6502::ILL, &MOS6502::IMP, 6 },
            { "BMI", &MOS6502::BMI, &MOS6502::REL, 2 },{ "AND", &MOS6502::AND, &MOS6502::IZY, 5 },
            { "ILL", &MOS6502::ILL, &MOS6502::IMP, 2 },{ "ILL", &MOS6502::ILL, &MOS6502::IMP, 8 },
            { "ILL", &MOS6502::NOP, &MOS6502::IMP, 4 },{ "AND", &MOS6502::AND, &MOS6502::ZPX, 4 },
            { "ROL", &MOS6502::ROL, &MOS6502::ZPX, 6 },{ "ILL", &MOS6502::ILL, &MOS6502::IMP, 6 },
            { "SEC", &MOS6502::SEC, &MOS6502::IMP, 2 },{ "AND", &MOS6502::AND, &MOS6502::ABY, 4 },
            { "ILL", &MOS6502::NOP, &MOS6502::IMP, 2 },{ "ILL", &MOS6502::ILL, &MOS6502::IMP, 7 },
            { "ILL", &MOS6502::NOP, &MOS6502::IMP, 4 },{ "AND", &MOS6502::AND, &MOS6502::ABX, 4 },
            { "ROL", &MOS6502::ROL, &MOS6502::ABX, 7 },{ "ILL", &MOS6502::ILL, &MOS6502::IMP, 7 },
            { "RTI", &MOS6502::RTI, &MOS6502::IMP, 6 },{ "EOR", &MOS6502::EOR, &MOS6502::IZX, 6 },
            { "ILL", &MOS6502::ILL, &MOS6502::IMP, 2 },{ "ILL", &MOS6502::ILL, &MOS6502::IMP, 8 },
            { "ILL", &MOS6502::NOP, &MOS6502::IMP, 3 },{ "EOR", &MOS6502::EOR, &MOS6502::ZP0, 3 },
            { "LSR", &MOS6502::LSR, &MOS6502::ZP0, 5 },{ "ILL", &MOS6502::ILL, &MOS6502::IMP, 5 },
            { "PHA", &MOS6502::PHA, &MOS6502::IMP, 3 },{ "EOR", &MOS6502::EOR, &MOS6502::IMM, 2 },
            { "LSR", &MOS6502::LSR, &MOS6502::IMP, 2 },{ "ILL", &MOS6502::ILL, &MOS6502::IMP, 2 },
            { "JMP", &MOS6502::JMP, &MOS6502::ABS, 3 },{ "EOR", &MOS6502::EOR, &MOS6502::ABS, 4 },
            { "LSR", &MOS6502::LSR, &MOS6502::ABS, 6 },{ "ILL", &MOS6502::ILL, &MOS6502::IMP, 6 },
            { "BVC", &MOS6502::BVC, &MOS6502::REL, 2 },{ "EOR", &MOS6502::EOR, &MOS6502::IZY, 5 },
            { "ILL", &MOS6502::ILL, &MOS6502::IMP, 2 },{ "ILL", &MOS6502::ILL, &MOS6502::IMP, 8 },
            { "ILL", &MOS6502::NOP, &MOS6502::IMP, 4 },{ "EOR", &MOS6502::EOR, &MOS6502::ZPX, 4 },
            { "LSR", &MOS6502::LSR, &MOS6502::ZPX, 6 },{ "ILL", &MOS6502::ILL, &MOS6502::IMP, 6 },
            { "CLI", &MOS6502::CLI, &MOS6502::IMP, 2 },{ "EOR", &MOS6502::EOR, &MOS6502::ABY, 4 },
            { "ILL", &MOS6502::NOP, &MOS6502::IMP, 2 },{ "ILL", &MOS6502::ILL, &MOS6502::IMP, 7 },
            { "ILL", &MOS6502::NOP, &MOS6502::IMP, 4 },{ "EOR", &MOS6502::EOR, &MOS6502::ABX, 4 },
            { "LSR", &MOS6502::LSR, &MOS6502::ABX, 7 },{ "ILL", &MOS6502::ILL, &MOS6502::IMP, 7 },
            { "RTS", &MOS6502::RTS, &MOS6502::IMP, 6 },{ "ADC", &MOS6502::ADC, &MOS6502::IZX, 6 },
            { "ILL", &MOS6502::ILL, &MOS6502::IMP, 2 },{ "ILL", &MOS6502::ILL, &MOS6502::IMP, 8 },
            { "ILL", &MOS6502::NOP, &MOS6502::IMP, 3 },{ "ADC", &MOS6502::ADC, &MOS6502::ZP0, 3 },
            { "ROR", &MOS6502::ROR, &MOS6502::ZP0, 5 },{ "ILL", &MOS6502::ILL, &MOS6502::IMP, 5 },
            { "PLA", &MOS6502::PLA, &MOS6502::IMP, 4 },{ "ADC", &MOS6502::ADC, &MOS6502::IMM, 2 },
            { "ROR", &MOS6502::ROR, &MOS6502::IMP, 2 },{ "ILL", &MOS6502::ILL, &MOS6502::IMP, 2 },
            { "JMP", &MOS6502::JMP, &MOS6502::IND, 5 },{ "ADC", &MOS6502::ADC, &MOS6502::ABS, 4 },
            { "ROR", &MOS6502::ROR, &MOS6502::ABS, 6 },{ "ILL", &MOS6502::ILL, &MOS6502::IMP, 6 },
            { "BVS", &MOS6502::BVS, &MOS6502::REL, 2 },{ "ADC", &MOS6502::ADC, &MOS6502::IZY, 5 },
            { "ILL", &MOS6502::ILL, &MOS6502::IMP, 2 },{ "ILL", &MOS6502::ILL, &MOS6502::IMP, 8 },
            { "ILL", &MOS6502::NOP, &MOS6502::IMP, 4 },{ "ADC", &MOS6502::ADC, &MOS6502::ZPX, 4 },
            { "ROR", &MOS6502::ROR, &MOS6502::ZPX, 6 },{ "ILL", &MOS6502::ILL, &MOS6502::IMP, 6 },
            { "SEI", &MOS6502::SEI, &MOS6502::IMP, 2 },{ "ADC", &MOS6502::ADC, &MOS6502::ABY, 4 },
            { "ILL", &MOS6502::NOP, &MOS6502::IMP, 2 },{ "ILL", &MOS6502::ILL, &MOS6502::IMP, 7 },
            { "ILL", &MOS6502::NOP, &MOS6502::IMP, 4 },{ "ADC", &MOS6502::ADC, &MOS6502::ABX, 4 },
            { "ROR", &MOS6502::ROR, &MOS6502::ABX, 7 },{ "ILL", &MOS6502::ILL, &MOS6502::IMP, 7 },
            { "ILL", &MOS6502::NOP, &MOS6502::IMP, 2 },{ "STA", &MOS6502::STA, &MOS6502::IZX, 6 },
            { "ILL", &MOS6502::NOP, &MOS6502::IMP, 2 },{ "ILL", &MOS6502::ILL, &MOS6502::IMP, 6 },
            { "STY", &MOS6502::STY, &MOS6502::ZP0, 3 },{ "STA", &MOS6502::STA, &MOS6502::ZP0, 3 },
            { "STX", &MOS6502::STX, &MOS6502::ZP0, 3 },{ "ILL", &MOS6502::ILL, &MOS6502::IMP, 3 },
            { "DEY", &MOS6502::DEY, &MOS6502::IMP, 2 },{ "ILL", &MOS6502::NOP, &MOS6502::IMP, 2 },
            { "TXA", &MOS6502::TXA, &MOS6502::IMP, 2 },{ "ILL", &MOS6502::ILL, &MOS6502::IMP, 2 },
            { "STY", &MOS6502::STY, &MOS6502::ABS, 4 },{ "STA", &MOS6502::STA, &MOS6502::ABS, 4 },
            { "STX", &MOS6502::STX, &MOS6502::ABS, 4 },{ "ILL", &MOS6502::ILL, &MOS6502::IMP, 4 },
            { "BCC", &MOS6502::BCC, &MOS6502::REL, 2 },{ "STA", &MOS6502::STA, &MOS6502::IZY, 6 },
            { "ILL", &MOS6502::ILL, &MOS6502::IMP, 2 },{ "ILL", &MOS6502::ILL, &MOS6502::IMP, 6 },
            { "STY", &MOS6502::STY, &MOS6502::ZPX, 4 },{ "STA", &MOS6502::STA, &MOS6502::ZPX, 4 },
            { "STX", &MOS6502::STX, &MOS6502::ZPY, 4 },{ "ILL", &MOS6502::ILL, &MOS6502::IMP, 4 },
            { "TYA", &MOS6502::TYA, &MOS6502::IMP, 2 },{ "STA", &MOS6502::STA, &MOS6502::ABY, 5 },
            { "TXS", &MOS6502::TXS, &MOS6502::IMP, 2 },{ "ILL", &MOS6502::ILL, &MOS6502::IMP, 5 },
            { "ILL", &MOS6502::NOP, &MOS6502::IMP, 5 },{ "STA", &MOS6502::STA, &MOS6502::ABX, 5 },
            { "ILL", &MOS6502::ILL, &MOS6502::IMP, 5 },{ "ILL", &MOS6502::ILL, &MOS6502::IMP, 5 },
            { "LDY", &MOS6502::LDY, &MOS6502::IMM, 2 },{ "LDA", &MOS6502::LDA, &MOS6502::IZX, 6 },
            { "LDX", &MOS6502::LDX, &MOS6502::IMM, 2 },{ "ILL", &MOS6502::ILL, &MOS6502::IMP, 6 },
            { "LDY", &MOS6502::LDY, &MOS6502::ZP0, 3 },{ "LDA", &MOS6502::LDA, &MOS6502::ZP0, 3 },
            { "LDX", &MOS6502::LDX, &MOS6502::ZP0, 3 },{ "ILL", &MOS6502::ILL, &MOS6502::IMP, 3 },
            { "TAY", &MOS6502::TAY, &MOS6502::IMP, 2 },{ "LDA", &MOS6502::LDA, &MOS6502::IMM, 2 },
            { "TAX", &MOS6502::TAX, &MOS6502::IMP, 2 },{ "ILL", &MOS6502::ILL, &MOS6502::IMP, 2 },
            { "LDY", &MOS6502::LDY, &MOS6502::ABS, 4 },{ "LDA", &MOS6502::LDA, &MOS6502::ABS, 4 },
            { "LDX", &MOS6502::LDX, &MOS6502::ABS, 4 },{ "ILL", &MOS6502::ILL, &MOS6502::IMP, 4 },
            { "BCS", &MOS6502::BCS, &MOS6502::REL, 2 },{ "LDA", &MOS6502::LDA, &MOS6502::IZY, 5 },
            { "ILL", &MOS6502::ILL, &MOS6502::IMP, 2 },{ "ILL", &MOS6502::ILL, &MOS6502::IMP, 5 },
            { "LDY", &MOS6502::LDY, &MOS6502::ZPX, 4 },{ "LDA", &MOS6502::LDA, &MOS6502::ZPX, 4 },
            { "LDX", &MOS6502::LDX, &MOS6502::ZPY, 4 },{ "ILL", &MOS6502::ILL, &MOS6502::IMP, 4 },
            { "CLV", &MOS6502::CLV, &MOS6502::IMP, 2 },{ "LDA", &MOS6502::LDA, &MOS6502::ABY, 4 },
            { "TSX", &MOS6502::TSX, &MOS6502::IMP, 2 },{ "ILL", &MOS6502::ILL, &MOS6502::IMP, 4 },
            { "LDY", &MOS6502::LDY, &MOS6502::ABX, 4 },{ "LDA", &MOS6502::LDA, &MOS6502::ABX, 4 },
            { "LDX", &MOS6502::LDX, &MOS6502::ABY, 4 },{ "ILL", &MOS6502::ILL, &MOS6502::IMP, 4 },
            { "CPY", &MOS6502::CPY, &MOS6502::IMM, 2 },{ "CMP", &MOS6502::CMP, &MOS6502::IZX, 6 },
            { "ILL", &MOS6502::NOP, &MOS6502::IMP, 2 },{ "ILL", &MOS6502::ILL, &MOS6502::IMP, 8 },
            { "CPY", &MOS6502::CPY, &MOS6502::ZP0, 3 },{ "CMP", &MOS6502::CMP, &MOS6502::ZP0, 3 },
            { "DEC", &MOS6502::DEC, &MOS6502::ZP0, 5 },{ "ILL", &MOS6502::ILL, &MOS6502::IMP, 5 },
            { "INY", &MOS6502::INY, &MOS6502::IMP, 2 },{ "CMP", &MOS6502::CMP, &MOS6502::IMM, 2 },
            { "DEX", &MOS6502::DEX, &MOS6502::IMP, 2 },{ "ILL", &MOS6502::ILL, &MOS6502::IMP, 2 },
            { "CPY", &MOS6502::CPY, &MOS6502::ABS, 4 },{ "CMP", &MOS6502::CMP, &MOS6502::ABS, 4 },
            { "DEC", &MOS6502::DEC, &MOS6502::ABS, 6 },{ "ILL", &MOS6502::ILL, &MOS6502::IMP, 6 },
            { "BNE", &MOS6502::BNE, &MOS6502::REL, 2 },{ "CMP", &MOS6502::CMP, &MOS6502::IZY, 5 },
            { "ILL", &MOS6502::ILL, &MOS6502::IMP, 2 },{ "ILL", &MOS6502::ILL, &MOS6502::IMP, 8 },
            { "ILL", &MOS6502::NOP, &MOS6502::IMP, 4 },{ "CMP", &MOS6502::CMP, &MOS6502::ZPX, 4 },
            { "DEC", &MOS6502::DEC, &MOS6502::ZPX, 6 },{ "ILL", &MOS6502::ILL, &MOS6502::IMP, 6 },
            { "CLD", &MOS6502::CLD, &MOS6502::IMP, 2 },{ "CMP", &MOS6502::CMP, &MOS6502::ABY, 4 },
            { "NOP", &MOS6502::NOP, &MOS6502::IMP, 2 },{ "ILL", &MOS6502::ILL, &MOS6502::IMP, 7 },
            { "ILL", &MOS6502::NOP, &MOS6502::IMP, 4 },{ "CMP", &MOS6502::CMP, &MOS6502::ABX, 4 },
            { "DEC", &MOS6502::DEC, &MOS6502::ABX, 7 },{ "ILL", &MOS6502::ILL, &MOS6502::IMP, 6 },
            { "CPX", &MOS6502::CPX, &MOS6502::IMM, 2 },{ "SBC", &MOS6502::SBC, &MOS6502::IZX, 6 },
            { "ILL", &MOS6502::NOP, &MOS6502::IMP, 2 },{ "ILL", &MOS6502::ILL, &MOS6502::IMP, 8 },
            { "CPX", &MOS6502::CPX, &MOS6502::ZP0, 3 },{ "SBC", &MOS6502::SBC, &MOS6502::ZP0, 3 },
            { "INC", &MOS6502::INC, &MOS6502::ZP0, 5 },{ "ILL", &MOS6502::ILL, &MOS6502::IMP, 5 },
            { "INX", &MOS6502::INX, &MOS6502::IMP, 2 },{ "SBC", &MOS6502::SBC, &MOS6502::IMM, 2 },
            { "NOP", &MOS6502::NOP, &MOS6502::IMP, 2 },{ "ILL", &MOS6502::SBC, &MOS6502::IMP, 2 },
            { "CPX", &MOS6502::CPX, &MOS6502::ABS, 4 },{ "SBC", &MOS6502::SBC, &MOS6502::ABS, 4 },
            { "INC", &MOS6502::INC, &MOS6502::ABS, 6 },{ "ILL", &MOS6502::ILL, &MOS6502::IMP, 6 },
            { "BEQ", &MOS6502::BEQ, &MOS6502::REL, 2 },{ "SBC", &MOS6502::SBC, &MOS6502::IZY, 5 },
            { "ILL", &MOS6502::ILL, &MOS6502::IMP, 2 },{ "ILL", &MOS6502::ILL, &MOS6502::IMP, 8 },
            { "ILL", &MOS6502::NOP, &MOS6502::IMP, 4 },{ "SBC", &MOS6502::SBC, &MOS6502::ZPX, 4 },
            { "INC", &MOS6502::INC, &MOS6502::ZPX, 6 },{ "ILL", &MOS6502::ILL, &MOS6502::IMP, 6 },
            { "SED", &MOS6502::SED, &MOS6502::IMP, 2 },{ "SBC", &MOS6502::SBC, &MOS6502::ABY, 4 },
            { "NOP", &MOS6502::NOP, &MOS6502::IMP, 2 },{ "ILL", &MOS6502::ILL, &MOS6502::IMP, 7 },
            { "ILL", &MOS6502::NOP, &MOS6502::IMP, 4 },{ "SBC", &MOS6502::SBC, &MOS6502::ABX, 4 },
            { "INC", &MOS6502::INC, &MOS6502::ABX, 7 },{ "ILL", &MOS6502::ILL, &MOS6502::IMP, 7 },};
};
#endif
