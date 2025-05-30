#include "CPU.h"
#include "BinaryUtils.h"
#include "instructions/Instruction.h"

Hardware::CPU::CPU(Machine& machine) : machine(machine), registerFile{0}, programCounter(0) {}

void Hardware::CPU::cycle() {
    if (programCounter >= machine.readMemory().memoryBounds.textBound) {    // cpu should own the bounds later
        machine.killed = true;
        return;
    }

    auto it = instructionCache.find(programCounter);
    if (it != instructionCache.end()) {
        it->second->run();
        programCounter += 4;
        return;
    }

    (
        instructionCache[programCounter] = decode( machine.readMemory().getWord(programCounter) )
    )->run(); // cool syntax

    programCounter += 4;
}

std::unique_ptr<Hardware::Instruction> Hardware::CPU::decode(const Word& binary_instruction) {
    // DECODE

    // Simplify local namespace
    using namespace Binary;
    auto& RAM = machine.accessMemory();
    auto& fpu = machine.accessCoprocessor(1);
    
    Opcode opcode = Opcode((binary_instruction >> 26) & 0b111111);  // For All

    if (opcode == FP_TYPE) return fpu->decode(binary_instruction);

    Word address = binary_instruction & 0x1FFFFFF;                  // For Jump

    Register rs = Register((binary_instruction >> 21) & 0b11111);   // For I/R
    Register rt = Register((binary_instruction >> 16) & 0b11111);   // For I/R

    Register rd = Register((binary_instruction >> 11) & 0b11111);   // For R
    Byte shamt = (binary_instruction >> 6) & 0b11111;               // For R
    Funct funct = Funct(binary_instruction & 0b111111);             // For R

    short immediate = binary_instruction & 0xFFFF;                  // For I

    // Return instruction

    #define R_VAR_INIT(oc, instr) case oc: return std::make_unique<instr>(registerFile[rd].i, registerFile[rt].i, registerFile[rs].i)
    #define R_SHFT_INIT(oc, instr) case oc: return std::make_unique<instr>(registerFile[rd].i, registerFile[rt].i, shamt)
    #define HL_MOVE_INIT(oc, instr, reg) case oc: return std::make_unique<instr>(registerFile[reg].i, hiLo)
    #define HL_OP_INIT(oc, instr) case oc: return std::make_unique<instr>(registerFile[rs].i, registerFile[rt].i, hiLo)
    if (!opcode) {  // Is an R-Type Instruction
        switch (funct) {
            R_VAR_INIT(ADD, Add);
            R_VAR_INIT(ADDU, AddUnsigned);
            R_VAR_INIT(AND, And);
            R_VAR_INIT(NOR, Nor);
            R_VAR_INIT(OR, Or);
            R_VAR_INIT(SLT, SetLessThan);
            R_VAR_INIT(SLTU, SetLessThanUnsigned);
            R_SHFT_INIT(SLL, ShiftLeftLogical);
            R_SHFT_INIT(SRL, ShiftRightLogical);
            R_SHFT_INIT(SRA, ShiftRightArithmetic);
            R_VAR_INIT(SLLV, ShiftLeftLogicalVariable);
            R_VAR_INIT(SRLV, ShiftRightLogicalVariable);
            R_VAR_INIT(SRAV, ShiftRightArithmeticVariable);
            R_VAR_INIT(SUB, Subtract);
            R_VAR_INIT(SUBU, SubtractUnsigned);
            R_VAR_INIT(XOR, Xor);
            HL_OP_INIT(MULT, Multiply);
            HL_OP_INIT(DIV, Divide);
            HL_OP_INIT(MULTU, MultiplyUnsigned);
            HL_OP_INIT(DIVU, DivideUnsigned);
            HL_MOVE_INIT(MFHI, MoveFromHi, rd);
            HL_MOVE_INIT(MFLO, MoveFromLo, rd);
            HL_MOVE_INIT(MTHI, MoveToHi, rs);
            HL_MOVE_INIT(MTLO, MoveToLo, rs);
            case SYSCALL: return std::make_unique<Syscall>(machine);
            case JR: return std::make_unique<JumpRegister>(programCounter, registerFile[RA].i);
            case JALR: return std::make_unique<JumpAndLinkRegister>(programCounter, registerFile[rd].i, registerFile[rs].i);
            default:
                throw 1;
        }
    }

    #define I_GEN_INIT(oc, instr) case oc: return std::make_unique<instr>(registerFile[rt].i, registerFile[rs].i, immediate)        // optimize instructions to use ui/i?
    #define I_MEM_INIT(oc, instr) case oc: return std::make_unique<instr>(registerFile[rt].i, registerFile[rs].i, immediate, RAM)
    #define I_BRANCH_INIT(oc, instr) case oc: return std::make_unique<instr>(registerFile[rt].i, registerFile[rs].i, immediate, programCounter)
    #define FPMEM_INIT(oc, instr) case oc: return std::make_unique<instr>(fpu->accessRegister(rt).f, registerFile[rs].i, immediate, RAM)
    switch (opcode) {
        I_GEN_INIT(ADDI, AddImmediate);
        I_GEN_INIT(ADDIU, AddImmediateUnsigned);
        I_GEN_INIT(ANDI, AndImmediate);
        I_GEN_INIT(SLTI, SetLessThanImmediate);
        I_GEN_INIT(SLTIU, SetLessThanImmediateUnsigned);
        I_GEN_INIT(ORI, OrImmediate);
        I_MEM_INIT(LW, LoadWord);
        I_MEM_INIT(SW, StoreWord);
        I_BRANCH_INIT(BEQ, BranchOnEqual);
        I_BRANCH_INIT(BNE, BranchOnNotEqual);
        FPMEM_INIT(LWC1, LoadFPSingle);
        FPMEM_INIT(SWC1, StoreFPSingle);  
        case J: return std::make_unique<Jump>(programCounter, address);
        case JAL: return std::make_unique<JumpAndLink>(programCounter, address, registerFile[RA].i);
        case LUI: return std::make_unique<LoadUpperImmediate>(registerFile[rt].i, immediate);
        default:
            throw 2;
    }

    throw 3;
    return nullptr;
}