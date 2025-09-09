/* 
for aligning logic www.gingerbill.org/article/2019/02/08/memory-allocation-strategies-002/
*/

#include <cstring>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>


template <size_t SIZE>  // allow user to input size. Needs to be defined as a template to ensure it is statically defined (at compile time)
class BumpAllocater {
    private:
        void * start = nullptr;
        size_t size = SIZE;
        size_t current_offset = 0;
        int alloc_counter = 0;

        void increase_alloc_counter() {
            alloc_counter++;
        }

        void decrease_alloc_counter() {
            alloc_counter--;
        }

        uintptr_t align_offset(uintptr_t ptr, size_t alignment) {
            //
            uintptr_t p, a, modulo;

            p = ptr;
            a = reinterpret_cast<uintptr_t>(alignment);

            // Check if the alignment is a power of 2
            if (alignment == 0) {
                std::cerr << "Alignment must be greater than zero" << std::endl;
                return 0;
            }

            if ((a & (a-1)) != 0) {
                std::cerr << "Alignment must be a power of two" << std::endl;
                return 0;
            }

            modulo = p % a;

            if (a > std::numeric_limits<uintptr_t>::max() - p) {        // check if p + a will not overflow
                std::cerr << ("Alignment adjustment overflowed") << std::endl;
                return 0;
            }

            if (modulo != 0) {
                p += (a-modulo);
                if (p < ptr) {      // overflow check, if p < ptr then it probably wrapped back round as a refult of overflow
                    std::cerr << ("Alignment adjustment overflowed") << std::endl;
                    return 0;
                }
            }

            return p;
        }


    public: 
        // Constructor
        // Initialise the start pointer to the start of the heap array
        BumpAllocater() {
            alloc_counter = 0;        
            
            start = malloc(size);
            }

        template <typename T>
        T* alloc(size_t num_bytes) {
            
            if (!start) {
                std::cerr << "Bad Alloc" << std::endl;
                return nullptr;
            }

            // Check for appropriate allocation size, 
            // Greater than 0 or if its greater than the maximum size
            if (num_bytes == 0 || num_bytes > size) {
                std::cerr << "Invalid allocation size. Should be greater than 0 or less than the maximum size allocated: " << size << std::endl;
                return nullptr;
            }

            // Calculate the current pointer where the allocation starts from
            uintptr_t current_pointer = reinterpret_cast<uintptr_t>(start) + reinterpret_cast<uintptr_t>(current_offset);

            // Checking for overflow. 'current_pointer' must always be greater than start, if it is smaller, 
            // it has wrapped around as a result of overflow
            if (current_pointer < reinterpret_cast<uintptr_t>(start)) {
                std::cerr << "Overflow Error" << std::endl;
                return nullptr;
            }

            // Calculate the alignment and then the offset relative to start
            uintptr_t relative_offset = align_offset(current_pointer, alignof(T));
            relative_offset -= reinterpret_cast<uintptr_t>(start); // calculate offset relative to start of allocation

            // Checking for overflow in offset calculation. If relative offset has wrapped around, to be greater than size
            // an overflow has occurred.
            if (relative_offset > size) {
                std::cerr << "Overflow Error" << std::endl;
                return nullptr;
            }
            
            if ( ((num_bytes) + relative_offset) <= size ) {
                uintptr_t next = reinterpret_cast<uintptr_t>(start) + relative_offset;
                current_offset = relative_offset+num_bytes;

                memset(reinterpret_cast<void*>(next), 0, num_bytes);

                increase_alloc_counter();

                return reinterpret_cast<T*>(next);
            }

            std::cout << "Not Enough space!" << std::endl;
            return nullptr;
        }

        void dealloc() {
            
            
            if (alloc_counter > 0) {
                decrease_alloc_counter();
            }
            if (alloc_counter == 0) {
                // std::cout << "Bumper Allocator Freed" << std::endl;
                current_offset = 0;
            }

        }

        ~BumpAllocater() {
            free(start);
        }

};
