#include <stdio.h>

// Time and space complexity of 1
int nthterm(int a1, int a2, int n)
{

  return a1 + ((a2 - a1) * (n - 1));
}

int main()
{
  int a1 = 2, a2 = 3, n = 4;

  printf("%d", nthterm(a1, a2, n));
}
