#pragma once
// TODO(me): Get rid of this once finished development
#include "core/os_core.hpp"
#include "unity.h" // so clangd knows where symbols are for unity build 



////////////////////////////////
// TODO: Setup

////////////////////////////////
// TODO: TLS

////////////////////////////////
// Memory Functions

internal void* OS_MemoryReserve(U64 size)
{
	void* result = VirtualAlloc(0, size, MEM_RESERVE, PAGE_READWRITE);
	return result;
}

internal B32   OS_MemoryCommit(void* memory, U64 size)
{
	B32 result = (VirtualAlloc(memory, size, MEM_COMMIT, PAGE_READWRITE) != 0);
	return result;
}

internal void  OS_MemoryDecommit(void* memory, U64 size)
{
	VirtualFree(memory, size, MEM_DECOMMIT);
}

internal void  OS_MemoryRelease(void* memory, U64 size)
{
	VirtualFree(memory, 0, MEM_RELEASE);
}

////////////////////////////////
// TODO: File Handling

////////////////////////////////
// TODO: Time

////////////////////////////////
// TODO: Libraries