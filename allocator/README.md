# Linear arena allocator
You can install as a shared library into your system with.
Completely based on [magicalbat's arena](https://www.youtube.com/watch?v=xuODcuVR_ZE) with small changes and omitting windows support.
```bash
# install.sh
```
## Usage
We have two intended ways of using the linear allocator and also two (with variations) of pushing to memory.
But first we gotta understand what a linear arena allocator is and what is meant for:
This is probably the easiest allocation strategy, appart from letting compilers/interpreters
manage memory for us or simply `malloc`/`realloc` everything(which can also become the hardest). 
We part from the monistic and modern approach of thinking about memory: Virtual Memory.
Much like an arena allocator is a lienar contiguous block of memory that we can split into sections.
So the heap, stack and disk(not exposed in this strategy) all become one (logically).
### How it works
At the most basic implementation we have the block of memory and an offset or reference to the latest allocated object.
We can keep allocating objects as long as we haven't ran out of memory in the block, we just update the offset and assign the pointer to the object.
### Main arena
Create an arena, specify reserve_size (virtually infinite 16 exabytes 2^64) and commit_size (actual physical size you want to start with) 
save the pointer and that is the space you have for that specific resource or life time that you want.
```c
mem_arena* arena_create(u64 reserve_size, u64 commit_size);
void arena_destroy(mem_arena* arena);
```
### Scratch arena
Makes use of a temporal arena for all resources that dont need to last the whole lifetime of a main arena.
You can also use temporal arena the only difference is scratch arena is thread local.
You can use them both for transient lifetimes (of course).
```c
mem_arena_temp arena_scratch_get(mem_arena** conflicts, u32 num_conflicts);
void arena_scratch_release(mem_arena_temp scratch);
```
Temporal (not thread local)
```c 
mem_arena_temp arena_temp_begin(mem_arena* arena);
void arena_temp_end(mem_arena_temp temp);
```
### You can push objects with the helper macros
Fills with zero and doesnt fills with zero for arrays and structs.
```c 
#define PUSH_STRUCT(arena, T) (T*)arena_push((arena), sizeof(T), false)
#define PUSH_ARRAY(arena, T, n) (T*)arena_push((arena), sizeof(T) * (n), false)
#define PUSH_STRUCT_NZ(arena, T) (T*)arena_push((arena), sizeof(T), true)
#define PUSH_ARRAY_NZ(arena, T, n) (T*)arena_push((arena), sizeof(T) * (n), true)
```
# Resources (reference)
This explain it way better that I could ever
## Arena
[magicalbat's arena](https://www.youtube.com/watch?v=xuODcuVR_ZE)
[Ginger Bill's series about allocation strategies](https://www.gingerbill.org/article/2019/02/08/memory-allocation-strategies-002/)
[Ryan Fleury's Untangling lifetimes](https://www.dgtlgrove.com/p/untangling-lifetimes-the-arena-allocator)
## Aligment
[IBM - Data alignment: Straighten up and fly right](https://developer.ibm.com/articles/pa-dalign/)
[Gallery of processor cache effects](https://igoro.com/archive/gallery-of-processor-cache-effects/)
[Protected Mode Basics](https://www.rcollins.org/articles/pmbasics/tspec_a1_doc.html)
