/* date = November 3rd 2025 4:06 am */

#ifndef OS_CORE_HPP
#define OS_CORE_HPP

////////////////////////////////
// TODO: Setup

////////////////////////////////
// TODO: TLS

////////////////////////////////
// Memory Allocation (Implemented Per-OS)

internal void *OS_Reserve(U64 size);
internal B32   OS_Commit(void* memory, U64 size);
internal void  OS_Decommit(void* memory, U64 size);
internal void  OS_Release(void* memory, U64 size);

////////////////////////////////
// TODO: File Handling

////////////////////////////////
// TODO: Time

////////////////////////////////
// TODO: Libraries

////////////////////////////////
//- Entry Point
// NOTE: If BUILD_ENTRY_POINT_DEFINING_UNIT is defined to 1, 
// then the translation unit will contain the entry point for 
// the executable. The OS layer will define main() and call 
// EntryPoint(), which must be defined by the application.

#endif // OS_CORE_HPP