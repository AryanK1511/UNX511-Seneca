#include "Process.h"
#include <fstream>
#include <iomanip>
#include <sstream>

// Constructor: Initialize the process with the provided PID and default values
// for name and memory usage
Process::Process(const std::string &pid)
    : pid(pid), name("Unknown"), vmrss_kb(-1){};

// Function to load the details from /proc/[pid]/status
bool Process::load_details() {
  string status_path = PROC_DIR + "/" + pid + "/" + STATUS_FILE;
  string line;

  ifstream file(status_path);

  if (!file.is_open()) {
    return false;
  }

  while (getline(file, line)) {
    if (line.find("Name:") != string::npos) {
      istringstream iss(line);
      string key;
      iss >> key >> name;
    }
    if (line.find("VmSize:") != string::npos) {
      istringstream iss(line);
      string key, value, unit;
      iss >> key >> value >> unit;
      vmsize_kb = stoi(value);
    }
    if (line.find("VmRSS:") != string::npos) {
      istringstream iss(line);
      string key, value, unit;
      iss >> key >> value >> unit;
      vmrss_kb = stoi(value);
    }
  }

  file.close();
  return true;
}

// Function to check if the memory usage exceeds the threshold
bool Process::exceeds_memory_threshold() const {
  return vmrss_kb > VMRSS_THRESHOLD;
}

// Function to print process details
void Process::print_details() const {
  const int label_width = 12;
  const int value_width = 10;

  cout << left;

  cout << setw(label_width) << "Process Name:" << setw(value_width) << name
       << endl;
  cout << setw(label_width) << "PID:" << setw(value_width) << pid << endl;
  cout << setw(label_width) << "VMSize:" << setw(value_width) << vmsize_kb
       << " kB" << endl;
  cout << setw(label_width) << "VmRSS:" << setw(value_width) << vmrss_kb
       << " kB" << endl;
  cout << endl;
}
