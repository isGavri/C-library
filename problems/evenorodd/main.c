#include <stdbool.h>
#include <stdio.h>

bool bitwiseIsEven(int n) {
  // and operand if n = 8 aka 1000 then
  // 1000 & 0001 == 0000 aka 0
  // if n = 7 aka 0111 then
  // 0111 & 0001 == 0001 aka 1
  // All even numbers finish on 0
  if ((n & 1) == 0) {
    return true;
  }
  return false;
}

bool isEven(int n) { return n % 2 == 0; }

int main() {
  int n;
  scanf("%d", &n);
  if (isEven(n)) {
    printf("true\n");
  } else {
    printf("false\n");
  }
  return 0;
}
