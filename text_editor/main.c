/*** includes ***/

#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include <asm-generic/errno-base.h>
#include <errno.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

/*** defines ***/

#define NOTSY_VERSION "0.0.1"
#define KILO_TAB_STOP 4

// Replicate behaviour of ctrl key on terminal
// bitwise AND operation with 0x1f aka 00011111
// So basically sets the first 3 bits of any key to 0
// so with a whose ascii code is 97 aka 0110 0001 it will end up being 0000 0001
// aka 1, exactly the code that CTRL-a sends to terminal
#define CTRL_KEY(k) ((k) & 0x1f) // 0x1f -> 00011111

enum editorKey {
  BACKSPACE = 127,
  ARROW_LEFT = 1000,
  ARROW_RIGHT,
  ARROW_UP,
  ARROW_DOWN,
  DEL_KEY,
  HOME_KEY,
  END_KEY,
  PAGE_UP,
  PAGE_DOWN
};

/*** data ***/

// Row (line) of text in the editor
typedef struct erow {
  int size;     // length of chars excluding null terminator
  int rsize;    // length of rendered text (render)
  char *chars;  // pointer to the raw text of the line (file content)
  char *render; // pointer to the rendered version of line (tabs expanded)

} erow;

// Global state of the editor - E
struct editorConfig {
  int cx, cy;            // cursor position
  int rx;                // rendered coordinate
  int rowoff;            // offset
  int coloff;            // offset
  int screenrows;        // number of rows in the terminal
  int screencols;        // number of columns on the terminal
  int numrows;           // row of the file
  erow *row;             // array of erow 0 - lines of file
  char *filename;        // name of the opened file
  char statusmsg[80];    // buffer for status message
  time_t statusmsg_time; // timestamp to when the statumsg was set
  struct termios
      orig_termios; // original terminal attributes for restoration on exit
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
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1)
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
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
    die("tcgetattr");
}

