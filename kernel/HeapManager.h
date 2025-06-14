#ifndef __HEAP_MANAGER_H__
#define __HEAP_MANAGER_H__


// replace these macros later
#define CURR_BRK sbrk(0)
#define MOVE_BRK(x) sbrk(x)
#define SET_BRK(x) brk(x)

#define align4(x) (((x) + 3) & ~0x3)

typedef decltype(sizeof(0)) size_t;

namespace Heap {

    struct BlockHeader {
        size_t size;
        BlockHeader* next;
        BlockHeader* prev;
        int free;   // bool
        void* ptr;
        char data[1];
    };

    #define BLOCKHEADER_OFFSET ((size_t)&(((Heap::BlockHeader*)0)->data))

    // Padding will force the data field to add to the size in full

    class FreeList {
        size_t block_count;
        size_t total_size;
        BlockHeader* head;
        BlockHeader* last_visited;
        void* heap_ptr;

        BlockHeader* getBlock(void* p);
        BlockHeader* findBlock(size_t s);
        BlockHeader* extendHeap(size_t s);
        void splitBlock(BlockHeader* block, size_t s);
        bool validAddr(void* p);
        BlockHeader* fusion(BlockHeader* block);

    public:
        FreeList() : block_count(0), total_size(0), head(nullptr), last_visited(nullptr), heap_ptr((void*)0xC0000000) {}

        inline const size_t& totalBytes() const { return total_size; }
        inline const size_t& totalBlocks() const { return block_count; }

        inline void* sbrk(int offset = 0) { if (!offset) return heap_ptr; return (heap_ptr = (char*)heap_ptr + offset); }
        inline void brk(void* addr) { heap_ptr = addr; }

        void* malloc(size_t s);
        void* calloc(size_t number, size_t size);
        void free(void* ptr);
    };

    static FreeList freeList;

}

#endif