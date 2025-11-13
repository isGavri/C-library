// swap using bitwise operators or sum and sub

#include <stdio.h>
void swapSumSub(int *a, int *b) {
  // example: a = 5, b = 7
  // a = 5 + 7 = 12
  // b = 12 - 7 = 5
  // a = 12 - 5 = 7
  // ends as a = 7, b = 5
  *a = *a + *b;
  *b = *a - *b;
  *a = *a - *b;
}

void swapXor(int *a, int *b) {
  // example a = 5, b = 7 so 0101 and 0111
  // a = 0101 ^ 0111 = 0010 - 2
  // b = 0010 ^ 0111 = 0101 - 5
  // a = 0010 ^ 0101 = 0111 - 7
  // ends as a = 7, b = 5
  *a = *a ^ *b;
  *b = *a ^ *b;
  *a = *a ^ *b;
}

int main() {
  int a = 2, b = 3;

  // Original
  printf("a = %d, b = %d\n", a, b);

  swapSumSub(&a, &b);

  // Swaped
  printf("a = %d, b = %d\n", a, b);

  swapXor(&a, &b);

  // Swaped to original again
  printf("a = %d, b = %d\n", a, b);

  return 0;
  ;
}
