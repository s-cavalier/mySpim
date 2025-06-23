#include "Process.h"
#include "kstl/String.h"
#include "kstl/File.h"
#include "kstl/Elf.h"

extern "C" void run_process(kernel::TrapFrame& trapFrame);

kernel::PCB::PCB(const char* binaryFile, bool fromSpim) : PID(1), state(READY) {
    // init non-zero regs
    trapFrame.sp  = 0x7ffffffc;
    trapFrame.gp  = 0x10008000;

    // write the binary into memory, will have to replace this with ELF handling later on

    if (fromSpim) {
        trapFrame.epc = 0x00400020; // will implicitly gain a +4
        char* placeFile = (char*)(trapFrame.epc + 4);

        ministl::vector<char> bytes;
        File file(binaryFile, O_RDONLY);

        while (!(bytes = file.read(256)).empty()) {
            for (uint32_t i = 0; i < bytes.size(); ++i) *(placeFile++) = bytes[i];
        }

        return;
    }


    // Using elf

    File file(binaryFile, O_RDONLY);

    // --- 1) Read and parse the ELF header ---
    auto ehdrBuf = file.read(sizeof(Elf32_Ehdr));
    const Elf32_Ehdr* ehdr = reinterpret_cast<const Elf32_Ehdr*>(ehdrBuf.data());
    struct { uint32_t entry; uint32_t text_offset; uint32_t text_size; uint32_t data_offset; uint32_t data_size; } info;
    info.entry = ehdr->e_entry;

    // 2) Read section headers
    ministl::vector<Elf32_Shdr> shdrs;
    shdrs.resize(ehdr->e_shnum);
    file.seek(ehdr->e_shoff, 0);
    for (uint16_t i = 0; i < ehdr->e_shnum; ++i) {
        auto secBuf = file.read(ehdr->e_shentsize);
        shdrs[i] = *reinterpret_cast<const Elf32_Shdr*>(secBuf.data());
    }

    // 3) Load section-header string table
    const Elf32_Shdr& shstr_sh = shdrs[ehdr->e_shstrndx];
    file.seek(shstr_sh.sh_offset, 0);
    auto shstrtab = file.read(shstr_sh.sh_size);
    const char* shstr = shstrtab.data();

    // 4) Find .text and .data sections
    for (uint16_t i = 0; i < ehdr->e_shnum; ++i) {
        const Elf32_Shdr& sh = shdrs[i];
        const char* name = shstr + sh.sh_name;
        if (ministl::streq(name, ".text")) {
            info.text_offset = sh.sh_offset;
            info.text_size   = sh.sh_size;
        } else if (ministl::streq(name, ".data")) {
            info.data_offset = sh.sh_offset;
            info.data_size   = sh.sh_size;
        }
    }

    // Now just place stuff
    file.seek(info.text_offset, 0);
    auto textSection = file.read(info.text_size);
    file.seek(info.data_offset, 0);
    auto dataSection = file.read(info.data_size);
    
    // place .text
    char* placeFile = (char*)0x00400024;
    for (uint32_t i = 0; i < textSection.size(); ++i) placeFile[i] = textSection[i];
    
    // place .data
    placeFile = (char*)0x10008000;
    for (uint32_t i = 0; i < dataSection.size(); ++i) placeFile[i] = dataSection[i];

    trapFrame.epc = info.entry - 4;
}

void kernel::PCB::run() {
    run_process(trapFrame);
}
