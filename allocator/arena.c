#if defined(__linux__) && !defined(_DEFAULT_SOURCE)
#define _DEFAULT_SOURCE
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// *** Types *** //
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef i8 b8;
typedef i32 b32;

// Shifts bits to the left, ex. 0000 0000 0000 0100 = 4 | 0000 0000 0000 0001 =
// 1
#define KiB(n) \
    ((u64)(n) << 10)  // 0001 0000 0000 0000 = 4092 bytes | 0000 0100 0000 0000
                      // = 1024 bytes
#define MiB(n) \
    ((u64)(n)  \
     << 20)  // Same concept but 20 spaces resulting in 2^20 = 1,048,576 bytes
#define GiB(n) \
    ((u64)(n) << 30)  // Same concept but 30 spaces resulting in 2^30 =
                      // 1,073,741,824 bytes

// inline max and min of two numbers
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

// Aligning is prefered because cpu reads on words (64 bits for modern systems
// so ARENA_ALIGN (size of pointer) for our case) and we dont want object to
// scatter across multiple cache lines ex. n = 16 and the alignment is 8 bytes
// so p = 8 16 - 0001 0000 + (8 - 1) = 23 - 0001 0111 8 - 1 = 7 - ~0000 0111 =
// 1111 1000 23 & ~7 0001 0111 & 1111 1000 = 0001 0000 = 16 no aligment needed
// stays the same ex. n = 19 p = 8 19 0001 0011 + 0000 0111 = 0001 1010 0000
// 0111 - 1 = ~0000 0110 - 1111 1001 0001 1000 & 1111 1001 = 0001 1000 = 24
// skipped the five bytes behind
#define ALIGN_UP_POW2(n, p) (((u64)(n) + ((u64)(p) - 1)) & (~((u64)(p) - 1)))

// We will always allocate this space which is the header of the arena
#define ARENA_BASE_POS (sizeof(mem_arena))
// Size of pointer for aligment
#define ARENA_ALIGN (sizeof(void*))

// Representation of out arena
typedef struct
{
    u64 reserve_size;
    u64 commit_size;

    u64 commit_pos;
    u64 pos;
} mem_arena;

// *** Prototypes for arena management *** //
mem_arena* arena_create(u64 reserve_size, u64 commit_size);
void arena_destroy(mem_arena* arena);
void* arena_push(mem_arena* arena, u64 size, b32 non_zero);
void arena_pop(mem_arena* arena, u64 size);
void arena_pop_to(mem_arena* arena, u64 pos);
void arena_clear(mem_arena* arena);

#define PUSH_STRUCT(arena, T) (T*)arena_push((arena), sizeof(T), false)
#define PUSH_ARRAY(arena, T, n) (T*)arena_push((arena), sizeof(T) * (n), false)
#define PUSH_STRUCT_NZ(arena, T) (T*)arena_push((arena), sizeof(T), true)
#define PUSH_ARRAY_NZ(arena, T, n) \
    (T*)arena_push((arena), sizeof(T) * (n), true)

// *** Prototypes for memory management *** //
u32 plat_get_pagesize(void);
void* plat_mem_reserve(u64 size);
b32 plat_mem_commit(void* ptr, u64 size);
b32 plat_mem_decommit(void* ptr, u64 size);
b32 plat_mem_release(void* ptr, u64 size);

typedef struct
{
    int x;
    int y;
    int z;
} Vector3;

int main(void)
{
    mem_arena* perm_arena = arena_create(GiB(1), KiB(1));

    Vector3* vectors = PUSH_ARRAY(perm_arena, Vector3, 3);

    for (int i = 0; i < 3; i++)
    {
        vectors[i].x = 1 + i * 3;
        vectors[i].y = 2 + i * 3;
        vectors[i].z = 3 + i * 3;
    }

    for (int i = 0; i < 3; i++)
    {
        printf("Vector %d: %d\tAddress %p\n", i, vectors[i].x, &vectors[i].x);
        printf("Vector %d: %d\tAddress %p\n", i, vectors[i].y, &vectors[i].y);
        printf("Vector %d: %d\tAddress %p\n", i, vectors[i].z, &vectors[i].z);
    }

    arena_destroy(perm_arena);

    return 0;
}

