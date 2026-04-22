#ifndef ARENA_H
#define ARENA_H

#include <stdbool.h>
#include <stdint.h>

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

// *** Public Macros *** //

// Shifts bits to the left, ex. 0000 0000 0000 0100 = 4 | 0000 0000 0000 0001 =
// 1
#define KiB(n)                                                                 \
    ((u64)(n) << 10) // 0001 0000 0000 0000 = 4092 bytes | 0000 0100 0000 0000
                     // = 1024 bytes

#define MiB(n)                                                                 \
    ((u64)(n)                                                                  \
     << 20) // Same concept but 20 spaces resulting in 2^20 = 1,048,576 bytes

#define GiB(n)                                                                 \
    ((u64)(n) << 30) // Same concept but 30 spaces resulting in 2^30 =
                     // 1,073,741,824 bytes

// *** Arena Structs *** //
typedef struct {
    u64 reserve_size; // Asked size
    u64 commit_size;  // Actually used size
    u64 commit_pos;
    u64 pos;
} mem_arena;

// logical arena that uses another arena for temporal allocation
typedef struct {
    mem_arena* arena;
    u64 start_pos;
} mem_arena_temp;

// *** Arena operations ***//
#define PUSH_STRUCT(arena, T) (T*)arena_push((arena), sizeof(T), false)
#define PUSH_ARRAY(arena, T, n) (T*)arena_push((arena), sizeof(T) * (n), false)
#define PUSH_STRUCT_NZ(arena, T) (T*)arena_push((arena), sizeof(T), true)
#define PUSH_ARRAY_NZ(arena, T, n)                                             \
    (T*)arena_push((arena), sizeof(T) * (n), true)

// *** Prototypes for arena management *** //
mem_arena* arena_create(u64 reserve_size, u64 commit_size);
void arena_destroy(mem_arena* arena);
void* arena_push(mem_arena* arena, u64 size, b32 non_zero);
void arena_pop(mem_arena* arena, u64 size);
void arena_pop_to(mem_arena* arena, u64 pos);
void arena_clear(mem_arena* arena);

// *** Temporary & Scratch Arenas *** //
mem_arena_temp arena_temp_begin(mem_arena* arena);
void arena_temp_end(mem_arena_temp temp);
mem_arena_temp arena_scratch_get(mem_arena** conflicts, u32 num_conflicts);
void arena_scratch_release(mem_arena_temp scratch);

// *** Prototypes for memory management (Platform) *** //
u32 plat_get_pagesize(void);
void* plat_mem_reserve(u64 size);
b32 plat_mem_commit(void* ptr, u64 size);
b32 plat_mem_decommit(void* ptr, u64 size);
b32 plat_mem_release(void* ptr, u64 size);

#endif
