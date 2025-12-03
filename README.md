"#DSP-but-BETTER" 

List of completed "things" for reference when writing project summary:
1. Parser
- parser utilises N-read N-write mechanism via a ring buffer that holds 64 tokens. So writes 64-tokens to ring buffer then reads 64-tokens. Advice from NeGate, helps keep data "hot" entirely in L1 cache. After writing 64 tokens, all cache lines for the buffer have been loaded so reading the 64 tokens hit the exact same cache lines that were fetched during writes and will not have been evicted because the working set is tiny. Also a lineary predictable memory access pattern helps. It's better than 1-write 1-read, alsways better to do it in batches, hence N-write N-read.

- expression tree nodes allocated onto an growable memory pool (arena-backed free list). Currently don't decommit committed memory at all, memory freeing policy is simply push onto freelist. Don't think I need to decommit memory at all, just let the OS reclaim that memory when the program closes. We'll see.

- parser uses a side-channel error and warning system so all the errors can be caught and displayed. Warnings won't stop the program but errors will. errors logged onto its own dedicated arena.