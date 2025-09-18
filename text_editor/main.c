#include <asm-generic/errno-base.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

// Replicate behaviour of ctrl key on terminal
// bitwise AND operation with 0x1f aka 00011111
// So basically sets the first 3 bits of any key to 0
// so with a whose ascii code is 97 aka 0110 0001 it will end up being 0000 0001
// aka 1, exactly the code that CTRL-a sends to terminal
#define CTRL_KEY(k) ((k) & 0x1f) // 0x1f -> 00011111

/*** data ***/

struct editorConfig {
  int screenrows;
  int screencols;
  struct termios orig_termios;
};

struct editorConfig E;

/*** terminal ***/

void die(const char *s) {
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);

  // Looks at the errno and prints the string
  perror(s);
  // Exit program with return of 1 - failure
  exit(1);
}

void disableRawMode() {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == 1)
    die("tcseattr");
}

void enableRawMode() {
  // Places the attributes from standard input as a struct on orig_termios
  if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1)
    die("tcgetattr");

  // Calls disableRawMode when the program exits
  atexit(disableRawMode);

  // Struct for our custom flags.
  struct termios raw = E.orig_termios;

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
  if (tcsetattr(STDERR_FILENO, TCSAFLUSH, &raw) == -1)
    die("tcgetattr");
}

// Waits fo keypress and return it and hanldes escape sequences
char editorReadKey() {
  int nread;
  char c;
  // Read from standard input, into c, one byte
  while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
    // if reading fails and the error is not EAGAIN macro, exit with fail on the
    // read process
    // EAGAIN - currently unaviable might succeed if attempted again later
    if (nread == -1 && errno != EAGAIN)
      die("read");
  }
  return c;
}

int getCursorPosition(int *rows, int *cols) {

  char buf[32];
  unsigned int i = 0;

  // Query the poisition
  // n command - Device status report
  // 6 argument - Cursor position
  // Prints \x1b[nn;nnR - n = digit
  if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4)
    return -1;

  // Reads cursor position until it reaches R
  while (i < sizeof(buf) - 1) {
    if (read(STDIN_FILENO, &buf[i], 1) != 1)
      break;
    if (buf[i] == 'R')
      break;
    i++;
  }
  buf[i] = '\0';

  // check if it returned a escape sequence
  if (buf[0] != '\x1b' || buf[1] != '[')
    return -1;
  // Reads and puts the values on memory
  if (sscanf(&buf[2], "%d;%d", rows, cols) != 2)
    return -1;

  return 0;
}

int getWindowSize(int *rows, int *cols) {
  // Window size
  struct winsize ws;

  // We get the Terminal Input Output Control Window Size of the standard output
  // and set it to the struct
  // Expands 0x5413
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
    // If fails we try to position the cursor at the bottom-right of the
    // screen,then use escape sequences to query the position.
    // C - Cursor muves fordward, 999 so it reaches the bottom and B - Cursor
    // moves down
    if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12)
      return -1;

    return getCursorPosition(rows, cols);
  } else {
    // Sets the terminal columns and rows and puts them into the passed
    // configuration
    *cols = ws.ws_col;
    *rows = ws.ws_row;
    return 0;
  }
}

/*** append buffer ***/

// String that allows easy concatenation
struct abuf {
  char *b;
  int len;
};

#define ABUF_INIT {NULL, 0}

// Function to append into a string
// Takes an append buffer (pointer to string and length)
// The new string to be appended
// The size of the strings to be appended
void abApppend(struct abuf *ab, const char *s, int len) {
  // We reallocate space for the new string plus the previous
  char *new = realloc(ab->b, ab->len + len); // Returns a new pointer to available memory

  // If the reallocation fails, finish the function
  if (new == NULL)
    return;

  // We copy the passed string into after the old string (on the new memory region)
  memcpy(&new[ab->len], s, len);
  // Update the pointer in the struct
  ab->b = new;
  // Update the length of the string
  ab->len += len;
}

void abFree(struct abuf *ab){
  free(ab->b);
}

/*** output ***/

void editorDrawRows() {
  int y;
  for (y = 0; y < E.screenrows; y++) {
    write(STDOUT_FILENO, "~", 1); // Write the ~ rows
    if (y < E.screenrows - 1) {
      write(STDOUT_FILENO, "\r\n", 2); // Passes lines if we are not on the last
    }
  }
}

// Writes escape sequence for clearing terminal
void editorRefreshScreen() {
  // \x1b is 27 in decimal which is the escape character (first of the bytes
  // that we write) [2j are the rest Escape sequences stat with the escape
  // character followed by [ and then the command We are using J command (Erase
  // In Display) with the argument 2 wich clears the whole screen
  // [0j clears from the cursor on
  // [1j clears up to the cursor
  write(STDOUT_FILENO, "\x1b[2J", 4);

  // Reposition de cursor
  // Three bytes long, 27,[,H
  // H command takes two arguments, row and column. Separated by ;
  // Row and columns start at 1. Which is also the default
  write(STDOUT_FILENO, "\x1b[H", 3);

  editorDrawRows();

  // Reposition cursor
  write(STDOUT_FILENO, "\x1b[H", 3);
}

/*** input ***/

// waits for keypress and then handles it.
void editorProcessKeypress() {
  char c = editorReadKey();
  // When ctrl + q is pressed exit the program with  status 0
  switch (c) {
  case CTRL_KEY('q'):
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);
    exit(0);
    break;
  }
}

/*** init ***/

void initEditor() {
  if (getWindowSize(&E.screenrows, &E.screencols) == -1)
    die("getWindowSize");
}

int main() {
  enableRawMode();
  // Initializes the E struct (aka global configuration)
  initEditor();
  // iscntrl tests whether a character is a control character. From 0-31 and 127

  // Reads 1 byte from file descriptor STDIN_FILENO aka 0 aka standard input on
  // Unix, into c (buffer) When it reads 'q' exits the program. NOTE: Leftover
  // characters after q will be fed into the shell
  while (1) {
    // Clears screen
    editorRefreshScreen();
    editorProcessKeypress();
  }

  return 0;
}
