# C Text Editor

A minimal text editor implementation in C, based on [antirez's kilo tutorial](http://antirez.com/news/108).

## Current State

This is an **early-stage implementation** that currently focuses on terminal manipulation and basic input handling. The editor is not yet ready for practical text editing use.

### ✅ Implemented Features

- **Raw Terminal Mode**: Switches from canonical to raw mode for character-by-character input
- **Terminal Configuration**: Properly handles terminal attributes using `tcgetattr` and `tcsetattr`
- **Screen Management**: Clears screen and basic display functionality using [VT100](https://vt100.net/docs/vt100-ug/chapter3.html) escape sequences
- **Window Size Detection**: Automatically detects terminal dimensions using `ioctl` with fallback
- **Basic Input Handling**: Processes keystrokes individually in real-time
- **Exit Functionality**: Ctrl+Q cleanly exits the program
- **Memory Management**: Includes append buffer for efficient string operations

### ❌ Not Yet Implemented

- **Text Editing**: No file loading, saving, or text manipulation
- **Cursor Movement**: Arrow keys and cursor positioning not functional
- **File Operations**: Cannot open, edit, or save files
- **Text Display**: Cannot display file contents
- **Search/Replace**: No search functionality
- **Multiple Lines**: No support for multi-line text editing
- **Syntax Highlighting**: No code highlighting features

## Building and Running

### Compile
```sh
make editor
```

### Run
```sh
./main
```

### Exit
Press `Ctrl+Q` to exit the editor.

## Technical Implementation

### Terminal Mode
We use raw mode instead of canonical mode to read input character by character. This requires:
- Modifying terminal attributes with `tcgetattr` and `tcsetattr`
- Disabling various terminal flags (ECHO, ICANON, ISIG, etc.)
- Setting up proper cleanup with `atexit()`

### Screen Control
Uses VT100 escape sequences for terminal control:
- `\x1b[2J` - Clear entire screen
- `\x1b[H` - Move cursor to top-left
- `\x1b[6n` - Query cursor position

### Current Limitations
- Only displays tildes (`~`) for empty lines
- No actual text editing capabilities
- Limited to basic terminal interaction
- No file I/O operations

## Development Status

This implementation represents **Part 1** of the kilo tutorial series, focusing on:
1. Terminal setup and raw mode configuration
2. Basic input/output handling
3. Screen management foundations

**Next Steps**: Implementation needs cursor movement, text display, file operations, and actual editing functionality to become a usable text editor.

## Terminal Compatibility

Currently uses VT100 escape sequences for broad terminal compatibility. For maximum terminal support, consider migrating to ncurses library in future iterations.

