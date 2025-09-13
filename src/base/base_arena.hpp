/* 
6/11/2024
www.gingerbill.org/article/2019/02/08/memory-allocation-strategies-002/
Refactored 12/09/2025
*/



template <size_t SIZE>  
struct BumpAllocator 
{
    unsigned char *memory;
    U64 size{SIZE};
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
    T* ArenaPush(U64 count, U64 align, B32 zero) 
    {
        
        if (!memory) 
        {
            std::cerr << "Bad Alloc" << std::endl;
            return nullptr;
        }

        U64 num_bytes = sizeof(T) * count; // size to allocate

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
            current_offset = aligned_offset+num_bytes;

            if (zero) { MemoryZero(allocated_ptr, num_bytes); }

            alloc_counter += static_cast<int>(count);

            return reinterpret_cast<T*>(allocated_ptr);
        }

        std::cout << "Not Enough space for aligned allocation!" 
                  << "Need: " << (aligned_offset + num_bytes)
                  <<", Available: " << size << std::endl;
        return nullptr;
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
        return ArenaPush<T>(count, align, zero);
    }
    template <typename T>
    T* PushArrayNoZero(U64 count, U64 align=DefaultAlign(alignof(T)))
    {
        return ArenaPush<T>(count, align, 0);
    }
};


