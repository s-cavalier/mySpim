#include "Memory.h"
#include "Hardware.h"
#include "BinaryUtils.h"
#include <cstring>
#include <cassert>
#include <random>
#include <iostream>

inline Word Hardware::TLBEntry::VPN() const { return (entryHi & VPN_MASK) >> 13; }
inline void Hardware::TLBEntry::setVPN(const Word& vpn) { entryHi = (entryHi & ~VPN_MASK) | ((vpn << 13) & VPN_MASK); }
inline bool Hardware::TLBEntry::global() const { return entryHi & G_MASK; }
inline void Hardware::TLBEntry::setGlobal(bool g) { entryHi = g ? entryHi | G_MASK : entryHi & ~G_MASK; }
inline bool Hardware::TLBEntry::read() const { return entryHi & R_MASK; }
inline void Hardware::TLBEntry::setRead(bool r) { entryHi = r ? entryHi | R_MASK : entryHi & ~R_MASK; }
inline bool Hardware::TLBEntry::write() const { return entryHi & W_MASK; }
inline void Hardware::TLBEntry::setWrite(bool w) { entryHi = w ? entryHi | W_MASK : entryHi & ~W_MASK; }
inline bool Hardware::TLBEntry::execute() const { return entryHi & X_MASK; }
inline void Hardware::TLBEntry::setExecute(bool x) { entryHi = x ? entryHi | X_MASK : entryHi & ~X_MASK; }
inline Byte Hardware::TLBEntry::ASID() const { return Byte( entryHi & ASID_MASK ); }
inline void Hardware::TLBEntry::setASID(Byte asid) { entryHi = (entryHi & ~ASID_MASK) | (asid & ASID_MASK); }
inline Word Hardware::TLBEntry::PFN() const { return (entryLo & PFN_MASK) >> 6; }
inline void Hardware::TLBEntry::setPFN(const Word& pfn) { entryLo = (entryLo & ~PFN_MASK) | ((pfn << 6) & PFN_MASK); }
inline Byte Hardware::TLBEntry::coherence() const { return Byte((entryLo & C_MASK) >> 3 ); }
inline void Hardware::TLBEntry::setCoherence(Byte c) { entryLo = (entryLo & ~C_MASK) | ((c << 3) & C_MASK); }
inline bool Hardware::TLBEntry::dirty() const { return entryLo & D_MASK; }
inline void Hardware::TLBEntry::setDirty(bool d) { entryLo = d ? entryLo | D_MASK : entryLo & ~D_MASK; }
inline bool Hardware::TLBEntry::valid() const { return entryLo & V_MASK; }
inline void Hardware::TLBEntry::setValid(bool v) { entryLo = v ? entryLo | V_MASK : entryLo & ~V_MASK; }

inline Word& Hardware::TLBEntry::accessHiRaw() { return entryHi; }
inline Word& Hardware::TLBEntry::accessLoRaw() { return entryLo; }
inline const Word& Hardware::TLBEntry::readHiRaw() const { return entryHi; }
inline const Word& Hardware::TLBEntry::readLoRaw() const { return entryLo; }

inline bool Hardware::TLBEntry::permits(AccessType access) const {
    switch (access) {
        case READ: return read();
        case WRITE: return write();
        case EXECUTE: return execute();
        default: break;
    }

    assert(false);  //should never hit here
    return false;
}

Hardware::TLBEntry Hardware::TLB::translate(const Word& vaddr, TLBEntry::AccessType access) const {
    Word vpn = vaddr >> 12;
    using AccessType = TLBEntry::AccessType;
    using ExceptionCode = Trap::ExceptionCode;

    for (Word i = 0; i < TLB_SIZE; ++i) {
        const TLBEntry& e = entries[i];
        if (!e.valid() || e.VPN() != vpn) continue;
        
        if (access == AccessType::WRITE && !e.dirty() ) throw Trap(ExceptionCode::TLB_MOD);

        if (!e.permits(access)) throw Trap( access == AccessType::WRITE ? ExceptionCode::TLB_MISS_S : ExceptionCode::TLB_MISS_L );

        return e;
    }

    throw Trap( access == AccessType::WRITE ? ExceptionCode::TLB_MISS_S : ExceptionCode::TLB_MISS_L );
}

