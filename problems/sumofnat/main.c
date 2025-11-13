// We could use recursion but the most effective way is loop, recursive takes n
// space complexity because of stack calls
#include <stdio.h>

int sum(int n) {
  int sum = 0;

  for (int i = 1; i <= n; i++) {
    sum = sum + 1;
  }
  return sum;
}

int main() {
  int n = 5;
  printf("%d\n", sum(n));
}
