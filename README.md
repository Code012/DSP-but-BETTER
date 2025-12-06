# DSP-but-BETTER

List of "things" for reference when writing project summary also just in general:

1. Parser
- parser utilises N-read N-write mechanism via a ring buffer that holds 64 tokens. So writes 64-tokens to ring buffer then reads 64-tokens. Advice from NeGate, helps keep data "hot" entirely in L1 cache. After writing 64 tokens, all cache lines for the buffer have been loaded so reading the 64 tokens hit the exact same cache lines that were fetched during writes and will not have been evicted because the working set is tiny. Also a lineary predictable memory access pattern helps. It's better than 1-write 1-read, alsways better to do it in batches, hence N-write N-read.

- expression tree nodes allocated onto an growable memory pool (arena-backed free list). Currently don't decommit committed memory at all, memory freeing policy is simply push onto freelist. Don't think I need to decommit memory at all, just let the OS reclaim that memory when the program closes. We'll see.

- parser uses a side-channel error and warning system so all the errors can be caught and displayed. Warnings won't stop the program but errors will. errors logged onto its own dedicated arena.


---

2. Entry point into program:
```
┌─────────────────────────────────────┐
│  OS Layer (Windows/Linux)           │
│  • os_core_win32.cpp                │
│  • main() → BaseMainEntry()         │
└──────────────┬──────────────────────┘
               │
               ↓
┌─────────────────────────────────────┐
│  Base Layer                         │
│  • BaseMainEntry()                  │
│  • Initialize subsystems            │
└──────────────┬──────────────────────┘
               │
               ↓
┌─────────────────────────────────────┐
│  Application                        │
│  • EntryPoint()                     │
│  • Your app code                    │
└─────────────────────────────────────┘
```

**Macro:** Set `BUILD_ENTRY_POINT_DEFINING_UNIT=1` in unity build

---

3. Code Tags Reference:
```
//@os_shared        // Code that's the same across all OS backends
//@os_per_backend   // Code that differs per OS
//@os_hooks         // Functions the OS layer expects you to implement
//@helpers          // Helper functions
//@thread_context   // Thread-local context stuff
```