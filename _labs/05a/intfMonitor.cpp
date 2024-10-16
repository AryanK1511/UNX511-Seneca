// intfMonitor_solution.cpp - An interface monitor
//
//  13-Jul-20  M. Watler         Created.

#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

using namespace std;

const int MAXBUF = 128;
bool isRunning = false;

// TODO: Declare your signal handler function prototype
static void sigHandler(int sig);

int main(int argc, char *argv[]) {
  // TODO: Declare a variable of type struct sigaction
  //       For sigaction, see
  //       http://man7.org/linux/man-pages/man2/sigaction.2.html
  struct sigaction sa;
  sa.sa_handler = sigHandler;
  sa.sa_flags = 0;
  sigemptyset(&sa.sa_mask);

  char interface[MAXBUF];
  char statPath[MAXBUF];
  const char logfile[] = "Network.log"; // store network data in Network.log
  int retVal = 0;

  // TODO: Register signal handlers for SIGUSR1, SIGUSR2, ctrl-C and ctrl-Z
  // TODO: Ensure there are no errors in registering the handlers
  if (sigaction(SIGUSR1, &sa, NULL) == -1) {
    perror("Failed to register SIGUSR1 handler");
    exit(1);
  }
  if (sigaction(SIGUSR2, &sa, NULL) == -1) {
    perror("Failed to register SIGUSR2 handler");
    exit(1);
  }
  if (sigaction(SIGINT, &sa, NULL) == -1) {
    perror("Failed to register SIGINT handler");
    exit(1);
  }
  if (sigaction(SIGTSTP, &sa, NULL) == -1) {
    perror("Failed to register SIGTSTP handler");
    exit(1);
  }

  strncpy(
      interface, argv[1],
      MAXBUF); // The interface has been passed as an argument to intfMonitor
  int fd = open(logfile, O_RDWR | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
  cout << "intfMonitor:main: interface:" << interface << ":  pid:" << getpid()
       << endl;

  // TODO: Wait for SIGUSR1 - the start signal from the parent
  while (!isRunning) {
    pause();
  }

  while (isRunning) {
    // gather some stats
    int tx_bytes = 0;
    int rx_bytes = 0;
    int tx_packets = 0;
    int rx_packets = 0;
    ifstream infile;
    sprintf(statPath, "/sys/class/net/%s/statistics/tx_bytes", interface);
    infile.open(statPath);
    if (infile.is_open()) {
      infile >> tx_bytes;
      infile.close();
    }
    sprintf(statPath, "/sys/class/net/%s/statistics/rx_bytes", interface);
    infile.open(statPath);
    if (infile.is_open()) {
      infile >> rx_bytes;
      infile.close();
    }
    sprintf(statPath, "/sys/class/net/%s/statistics/tx_packets", interface);
    infile.open(statPath);
    if (infile.is_open()) {
      infile >> tx_packets;
      infile.close();
    }
    sprintf(statPath, "/sys/class/net/%s/statistics/rx_packets", interface);
    infile.open(statPath);
    if (infile.is_open()) {
      infile >> rx_packets;
      infile.close();
    }
    char data[MAXBUF];
    // write the stats into Network.log
    int len = sprintf(
        data, "%s: tx_bytes:%d rx_bytes:%d tx_packets:%d rx_packets: %d\n",
        interface, tx_bytes, rx_bytes, tx_packets, rx_packets);
    write(fd, data, len);
    sleep(1);
  }
  close(fd);

  return 0;
}

// TODO: Create a signal handler that
static void sigHandler(int sig) {
  switch (sig) {

  case SIGUSR1:
    // starts your program on SIGUSR1 (sets isRunning to true)
    isRunning = true;
    cout << "intfMonitor: starting up" << endl;
    break;

  case SIGUSR2:
    // stops your program on SIGUSR2 (sets isRunning to false)
    isRunning = false;
    cout << "intfMonitor: shutting down" << endl;
    break;

  case SIGINT:
    // discards any ctrl-C
    cout << "intfMonitor: ctrl-C discarded" << endl;
    break;

  case SIGTSTP:
    // discards any ctrl-Z
    cout << "intfMonitor: ctrl-Z discarded" << endl;
    break;

  default:
    // signal handler receives any other signal
    cout << "intfMonitor: undefined signal" << endl;
    break;
  }
}
