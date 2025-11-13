// Best approach is  using mathematical formula
// space and time complexity of 1
// n(n+1)(2n+1) / 6

#include <stdio.h>
int sum(int n) {
  // return (n * (n + 1) * (2 * n + 1)) / 6; can overflow
  return (n * (n + 1) / 2) * (2 * n + 1) /
         3; // its also know and valid as a formula (the exact same), also can
            // overflow but can calculate larger values than the other one than
            // the other one than the other one than the other one than the
            // other one than the other one than the other one than the other
            // one than the other one
}

int main() {
  int n = 10;
  printf("%d", sum(n));
  return 0;
}
