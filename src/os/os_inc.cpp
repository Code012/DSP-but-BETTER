/* date = November 3rd 2025*/

#if OS_WINDOWS
# include "os/core/win32/os_core_win32.cpp"
#elif OS_LINUX
# include "os/core/linux/os_core_linux.cpp"
#else
# error OS core layer not implemented for this operating system.
#endif