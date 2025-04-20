/*
File: logger.h
Author: Ryeleigh Avila
Course: CS 4323 Operating Systems
Project 4 (Group H)// By: Ryeleigh Avila
// This header sets up a shared simulated clock and declares logging functions in order to rcord events
// This is done with sychronized timestamps during the simulation.
*/
#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <pthread.h>

// Simulated clock structure
struct SimClock {
    int sim_time;
    pthread_mutex_t time_mutex;
};

// Logger functions
void logger_init(SimClock* clock);
void log_event(const std::string& message); // logs a message increases simuated time by 1 unit
void log_delay(int delay, const std::string& message);// logs message and simulates time by time delay
void logger_close();
// closes log file when simulation ends
#endif

