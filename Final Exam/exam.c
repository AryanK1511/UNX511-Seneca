#include <assert.h>
#include <curses.h>
#include <inttypes.h>
#include <locale.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>


#define us2ns(c) (1000L * c)
#define ms2ns(c) (1000000L * c)

void init() {
  initscr();
  cbreak();
  nodelay(stdscr, true);
  noecho();
  intrflush(stdscr, false);
  keypad(stdscr, true);
  curs_set(0);
}

void deinit() { endwin(); }

void print_time(int x, int y) {
  struct timespec current_timespec;
  clock_gettime(CLOCK_REALTIME_COARSE, &current_timespec);
  char time_str[21];
  strftime(time_str, 20, "%F %T", localtime(&current_timespec.tv_sec));
  mvprintw(y, x, "Current time: %s", time_str);
}

typedef enum { command_nop, command_finish } command;

command process_input() {
  uint32_t key = getch();
  switch ((char)key) {
  case ' ':
    return command_finish;
  }
  return command_nop;
}

bool main_loop() {
  clear();
  print_time(2, 1);
  switch (process_input()) {
  case command_finish:
    return false;
  }

  // Declare the struct timespec before passing its address
  struct timespec ts = {0, ms2ns(100)};
  nanosleep(&ts, 0);

  refresh();
  return true;
}

int main(int argc, char **argv) {
  setlocale(LC_ALL, "en_US.UTF-8");
  init();
  while (main_loop())
    ;
  deinit();
  return EXIT_SUCCESS;
}
