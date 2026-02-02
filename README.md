# DSP-but-BETTER

### Project: "Equation solver with stepped solutions"

This project is not yet complete so there's no binary I can just give to you, however I do estimate this would be complete in about a month and a half. 

If you're curious about what's been done so far and want to have a look around. Here is a list of notable things done in this project so far:

- Code and architecture written to easily target other platforms if needed (see os layer for platform code)
- Custom base layer for memory, length-based strings, math, os and compiler specific attributes
- Custom expression parser with full UTF-8 support via codepoint-based tokenisation and custom side-channel warning and error reporting system (I go in depth about this [here](#parser))
- Custom single-line UI text edit widget written from scratch. However missing selection and common hotkeys (for NOW)
- Using Clay for UI layouting and raylib for windowing and rendering. (don't know how to do either of these things from scratch YET)
- Built in a single translation unit or a unity build with layered dependencies forming a DAG.
- Programming w.r.t. memory and hardware. Using an arena-backed freelist to store and recycle expression tree nodes as I modify them in my simplification engine. Make extensive use of arenas for contiguous allocations, leveraging a pre-reserved virtual memory region and commiting pages on demand for extremely cheap allocations.

If you're curious to know how the project will turn out. Here is a list of things to expect:

- Algebra simplification engine written from scratch (engine sounds fancy)
- Add selection, common hot keys, limit input based on number of codepoints and copy/paste or cut functionality to my single-line text input widget
- Stepped solutions for solving the equation (which will have all sorts of nice UX, like hovering over a term in a previous step will highlight how that term changed over the next steps and it'll have collapsible buttons to toggle between shorter explanations or longer ones, displayed in a scrollable container etc etc. It will look amazing and be very performant.)
- And some other things that don't yet come to mind :).

Hopefully this gives you an idea of how this project is meant to turn out.

---

List of "things" for reference when writing project summary also just in general:

<a id="parser"></a>
1. Parser
- parser utilises N-read N-write mechanism via a ring buffer that holds 64 tokens. So writes 64-tokens to ring buffer then reads 64-tokens. Advice from NeGate, helps keep data "hot" entirely in L1 cache. After writing 64 tokens, all cache lines for the buffer have been loaded so reading the 64 tokens hit the exact same cache lines that were fetched during writes and will not have been evicted because the working set is tiny. Also a lineary predictable memory access pattern helps. It's better than 1-write 1-read, always better to do it in batches, hence N-write N-read.

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


Code base heavily inspired by Ryan Fleury and Allen Webster. I owe them a big thanks.