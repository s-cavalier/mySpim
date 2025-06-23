#include "VirtualMemory.h"
#include "ASMInterface.h"

extern "C" char _end[];

kernel::MemoryManager::MemoryManager() {
    kernelReservedBoundary = ((size_t(_end) - 0x80000000) + PAGE_SIZE - 1) / PAGE_SIZE; // round up to closet page boundary in kernel image
}

size_t kernel::MemoryManager::reserveFreeFrame() {
    size_t frame = size_t(-1);
    for (size_t i = kernelReservedBoundary; i < freePages.size(); ++i) {
        if (freePages[i]) continue; // reserved if bit[i] == 1
        frame = i;
        freePages.set(i, true);
        break;
    }

    assert(frame != size_t(-1)); // ran out of pages

    return frame; // frameNumber * 4096 -> physical address
}

size_t kernel::PageTableEntry::getPFN() const { return (data >> PFN_SHIFT) & ((1u << PFN_BIT_SIZE) - 1); }
void kernel::PageTableEntry::setPFN(size_t pfn) { data = (data & ~PFN_MASK) | ((pfn & ((1u << PFN_BIT_SIZE) - 1)) << PFN_SHIFT ); }
bool kernel::PageTableEntry::global() const { return (data & G_MASK); }
void kernel::PageTableEntry::setGlobal(bool value) {
    if (value) data |= G_MASK;
    else data &= ~G_MASK;
}
bool kernel::PageTableEntry::writable() const { return (data & W_MASK); }
void kernel::PageTableEntry::setWritable(bool value) {
    if (value) data |= W_MASK;
    else data &= ~W_MASK;
}
bool kernel::PageTableEntry::valid() const { return (data & P_MASK); }
void kernel::PageTableEntry::setValid(bool value) {
    if (value) data |= P_MASK;
    else data &= ~P_MASK;
}

size_t kernel::KernelPageTable::walkTableImpl(size_t vaddr) {
    assert(vaddr >= 0xC0000000 && "Non-kernel address attempted to walk kernel page table");
    vaddr -= 0xC0000000;

    size_t offset = vaddr & 0xFFF; // bottom 12 bits
    size_t index = (vaddr >> 12);
    assert(index < KERNEL_DYNAMIC_PAGE_COUNT && "Ran out of kernel heap pages"); // current reserved space [0xC0000000, 0xC0010000)

    PageTableEntry& pte = pages[index];
    if (pte.valid()) return (pte.getPFN() << 12) | offset;

    PrintString("getting new page\n");

    size_t pfn = MemoryManager::instance().reserveFreeFrame();
    pte.setPFN( pfn );
    pte.setValid(true);
    return (pfn << 12) | offset;
}