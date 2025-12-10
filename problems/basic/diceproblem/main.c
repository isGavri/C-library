// The sum of two opposite sides of a dice is equal to 7, so we just substract n from 7
// Time and space complexity of O(1)
#include <stdio.h>

int oppositeFaceOfDice(int n)
{
  int ans = 7 - n;
  return ans;
}

int main()
{
  int n = 6;
  printf("%d", oppositeFaceOfDice(n));
}
