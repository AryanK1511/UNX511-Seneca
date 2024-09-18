#include "ProcessManager.h"

// Function to check if a string is a number (valid PID)
bool ProcessManager::is_number(const string &s) const {
  for (char const &ch : s) {
    if (!isdigit(ch)) {
      return false;
    }
  }
  return true;
}

// Function to scan the /proc directory and check each process
void ProcessManager::scan_processes() const {
  DIR *dir = opendir(PROC_DIR.c_str());
  struct dirent *entry;

  if (dir == nullptr) {
    cerr << "Failed to open /proc directory" << endl;
    return;
  }

  while ((entry = readdir(dir)) != nullptr) {
    string dir_name = string(entry->d_name);

    // Only process directories with numerical names (PIDs)
    if (is_number(dir_name)) {
      Process process(dir_name);
      if (process.load_details() && process.exceeds_memory_threshold()) {
        process.print_details();
      }
    }
  }

  closedir(dir);
}
