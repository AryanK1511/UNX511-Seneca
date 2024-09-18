#include "Process.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <dirent.h>
#include <cstdlib>

using namespace std;

class ProcessManager {
public:
    // Function to check if a string is a number (valid PID)
    bool is_number(const string& s) const;

    // Function to scan the /proc directory and check each process
    void scan_processes() const;
};