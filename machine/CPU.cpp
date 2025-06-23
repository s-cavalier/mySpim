#include "CPU.h"
#include "BinaryUtils.h"
#include "instructions/Instruction.h"

Hardware::CPU::CPU(Machine& machine) : machine(machine), registerFile{0}, programCounter(0) {}

void Hardware::CPU::cycle() {
    try {
        
        auto& instr = instructionCache[programCounter];
        if (!instr) instr = decode( machine.readMemory().getWord(programCounter) );

        instr->run();
        programCounter += 4;
        
    } catch (const Hardware::Trap& trap) {
        machine.raiseTrap(trap.exceptionCode);
    }
}

std::unique_ptr<Hardware::Instruction> Hardware::CPU::decode(const Word& binary_instruction) {
    // DECODE


    // Simplify local namespace

    using namespace Binary;

    auto& scu = machine.accessCoprocessor(0);
    auto& fpu = machine.accessCoprocessor(1);
    
    Opcode opcode = Opcode((binary_instruction >> 26) & 0b111111);  // For All

    if (opcode == FP_TYPE && fpu) return fpu->decode(binary_instruction);
    else if (opcode == FP_TYPE) throw Trap(Trap::CP_UNUSABLE);

    if (opcode == K_TYPE && scu) return scu->decode(binary_instruction);
    else if (opcode == K_TYPE) throw Trap(Trap::CP_UNUSABLE);

    auto& RAM = machine.accessMemory();
    auto& statusReg = machine.accessCoprocessor(0)->accessRegister(STATUS).ui;

    Word address = binary_instruction & 0x1FFFFFF;                  // For Jump

    Register rs = Register((binary_instruction >> 21) & 0b11111);   // For I/R
    Register rt = Register((binary_instruction >> 16) & 0b11111);   // For I/R

    Register rd = Register((binary_instruction >> 11) & 0b11111);   // For R
    Byte shamt = (binary_instruction >> 6) & 0b11111;               // For R
    Funct funct = Funct(binary_instruction & 0b111111);             // For R

    short immediate = binary_instruction & 0xFFFF;                  // For I

    // Return instruction

    #define I_BRANCH_ZERO_INIT(oc, instr) case oc: return std::make_unique<instr>(immediate, programCounter, registerFile[rs].i)
    if (opcode == REGIMM) {
        switch (rt) {
            I_BRANCH_ZERO_INIT(1, BranchOnGreaterThanOrEqualZero);
            I_BRANCH_ZERO_INIT(0, BranchOnLessThanZero);
            default:
                throw Trap(Trap::RI);
        }
    }

    #define R_VAR_INIT(oc, instr) case oc: return std::make_unique<instr>(registerFile[rd].i, registerFile[rt].i, registerFile[rs].i)
    #define R_SHFT_INIT(oc, instr) case oc: return std::make_unique<instr>(registerFile[rd].i, registerFile[rt].i, shamt)
    #define HL_MOVE_INIT(oc, instr, reg) case oc: return std::make_unique<instr>(registerFile[reg].i, hiLo)
    #define HL_OP_INIT(oc, instr) case oc: return std::make_unique<instr>(registerFile[rs].i, registerFile[rt].i, hiLo)
    if (!opcode) {  // Is an R-Type Instruction
        switch (funct) {
            R_VAR_INIT(ADDU, AddUnsigned);
            R_VAR_INIT(AND, And);
            R_VAR_INIT(NOR, Nor);
            R_VAR_INIT(OR, Or);
            R_VAR_INIT(SLT, SetLessThan);
            R_VAR_INIT(SLTU, SetLessThanUnsigned);
            R_VAR_INIT(MOVN, MoveOnNotZero);
            R_SHFT_INIT(SLL, ShiftLeftLogical);
            R_SHFT_INIT(SRL, ShiftRightLogical);
            R_SHFT_INIT(SRA, ShiftRightArithmetic);
            R_VAR_INIT(SLLV, ShiftLeftLogicalVariable);
            R_VAR_INIT(SRLV, ShiftRightLogicalVariable);
            R_VAR_INIT(SRAV, ShiftRightArithmeticVariable);
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
            case ADD: return std::make_unique<Add>(registerFile[rd].i, registerFile[rt].i, registerFile[rs].i);
            case SUB: return std::make_unique<Subtract>(registerFile[rd].i, registerFile[rt].i, registerFile[rs].i);
            case SYSCALL: return std::make_unique<Syscall>();
            case SYNC: return std::make_unique<Sync>();
            case VMTUNNEL: return std::make_unique<VMTunnel>(statusReg, machine);
            case JR: return std::make_unique<JumpRegister>(programCounter, registerFile[RA].i);
            case JALR: return std::make_unique<JumpAndLinkRegister>(programCounter, registerFile[rd].i, registerFile[rs].i);
            default:
                throw Trap(Trap::RI);
        }
    }

    #define I_GEN_INIT(oc, instr) case oc: return std::make_unique<instr>(registerFile[rt].i, registerFile[rs].i, immediate)        // optimize instructions to use ui/i?
    #define I_MEM_INIT(oc, instr) case oc: return std::make_unique<instr>(registerFile[rt].i, registerFile[rs].i, immediate, RAM)
    #define I_BRANCH_INIT(oc, instr) case oc: return std::make_unique<instr>(registerFile[rt].i, registerFile[rs].i, immediate, programCounter)
    #define FPMEM_INIT(oc, instr) case oc: return std::make_unique<instr>(fpu->accessRegister(rt).f, registerFile[rs].i, immediate, RAM)
    switch (opcode) {
        I_GEN_INIT(ADDIU, AddImmediateUnsigned);
        I_GEN_INIT(ANDI, AndImmediate);
        I_GEN_INIT(SLTI, SetLessThanImmediate);
        I_GEN_INIT(XORI, XorImmediate);
        I_GEN_INIT(SLTIU, SetLessThanImmediateUnsigned);
        I_GEN_INIT(ORI, OrImmediate);
        I_MEM_INIT(LW, LoadWord);
        I_MEM_INIT(SW, StoreWord);
        I_MEM_INIT(SB, StoreByte);
        I_MEM_INIT(LBU, LoadByteUnsigned);
        I_MEM_INIT(LB, LoadByte);
        I_MEM_INIT(LHU, LoadHalfwordUnsigned);
        I_MEM_INIT(LH, LoadHalfword);
        I_MEM_INIT(SH, StoreHalfword);
        I_BRANCH_INIT(BEQ, BranchOnEqual);
        I_BRANCH_INIT(BNE, BranchOnNotEqual);
        FPMEM_INIT(LWC1, LoadFPSingle);
        FPMEM_INIT(SWC1, StoreFPSingle);
        I_BRANCH_ZERO_INIT(BGTZ, BranchOnGreaterThanZero);
        I_BRANCH_ZERO_INIT(BLEZ, BranchOnLessThanOrEqualZero);
        case ADDI: return std::make_unique<AddImmediate>(registerFile[rt].i, registerFile[rs].i, immediate);
        case J: return std::make_unique<Jump>(programCounter, address);
        case JAL: return std::make_unique<JumpAndLink>(programCounter, address, registerFile[RA].i);
        case LUI: return std::make_unique<LoadUpperImmediate>(registerFile[rt].i, immediate);
        default:
            throw Trap(Trap::RI);
    }

    machine.raiseTrap(Word(Trap::RI)); 
    return std::make_unique<NoOp>();
}