void Hardware::TLB::insert(const Word& index, const TLBEntry& tlbEntry) {
    if (index >= TLB_SIZE) return;
    entries[index] = tlbEntry;
}

void Hardware::TLB::insertRandom(const TLBEntry& tlbEntry) {
    Word idx = rand() % TLB_SIZE;
    entries[idx] = tlbEntry;
}

void Hardware::TLB::flush() {
    for (Word i = 0; i < TLB_SIZE; ++i) {
        entries[i].accessHiRaw() = 0;
        entries[i].accessLoRaw() = 0;
    }
}

inline Hardware::TLBEntry& Hardware::TLB::operator[](const Word& idx) {
    assert(idx < TLB_SIZE);
    return entries[idx];
}

inline Hardware::TLBEntry& Hardware::TLB::at(const Word& idx) {
    assert(idx < TLB_SIZE);
    return entries[idx];
}

inline const Hardware::TLBEntry& Hardware::TLB::at(const Word& idx) const {
    assert(idx < TLB_SIZE);
    return entries[idx];
}

inline Word Hardware::Memory::runTLB(const Word& addr, TLBEntry::AccessType access) const {
    if (addr > 0x7FFFFFFF && addr < 0xC0000000) return addr - 0x80000000;

    std::cout << "addr: " << addr << std::endl;
    assert(false);

    auto tlbEntry = tlb.translate(addr, access); // Exception is thrown if failed an instruction (should be) retried.
    return (tlbEntry.PFN() << 12) | (addr & 0xFFF);
}

Hardware::Memory::Memory() : physicalMemory(new Byte[PHYS_MEM_SIZE]) {}

inline Hardware::TLB& Hardware::Memory::accessTLB() { return tlb; }
inline const Hardware::TLB& Hardware::Memory::readTLB() const { return tlb; }

// later need to add checks for bad-addr accesses
Word Hardware::Memory::getWord(const Word& addr) const {
    Word phys_addr = runTLB(addr, TLBEntry::AccessType::READ);
    return Binary::loadBigEndian( physicalMemory.get() + phys_addr );
}

HalfWord Hardware::Memory::getHalfWord(const Word& addr) const {
    Word phys_addr = runTLB(addr, TLBEntry::AccessType::READ);
    Byte* halfword = physicalMemory.get() + phys_addr;
    return (halfword[0] << 8) | halfword[1];
}

Byte Hardware::Memory::getByte(const Word& addr) const {
    Word phys_addr = runTLB(addr, TLBEntry::AccessType::READ);
    return physicalMemory[ phys_addr ];
}

void Hardware::Memory::setWord(const Word& addr, const Word& word) {
    Word phys_addr = runTLB(addr, TLBEntry::AccessType::WRITE);
    physicalMemory[ phys_addr ]     = word >> 24;
    physicalMemory[ phys_addr + 1 ] = (word >> 16) & 0xFF;
    physicalMemory[ phys_addr + 2 ] = (word >> 8) & 0xFF;
    physicalMemory[ phys_addr + 3 ] = word & 0xFF;
}

void Hardware::Memory::setHalfWord(const Word& addr, const HalfWord& halfword) {
    Word phys_addr = runTLB(addr, TLBEntry::AccessType::WRITE);
    physicalMemory[ phys_addr ]     = halfword >> 8;
    physicalMemory[ phys_addr + 1 ] = halfword & 0xFF;
}
void Hardware::Memory::setByte(const Word& addr, const Byte& byte) {
    Word phys_addr = runTLB(addr, TLBEntry::AccessType::WRITE);
    physicalMemory[ phys_addr ] = byte;
}

float Hardware::Memory::getSingle(const Word& addr) const {
    Word tmpw = getWord(addr);
    float tmpf = 0;
    memcpy(&tmpf, &tmpw, 4);
    return tmpf;
}
void Hardware::Memory::setSingle(const Word& addr, const float& single) {
    Word tmpw = 0;
    memcpy(&tmpw, &single, 4);
    setWord(addr, tmpw);
}
