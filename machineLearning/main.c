#include <arena.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
  mem_arena* arena = arena_create(MiB(10), MiB(1));
  return EXIT_SUCCESS;
}
