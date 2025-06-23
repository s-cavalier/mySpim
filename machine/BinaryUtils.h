#ifndef __BINARY_UTILS_H__
#define __BINARY_UTILS_H__

// TODO: ADD MORE INSTRUCTIONS
// Maybe put in namespace to clear up global namespace?

namespace Binary {
    enum Opcode : unsigned char {
        R_TYPE = 0x0,
        REGIMM = 0x1,
        K_TYPE = 0x10,
        FP_TYPE = 0x11,
        J = 0x2,            //
        JAL = 0x3,          //

        ADDI = 0x8,         //
        ADDIU = 0x9,        //
        SLTI = 0xa,         //
        SLTIU = 0xb,        //
        ANDI = 0xc,         //
        BEQ = 0x4,          //
        BNE = 0x5,          //
        BLEZ = 0x6,         //
        BGTZ = 0x7,         //
        LB = 0x20,          //
        LBU = 0x24,         //
        LH = 0x21,          //
        LHU = 0x25,         //
        LUI = 0xf,          //
        LW = 0x23,          //
        ORI = 0xd,          //
        XORI = 0xe,         //
        SB = 0x28,          //
        SH = 0x29,          //
        SW = 0x2b,          //

        LWC1 = 0x31,        //
        LDC1 = 0x35,        
        SWC1 = 0x39,        //
        SDC1 = 0x3d,        

        LL = 0x30,  // not important rn
        SC = 0x38   // only useful when implementing OS or multithreading (green threads)
    };

    enum Funct : unsigned char {
        SYSCALL = 0xc,      //
        SYNC = 0xf,
        MOVN = 0xb,
        BREAK = 0xd,
        ERET = 0x18,

        // CUSTOM INSTR
        VMTUNNEL = 0x3f,

        ADD = 0x20,         //
        ADDU = 0x21,        //
        AND = 0x24,         //
        JR = 0x8,           //
        JALR = 0x9,         //
        NOR = 0x27,         //
        OR = 0x25,          //
        SLT = 0x2a,         //
        SLTU = 0x2b,        //
        SLL = 0x0,          //
        SRL = 0x2,          //
        SRA = 0x3,          //
        SLLV = 0x4,         //
        SRLV = 0x6,         //
        SRAV = 0x7,         //
        SUB = 0x22,         //
        SUBU = 0x23,        //
        XOR = 0x26,         //
        MULT = 0x18,        //
        DIV = 0x1a,         //
        MULTU = 0x19,       //
        DIVU = 0x1b,        //
        MFHI = 0x10,        //
        MFLO = 0x12,        //
        MTHI = 0x11,        //
        MTLO = 0x13,        //
    };

    // Maybe consider extended/quad precision
    enum FMT : unsigned char {
        BC = 0x8,
        S = 0x10,
        D = 0x11,
        W = 0x14,
        L = 0x15,
    };

    // Single/Double Instructions have same Funct, so just decide what to return based on FMT, these functs are stricly generic to either S or D
    enum FPFunct : unsigned char {
        FPABS = 0x5,
        FPADD = 0,
        FPCEQ = 0x32,
        FPCLT = 0x3c,
        FPCLE = 0x3e,
        FPDIV = 3,
        FPMUL = 2,
        FPSUB = 1
    };

    enum Register : unsigned char {
        ZERO = 0,
        AT = 1,
        V0 = 2,
        V1 = 3,
        A0 = 4,
        A1 = 5,
        A2 = 6,
        A3 = 7,
        T0 = 8,
        T1 = 9,
        T2 = 10,
        T3 = 11,
        T4 = 12,
        T5 = 13,
        T6 = 14,
        T7 = 15,
        S0 = 16,
        S1 = 17,
        S2 = 18,
        S3 = 19,
        S4 = 20,
        S5 = 21,
        S6 = 22,
        S7 = 23,
        T8 = 24,
        T9 = 25,
        K0 = 26,
        K1 = 27,
        GP = 28,
        SP = 29,
        FP = 30,
        RA = 31

    };

    enum SYSRegister : unsigned char {
        INDEX = 0,
        RANDOM = 1,
        ENTRYLO0 = 2,
        ENTRYLO1 = 3,
        CONTEXT = 4,
        PAGEMASK = 5,
        BAD_VADDR = 8,
        ENTRYHI = 10,
        STATUS = 12,
        CAUSE = 13,
        EPC = 14,
        CONFIG = 16,
        K_SP = 20,  // kernel stack ptr
        K_TF = 21,  // kernel trap frame
        ERROR_EPC = 30,
        SYS_DEBUG = 31
    };

    unsigned int loadBigEndian(const unsigned char bytes[4]);

    static char regToString[32][3] = {
        "0\0",  // null terminator so it ends earlier
        "at",
        "v0",
        "v1",
        "a0",
        "a1",
        "a2",
        "a3",
        "t0",
        "t1",
        "t2",
        "t3",
        "t4",
        "t5",
        "t6",
        "t7",
        "s0",
        "s1",
        "s2",
        "s3",
        "s4",
        "s5",
        "s6",
        "s7",
        "t8",
        "t9",
        "k0",
        "k1",
        "gp",
        "sp",
        "fp",
        "ra"
    };

};

#endif