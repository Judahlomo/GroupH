/*
  File: logger.cpp
  Author: Ryeleigh Avila
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

// Global pointer to shared clock and output file
SimClock* clock_ptr = nullptr;
std::ofstream logFile;

// Initializes the logger and open log file
void logger_init(SimClock* clock) {
    clock_ptr = clock;
    logFile.open("simulation.log", std::ios::out);  // Overwrites on each run
    if (!logFile.is_open()) {
        std::cerr << "ERROR: Cannot open simulation.log.\n";
        exit(1);
    }
}

// Convert sim_time to [HH:MM:SS]
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

// Log with +1 time increment
void log_event(const std::string& message) {
    if (!clock_ptr || !logFile.is_open()) return;
    pthread_mutex_lock(&clock_ptr->time_mutex);
    clock_ptr->sim_time += 1;
    logFile << format_timestamp(clock_ptr->sim_time) << " " << message << std::endl;
    pthread_mutex_unlock(&clock_ptr->time_mutex);
}

// Log with custom delay
void log_delay(int delay, const std::string& message) {
    if (!clock_ptr || !logFile.is_open()) return;
    pthread_mutex_lock(&clock_ptr->time_mutex);
    clock_ptr->sim_time += delay;
    logFile << format_timestamp(clock_ptr->sim_time) << " " << message << std::endl;
    pthread_mutex_unlock(&clock_ptr->time_mutex);
}

// Close file
void logger_close() {
    if (logFile.is_open()) {
        logFile.close();
    }
}
