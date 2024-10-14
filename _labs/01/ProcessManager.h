#include "Process.h"
#include <cstdlib>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;

class ProcessManager {
public:
  // Function to check if a string is a number (valid PID)
  bool is_number(const string &s) const;

  // Function to scan the /proc directory and check each process
  void scan_processes() const;
};
