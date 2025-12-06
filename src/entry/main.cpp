
//////////////////////////////
//- Build Options

#define BUILD_TITLE "main"
#define BUILD_ENTRY_POINT_DEFINING_UNIT 1 // export to batch file
#define BUILD_COMMAND_LINE_INTERFACE 1 // export to batch file

//////////////////////////////
//- Includes

//- foreign includes


//- [h] root
#include "base/base_inc.h"
#include "os/os_inc.h"
#include "parse/parse_inc.h"
#include "tester/simpletest.h"

//- [cpp] root
#include "base/base_inc.cpp"
#include "os/os_inc.cpp"
#include "parse/parse_inc.cpp"
#include "tester/simpletest.cpp"


internal void 
EntryPoint(U64 argument_count, char** arguments) 
{


}
