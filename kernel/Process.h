#ifndef __PROCESS_H__
#define __PROCESS_H__
#include "ASMInterface.h"

namespace kernel {

    enum ProcessState : uint32_t {
        READY,
        RUNNING,
        BLOCKED,
        ZOMBIE
    };

    class PCB {
        uint32_t PID;

    public:
        ProcessState state;
        TrapFrame trapFrame; // maybe just use a pointer?
        
        PCB(const char* binaryFile, bool fromSpim = false);

        void run();
    };


}

#endif