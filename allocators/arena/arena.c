#if defined(__linux__) && !defined(_DEFAULT_SOURCE)
#define _DEFAULT_SOURCE
#endif

#include "arena.h"
#include <string.h>

// *** Prototypes for memory management (Platform) *** //
static u32 plat_get_pagesize(void);
static ArenaError plat_mem_reserve(u64 size, void** out);
static b32 plat_mem_commit(void* ptr, u64 size);
static b32 plat_mem_decommit(void* ptr, u64 size);
static b32 plat_mem_release(void* ptr, u64 size);

// inline max and min of two numbers
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#define ARENA_MAGIC 0x4152454E // AREN in ascii
#define MAGIC_FREE 0xFFFFFFFF

// Aligning is prefered because cpu reads on its own words size (64 bytes for
// modern systems so ARENA_ALIGN (size of pointer) for our case) and we dont
// want object to scatter across multiple cache lines ex. n = 16 and the
// alignment is 8 bytes so p = 8 16 - 0001 0000 + (8 - 1) = 23 - 0001 0111 8 - 1
// = 7 - ~0000 0111 = 1111 1000 23 & ~7 0001 0111 & 1111 1000 = 0001 0000 = 16
// no aligment needed stays the same ex. n = 19 p = 8 19 0001 0011 + 0000 0111 =
// 0001 1010 0000 0111 - 1 = ~0000 0110 - 1111 1001 0001 1000 & 1111 1001 = 0001
// 1000 = 24 skipped the five bytes behind
// (n + p - 1) & ~(p - 1)
#define ALIGN_UP_POW2(n, p) (((u64)(n) + ((u64)(p) - 1)) & (~((u64)(p) - 1)))

// We will always allocate this space which is the header of the arena
#define ARENA_BASE_POS (sizeof(mem_arena))
// Size of pointer for aligment
#define ARENA_ALIGN (sizeof(void*))

/* Validate arena argument */
#define ARENA_VALIDATE(a)                                                      \
  do {                                                                         \
    if (!(a) || (a)->magic != ARENA_MAGIC)                                     \
      return ARENA_ERROR_INVALID_PARAM;                                        \
  } while (0)

/* *** Opaque arena structs *** */
typedef struct mem_arena {
  u32 magic;
  u64 reserve_size; // Asked size
  u64 commit_size;  // Actually used size
  u64 commit_pos;
  u64 pos;
  u64 max_pos;
} mem_arena;

// List of scratch arenas
static __thread mem_arena* _scratch_arenas[ARENA_SCRATCH_COUNT] = {NULL};
static __thread ArenaError _arena_last_error = ARENA_SUCCESS;

// *** Error management *** //
const char* arena_error_gets(const ArenaError error) {
  switch (error) {
  case ARENA_ERROR_COMMIT_FAILED:
    return "Error: Commiting memory for the arena failed";
    break;
  case ARENA_ERROR_RESERVE_FAILED:
    return "Error: Reserving memory for the arena failed";
    break;
  case ARENA_ERROR_OUT_OF_MEMORY:
    return "Error: Arena ran out of reserve memory\n\tIf you are not directly "
           "using the arena, you can define RESERVE_SIZE with you prefered "
           "value";
    break;
  case ARENA_ERROR_INVALID_PARAM:
    return "Error: Invalid parameter. NULL pointer or invalid size";
    break;
  case ARENA_ERROR_NO_SCRATCH_AVAILABLE:
    return "Error: No scratch arena available";
    break;
  case ARENA_SUCCESS:
    return "No error";
    break;
  }
  return "No error";
}

ArenaError arena_get_last_error(void) {
  return _arena_last_error;
}

// *** Arena Management *** //