// *** Arena Management *** //

// Creates an arena with a given reserved size (virtual memory) and commit_size
// (initial physical memory)
mem_arena* arena_create(u64 reserve_size, u64 commit_size)
{
    u32 pagesize = plat_get_pagesize();

    reserve_size = ALIGN_UP_POW2(reserve_size, pagesize);
    commit_size = ALIGN_UP_POW2(commit_size, pagesize);

    mem_arena* arena = plat_mem_reserve(reserve_size);

    if (!plat_mem_commit(arena, commit_size))
    {
        return NULL;
    }

    arena->reserve_size = reserve_size;
    arena->commit_size = commit_size;
    arena->pos = ARENA_BASE_POS;
    arena->commit_pos = commit_size;

    return arena;
}

// Wrapper to release the memmory from the OS
void arena_destroy(mem_arena* arena)
{
    plat_mem_release(arena, arena->reserve_size);
}

// Pushes bytes of memory to the arena and returns a pointer to the start of the
// block
void* arena_push(mem_arena* arena, u64 size, b32 non_zero)
{
    u64 pos_aligned = ALIGN_UP_POW2(arena->pos, ARENA_ALIGN);
    u64 new_pos = pos_aligned + size;

    if (new_pos > arena->reserve_size)
        return NULL;

    if (new_pos > arena->commit_pos)
    {
        u64 new_commit_pos = new_pos;
        new_commit_pos += arena->commit_size - 1;
        new_commit_pos -= new_commit_pos % arena->commit_size;
        new_commit_pos = MIN(new_commit_pos, arena->reserve_size);

        u8* mem = (u8*)arena + arena->commit_pos;
        u64 commit_size = new_commit_pos - arena->commit_pos;

        if (!plat_mem_commit(mem, commit_size))
        {
            return NULL;
        }

        arena->commit_pos = new_commit_pos;
    }

    arena->pos = new_pos;

    u8* out = (u8*)arena + pos_aligned;

    if (!non_zero)
    {
        memset(out, 0, size);
    }

    return out;
}

// Moves back the position of the arena by given size so we can "reallocate"
void arena_pop(mem_arena* arena, u64 size)
{
    size = MIN(size, arena->pos - ARENA_BASE_POS);
    arena->pos -= size;
}

// Pops to a given position on the arena
void arena_pop_to(mem_arena* arena, u64 pos)
{
    u64 size = pos < arena->pos ? arena->pos - pos : 0;
    arena_pop(arena, size);
}

// Clears the arena to the start
void arena_clear(mem_arena* arena)
{
    arena_pop_to(arena, ARENA_BASE_POS);
}

#if defined(__linux__)

#include <sys/mman.h>
#include <unistd.h>

// Returns the pagesize of memory (usually 4KiB)
u32 plat_get_pagesize(void)
{
    return (u32)sysconf(_SC_PAGESIZE);
}

// Has the platform reserve given size of memory
void* plat_mem_reserve(u64 size)
{
    void* out = mmap(NULL, size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (out == MAP_FAILED)
    {
        return NULL;
    }
    return out;
}

// Changes protection flags of blocks of memory reserved with mmap so that we
// can read and write to them
b32 plat_mem_commit(void* ptr, u64 size)
{
    i32 ret = mprotect(ptr, size, PROT_READ | PROT_WRITE);
    return ret == 0;
}

// Releases memory from give pointer up to size
b32 plat_mem_release(void* ptr, u64 size)
{
    i32 ret = munmap(ptr, size);
    return ret == 0;
}

#endif
