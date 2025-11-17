#ifndef BASE_CORE_H
#define BASE_CORE_H



///////////////////
// Foreign Includes

#include <cstdint>
#include <cstring>
#include <cstddef>
#include <iostream>
#include <limits>

//////////////////
// Codebase Keywords

#define internal static
#define global static
#define local_persist static

////////////////////////////////
// Linkage Keyword Macros

#if LANG_CPP
# define C_LINKAGE_BEGIN extern "C"{
# define C_LINKAGE_END }
# define C_LINKAGE extern "C"
#else
# define C_LINKAGE_BEGIN
# define C_LINKAGE_END
# define C_LINKAGE
#endif


////////////////////////////////
//~ Misc. Helper Macros

#define Stringify_(S) #S 
#define Stringify(S) Stringify_(S)

#define Glue_(A,B) A##B 
#define Glue(A,B) Glue_(A,B)

#define ArrayCount(a) (sizeof(a) / sizeof((a)[0]))

#define CeilIntegerDiv(a,b) (((a) + (b) - 1)/(b))

#if LANG_CPP
# define zero_struct {}
#else
# define zero_struct {0}
#endif

//////////////////
// Base Types

using U8  = uint8_t;
using U16 = uint16_t;
using U32 = uint32_t;
using U64 = uint64_t;
using S8  = int8_t;
using S16 = int16_t;
using S32 = int32_t;
using S64 = int64_t;
using B32 = S32; // bool32
using F32 = float;
using F64 = double;

////////////////////////////////
// Toolchain/Environment Enums

enum class OperatingSystem : U8
{
	Null,
	Windows,
	Linux,
	Mac,
	COUNT,
#if OS_WINDOWS
	CURRENT = Windows,
#elif OS_LINUX
	CURRENT = Linux,
#elif OS_MAC
	CURRENT = Mac,
#else
	CURRENT = Null,
#endif
};

enum class Arch : U8
{
	Null,
	x64,
	x86,
	arm64,
	arm32,
	COUNT,
#if X64
	CURRENT = x64,
#elif X86
	CURRENT = x86,
#elif ARM64
	CURRENT = arm64,
#elif ARM32
	CURRENT = arm32,
#else
	CURRENT = Null,
#endif
};

enum class Compiler : U8
{
	Null,
	msvc,
	gcc,
	clang,
	COUNT,
#if MSVC
	CURRENT = msvc,
#elif GCC
	CURRENT = gcc,
#elif CLANG
	CURRENT = clang,
#else
 	CURRENT = Null,
#endif
};

////////////////////////////////
// Asserts

#if MSVC
# define Trap() __debugbreak()
#elif CLANG || GCC
# define Trap() __builtin_trap()
#else
# error Unknown trap intrinsic for this compiler.
#endif

// remember never call functions in asserts
#define AssertAlways(x) do{if(!(x)) {Trap();}}while(0)

#undef Assert
#if BUILD_DEBUG
# define Assert(x) AssertAlways(x)
#else
# define Assert(x) (void)(x)
#endif
#define InvalidPath        Assert(!"Invalid Path!")
#define NotImplemented     Assert(!"Not Implemented!")
#if LANG_CPP
# define StaticAssert(C, MESSAGE) static_assert(C, Stringify(MESSAGE))
#else
# define StaticAssert(C, ID)      global U8 Glue(ID, __LINE__)[(C)?1:-1]
#endif
//////////////////
// Units

#define KB(n)  (((U64)(n)) << 10)
#define MB(n)  (((U64)(n)) << 20)
#define GB(n)  (((U64)(n)) << 30)
#define TB(n)  (((U64)(n)) << 40)
#define Thousand(n)   ((n)*1000)
#define Million(n)    ((n)*1000000)
#define Billion(n)    ((n)*1000000000)

/////////////////
// Clamps, Mins, Maxes

#define Min(A,B) (((A)<(B))?(A):(B))
#define Max(A,B) (((A)>(B))?(A):(B))
#define ClampTop(A,X) Min(A,X)
#define ClampBot(X,B) Max(X,B)

