#define SYS_write 1
#define SYS_nanosleep 35
#define SYS_exit 60

struct timespec {
  long tv_sec;
  long tv_nsec;
} timespec;

long syscall1(long number, long arg1)
{
  long result;

  __asm__ __volatile__("syscall\n"
                       : "=a"(result)
                       : "a"(number), "D"(arg1)
                       : "rcx", "r11", "memory");

  return result;
}

long syscall2(long number, long arg1, long arg2)
{
  long result;

  __asm__ __volatile__(
    "syscall"
    : "=a" (result)
    : "a" (number), "D" (arg1), "S" (arg2)
    : "rcx", "r11", "memory"
  );

  return result;
}


long syscall3(long number, long arg1, long arg2, long arg3)
{
  long result;

  __asm__ __volatile__("syscall"
                       : "=a"(result)
                       : "a"(number), "D"(arg1), "S"(arg2), "d"(arg3)
                       : "rcx", "r11", "memory");
  return result;
}

/*
 * Parses a raw string into an integer
 */
int parse_int(char *raw_int)
{
  int result;

  // Traverse the string
  char *cursor = raw_int;

  // For every numerical character
  while (*cursor >= '0' && *cursor <= '9')
  {
    // We make the previously stored result times 10
    // We add the new parsed number (ASCII value - ASCII value of 0 (0x30 | 48))
    // "432" - 0 * 10 + (52 - 48) = 4
    // "32" - 4 * 10 + (51 - 48) = 43
    // "2" - 43 * 10 + (50 - 48) = 432
    result = result * 10 + (*cursor - '0');
    cursor++;
  }
  return result;
}

long unsigned strlen(char *string)
{
  char *cursor = string;

  while (*cursor)
  {
    cursor++;
  }

  long unsigned result = cursor - string;

  return result;
}


void sleep(long seconds)
{
  struct timespec duration = {0};
  duration.tv_sec = seconds;
  syscall2(SYS_nanosleep, (long)(&duration), 0);
}

void print(char *str)
{
  syscall3(SYS_write, 1, (long)str, strlen(str));
}

int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    print("Usage: sleep NUMBER\nPause for NUMBER seconds\n");
    return 1;
  }

  char *raw_seconds = argv[1];

  // Convert characters to long
  long seconds = parse_int(raw_seconds);

  print("Sleeping for ");
  print(raw_seconds);
  print(" seconds\n");
  // Sleeps given seconds
  sleep(seconds);

  return 0;
}


void exit(long code)
{
  syscall1(SYS_exit, code);

  for (;;)
  {
  }
}

// Linker search for this function
__attribute__((naked)) void _start()
{
  __asm__ __volatile__(
      "xor %ebp, %ebp\n"     // clean ebp
      "mov (%rsp), %rdi\n"   // move value of rsp into rdi
      "lea 8(%rsp), %rsi\n"  // load address of 8 byte offset of rsp to rsi
      "and $-16, %rsp\n"     // loads immediate $-16 to rsp
      "call main\n"          // calls main function
      "mov %rax, %rdi\n"     // sets exit code
      "call exit\n");
}