// Creates an arena with a given reserved size (virtual memory) and commit_size
// (initial physical memory)
ArenaError arena_create(u64 reserve_size, u64 commit_size, mem_arena** out) {
  if (!out) {
    _arena_last_error = ARENA_ERROR_INVALID_PARAM;
    return ARENA_ERROR_INVALID_PARAM;
  }

  // Get system pagesize
  u32 pagesize = plat_get_pagesize();
  // Align the reserve and commit sizes to the system pagesize
  reserve_size = ALIGN_UP_POW2(reserve_size, pagesize);
  commit_size = ALIGN_UP_POW2(commit_size, pagesize);

  void* reserved_ptr = NULL;
  ArenaError err = plat_mem_reserve(reserve_size, &reserved_ptr);
  if (err != ARENA_SUCCESS) {
    _arena_last_error = err;
    return err;
  }

  if (!plat_mem_commit(reserved_ptr, commit_size)) {
    plat_mem_release(reserved_ptr, reserve_size);
    _arena_last_error = ARENA_ERROR_COMMIT_FAILED;
    return ARENA_ERROR_COMMIT_FAILED;
  }

  mem_arena* arena = (mem_arena*)reserved_ptr;
  arena->magic = ARENA_MAGIC;
  arena->reserve_size = reserve_size;
  arena->commit_size = commit_size;
  arena->pos = ARENA_BASE_POS;
  arena->commit_pos = commit_size;
  arena->max_pos = ARENA_BASE_POS;

  *out = arena;
  _arena_last_error = ARENA_SUCCESS;
  return ARENA_SUCCESS;
}

// Wrapper to release the memmory from the OS
void arena_destroy(mem_arena** out) {
  if (out && *out && (*out)->magic == ARENA_MAGIC) {
    mem_arena* arena = *out;
    arena->magic = MAGIC_FREE;
    plat_mem_release(arena, arena->reserve_size);
    *out = NULL;
  }
}

// Pushes bytes of memory to the arena and returns a pointer to the start of the
// block
void* arena_push(mem_arena* arena, u64 size, b32 non_zero) {
  if (!arena || arena->magic != ARENA_MAGIC) {
    _arena_last_error = ARENA_ERROR_INVALID_PARAM;
    return NULL;
  }

  // Align
  u64 pos_aligned = ALIGN_UP_POW2(arena->pos, ARENA_ALIGN);
  u64 new_pos = pos_aligned + size;

  // Error if we run out or reserve memory
  if (new_pos > arena->reserve_size) {
    _arena_last_error = ARENA_ERROR_OUT_OF_MEMORY;
    return NULL;
  }

  // If we ran out of commited memory we ask for more
  if (new_pos > arena->commit_pos) {
    u64 new_commit_pos = new_pos;
    new_commit_pos += arena->commit_size - 1;
    new_commit_pos -= new_commit_pos % arena->commit_size;
    new_commit_pos = MIN(new_commit_pos, arena->reserve_size);

    u8* mem = (u8*)arena + arena->commit_pos;
    u64 commit_size = new_commit_pos - arena->commit_pos;

    if (!plat_mem_commit(mem, commit_size)) {
      _arena_last_error = ARENA_ERROR_COMMIT_FAILED;
      return NULL;
    }

    arena->commit_pos = new_commit_pos;
  }

  void* result = (u8*)arena + pos_aligned;

  if (!non_zero) {
    // Optimization: OS-committed memory is zeroed.
    // We only need to zero the portion that was previously touched (dirty).
    if (pos_aligned < arena->max_pos) {
      u64 zero_size = MIN(size, arena->max_pos - pos_aligned);
      memset(result, 0, zero_size);
    }
  }

  // Update position of commited (and occupied) memory
  arena->pos = new_pos;
  if (new_pos > arena->max_pos) {
    arena->max_pos = new_pos;
  }

  _arena_last_error = ARENA_SUCCESS;
  return result;
}

// Moves back the position of the arena by given size so we can "reallocate"
// But doesnt decommit it
void arena_pop(mem_arena* arena, u64 size) {
  size = MIN(size, arena->pos - ARENA_BASE_POS);
  arena->pos -= size;
}

// Pops to a given position on the arena
void arena_pop_to(mem_arena* arena, u64 pos) {
  u64 size = pos < arena->pos ? arena->pos - pos : 0;
  arena_pop(arena, size);
}

// Clears the arena to the start
void arena_clear(mem_arena* arena) {
  arena_pop_to(arena, ARENA_BASE_POS);
}

