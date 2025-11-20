



////////////////////////////////
// TODO: Setup

////////////////////////////////
// TLS



////////////////////////////////
// Memory Functions

internal void* OS_Reserve(U64 size)
{
	void* result = VirtualAlloc(0, size, MEM_RESERVE, PAGE_NOACCESS);
	return result;
}

internal B32 OS_Commit(void* memory, U64 size)
{
	B32 result = (VirtualAlloc(memory, size, MEM_COMMIT, PAGE_READWRITE) != 0);
	return result;
}

internal void OS_Decommit(void* memory, U64 size)
{
	VirtualFree(memory, size, MEM_DECOMMIT);
}

internal void OS_Release(void* memory, U64 size)
{
	VirtualFree(memory, 0, MEM_RELEASE);
}

////////////////////////////////
// TODO: File Handling

////////////////////////////////
// TODO: Time

////////////////////////////////
// TODO: Libraries