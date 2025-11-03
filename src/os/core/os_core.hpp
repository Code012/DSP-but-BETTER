/* date = November 3rd 2025 4:06 am */

#ifndef OS_CORE_HPP
#define OS_CORE_HPP

// TODO(me): Get rid of this once finished development
#include "unity.h" // so clangd knows where symbols are for unity build 

////////////////////////////////
// TODO: Setup

////////////////////////////////
// TODO: TLS

////////////////////////////////
// Memory Functions

internal void* OS_MemoryReserve(U64 size);
internal B32   OS_MemoryCommit(void* memory, U64 size);
internal void  OS_MemoryDecommit(void* memory, U64 size);
internal void  OS_MemoryRelease(void* memory, U64 size);

////////////////////////////////
// TODO: File Handling

////////////////////////////////
// TODO: Time

////////////////////////////////
// TODO: Libraries


#endif // OS_CORE_HPP