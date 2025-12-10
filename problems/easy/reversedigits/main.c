// We extract each digit using %, and add it


#include <stdio.h>
int reverseDigits (int n)
{
  int rev = 0;
  while (n > 0)
  {
    rev = rev  * 10 + n % 10;
    n = n / 10;
  }
  return rev;
}


int main()
{
  int n = 4562;
  printf("%d", reverseDigits(n));
  return 0;
}
