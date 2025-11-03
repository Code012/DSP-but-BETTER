/* date = November 3rd 2025*/


#ifndef OS_INC_H
#define OS_INC_H


#include "os/core/os_core.hpp"

#if OS_WINDOWS
# include "os/core/win32/os_core_win32.hpp"
#elif OS_LINUX
# include "os/core/linux/os_core_linux.hpp"
#else
# error OS core layer not implemented for this operating system.
#endif



#endif // OS_INC_H