// Decommits all memory beyond the current position
void arena_decommit(mem_arena* arena) {
  if (arena && arena->magic == ARENA_MAGIC) {
    u32 pagesize = plat_get_pagesize();
    u64 pos_aligned = ALIGN_UP_POW2(arena->pos, pagesize);
    if (pos_aligned < arena->commit_pos) {
      void* ptr = (u8*)arena + pos_aligned;
      u64 size = arena->commit_pos - pos_aligned;
      if (plat_mem_decommit(ptr, size)) {
        arena->commit_pos = pos_aligned;
        arena->max_pos = MIN(arena->max_pos, pos_aligned);
      }
    }
  }
}

// Starts at the current posision and saves it to rewind to it later
mem_arena_temp arena_temp_begin(mem_arena* arena) {
  return (mem_arena_temp){.arena = arena, .start_pos = arena->pos};
}
// Rewinds to saved start position
void arena_temp_end(mem_arena_temp temp) {
  arena_pop_to(temp.arena, temp.start_pos);
}

// Uses arena not used by the caller
ArenaError arena_scratch_get(mem_arena** conflicts, u32 num_conflicts,
                             mem_arena_temp* arena) {
  if (!arena) {
    _arena_last_error = ARENA_ERROR_INVALID_PARAM;
    return ARENA_ERROR_INVALID_PARAM;
  }

  i32 scratch_index = -1;

  // Loop through scratch arenas
  for (i32 i = 0; i < ARENA_SCRATCH_COUNT; i++) {
    b32 conflict_found = false;

    // Loop through currently used arenas
    for (u32 j = 0; j < num_conflicts; j++) {
      // They reference the same arena go to the next scratch
      if (_scratch_arenas[i] == conflicts[j]) {
        conflict_found = true;
        break;
      }
    }

    // No conflict use the scratch arena
    if (!conflict_found) {
      scratch_index = i;
      break;
    }
  }

  // No scratch arena available
  if (scratch_index == -1) {
    _arena_last_error = ARENA_ERROR_NO_SCRATCH_AVAILABLE;
    return ARENA_ERROR_NO_SCRATCH_AVAILABLE;
  }

  mem_arena** selected = &_scratch_arenas[scratch_index];

  // Creates new arena if it doesnt exist yet
  if (*selected == NULL) {
    ArenaError err = arena_create(MiB(64), MiB(1), selected);
    if (err != ARENA_SUCCESS) {
      _arena_last_error = err;
      return err;
    }
  }

  // Returns pointer to the new temporary arena
  *arena = arena_temp_begin(*selected);

  _arena_last_error = ARENA_SUCCESS;
  return ARENA_SUCCESS;
}

// Releases arena
void arena_scratch_release(mem_arena_temp scratch) {
  arena_temp_end(scratch);
}

#if defined(__linux__)

#include <sys/mman.h>
#include <unistd.h>

// Returns the pagesize of memory (usually 4KiB)
static u32 plat_get_pagesize(void) {
  return (u32)sysconf(_SC_PAGESIZE);
}

// Has the platform reserve given size of memory
static ArenaError plat_mem_reserve(u64 size, void** out) {
  *out = mmap(NULL, size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (*out == MAP_FAILED) {
    return ARENA_ERROR_RESERVE_FAILED;
  }
  return ARENA_SUCCESS;
}

// Changes protection flags of blocks of memory reserved with mmap so that we
// can read and write to them
static b32 plat_mem_commit(void* ptr, u64 size) {
  i32 ret = mprotect(ptr, size, PROT_READ | PROT_WRITE);
  return ret == 0;
}

// decommits memory
static b32 plat_mem_decommit(void* ptr, u64 size) {
  i32 ret = mprotect(ptr, size, PROT_NONE);
  if (ret != 0)
    return false;
  ret = madvise(ptr, size, MADV_DONTNEED);
  return ret == 0;
}

// Releases memory from give pointer up to size
static b32 plat_mem_release(void* ptr, u64 size) {
  i32 ret = munmap(ptr, size);
  return ret == 0;
}

#endif
