#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    printf("Usage: sleep NUMBER\nPause for NUMBER seconds\n");
    return 1;
  }

  char *raw_seconds = argv[1];

  // Convert characters to long
  long seconds = atol(raw_seconds);

  printf("Sleeping for %ld seconds\n", seconds);
  // Sleeps given seconds
  sleep(seconds);

  return 0;
}
