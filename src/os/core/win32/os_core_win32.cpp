



////////////////////////////////
// TODO: Setup

////////////////////////////////
// TLS



////////////////////////////////
//- Memory Functions

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

//////////////////////////////
//- @os_per_backend Entry Point (no multi-threading support yet)

#if BUILD_ENTRY_POINT_DEFINING_UNIT
internal void EntryPoint(U64 argument_count, char** arguments); // forward declare

#if BUILD_COMMAND_LINE_INTERFACE
int main(int argument_count, char** arguments)
{
	BaseMainEntry(EntryPoint, (U64)argument_count, arguments);
	return 0;
}
#else
int WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR lp_cmd_line, int n_show_cmd)
{
	BaseMainEntry(EntryPoint, (U64)__argc, __argv);
	return 0;
}
#endif // BUILD_COMMAND_LINE_INTERFACE

#endif // BUILD_ENTRY_POINT_DEFINING_UNIT