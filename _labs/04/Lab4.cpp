#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <limits>
#include <linux/fb.h> // Header file for the screen hardware
#include <sys/ioctl.h>
#include <unistd.h>

using namespace std;

// Function prototypes

void handleUserInput(int &fd);
int getFileDescriptor(const char *filename);
void redirectAllErrorsToLogFile(const char *filename);
void reportFixedScreenInfo(int fd);
void reportVariableScreenInfo(int fd);

// Constants

const char *SCREEN_HW_FILE = "/dev/fb0";
const char *LOG_FILE = "/home/aryank1511/Desktop/Screen.log";

// ========== MAIN FUNCTION ==========

int main(void) {
  // Make sure to redirect all errors to the log file
  redirectAllErrorsToLogFile(LOG_FILE);

  // Get the file descriptor for the HW File
  int fd = getFileDescriptor(SCREEN_HW_FILE);

  // Use the file descriptor to perform required actions
  handleUserInput(fd);

  // Close the file descriptor
  close(fd);

  return 0;
}

// Function definitions

void handleUserInput(int &fd) {
  int userInput;

  while (true) {
    do {
      cout << "========== SELECT AN OPTION ==========" << endl;
      cout << "1. Fixed Screen Info" << endl;
      cout << "2. Variable Screen Info" << endl;
      cout << "3. Exit" << endl;
      cout << endl;
      cout << "Enter your choice here: ";

      if (!(cin >> userInput)) {
        cout << "Invalid input. Please enter a number between 1 and 3." << endl;
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
      } else if (userInput < 1 || userInput > 3) {
        cout << "Invalid choice. Please select an option between 1 and 3."
             << endl;
      } else {
        break;
      }

      cout << endl;
    } while (true);

    switch (userInput) {
    case 1:
      reportFixedScreenInfo(fd);
      break;
    case 2:
      reportVariableScreenInfo(fd);
      break;
    case 3:
      cout << "\nExiting program...\n" << endl;
      return;
    }
  }
}

int getFileDescriptor(const char *filename) {
  int openFlags = O_RDONLY | O_NONBLOCK;
  int fd = open(filename, openFlags);
  if (fd < 0) {
    cerr << "Error opening file " << filename << ": " << strerror(errno)
         << endl;
    exit(1);
  }
  return fd;
}

void redirectAllErrorsToLogFile(const char *filename) {
  int openFlags = O_WRONLY | O_CREAT | O_TRUNC;
  int openPermissions = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

  int fdLog = open(filename, openFlags, openPermissions);

  if (fdLog < 0) {
    cerr << "Error opening log file " << LOG_FILE << ": " << strerror(errno)
         << endl;
    exit(1);
  }

  if (dup2(fdLog, STDERR_FILENO) < 0) {
    cerr << "Error redirecting stderr to log file: " << strerror(errno) << endl;
    close(fdLog);
    exit(1);
  }

  close(fdLog);
}

// Ref Doc: Line 156 ->
// https://docs.huihoo.com/doxygen/linux/kernel/3.7/include_2uapi_2linux_2fb_8h_source.html
void reportFixedScreenInfo(int fd) {
  struct fb_fix_screeninfo fixInfo;
  if (ioctl(fd, FBIOGET_FSCREENINFO, &fixInfo) < 0) {
    cerr << "Error reading fixed screen information: " << strerror(errno)
         << endl;
    exit(1);
  }

  cout << "=> Fixed Screen Information:" << endl;
  cout << " Screen visual: " << fixInfo.visual << endl;
  cout << " Screen accelerator: " << fixInfo.accel << endl;
  cout << " Screen capabilities: " << fixInfo.capabilities << endl;

  /*
  Output:
  => Fixed Screen Information:
    Screen visual: 2
    Screen accelerator: 0
    Screen capabilities: 0
  */

  cout << endl;
}

// Ref Doc: Line 240 =>
// https://docs.huihoo.com/doxygen/linux/kernel/3.7/include_2uapi_2linux_2fb_8h_source.html
void reportVariableScreenInfo(int fd) {
  struct fb_var_screeninfo varInfo;
  if (ioctl(fd, FBIOGET_VSCREENINFO, &varInfo) < 0) {
    cerr << "Error reading variable screen information: " << strerror(errno)
         << endl;
    exit(1);
  }

  cout << "=> Fixed Screen Information:" << endl;
  cout << " Screen xresolution: " << varInfo.xres << endl;
  cout << " Screen yresolution: " << varInfo.yres << endl;
  cout << " Bits per pixel: " << varInfo.bits_per_pixel << endl;

  /*
  Output:
  => Fixed Screen Information:
    Screen xresolution: 800
    Screen yresolution: 600
    Bits per pixel: 32
  */

  cout << endl;
}
