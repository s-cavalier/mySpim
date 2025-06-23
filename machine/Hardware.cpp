#include "Hardware.h"
#include "BinaryUtils.h"
#include "instructions/Instruction.h"

#ifdef DEBUG
    #include <iostream>
    #include <iomanip>
    #define debug(x) x
#else
    #define debug(x)
#endif

#define DATA_ENTRY 0x10008000
#define TEXT_START 0x00400024

// -------------------------------------------------------------
// Hardware Emulation
// -------------------------------------------------------------

Hardware::Machine::Machine() : cpu(*this), killed(false) {

    coprocessors[0] = std::make_unique<SystemControlUnit>(*this);
    coprocessors[1] = std::make_unique<FloatingPointUnit>(*this);
    coprocessors[2] = nullptr;
}

void Hardware::Machine::raiseTrap(const Byte& exceptionCode) {
    using namespace Binary;

    SystemControlUnit* sys_ctrl = dynamic_cast<SystemControlUnit*>(coprocessors[0].get());  // if this errors, we can just get rid of it since all that happens is reg interaction
    
    sys_ctrl->setEPC( cpu.readProgramCounter() );
    sys_ctrl->setEXL( true );


    sys_ctrl->setCause(exceptionCode);

    cpu.accessProgramCounter() = trapEntry;

}

void Hardware::Machine::loadKernel(const ExternalInfo::KernelBootInformation& kernelInfo, const std::vector<std::string>& kernelArgs) {
    trapEntry = kernelInfo.trapEntry;
    Word at = kernelInfo.textStart;


    for (const auto& instr : kernelInfo.text) {
        RAM.setWord(at, instr);
        at += 4;
    }


    at = kernelInfo.dataStart;
    for (const auto& byte : kernelInfo.data) {
        RAM.setByte(at, byte);
        ++at;
    }

    RAM.setWord(kernelInfo.argc, kernelArgs.size());
    
    for (Word i = 0; i < kernelArgs.size(); ++i) {
        Word indirectPtr = kernelInfo.argv + 64 * i;    // argv[i] = *(argv + i)
        for (Word j = 0; j < kernelArgs[i].size(); ++j) RAM.setByte(indirectPtr + j, kernelArgs[i][j]); // argv[i][j]
    }


    cpu.accessProgramCounter() = kernelInfo.bootEntry;
    coprocessors[0]->accessRegister(Binary::STATUS).ui = 0b10;  // enable exl at boot <- has to be done by hardware

    

}

// effectively deprecated
void Hardware::Machine::loadProgram(const std::vector<Word>& instructions, const std::vector<Byte>& bytes, const Word& entry) {
    // for right now, just load according to mips for no patricular reason
    // will figure out exact specifications later
    Word at = TEXT_START;
    for (const auto& instr : instructions) {
        RAM.setWord(at, instr);
        at += 4;
    }

    at = DATA_ENTRY;
    for (const auto& byte : bytes) {
        RAM.setByte(at, byte);
        ++at;
    }

    // set memory bounds
}

void Hardware::Machine::step() {
    cpu.cycle();
}

void Hardware::Machine::run(instrDebugHook hook) {
    while (!killed) {
        step();
        if (hook) hook(*this);
    }
}