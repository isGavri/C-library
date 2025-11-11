# C Text Editor "Notsy"

A minimal text editor implementation in C, based on [antirez's kilo tutorial](http://antirez.com/news/108). This project extends the basic terminal setup to include file viewing, vertical scrolling, and cursor navigation. The README was mostly AI generated, will check on it later.

## Current State

This implementation has progressed beyond basic terminal I/O and now functions as a simple file viewer. It can open and display text files, handling files larger than the screen through scrolling.

### ✅ Implemented Features

- **Raw Terminal Mode**: Switches from canonical to raw mode for character-by-character input processing.
- **Terminal Configuration**: Properly saves and restores terminal attributes using `tcgetattr` and `tcsetattr`.
- **Screen Management**: Clears the screen and manages display using [VT100](https://vt100.net/docs/vt100-ug/chapter3.html) escape sequences.
- **Window Size Detection**: Automatically detects terminal dimensions on startup.
- **File I/O**: Opens and reads a specified file into a row-based memory structure.
- **Text Display**: Renders the content of the opened file on the screen.
- **Vertical Scrolling**: Allows scrolling through the file if its content exceeds the screen size (`E.rowoff`).
- **Cursor Movement**: Supports `Arrow Keys` for navigation, `Home`/`End` to jump to the start/end of a line, and `Page Up`/`Page Down` for block scrolling.
- **Append Buffer**: Uses a dynamic string buffer (`abuf`) for efficient screen rendering, minimizing `write()` calls.
- **Exit Functionality**: `Ctrl+Q` cleanly exits the program and restores the terminal state.
- **Status Bar**: Displays filename and current line position.
- **Status Messages**: Shows help text and messages to the user.
- **Horizontal Scrolling**: Lines wider than the screen can be scrolled with `E.coloff`.
- **Tab Rendering**: Tabs are rendered as spaces with configurable tab stop width.

### ❌ Not Yet Implemented

- **Text Editing**: No character insertion, deletion, or modification.
- **Saving Files**: Cannot write changes back to a file.
- **Search/Replace**: No search functionality.
- **Syntax Highlighting**: No code highlighting features.
- **Undo/Redo**: No undo or redo functionality.

## Building and Running

### Compile
```sh
make editor
```

### Run
To open a file:
```sh
./main <filename>
```
To start with an empty buffer:
```sh
./main
```

### Controls
- **Arrow Keys**: Move the cursor up, down, left, or right.
- **Page Up / Page Down**: Scroll up or down by a full screen length.
- **Home / End**: Move the cursor to the beginning or end of the current line.
- **Ctrl+Q**: Exit the editor.

## Technical Implementation

### Core Data Structures

#### `typedef struct erow`
Represents a single row (line) of text in the editor.
- **`int size`**: The length of the actual text content in `chars` (excludes null terminator).
- **`int rsize`**: The length of the rendered text in `render` (after processing tabs, etc.).
- **`char *chars`**: Pointer to the raw text content of the line (actual file content).
- **`char *render`**: Pointer to the rendered version of the line (with tabs expanded to spaces).

#### `struct editorConfig`
The global state of the editor, stored in variable `E`.
- **`int cx, cy`**: Cursor position - `cx` is the column (x-coordinate), `cy` is the row (y-coordinate) in the file.
- **`int rx`**: The horizontal coordinate in the rendered line (accounting for tabs).
- **`int rowoff`**: Vertical scroll offset - the row of the file at the top of the screen.
- **`int coloff`**: Horizontal scroll offset - the column of the file at the left edge of the screen.
- **`int screenrows`**: Number of rows available in the terminal for displaying text (excludes status bar).
- **`int screencols`**: Number of columns available in the terminal.
- **`int numrows`**: Total number of rows (lines) in the file.
- **`erow *row`**: Dynamic array of `erow` structs, one for each line in the file.
- **`char *filename`**: Name of the currently opened file (NULL if no file is open).
- **`char statusmsg[80]`**: Buffer for the status message displayed at the bottom of the screen.
- **`time_t statusmsg_time`**: Timestamp when the status message was set (for auto-clearing).
- **`struct termios orig_termios`**: Original terminal attributes, saved for restoration on exit.

#### `struct abuf`
An append buffer used for efficient screen rendering.
- **`char *b`**: Pointer to the dynamically allocated buffer containing the accumulated string.
- **`int len`**: Current length of the string in the buffer.

This structure minimizes the number of `write()` system calls by building the entire screen in memory first.

### Program Flow
1.  **Initialization (`main`, `initEditor`)**:
    - `enableRawMode()` is called to switch the terminal to raw mode.
    - `initEditor()` initializes the `E` config struct and gets the terminal window size.
    - If a filename is provided as a command-line argument, `editorOpen()` is called.
2.  **File Loading (`editorOpen`)**:
    - The file is opened in read mode (`fopen`).
    - `getline()` reads the file line by line.
    - For each line, `editorAppendRow()` allocates memory for a new `erow` struct and copies the line's content into it, storing it in `E.row`.
3.  **Main Loop (`while(1)`)**:
    - `editorRefreshScreen()`: This is the core rendering function. It clears the screen, draws all the visible rows of the file, and positions the cursor.
    - `editorProcessKeypress()`: This function waits for a keypress using `editorReadKey()` and processes it. This includes handling movement keys and the exit command (`Ctrl+Q`).
4.  **Screen Rendering (`editorRefreshScreen`, `editorDrawRows`)**:
    - An `abuf` is created to buffer the output.
    - `editorScroll()` is called to adjust `E.rowoff` if the cursor has moved outside the visible window.
    - `editorDrawRows()` iterates from `0` to `E.screenrows`. For each screen row, it calculates the corresponding file row (`y + E.rowoff`).
    - If the file row exists, its content is appended to the `abuf`. Otherwise, a tilde (`~`) is drawn (or a welcome message on an empty editor).
    - After drawing the rows, the cursor is moved to its correct position (`E.cx`, `E.cy`).
    - Finally, the entire content of the `abuf` is written to standard output.
5.  **Termination**:
    - When `Ctrl+Q` is pressed, `exit(0)` is called.
    - The `atexit(disableRawMode)` hook, set during initialization, ensures the original terminal settings are restored upon exit.

### Input Handling (`editorReadKey`)
- Input is read byte-by-byte.
- If an escape character (``) is detected, the function reads the following bytes to determine if it's a recognized escape sequence (like arrow keys, Home/End, etc.). These are mapped to a custom `enum editorKey` for clear and simple processing in `editorProcessKeypress()`.
### Key Functions

#### Terminal Setup
- **`enableRawMode()`**: Disables canonical mode and echo, enabling raw character input.
- **`disableRawMode()`**: Restores original terminal settings.
- **`die()`**: Error handler that clears the screen and exits with an error message.

#### File Operations
- **`editorOpen()`**: Opens a file and loads its content line by line into the `E.row` array.
- **`editorAppendRow()`**: Adds a new row to the editor, allocating memory and copying content.
- **`editorUpdateRow()`**: Updates the rendered version of a row (expands tabs to spaces).

#### Rendering
- **`editorScroll()`**: Adjusts `rowoff` and `coloff` to keep the cursor visible on screen.
- **`editorDrawRows()`**: Renders all visible rows of the file to the append buffer.
- **`editorDrawStatusBar()`**: Renders the status bar showing filename and position.
- **`editorDrawMessageBar()`**: Renders the message bar at the bottom of the screen.
- **`editorRefreshScreen()`**: Main rendering function that orchestrates the screen update.

#### Input Processing
- **`editorReadKey()`**: Reads a keypress and handles escape sequences.
- **`editorMoveCursor()`**: Moves the cursor based on arrow key input with boundary checking.
- **`editorProcessKeypress()`**: Main input handler that processes keypresses and executes commands.

#### Utilities
- **`editorRowCxToRx()`**: Converts cursor column position to render position (handles tabs).
- **`getWindowSize()`**: Determines the terminal window size using `ioctl` or cursor positioning.
- **`editorSetStatusMessage()`**: Sets a status message with printf-style formatting.
