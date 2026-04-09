// Makes uses Permuted Congruential Generation
// algorithms used to automatically create content according to rules
// this time generate random numbers
// Unlike RNG this are harder to predict
#include <math.h>
#include <stdint.h>

typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;

#define PI 3.14159265358979

// Based o n pcg random n umber generator (https://pcg-random.org)
// Licensed under the Apache License, Version 2.0 (the "License");
// Full license under the LICENSE.txt file

// we basically follow
// new state is the source of any new random number
// multiplier is a constant
// inc or stream :TODO
// new_state = (old_state * multipier) + inc
// we use 64 bit unsinged interger so the overflow does the modulus of 2^64

// *** Prototypes ***

// state of pseudo random number generator
typedef struct {
  u64 state;
  u64 inc;

  f32 prev_norm;
} prng_state;

// Initializing-seeding the generator
void prng_seed_r(prng_state* rng, u64 initstate, u64 initseq);
void prng_seed(u64 initstate, u64 initseq);

// Returns the random
u32 prng_rand_r(prng_state* rng);
u32 prng_rand(void);

// Random point floating number between [0-1]
f32 prng_randf_r(prng_state* rng);
f32 prng_randf(void);

// Normally distrubuted pseudo random number generation
f32 prng_rand_norm_r(prng_state* rng);
f32 prng_rand_norm(void);

// global hidden state for non reentrant functions
static __thread prng_state s_prng_state = {0x853c49e6748fea9bULL,
                                           0xda3e39cb94b95bdbULL, NAN};

// *** Seeding the generator ***

// reentrant
void prng_seed_r(prng_state* rng, u64 initstate, u64 initseq) {
  rng->state = 0ULL;
  rng->inc = (initseq << 1u) | 1u;
  prng_rand_r(rng);
  rng->state += initstate;
  prng_rand_r(rng);

  rng->prev_norm = NAN;
}

void prng_seed(u64 initstate, u64 initseq) {
  prng_seed_r(&s_prng_state, initstate, initseq);
}

// *** Random 32 bit number ***

// reentrant
u32 prng_rand_r(prng_state* rng) {
  uint64_t oldstate = rng->state;
  rng->state = oldstate * 6364136223846793005ULL + rng->inc;
  uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
  uint32_t rot = oldstate >> 59u;
  return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}
u32 prng_rand(void) {
  return prng_rand_r(&s_prng_state);
}

// *** Random [0-1] floating point number
f32 prng_randf_r(prng_state* rng) {
  return (f32)prng_rand_r(rng) / (f32)UINT32_MAX;
}

f32 prng_randf(void) {
  return prng_randf_r(&s_prng_state);
}

// Normal distrubuted  psuedo random number generation
f32 prng_rand_norm_r(prng_state* rng) {
  // if we already have one normal distributed number then we return it
  if (!isnan(rng->prev_norm)) {
    f32 out = rng->prev_norm;
    rng->prev_norm = NAN;
    return out;
  }

  // Calculate a pair, return one and store the other
  f32 u1 = 0.0f;
  // we just want to ensure one of them is different than cero and we chose u1
  // for that. u2 can still be 0
  do {
    u1 = prng_randf_r(rng);
  } while (u1 == 0.0f);

  f32 u2 = prng_rand_r(rng);

  // formula for normal distribution
  f32 mag = sqrt(-2.0f * logf(u1));
  f32 z0 = mag * cosf(2.0 * PI * u2);
  f32 z1 = mag * sinf(2.0 * PI * u2);

  // returns z0 and stores the other distrubuted number (z1)
  rng->prev_norm = z1;
  return z0;
}

f32 prng_rand_norm(void) {
  return prng_rand_norm_r(&s_prng_state);
}