////////////////////////////////
// Type -> Alignment

#if COMPILER_MSVC
# define AlignOf(T) __alignof(T)
#elif COMPILER_CLANG
# define AlignOf(T) __alignof(T)
#elif COMPILER_GCC
# define AlignOf(T) __alignof__(T)
#else
# error AlignOf not defined for this compiler.
#endif

/////////////////
// Memory Operations

// TODO




//////////////////
// Linked List Building Macros

// doubly-linked lists
#define DLLInsert_NP(f,l,p,n,next,prev) ((f)==nullptr?\
((f) = (l) = (n), (n)->next=nullptr, (n)->prev=nullptr) :\
(p)==nullptr ? \
((n)->next = (f), (f)->prev = (n), (f) = (n), (n)->prev=nullptr) :\
((p)==(l)) ? \
((l)->next = (n), (n)->prev = (l), (l) = (n), (n)->next=nullptr) :\
((p!=nullptr && (p)->next==nullptr)) ? (nullptr) : ((p)->next->prev = (n), ((n)->next = (p)->next), ((p)->next = (n)), ((n)->prev = (p))))
#define DLLPushBack_NP(f,l,n,next,prev) ((f)==nullptr?\
((f)=(l)=(n),(n)->next=(n)->prev=nullptr):\
((n)->prev=(l),(l)->next=(n),(l)=(n),(n)->next=nullptr))
#define DLLRemove_NP(f,l,n,next,prev) ((f)==(n)?\
((f)==(l)?\
((f)=(l)=(0)):\
((f)=(f)->next,(f)->prev=nullptr)):\
(l)==(n)?\
((l)=(l)->prev,(l)->next=nullptr):\
((n)->next->prev=(n)->prev,\
(n)->prev->next=(n)->next))

// singly-linked, doubly-headed lists (queues)
#define SLLQueuePush_N(f,l,n,next) (((f)==nullptr?\
(f)=(l)=(n):\
((l)->next=(n),(l)=(n))),\
(n)->next=nullptr)
#define SLLQueuePushFront_N(f,l,n,next) ((f)==nullptr?\
((f)=(l)=(n),(n)->next=nullptr):\
((n)->next=(f),(f)=(n)))
#define SLLQueuePop_N(f,l,next) ((f)==(l)?\
(f)=(l)=nullptr:\
((f)=(f)->next))

// singly-linked, single-headed lists (stacks)
#define SLLStackPush_N(f,n,next) ((n)->next=(f),(f)=(n))
#define SLLStackPop_N(f,next) ((f)==nullptr?nullptr:\
((f)=(f)->next))

// doubly-linked list helpers
#define DLLInsert(f,l,p,n) DLLInsert_NP(f,l,p,n,next,prev)
#define DLLPushBack(f,l,n) DLLPushBack_NP(f,l,n,next,prev)
#define DLLPushFront(f,l,n) DLLPushBack_NP(l,f,n,prev,next)
#define DLLRemove(f,l,n) DLLRemove_NP(f,l,n,next,prev)

// singly-linked, doubly-headed lists helpers
#define SLLQueuePush(f,l,n) SLLQueuePush_N(f,l,n,next)
#define SLLQueuePushFront(f,l,n) SLLQueuePushFront_N(f,l,n,next)
#define SLLQueuePop(f,l) SLLQueuePop_N(f,l,next)

// singly-linked, singly-headed lists helpers
#define SLLStackPush(f,n) SLLStackPush_N(f,n,next)
#define SLLStackPop(f) SLLStackPop_N(f,next)

#if 0
template <typename T>
internal void 
SLLQueuePush(T*& first, T*& last, T*& node)
{
	if (first == nullptr)
	{
		first = node;
	} 
	else
	{
		last->next = node;
	}

	last = node;
	node->next = nullptr;
}
#endif


#endif // BASE_CORE_H
