#include <stdio.h>

// Recursive approach, same logic as a loop
void printTable(int n, int i) {
  if (i == 11)
    return;
  printf("%d * %d = %d\n", n, i, n * i);
  i++;
  printTable(n, i);
}

int main() {
  int n = 5;
  printTable(n, 1);
  return 0;
}
