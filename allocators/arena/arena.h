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
  ((u64)(n)                                                                    \
   << 20) // Same concept but 20 spaces resulting in 2^20 = 1,048,576 bytes

#define GiB(n)                                                                 \
  ((u64)(n) << 30) // Same concept but 30 spaces resulting in 2^30 =
                   // 1,073,741,824 bytes


/* --- Error Codes --- */
typedef enum {
  ARENA_SUCCESS = 0,
  ARENA_ERROR_COMMIT_FAILED,
  ARENA_ERROR_RESERVE_FAILED,
  ARENA_ERROR_OUT_OF_MEMORY,
  ARENA_ERROR_INVALID_PARAM,
  ARENA_ERROR_NO_SCRATCH_AVAILABLE
} ArenaError;

// *** Arena Structs *** //
typedef struct mem_arena mem_arena;

// logical arena that uses another arena for temporal allocation
typedef struct {
  mem_arena* arena;
  u64 start_pos;
} mem_arena_temp;

// *** Arena operations ***//
#define PUSH_STRUCT(arena, T, o) arena_push((arena), sizeof(T), false, o)
#define PUSH_ARRAY(arena, T, n, o)                                             \
  arena_push((arena), sizeof(T) * (n), false, o)
#define PUSH_STRUCT_NZ(arena, T, o) arena_push((arena), sizeof(T), true, o)
#define PUSH_ARRAY_NZ(arena, T, n, o)                                          \
  arena_push((arena), sizeof(T) * (n), true, o)

/* *** Get String of ArenaError *** */
const char* arena_error_gets(const ArenaError error);

// *** Prototypes for arena management *** //
ArenaError arena_create(u64 reserve_size, u64 commit_size, mem_arena** arena);
void arena_destroy(mem_arena* arena);
ArenaError arena_push(mem_arena* arena, u64 size, b32 non_zero, void** out);
void arena_pop(mem_arena* arena, u64 size);
void arena_pop_to(mem_arena* arena, u64 pos);
void arena_clear(mem_arena* arena);

// *** Temporary & Scratch Arenas *** //
mem_arena_temp arena_temp_begin(mem_arena* arena);
void arena_temp_end(mem_arena_temp temp);
ArenaError arena_scratch_get(mem_arena** conflicts, u32 num_conflicts,
                             mem_arena_temp* arena);
void arena_scratch_release(mem_arena_temp scratch);


#endif
