#ifndef __VIRTUAL_MEMORY_H__
#define __VIRTUAL_MEMORY_H__
#include "kstl/Bitset.h"

namespace kernel {

    size_t constexpr MEM_AVAIL = 256 * 1024 * 1024;
    size_t constexpr PAGE_SIZE = 4096;
    size_t constexpr NUM_PAGES = MEM_AVAIL / PAGE_SIZE;

    class MemoryManager {
        ministl::bitset<NUM_PAGES> freePages;
        size_t kernelReservedBoundary;

    public:
        MemoryManager();

        static MemoryManager& instance() {
            static MemoryManager inst;
            return inst;
        }

        size_t reserveFreeFrame();
        size_t freeFrame();
    };

    class PageTableEntry {
        size_t data;

        static constexpr size_t PFN_BIT_SIZE = 16;
        static constexpr size_t PFN_SHIFT = 32 - PFN_BIT_SIZE;
        static constexpr size_t PFN_MASK = ((1u << PFN_BIT_SIZE) - 1) << PFN_SHIFT;

        static constexpr size_t G_MASK = 1u << 8;
        static constexpr size_t W_MASK = 1u << 1;
        static constexpr size_t P_MASK = 1u;

    public:
        PageTableEntry() : data(0) {}

        size_t getPFN() const;
        void setPFN(size_t pfn);
        bool global() const;
        void setGlobal(bool value);
        bool writable() const;
        void setWritable(bool value);
        bool valid() const;
        void setValid(bool value);
        // todo: more pfn stuff
    };

    template <typename Derived>
    struct PageTable {
        size_t walkTable(size_t vaddr) { return static_cast<Derived*>(this)->walkTableImpl(vaddr); }
    };

    class KernelPageTable final : public PageTable<KernelPageTable> {
        static constexpr size_t KERNEL_DYNAMIC_PAGE_COUNT = 16;
        static constexpr size_t KERNEL_DYNAMIC_MEM_SIZE = KERNEL_DYNAMIC_PAGE_COUNT * PAGE_SIZE;

        ministl::array<PageTableEntry, KERNEL_DYNAMIC_PAGE_COUNT> pages;
        size_t walkTableImpl(size_t vaddr);


        friend class PageTable;
    };

}

#endif