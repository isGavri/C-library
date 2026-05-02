#include <bits/time.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define u32 uint32_t

// 8 mill
#define SIZE (1024 * 1024 * 8)
#define MiB(n) ((u32)(n) << 20)
#define KiB(n) ((u32)(n) << 10)

int main(int argc, char* argv[]) {
  u32* arr = calloc(SIZE, sizeof(u32));
  u32 steps = SIZE * 4;
  u32 lengthMod = SIZE - 1;
  //
  u32 accessCount = 0;
  u32 limit = KiB(32) / 4;
  struct timespec start, end;
  int inner_steps = 1000;
  for (u32 i = 0; i < steps; i++) {
    u32 index = (i * 16) & lengthMod;
    u32 offset = index * 4;

    if (accessCount < 10 && offset >= limit) {
      clock_gettime(CLOCK_MONOTONIC, &start);
      arr[index]++;
      for (u32 j = 0; j < inner_steps; j++) {
        // Use a pointer-chase or random stride to break prefetcher
        // For now, let's just do the linear one to see the shift
        arr[((i + j) * 16) & lengthMod]++;
      }

      clock_gettime(CLOCK_MONOTONIC, &end);

      long total_ns = end.tv_nsec - start.tv_nsec;
      double avg_ns = (double)total_ns / inner_steps;

      printf("Access %d: Offset %7d bytes, Avg Time: %.2f ns\n", accessCount,
             offset, avg_ns);

      accessCount++;
      if (accessCount == 10) {
        if (limit == KiB(32) / 4) {
          limit = KiB(512) / 4;
          accessCount = 0;
        } else if (limit == KiB(512) / 4) {
          limit = MiB(8);
          accessCount = 0;
        }
      }
      continue;
    }

    arr[index]++;
  }
  return EXIT_SUCCESS;
}
