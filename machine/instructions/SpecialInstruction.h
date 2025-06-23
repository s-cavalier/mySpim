#ifndef __SPECIAL_INSTRUCTION_H__
#define __SPECIAL_INSTRUCTION_H__
#include "../Hardware.h"

// -----------------
// Jump Instructions
// -----------------

#define J_INSTR_ARGS Word& pc
class JInstruction : public Hardware::Instruction {
protected:
    Word& pc;

public:
    JInstruction(J_INSTR_ARGS);

    virtual void run() = 0;
};

// ------------------------------------------------------------------------
// Jump Instruction Prototypes
// Each branch instr is pretty different, so each once gets their own class
// ------------------------------------------------------------------------

class Jump : public JInstruction {
    Word target;

public:
    Jump(J_INSTR_ARGS, const Word& target);
    void run();
};

class JumpAndLink : public JInstruction {
    Word target;
    int& ra;

public:
    JumpAndLink(J_INSTR_ARGS, const Word& target, int& ra);
    void run();
};

class JumpAndLinkRegister : public JInstruction {
    int& rd;
    const int& rs;

public:
    JumpAndLinkRegister(J_INSTR_ARGS, int& rd, const int& rs);
    void run();
};

class JumpRegister : public JInstruction {
    const int& ra;

public:
    JumpRegister(J_INSTR_ARGS, const int& ra);
    void run();
};

// --------------------------------------
// Instructions that use Hi, Lo registers
// --------------------------------------
class HiLoInstruction : public Hardware::Instruction {
protected:
    Hardware::HiLoRegisters& hiLo;

public:
    HiLoInstruction(Hardware::HiLoRegisters& hiLo);
    virtual void run() = 0;
};

#define HL_MOVE_INSTR_ARGS int& storage_register, Hardware::HiLoRegisters& hiLo
// MoveFrom uses rd, MoveTo uses rs
class HLMoveInstruction : public HiLoInstruction {
protected:
    // MoveTo uses rd, MoveTo uses rs
    int& storage_register;

public:
    HLMoveInstruction(HL_MOVE_INSTR_ARGS);
    virtual void run() = 0;
};

#define HL_OP_INSTR_ARGS int& rs, int& rt, Hardware::HiLoRegisters& hiLo
class HLOpInstruction : public HiLoInstruction {
protected:
    int& rs;
    int& rt;

public:
    HLOpInstruction(HL_OP_INSTR_ARGS);
    virtual void run() = 0;
};

#define HL_MOVE_INSTR(x) struct x : public HLMoveInstruction { x(HL_MOVE_INSTR_ARGS); void run(); }
HL_MOVE_INSTR(MoveFromHi);
HL_MOVE_INSTR(MoveFromLo);
HL_MOVE_INSTR(MoveToHi);
HL_MOVE_INSTR(MoveToLo);

#define HL_OP_INSTR(x) struct x : public HLOpInstruction { x(HL_OP_INSTR_ARGS); void run(); }
HL_OP_INSTR(Multiply);
HL_OP_INSTR(MultiplyUnsigned);
HL_OP_INSTR(Divide);
HL_OP_INSTR(DivideUnsigned);

// -------
// Syscall
// -------

struct Syscall : public Hardware::Instruction {
    Syscall();
    void run();
};

// -----
// No-Op
// -----
struct NoOp : public Hardware::Instruction {
    inline void run() {}
};

struct Sync : public Hardware::Instruction {
    inline void run() {}
}; // needs impl when making multi-core systems


#endif