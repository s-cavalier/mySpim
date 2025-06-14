#ifndef __ASM_INTERFACE_H__
#define __ASM_INTERFACE_H__

#define VMTUNNEL ".word 0x0000003F\n"

namespace kernel {
    using uint32_t = unsigned int;

    struct TrapFrame {
        unsigned int at;
        unsigned int v0;
        unsigned int v1;
        unsigned int a0;
        unsigned int a1;
        unsigned int a2;
        unsigned int a3;

        unsigned int t0;
        unsigned int t1;
        unsigned int t2; 
        unsigned int t3; 
        unsigned int t4; 
        unsigned int t5; 
        unsigned int t6; 
        unsigned int t7; 

        unsigned int s0; 
        unsigned int s1; 
        unsigned int s2; 
        unsigned int s3; 
        unsigned int s4; 
        unsigned int s5; 
        unsigned int s6; 
        unsigned int s7; 

        unsigned int t8; 
        unsigned int t9; 

        unsigned int k0;    
        unsigned int k1;    
        unsigned int gp; 
        unsigned int sp;    
        unsigned int fp; 
        unsigned int ra; 

        unsigned int epc;
        unsigned int status;
        unsigned int cause;

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
    
    enum VMRequestType : uint32_t {
        UNKNOWN,
        HALT,
        PRINT_STRING,
        PRINT_INTEGER,
        READ_INTEGER
    };

    struct VMResponse {
        uint32_t res;
        uint32_t err;
    };

    struct VMPackage {
        VMRequestType reqType;
        uint32_t arg0;
        uint32_t arg1;
        uint32_t arg2;

        VMPackage(VMRequestType reqType = UNKNOWN, uint32_t arg0 = 0, uint32_t arg1 = 0, uint32_t arg2 = 0) : reqType(reqType), arg0(arg0), arg1(arg1), arg2(arg2) {}

        VMResponse send() const;
    };

    // frame needs to be manually loaded
    TrapFrame* loadTrapFrame();

    int getK0Register();
    int getK1Register();

}

#define Halt                kernel::VMPackage(kernel::HALT).send()
#define PrintString(ptr)    kernel::VMPackage(kernel::PRINT_STRING, (kernel::uint32_t)(ptr) ).send()
#define PrintInteger(num)   kernel::VMPackage(kernel::PRINT_INTEGER, (num)).send()
#define ReadInteger         kernel::VMPackage(kernel::READ_INTEGER).send()

#endif