// Waits fo keypress and return it and hanldes escape sequences
int editorReadKey() {
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

  if (c == '\x1b') {
    char seq[3];
    if (read(STDIN_FILENO, &seq[0], 1) != 1)
      return '\x1b';
    if (read(STDIN_FILENO, &seq[1], 1) != 1)
      return '\x1b';

    if (seq[0] == '[') {
      if (seq[1] >= '0' && seq[1] <= '9') {
        if (read(STDIN_FILENO, &seq[2], 1) != 1)
          return '\x1b';
        if (seq[2] == '~') {
          switch (seq[1]) {
          case '1':
            return HOME_KEY;
          case '3':
            return DEL_KEY;
          case '4':
            return END_KEY;
          case '5':
            return PAGE_UP;
          case '6':
            return PAGE_DOWN;
          case '7':
            return HOME_KEY;
          case '8':
            return END_KEY;
          }
        }
      } else {
        switch (seq[1]) {
        case 'A':
          return ARROW_UP;
        case 'B':
          return ARROW_DOWN;
        case 'C':
          return ARROW_RIGHT;
        case 'D':
          return ARROW_LEFT;
        case 'H':
          return HOME_KEY;
        case 'F':
          return END_KEY;
        }
      }
    } else if (seq[0] == 'O') {
      switch (seq[1]) {
      case 'H':
        return HOME_KEY;
      case 'F':
        return END_KEY;
      }
    }

    return '\x1b'; // we return escape character
  } else {
    return c;
  }
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

/*** row operations ***/

int editorRowCxToRx(erow *row, int cx) {
  int rx = 0;
  int j;
  for (j = 0; j < cx; j++) {
    if (row->chars[j] == '\t') {
      rx += (KILO_TAB_STOP - 1) - (rx % KILO_TAB_STOP);
    }
    rx++;
  }
  return rx;
}

void editorUpdateRow(erow *row) {

  int tabs = 0;
  int j;

  for (j = 0; j < row->size; j++) {
    if (row->chars[j] == '\t')
      tabs++;
  }

  free(row->render);
  row->render = malloc(row->size + tabs * (KILO_TAB_STOP - 1) + 1);

  int idx = 0;

  for (j = 0; j < row->size; j++) {
    if (row->chars[j] == '\t') {
      row->render[idx++] = ' ';
      while (idx % KILO_TAB_STOP != 0)
        row->render[idx++] = ' ';
    } else {
      row->render[idx++] = row->chars[j];
    }
  }

  row->render[idx] = '\0';
  row->rsize = idx;
}

void editorAppendRow(char *s, size_t len) {
  E.row = realloc(E.row, sizeof(erow) * (E.numrows + 1));

  int at = E.numrows;
  E.row[at].size = len;
  E.row[at].chars = malloc(len + 1);
  memcpy(E.row[at].chars, s, len);
  E.row[at].chars[len] = '\0';

  E.row[at].rsize = 0;
  E.row[at].render = NULL;

  editorUpdateRow(&E.row[at]);

  E.numrows++;
}
void editorRowInsertChar(erow *row, int at, int c) {
  if (at < 0 || at > row->size)
    at = row->size;
  row->chars = realloc(row->chars, row->size + 2);
  memmove(&row->chars[at + 1], &row->chars[at], row->size - at + 1);
  row->size++;
  row->chars[at] = c;
  editorUpdateRow(row);
}

/*** editor operations ***/
void editorInsertChar(int c) {
  if (E.cy == E.numrows) {
    editorAppendRow("", 0);
  }
  editorRowInsertChar(&E.row[E.cy], E.cx, c);
  E.cx++;
}

/*** file i/o ***/

void editorOpen(char *filename) {
  // Open file read mode
  free(E.filename);
  E.filename = strdup(filename);

  FILE *fp = fopen(filename, "r");
  if (!fp)
    die("fopen");

  char *line = NULL;
  size_t linecap = 0;
  ssize_t linelen;

  // While getting a line from the file and storing length and content doesnt
  // fail
  while ((linelen = getline(&line, &linecap, fp)) != -1) {
    // While it has text, it has a \n or \r at the end
    while (linelen > 0 &&
           (line[linelen - 1] == '\n' || line[linelen - 1] == '\r'))
      linelen--;
    // Append string without the \n or \r
    editorAppendRow(line, linelen);
  }
  free(line);
  fclose(fp);
}

/*** append buffer ***/

// Append buffer for efficient screen rendering
struct abuf {
  char *b; // Pointer to the dynamically allocated buffer
  int len; // length of the string in the buffer
};

#define ABUF_INIT {NULL, 0}

// Function to append into a string
// Takes an append buffer (pointer to string and length)
// The new string to be appended
// The size of the strings to be appended
void abApppend(struct abuf *ab, const char *s, int len) {
  // We reallocate space for the new string plus the previous
  char *new = realloc(
      ab->b, ab->len + len); // Returns a new pointer to available memory

  // If the reallocation fails, finish the function
  if (new == NULL)
    return;

  // We copy the passed string into after the old string (on the new memory
  // region)
  memcpy(&new[ab->len], s, len);
  // Update the pointer in the struct
  ab->b = new;
  // Update the length of the string
  ab->len += len;
}

void abFree(struct abuf *ab) { free(ab->b); }

/*** output ***/

void editorScroll() {
  E.rx = 0;
  if (E.cy < E.numrows) {
    E.rx = editorRowCxToRx(&E.row[E.cy], E.cx);
  }

  // Checks if its above visible window
  if (E.cy < E.rowoff) {
    E.rowoff = E.cy;
  }

  // Checks if the cursor is past bottom visible window
  if (E.cy >= E.rowoff + E.screenrows) {
    E.rowoff = E.cy - E.screenrows + 1;
  }
  // So it doesnt go behind the visible window
  if (E.rx < E.coloff) {
    E.coloff = E.rx;
  }
  // So it doesnt go past visible window
  if (E.rx >= E.coloff + E.screencols) {
    E.coloff = E.rx - E.screencols + 1;
  }
}

void editorDrawRows(struct abuf *ab) {
  int y;

  // For every available terminal emulator rows
  for (y = 0; y < E.screenrows; y++) {

    int filerow = y + E.rowoff;

    // If the file has more rows than the editor
    if (filerow >= E.numrows) {

      // When it gets to the top center of the screen and we havent printed
      // anything to editor buffer
      if (E.numrows == 0 && y == E.screenrows / 3) {

        char welcome[80];
        int welcomelen = snprintf(welcome, sizeof(welcome),
                                  "Notsy editor -- version %s", NOTSY_VERSION);

        // Reduce the size of message buffer if the columns size is lesser than
        // that
        if (welcomelen > E.screencols)
          welcomelen = E.screencols;

        // Horizontal padding, it has to be centered
        int padding = (E.screencols - welcomelen) / 2;

        // Diferent than cero (not centered)
        if (padding) {

          // We draw ~ and pass to the next column
          abApppend(ab, "~", 1);
          padding--;
        }

        // Until cero, centered, keep printing spaces
        while (padding--)
          abApppend(ab, " ", 1);

        // Print the editor message
        abApppend(ab, welcome, welcomelen);

        // Either before of after top third of the screen
      } else {

        abApppend(ab, "~", 1); // Updates buffer appending `~`
      }
    } else {

      // Size of row
      int len = E.row[filerow].rsize - E.coloff;
      if (len < 0)
        len = 0;

      // Truncate in case of exceding size of screen
      if (len > E.screencols)
        len = E.screencols;

      // Append to append buffer
      abApppend(ab, &E.row[filerow].render[E.coloff], len);
    }

    abApppend(ab, "\x1b[K", 3); // Clears the line from cursor to end

    abApppend(ab, "\r\n", 2);
  }
}

void editorDrawStatusBar(struct abuf *ab) {
  abApppend(ab, "\x1b[7m", 4);
  char status[80], rstatus[80];
  int len = snprintf(status, sizeof(status), "%.20s - %d lines",
                     E.filename ? E.filename : "[No Name]", E.numrows);
  int rlen = snprintf(rstatus, sizeof(rstatus), "%d/%d", E.cy + 1, E.numrows);
  if (len > E.screencols)
    len = E.screencols;
  abApppend(ab, status, len);
  while (len < E.screencols) {
    if (E.screencols - len == rlen) {
      abApppend(ab, rstatus, rlen);
      break;
    } else {
      abApppend(ab, " ", 1);
      len++;
    }
  }
  abApppend(ab, "\x1b[m", 3);
  abApppend(ab, "\r\n", 2);
}

void editorDrawMessageBar(struct abuf *ab) {
  abApppend(ab, "\x1b[K", 3);
  int msglen = strlen(E.statusmsg);
  if (msglen > E.screencols)
    msglen = E.screencols;
  if (msglen && time(NULL) - E.statusmsg_time < 5)
    abApppend(ab, E.statusmsg, msglen);
}

// Writes escape sequence for clearing terminal
void editorRefreshScreen() {
  editorScroll();

  struct abuf ab = ABUF_INIT; // Intializes append buffer

  // Hide the cursor
  abApppend(&ab, "\x1b[?25l", 6);
  // \x1b is 27 in decimal which is the escape character (first of the bytes
  // that we write) [2j are the rest Escape sequences stat with the escape
  // character followed by [ and then the command We are using J command (Erase
  // In Display) with the argument 2 wich clears the whole screen
  // [0j clears from the cursor on
  // [1j clears up to the cursor
  // abApppend(&ab, "\x1b[2J", 4); // Deleted, now we just clear to the end of
  // the screen

  // Reposition de cursor
  // Three bytes long, 27,[,H
  // H command takes two arguments, row and column. Separated by ;
  // Row and columns start at 1. Which is also the default
  abApppend(&ab, "\x1b[H", 3);

  editorDrawRows(&ab);
  editorDrawStatusBar(&ab);
  editorDrawMessageBar(&ab);

  char buf[32];

  snprintf(buf, sizeof(buf), "\x1b[%d;%dH", (E.cy - E.rowoff) + 1,
           (E.rx - E.coloff) + 1); // Moves cursor to the current coordinates
  abApppend(&ab, buf, strlen(buf));

  // Show the cursor
  abApppend(&ab, "\x1b[?25h", 6);

  write(STDOUT_FILENO, ab.b, ab.len); // Writes the lines to the buffer
  abFree(&ab);
}

void editorSetStatusMessage(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(E.statusmsg, sizeof(E.statusmsg), fmt, ap);
  va_end(ap);
  E.statusmsg_time = time(NULL);
}

/*** input ***/

void editorMoveCursor(int key) {
  erow *row = (E.cy >= E.numrows) ? NULL : &E.row[E.cy];
  switch (key) {
    // Will later modify to use vim motions
  case ARROW_LEFT:
    if (E.cx != 0) {
      E.cx--;
    } else if (E.cy > 0) {
      E.cy--;
      E.cx = E.row[E.cy].size;
    }
    break;
  case ARROW_RIGHT:
    if (row && E.cx < row->size) {
      E.cx++;
    } else if (row && E.cx == row->size) {
      E.cy++;
      E.cx = 0;
    }
    break;
  case ARROW_UP:
    if (E.cy != 0) {
      E.cy--;
    }
    break;
  case ARROW_DOWN:
    if (E.cy < E.numrows) {
      E.cy++;
    }
    break;
  }
  row = (E.cy >= E.numrows) ? NULL : &E.row[E.cy];
  int rowlen = row ? row->size : 0;
  if (E.cx > rowlen) {
    E.cx = rowlen;
  }
}

// waits for keypress and then handles it.
void editorProcessKeypress() {
  int c = editorReadKey();
  // When ctrl + q is pressed exit the program with  status 0
  switch (c) {
  case '\r':
    /* TODO: */
    break;
  case CTRL_KEY('q'):
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);
    exit(0);
    break;

    /** movement **/

  case HOME_KEY:
    E.cx = 0;
    break;
  case END_KEY:
    if (E.cy < E.numrows)
      E.cx = E.row[E.cy].size;
    break;
  case BACKSPACE:
  case CTRL_KEY('h'):
  case DEL_KEY:
    /* TODO: */
    break;
  case PAGE_UP:
  case PAGE_DOWN: {
    if (c == PAGE_UP) {
      E.cy = E.rowoff;
    } else if (c == PAGE_DOWN) {
      E.cy = E.rowoff + E.screenrows - 1;
      if (E.cy > E.numrows)
        E.cy = E.numrows;
    }
    int times = E.screenrows;
    while (times--)
      editorMoveCursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
  } break;

  case ARROW_UP:
  case ARROW_DOWN:
  case ARROW_LEFT:
  case ARROW_RIGHT:
    editorMoveCursor(c);
    break;
  case CTRL_KEY('l'):
  case '\x1b':
    break;
  default:
    editorInsertChar(c);
    break;
  }
}

/*** init ***/

void initEditor() {
  E.cx = 0;
  E.cy = 0;
  E.rx = 0;
  E.rowoff = 0;
  E.coloff = 0;
  E.numrows = 0;
  E.row = NULL;
  E.filename = NULL;
  E.statusmsg[0] = '\0';
  E.statusmsg_time = 0;
  if (getWindowSize(&E.screenrows, &E.screencols) == -1)
    die("getWindowSize");
  E.screenrows -= 2;
}

int main(int argc, char *argv[]) {
  enableRawMode();
  // Initializes the E struct (aka global configuration)
  initEditor();
  if (argc >= 2) {
    editorOpen(argv[1]);
  }

  editorSetStatusMessage("HELP: Ctrl-Q = quit");

  // Reads 1 byte from file descriptor STDIN_FILENO aka 0 aka standard input on
  // Unix, into c (buffer) When it reads 'q' exits the program.
  // NOTE: Leftover characters after q will be fed into the shell
  while (1) {
    // Clears screen
    editorRefreshScreen();
    editorProcessKeypress();
  }

  return 0;
}
