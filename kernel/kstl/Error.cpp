#include "Error.h"

extern "C" {

    // Static initialization atomic functions. Need to finish when implementing threading
    int __cxa_guard_acquire(char* g) {
        return !*g;
    }

    void __cxa_guard_release(char* g) {
        *g = 1;
    }

    void __cxa_guard_abort(char*) {

    }
}