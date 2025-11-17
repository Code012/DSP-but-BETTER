/* 
6/11/2024
www.gingerbill.org/article/2019/02/08/memory-allocation-strategies-002/
Refactored 12/09/2025
*/
#ifndef BASE_ARENA_HPP
#define BASE_ARENA_HPP



// TODO(me): Get rid of this once finished development
#include "unity.h" // so clangd knows where symbols are for unity build 



#include <cstdint>
template <size_t SIZE>  
struct BumpAllocator 
{
    unsigned char *memory;
    U64 size{SIZE};
    U64 previous_offset{};
    U64 current_offset{};
    int alloc_counter{};

    // Constructor
    // Initialise the memory pointer to the memory of the heap array
    BumpAllocator()
        : memory{static_cast<unsigned char *>(malloc(SIZE))}
    {}

    BumpAllocator(const BumpAllocator&) = delete;
    BumpAllocator& operator=(const BumpAllocator&) = delete;
    BumpAllocator(BumpAllocator&&) = delete;
    BumpAllocator& operator=(BumpAllocator&&) = delete;

    template <typename T>
    T* ArenaPush(U64 num_bytes, U64 align, B32 zero) 
    {
        
        if (!memory) 
        {
            std::cerr << "Push: Bad Alloc" << std::endl;
            return nullptr;
        }


        // Check for appropriate allocation size, 
        // Greater than 0 or if its greater than the maximum size
        if (num_bytes == 0 || num_bytes > size) 
        {
            std::cerr << "Invalid allocation size. Should be greater than 0 or less than the maximum size allocated: " << size << std::endl;
            return nullptr;
        }

        // Calculate the current pointer where the allocation memorys from
        uintptr_t current_pointer = reinterpret_cast<uintptr_t>(memory) + static_cast<uintptr_t>(current_offset);

        // Calculate the alignment and then the offset relative to memory
        uintptr_t aligned_pointer = (current_pointer + align - 1) & ~(align - 1);
        U64 aligned_offset = aligned_pointer - reinterpret_cast<uintptr_t>(memory); // calculate offset relative to memory of allocation

        if ( (num_bytes + aligned_offset) <= size ) 
        {
            void *allocated_ptr = &memory[aligned_offset];
            previous_offset = aligned_offset;
            current_offset = aligned_offset+num_bytes;

            if (zero) { MemoryZero(allocated_ptr, num_bytes); }

            alloc_counter += static_cast<int>(num_bytes/sizeof(T));

            return reinterpret_cast<T*>(allocated_ptr);
        }

        std::cout << "Not Enough space for aligned allocation!" 
                  << "Need: " << (aligned_offset + num_bytes)
                  <<", Available: " << size << std::endl;
        return nullptr;
    }

    template <typename T>
    T* ArenaResize(void* old_memory, U64 old_size, U64 new_size, U64 align=DefaultAlign(1))
    {
        if (!memory) 
        {
            std::cerr << "Resize: Bad Alloc" << std::endl;
            return nullptr;
        }

        unsigned char *old_mem = static_cast<unsigned char*>(old_memory);
        // Mimics realloc. When ptr is NULL, just allocate memory
        if (!old_mem || old_size==0)
        {
            return ArenaPush<T>(new_size, align, 1);
        }

        uintptr_t old_ptr = reinterpret_cast<uintptr_t>(old_mem);
        uintptr_t base_ptr = reinterpret_cast<uintptr_t>(memory);
        // Out of bounds
        if (old_ptr < base_ptr || old_ptr >= base_ptr + size)
        {
            std::cerr << "Old memory is out of bounds!\n";
            return nullptr;
        }

        // Fast path: old memory is latest allocation
        if (memory+previous_offset == old_mem)
        {
            if (previous_offset + new_size > size) { return nullptr;  } // not enough memory
            current_offset = previous_offset + new_size;
            if (new_size > old_size)
            {
                MemoryZero(&memory[current_offset], new_size-old_size);
            }
            return reinterpret_cast<T*>(old_memory);
        }

        // Slow path: allocate new and copy
        T* new_mem = ArenaPush<T>(new_size, align, 1);

        if (new_mem)
        {
            U64 copy_size = (old_size < new_size) ? old_size : new_size;
            MemoryCopy(new_mem, old_mem, copy_size);
        }
        return new_mem;
    }

    void ArenaRelease()
    {
        if (memory)
        {
            free(memory);
            memory = nullptr;
        }
    }

    // Get # of bytes allocated
    U64 ArenaGetPos()
    {
        return current_offset;
    }

    // Get remaining bytes
    U64 ArenaGetRemaining() 
    {
        return (current_offset <= size) ? (size - current_offset) : 0;
    }

    // Popping functions
    void ArenaSetPosBack(U64 pos)
    {
        if (pos <= size) { current_offset = pos; }
        else { std::cerr << "Position " << pos << " exceeds arena size " << size << std::endl; }
    }
    void ArenaClear()
    {
        current_offset = 0;
        alloc_counter = 0;
    }

    ~BumpAllocator() 
    {
        ArenaRelease();
    }

    // Pushing helper
    template <typename T>
    T* PushArray(U64 count, U64 align=DefaultAlign(alignof(T)), B32 zero=1)
    {
        return ArenaPush<T>(sizeof(T) * count, align, zero);
    }
    template <typename T>
    T* PushArrayNoZero(U64 count, U64 align=DefaultAlign(alignof(T)))
    {
        return ArenaPush<T>(sizeof(T) * count, align, 0);
    }
};


#endif // BASE_ARENA_HPP