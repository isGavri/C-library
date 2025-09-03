#include <asm-generic/errno-base.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

/*** data ***/

struct termios orig_termios;

/*** terminal ***/

void die(const char *s) {
  // Looks at the errno and prints the string
  perror(s);
  // Exit program with return of 1 - failure
  exit(1);
}

void disableRawMode() {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == 1)
    die("tcseattr");
}

void enableRawMode() {
  // Places the attributes from standard input as a struct on orig_termios
  if (tcgetattr(STDIN_FILENO, &orig_termios) == -1)
    die("tcgetattr");

  // Calls disableRawMode when the program exits
  atexit(disableRawMode);

  // Struct for our custom flags.
  struct termios raw = orig_termios;

  // Disable input flags
  raw.c_iflag &=
      ~(BRKINT | ICRNL | INPCK | ISTRIP |
        IXON); // IXON Disables sofware flow control signals XON and XOFF
  // BRKINT break signals are sent to the program
  // ISTRIP causes the 8th bit of each input byte to be stripped. Probably
  // already off ICRNL Input carriage return new line. Stops any carriage return
  // from converting on a new line

  // Disables output flags
  raw.c_oflag &= ~(OPOST); // Output processing

  // Sets the character size to 8 bits per byte
  // Now we are setting a flag instead of unsetting
  raw.c_cflag |= (CS8);

  // Local flags c_lflag on the raw termios struct
  // Expands to o0000010 aka 8 which is the value flag for the echo feature.
  // This feature causes each  key to be printed on the terminal.
  // An example of when this is used is then typing a password on the terminal
  // ~ bitwise NOT operator which inverts all the bits it passes from 00001000
  // to 11110111 (must be noted that we aren't using the whole 32 bit integer)
  // finally bitwise AND operator which is performed between original c_lflag
  // and our inverted flag to then be stored as the new c_lflag
  raw.c_lflag &=
      ~(ECHO | ICANON | IEXTEN |
        ISIG); // Also turns off canonical mode which means now we read input
               // byte by byte not line by line. And ISIG which disables signals
               // (not ctrl-s and ctrl-q) IEXTEN disables ctrl-v

  // Indexes from c_cc field that stand for control characters
  // VMIN minimum number of bytes of input needed before read can retrn
  // VTIME maximun amount of time to wait befor read() 1/10 of a second or 100
  // miliseconds.
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;

  // Sets the termios attributes from raw
  // TCSAFLUSH waits for all pending output to be written to terminal. And
  // discards any input that hasn't been read. This will prevent leftover input
  // to be fed into your shell
  if ( tcsetattr(STDERR_FILENO, TCSAFLUSH, &raw) == -1 ) die("tcgetattr");
}

/*** init ***/

int main() {
  enableRawMode();

  // Reads 1 byte from file descriptor STDIN_FILENO aka 0 aka standard input on
  // Unix, into c (buffer) When it reads 'q' exits the program. NOTE: Leftover
  // characters after q will be fed into the shell
  while (1) {
    char c = '\0';
    if(read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN) die("read");
    // Tests whether a character is a control character. From 0-31 and 127
    if (iscntrl(c)) {
      // Prints character ascii code if it is a control sequence and then starts
      // a new line
      printf("%d\r\n", c);
    } else {
      // Prints ascii code and character if its printable character and then
      // starts a new line
      printf("%d ('%c')\r\n", c, c);
    }
    if (c == 'q')
      break;
  }

  return 0;
}
