# include "Process.h"

// Constructor: Initialize the process with the provided PID and default values for name and memory usage
Process::Process(const std::string& pid) : pid(pid), name("Unknown"), vmrss_kb(-1) {};

// Function to load the details from /proc/[pid]/status
bool Process::load_details() {
    cout << "Hello";
    return true;
}

// Function to check whether the memory usage exceeds the threshold
bool Process::exceeds_memory_threshold() const { return true; }

// Function to print the process details in the terminal
void Process::print_details() const {}