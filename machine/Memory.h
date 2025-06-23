#ifndef __MEMORY_H__
#define __MEMORY_H__
#include <unordered_map>
#include <memory>

using Byte = unsigned char;
using HalfWord = unsigned short;
using Word = unsigned int;

#define PAGE_SIZE 4096UL
#define PHYS_MEM_SIZE 268435456UL
#define TLB_SIZE 32

namespace Hardware {

    class TLBEntry {
        Word entryHi;
        Word entryLo;

        static Word constexpr VPN_MASK = 0x7FFFF << 13;
        static Word constexpr G_MASK = 1 << 12;         //global
        static Word constexpr R_MASK = 1 << 11;         //read
        static Word constexpr W_MASK = 1 << 10;         //write
        static Word constexpr X_MASK = 1 << 9;          //execute
        static Word constexpr ASID_MASK = 0xFF;
        static Word constexpr PFN_MASK = 0xFFFFFF << 6;
        static Word constexpr C_MASK = 7 << 3;          //coherence
        static Word constexpr D_MASK = 1 << 2;          //dirty
        static Word constexpr V_MASK = 1 << 1;          //valid
        
    public:
        enum AccessType : Byte {
            READ = 0,
            WRITE = 1,
            EXECUTE = 2
        };

        TLBEntry() = default;
        TLBEntry(const Word& entryHi, const Word& entryLo) : entryHi(entryHi), entryLo(entryLo) {}

        Word VPN() const;
        void setVPN(const Word& vpn);
        bool global() const;
        void setGlobal(bool g);
        bool read() const;
        void setRead(bool r);
        bool write() const;
        void setWrite(bool w);
        bool execute() const;
        void setExecute(bool x);
        Byte ASID() const;
        void setASID(Byte asid);
        Word PFN() const;
        void setPFN(const Word& pfn);
        Byte coherence() const;
        void setCoherence(Byte c);
        bool dirty() const;
        void setDirty(bool d);
        bool valid() const;
        void setValid(bool v);

        bool permits(AccessType access) const;

        Word& accessHiRaw();
        Word& accessLoRaw();
        const Word& readHiRaw() const;
        const Word& readLoRaw() const;
    };

    class TLB {
        TLBEntry entries[TLB_SIZE];

    public:

        TLB() : entries{} {};
        TLBEntry translate(const Word& vaddr, TLBEntry::AccessType access) const;   // returns nullptr on TLB miss
        void insert(const Word& index, const TLBEntry& tlbEntry);
        void insertRandom(const TLBEntry& tlbEntry);
        void flush();

        TLBEntry& operator[](const Word& idx);
        TLBEntry& at(const Word& idx);
        const TLBEntry& at(const Word& idx) const;
    };


    class Memory {
        std::unique_ptr<Byte[]> physicalMemory;
        TLB tlb;
        Word runTLB(const Word& addr, TLBEntry::AccessType access) const;

    public:
        Memory();

        TLB& accessTLB();
        const TLB& readTLB() const;
    
        Word getWord(const Word& addr) const;
        HalfWord getHalfWord(const Word& addr) const;
        Byte getByte(const Word& addr) const;

        void setWord(const Word& addr, const Word& word);
        void setHalfWord(const Word& addr, const HalfWord& halfword);
        void setByte(const Word& addr, const Byte& byte);

        float getSingle(const Word& addr) const;
        double getDouble(const Word& addr) const;

        void setSingle(const Word& addr, const float& single);
        void setDouble(const Word& addr, const double& dble);

    };

};

#endif