# C Text Editor "Notsy"

A minimal text editor implementation in C, based on [antirez's kilo tutorial](http://antirez.com/news/108). This project extends the basic terminal setup to include file viewing, vertical scrolling, and cursor navigation.

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

### ❌ Not Yet Implemented

- **Text Editing**: No character insertion, deletion, or modification.
- **Saving Files**: Cannot write changes back to a file.
- **Horizontal Scrolling**: Lines wider than the screen are truncated.
- **Search/Replace**: No search functionality.
- **Status Bar/Messages**: No status bar to display filename, line number, etc.
- **Syntax Highlighting**: No code highlighting features.

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
- **`struct editorConfig E`**: A global struct holding the editor's state, including cursor position (`cx`, `cy`), screen dimensions (`screenrows`, `screencols`), file content (`row`, `numrows`), and the vertical scroll offset (`rowoff`).
- **`typedef struct erow`**: Represents a single line of text in the file, containing the line's content (`chars`) and its length (`size`).
- **`struct abuf`**: An "append buffer" used to build the entire screen content in memory before writing it to the terminal with a single `write()` system call. This is far more efficient than making many small `write()` calls.

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