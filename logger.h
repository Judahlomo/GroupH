#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <pthread.h>

// Hold simulated time and synchronization
struct SimClock {
    int sim_time;                   // Shared simulated time counter
    pthread_mutex_t time_mutex;    // Mutex to atomic will updates
};

// Logger function declarations

// Initializes logger with the shared clock
void logger_init(SimClock* clock);

// Logs a message and automatically increments sim_time by 1
void log_event(const std::string& message);

// Logs a message after incrementing sim_time by a custom delay
void log_delay(int delay, const std::string& message);

// Close log file
void logger_close();

#endif
