#ifndef PROCESS_H
#define PROCESS_H

#include <iostream>
using namespace std;

const string PROC_DIR = "/proc";
const string STATUS_FILE = "status";
const int VMRSS_THRESHOLD = 10000; // Resident Memory: 10000 KB

class Process {
private:
  string pid;
  string name;
  int vmrss_kb;
  int vmsize_kb;

public:
  Process(const string &pid);

  // Function to load the details from /proc/[pid]/status
  bool load_details();

  // Function to check whether the memory usage exceeds the threshold
  bool exceeds_memory_threshold() const;

  // Function to print the process details in the terminal
  void print_details() const;
};

#endif