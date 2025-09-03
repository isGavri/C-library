#include <ncurses.h>
#include <stdlib.h>

int main() {
  initscr();
  printw("Hello, printing to the window");
  refresh();
  getch();
  endwin();
  return EXIT_SUCCESS;
}
