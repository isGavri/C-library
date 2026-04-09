#include "rand.c"
#include <stdio.h>
#include <unistd.h>

int main(int argc, char* argv[]) {

  u64 state[2] = {0};
  prng_state rng = {};
  getentropy(state, sizeof(state));

  prng_seed_r(&rng, state[0], state[1]);

  for (u32 i = 0; i < 10; i++) {
    printf("%d: %f\n", i, prng_rand_norm_r(&rng));
  }
  return 0;
}
