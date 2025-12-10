

#include <stdio.h>
int sumOfDigits(int n)
{
  int sum = 0;
  while (n != 0)
  {
    int last = n % 10;
    printf("%d\n", last);

    sum += last;

    n /= 10;
  }

  return sum;
}

int main()
{
  int n = 12345;
  printf("%d\n", sumOfDigits(n));
}
