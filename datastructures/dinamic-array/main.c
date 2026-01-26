#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct
{
  float *items;
  size_t count;
  size_t capacity;
} Prices;

typedef struct
{
  int *items;
  size_t count;
  size_t capacity;
} Numbers;

typedef struct
{
  char **items;
  size_t count;
  size_t capacity;
} Names;

#define append(xs, x)                                                \
  do                                                                 \
  {                                                                  \
    if (xs.count >= xs.capacity)                                     \
    {                                                                \
      if (xs.capacity == 0)                                          \
        xs.capacity = 256;                                           \
      else                                                           \
        xs.capacity *= 2;                                            \
      xs.items = realloc(xs.items, xs.capacity * sizeof(*xs.items)); \
    }                                                                \
    xs.items[xs.count++] = x;                                        \
  } while (0)

int main()
{
  Numbers xs = {0};

  Prices p = {0};

  Names n = {0};

  for (int x = 0; x < 10; ++x) append(xs, x);
  for (int x = 0; x < 10; ++x) append(p, x);
  for (char x = 0; x < 10; ++x)
  {
    char *name = malloc(20);
    fgets(name, 20, stdin);
    append(n, name);
  }

  for (size_t i = 0; i < xs.count; ++i) printf("%d\n", xs.items[i]);

  for (size_t i = 0; i < p.count; ++i) printf("%f\n", p.items[i]);

  for (size_t i = 0; i < n.count; ++i) printf("%s\n", n.items[i]);

  return 0;
}
