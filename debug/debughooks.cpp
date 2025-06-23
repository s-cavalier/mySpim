#include "../machine/Hardware.h"
#include "../machine/BinaryUtils.h"
#include <iostream>
#include <iomanip>

// -------------------
// Debugging Interface
// -------------------
// Written as just a .cpp file so singletons can be established as need be
// Allows for a lot of extension/modulatrity in the hooks using templates
// DBG stream can also be replaced if need be as well, along with other condEvals
// If need be, we add a Hook Manager for more fine-grained access

std::ostream* dbg_output_stream = &std::cout;  

using dbgHook = void (*)(const Hardware::Machine& machine);

// conditionals
using condEval = bool(*)(const Hardware::Machine& machine);
bool onKilled(const Hardware::Machine& machine) { return machine.killed; }
bool ignoreKernel(const Hardware::Machine& machine) { return !(machine.readCoprocessor(0)->readRegister(12).ui & 0b10); }
bool focusKernel(const Hardware::Machine& machine) { return (machine.readCoprocessor(0)->readRegister(12).ui & 0b10); }

#define DBG_OUT *dbg_output_stream

// DBG_END defines what to print at the end of a print, mainly to differentiate between endl and '\n' for immediate flushes or not
#define DBG_END std::endl
#define HOOK_TEMPLATE(func_name) void func_name (const Hardware::Machine& machine)
// print all written memory above this addr

HOOK_TEMPLATE(printInstr) {
    const Word& pc = machine.readCPU().readProgramCounter();
    DBG_OUT << "Reading instruction 0x"<< std::hex << std::setw(8) << std::setfill('0') << machine.readMemory().getWord(pc) << " at program counter 0x" << std::setw(8) << std::setfill('0') << pc << std::dec << DBG_END;
}

HOOK_TEMPLATE(printEXL) {
    const Word& statusReg = machine.readCoprocessor(0)->readRegister(Binary::STATUS).ui;
    DBG_OUT << "EXL_STATUS: " << std::boolalpha << bool(statusReg & 0b10) << DBG_END;
}

HOOK_TEMPLATE(printStatus) {
    const Word& statusReg = machine.readCoprocessor(0)->readRegister(Binary::STATUS).ui;
    DBG_OUT << "STATUS: " << std::hex << std::setw(8) << std::setfill('0') << statusReg << std::dec << DBG_END;
}

HOOK_TEMPLATE(printRegs) {
    DBG_OUT << "REGISTERS:\n";
    for (int i = 0; i < 32; ++i) DBG_OUT << '$' << Binary::regToString[i] << ":" << machine.readCPU().readRegister(i).ui << ' ';
    DBG_OUT << DBG_END;
}

HOOK_TEMPLATE(printFPRegs) {
    DBG_OUT << "FP REGISTERS:\n";
    for (int i = 0; i < 32; ++i) DBG_OUT << "$f" << i << ":" << machine.readCoprocessor(1)->readRegister(i).f << ' ';
    DBG_OUT << DBG_END;
}


template <condEval condition, dbgHook Hook>
HOOK_TEMPLATE(conditionalHook) {
    if (condition(machine)) Hook(machine);
}

template <dbgHook... Hooks>
HOOK_TEMPLATE(combinedHook) { 
    (Hooks(machine), ...); 
}

template <condEval condition, dbgHook... Hooks>
HOOK_TEMPLATE(conditionalCombinedHook) {
    if (condition(machine)) {
        (Hooks(machine), ...);
    }
}

template <dbgHook... Hooks>
dbgHook makeCombinedHook() {
    return &combinedHook<Hooks...>;
}

template <condEval condition, dbgHook... Hooks>
dbgHook makeConditionalCombinedHook() {
    return &conditionalCombinedHook<condition, Hooks...>;
}