/*
  File: logger.cpp
  Author: Ryleigh Avila
  Course: CS 4323 Operating Systems
  Project 4 (Group H)
 
  Description: Manages logging for the railway simulation.It maintains a shared simulated time counter and writes synchronized
  log entries to simulation.log using [HH:MM:SS] format. Each event triggers a time increment and ensures deterministic output for
  analysis and debugging. Mutex locks are used here to grant atomic access to the time counter across processes.
 
  Functions included within code:
  *logger_init(SimClock* clock): Initializes logger and opens log file
  *log_event(string message): Logs a single event with a +1 time increment
  *log_delay(int delay, string message): Logs event with custom delay
  *logger_close(): Closes the log file
 */
 
// NOTE group H:
/* To use this logger, be sure to include the header file in your file:
   #include "logger.h"
    Additionally, Then you can call log_event("message") or log_delay(delay, "message"). In your own modules (parent_process.cpp, train_process.cpp, etc.).
    This keeps our simulation.log consistent and time-synced. Correct me if I am wrong on this andw please let me knwo if you have any questions. 
*/

#include "logger.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <pthread.h>

// Global pointer to shared clock (in shared memory)
SimClock* clock_ptr = nullptr;
std::ofstream logFile;

// Initialize logger with pointer to shared SimClock
void logger_init(SimClock* clock) {
    clock_ptr = clock;
    logFile.open("simulation.log", std::ios::out);  // Overwrite each time
    if (!logFile.is_open()) {
        std::cerr << "ERROR: Could not open simulation.log file.\n";
        exit(1);
    }
}

// Format [HH:MM:SS] from sim_time
std::string format_timestamp(int sim_time) {
    int hours = sim_time / 3600;
    int minutes = (sim_time % 3600) / 60;
    int seconds = sim_time % 60;

    std::ostringstream oss;
    oss << "[" << std::setw(2) << std::setfill('0') << hours << ":"
        << std::setw(2) << std::setfill('0') << minutes << ":"
        << std::setw(2) << std::setfill('0') << seconds << "]";
    return oss.str();
}

// Log an event and auto-increment time by 1
void log_event(const std::string& message) {
    if (!clock_ptr || !logFile.is_open()) return;
// Lock the mutex to safely update sim_time (shared across processes)
    pthread_mutex_lock(&clock_ptr->time_mutex);
    // Increment the simulated time by the specified delay
    clock_ptr->sim_time += 1;
    std::string timestamp = format_timestamp(clock_ptr->sim_time);
    logFile << timestamp << " " << message << std::endl;
    pthread_mutex_unlock(&clock_ptr->time_mutex);
}

// Log an event with a manual time delay (Like, simulating sleep)
// Will be used when a train takes time during pass through of intersection
// 'message' is the log text (Ex: "TRAIN1: Crossing IntersectionA").
void log_delay(int delay, const std::string& message) {
    if (!clock_ptr || !logFile.is_open()) return;

// Lock the mutex to safely update sim_time (shared across processes)
    pthread_mutex_lock(&clock_ptr->time_mutex);
    // Increment the simulated time by the specified delay
    clock_ptr->sim_time += delay;
    std::string timestamp = format_timestamp(clock_ptr->sim_time); // Format the updated time as [HH:MM:SS]
    logFile << timestamp << " " << message << std::endl;
    pthread_mutex_unlock(&clock_ptr->time_mutex);
}

// Close the log file
void logger_close() {
    if (logFile.is_open()) {
        logFile.close();
    }
}
