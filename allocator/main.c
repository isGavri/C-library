#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

// Shifts bits to the left, ex. 0000 0000 0000 0100 = 4 | 0000 0000 0000 0001 = 1
#define KiB(n) ((u64)(n) << 10) // 0001 0000 0000 0000 = 4092 bytes | 0000 0100 0000 0000 = 1024 bytes
#define MiB(n) ((u64)(n) << 20) // Same concept but 20 spaces resulting in 2^20 = 1,048,576 bytes
#define GiB(n) ((u64)(n) << 30) // Same concept but 30 spaces resulting in 2^30 = 1,073,741,824 bytes

// inline max and min of two numbers
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

// Cpu reads on words (64bits for modern systems)
// Aligning is neccessary because lets say we are at memory position
#define ALIGN_UP_POW2(n, p) (((u64)(n) + ((u64)(p) - 1)) & (~((u64)(p) - 1)))

#define ARENA_BASE_POS (sizeof(mem_arena))
#define ARENA_ALIGN (sizeof(void*))

typedef struct
{
    u64 reserve_size;
    u64 commit_size;

    u64 capacity;
    u64 pos;
} mem_arena;

mem_arena* arena_create(u64 capacity);
void arena_destroy(mem_arena* arena);
void* arena_push(mem_arena* arena, u64 size, b32 non_zero);
void arena_pop(mem_arena* arena, u64 size);
void arena_pop_to(mem_arena* arena, u64 size);
void arena_clear(mem_arena* arena);

#define PUSH_STRUCT(arena, T) (T*)arena_push((arena), sizeof(T), false)
#define PUSH_ARRAY(arena, T) (T*)arena_push((arena), sizeof(T) * (n), true)
#define PUSH_STRUCT_NZ(arena, T, n) (T*)arena_push((arena), sizeof(T), true)
#define PUSH_ARRAY_NZ(arena, T, n) \
    (T*)arena_push((arena), sizeof(T) * (n), true)

u32 plat_get_pagesize(void);

void* plat_mem_reserve(u64 size);
b32 plat_mem_commit(void* ptr, u64 size);
b32 plat_mem_decommit(void* ptr, u64 size);
b32 plat_mem_release(void* ptr, u64 size);

int main(void)
{
    mem_arena* perm_arena = arena_create(MiB(1));

    while (1)
    {
        arena_push(perm_arena, MiB(16), false);
        getc(stdin);
    }

    arena_destroy(perm_arena);

    return 0;
}

mem_arena* arena_create(u64 capacity)
{
    mem_arena* arena = (mem_arena*)malloc(capacity);
    arena->capacity = capacity;
    arena->pos = ARENA_BASE_POS;

    return arena;
}

void arena_destroy(mem_arena* arena)
{
    free(arena);
}

void* arena_push(mem_arena* arena, u64 size, b32 non_zero)
{
    u64 pos_aligned = ALIGN_UP_POW2(arena->pos, ARENA_ALIGN);
    u64 new_pos = pos_aligned + size;

    if (new_pos > arena->capacity)
    {
        return NULL;
    }

    arena->pos = new_pos;
    u8* out = (u8*)arena + pos_aligned;

    if (!non_zero)
    {
        memset(out, 0, size);
    }

    return out;
}

void arena_pop(mem_arena* arena, u64 size)
{
    size = MIN(size, arena->pos - ARENA_BASE_POS);
    arena->pos -= size;
}
void arena_pop_to(mem_arena* arena, u64 pos)
{
    u64 size = pos < arena->pos ? arena->pos - pos : 0;
    arena_pop(arena, size);
}
void arena_clear(mem_arena* arena)
{
    arena_pop_to(arena, ARENA_BASE_POS);
}
