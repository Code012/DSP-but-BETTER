/* date = November 3rd 2025*/


#ifndef OS_INC_H
#define OS_INC_H

#if !defined(ArenaImpl_Reserve)
# define ArenaImpl_Reserve OS_Reserve
#endif
#if !defined(ArenaImpl_Release)
#define ArenaImpl_Release OS_Release
#endif
#if !defined(ArenaImpl_Commit)
# define ArenaImpl_Commit OS_Commit
#endif
#if !defined(ArenaImpl_Decommit)
# define ArenaImpl_Decommit OS_Decommit
#endif

#include "os/core/os_core.h"

#if OS_WINDOWS
# include "os/core/win32/os_core_win32.h"
#elif OS_LINUX
# include "os/core/linux/os_core_linux.h"
#else
# error OS core layer not implemented for this operating system.
#endif


#endif // OS_INC_H