#ifndef __CPU_H__
#define __CPU_H__
#include "Processors.h"
#include <optional>

namespace Hardware {

    struct Trap final : std::exception {
        enum ExceptionCode : unsigned char {
            INTERRUPT = 0,   // Interrupt (hardware)

            TLB_MOD = 1,
            TLB_MISS_L = 2,
            TLB_MISS_S = 3,

            ADDRL = 4,   // Address error on load or fetch
            ADDRS = 5,   // Address error on store
            SYSCALL = 8,   // SYSCALL instruction
            BREAK = 9,   // BREAK instruction
            RI = 10,  // Reserved instruction
            CP_UNUSABLE = 11,  // Coprocessor unusable
            OVERFLOW = 12,  // Arithmetic overflow
            TRAP = 13,  // TRAP instruction (e.g., TEQ + trap conditions)
            FP_EXC = 15,  // Floating-point exception
            WATCH = 23,  // Watchpoint reference
            // Other values are reserved or implementation-specific
        };

        ExceptionCode exceptionCode;
        std::optional<Word> badAddr;
        explicit Trap(ExceptionCode exceptionCode) : exceptionCode(exceptionCode) {}
        explicit Trap(ExceptionCode exceptionCode, const Word& badAddr) : exceptionCode(exceptionCode), badAddr(badAddr) {}
    };

    struct HiLoRegisters { Word hi; Word lo; };

    class CPU {
        Machine& machine;
        reg32_t registerFile[32];
        HiLoRegisters hiLo;
        Word programCounter;
        std::unordered_map<Word, std::unique_ptr<Instruction>> instructionCache;
    
    public:
        CPU(Machine& machine);       // maybe cache later down the road
        inline const reg32_t& readRegister(const Byte& reg) const { return registerFile[reg]; }
        inline reg32_t& accessRegister(const Byte& reg) { return registerFile[reg]; }
        inline const Word& readProgramCounter() const { return programCounter; }
        inline Word& accessProgramCounter() { return programCounter; }

        void cycle();

        std::unique_ptr<Instruction> decode(const Word& bin_instr);

    };
}

#endif