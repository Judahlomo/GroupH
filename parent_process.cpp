/*
  Course: CS4323, Spring 2025, Dr. Shital Joshi
  Group Project: Multi-Train Railway Simulation (Parent Process)
  Author: Roberts Kovalonoks

  Description:
  This file runs the main controller for the train simulation. It sets up shared memory,
  the message queue, and a shared clock that all processes use. It waits for requests
  from the train programs to either take or release intersections.

  Intersections are protected with mutexes (for single access) or semaphores (for shared access).
  The parent also keeps track of what each train is holding or waiting for using a graph. If a
  deadlock is found, it picks one train to stop and clears it so the rest can move on.

  All actions, like grants and releases, are written to a log file with timestamps from the
  shared clock. This helps keep the simulation organized and easy to follow.
*/

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <pthread.h>
#include <semaphore.h>
#include <cstring>

#include "logger.h"
#include "structures.h"
#include "ipc_setup.h"

// Brings in the logging tools so we can record events and actions during the simulation.
#define SHM_KEY 1234
// Loads definitions for data structures like TrainMessage and Intersection that we'll be working with.
#define MSG_Q_KEY 5678
// Gives access to setup functions for shared memory, semaphores, message queues, and other IPC components.
#define CLOCK_SHM_KEY 2468

/* Summary:
* This code defines a structure named TrainMessage that is used for inter-process
* communication, likely in a message queuing context. The structure contains:
*   - mtype: a long integer representing the type of the message.
*   - mtext: a character array of 100 characters intended to hold the message text. 
*/
struct TrainMessage {
    long mtype;
    char mtext[100];
};

/**
 * Pointer to the ResourceAllocationTable object
 * This pointer is used to manage and track the allocation of resources
 * within the system. It is initialized to nullptr and should be assigned
 * a valid ResourceAllocationTable instance before use.
 */
ResourceAllocationTable* resourceTable = nullptr;


/**
 * Splits a given string into a vector of words based on whitespace.
 *
 * This function takes an input string and splits it into individual words
 * using whitespace as the delimiter. The resulting words are stored in a
 * vector of strings and returned.
 *
 * parameters: input The input string to be split into words.
 * return: A vector of strings, where each element is a word from the input string.
 */
std::vector<std::string> split_message(const std::string& input) {
    std::istringstream ss(input);
    std::string word;
    std::vector<std::string> parts;
    while (ss >> word)
        parts.push_back(word);
    return parts;
}

// Helper: get index of intersection
int get_intersection_index(const std::string& name) {
    for (int i = 0; i < resourceTable->num_intersections; i++) {
        std::string interName = "Intersection";
        interName += ('A' + i);
        if (interName == name)
            return i;
    }
    return -1;
}

// Initialize IPC and logger
void initialize_ipc() {
    setupIPC(resourceTable);

    int clk_id = shmget(CLOCK_SHM_KEY, sizeof(SimClock), IPC_CREAT | 0666);
    if (clk_id == -1) { perror("shmget clock failed"); exit(1); }

    SimClock* clock = (SimClock*)shmat(clk_id, NULL, 0);
    if (clock == (void*)-1) { perror("shmat clock failed"); exit(1); }

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&clock->time_mutex, &attr);
    clock->sim_time = 0;

    logger_init(clock);
    log_event("SERVER: Logger and SimClock initialized");

    log_event("SERVER: IPC setup complete");
}

// Respond to a train
void send_response(pid_t pid, const std::string& status) {
    TrainMessage response;
    response.mtype = pid;
    snprintf(response.mtext, sizeof(response.mtext), "%s", status.c_str());
    msgsnd(msgid, &response, sizeof(response.mtext), 0);
}

// Add train to intersection's holding list
void add_holding_train(Intersection& inter, const std::string& trainName) {
    for (int i = 0; i < MAX_TRAINS; i++) {
        if (inter.holding_trains[i][0] == '\0') {
            strncpy(inter.holding_trains[i], trainName.c_str(), MAX_NAME);
            break;
        }
    }
}

// Remove train from holding list
void remove_holding_train(Intersection& inter, const std::string& trainName) {
    for (int i = 0; i < MAX_TRAINS; i++) {
        if (trainName == inter.holding_trains[i]) {
            inter.holding_trains[i][0] = '\0';
        }
    }
}

/*
  This function handles a train's request to acquire an intersection.
  Depending on whether it's a mutex or semaphore type, it checks availability,
  updates the state, logs the result, and sends either a GRANT or WAIT response.
*/
void handle_acquire(const std::string& train, pid_t pid, const std::string& intersectionName) {
    int index = get_intersection_index(intersectionName);
    if (index == -1) {
        log_event("SERVER: Invalid intersection - " + intersectionName);
        send_response(pid, "DENY");
        return;
    }

    Intersection& inter = resourceTable->intersections[index];

    if (inter.type == 0) { // Mutex
        if (pthread_mutex_trylock(&inter.mutex) == 0) {
            inter.available = false;
            inter.occupiedBy = pid;
            add_holding_train(inter, train);
            log_event("SERVER: GRANTED " + intersectionName + " to " + train);
            send_response(pid, "GRANT");
        } else {
            log_event("SERVER: " + intersectionName + " is locked. " + train + " must WAIT.");
            send_response(pid, "WAIT");
        }
    } else { // Semaphore
        if (inter.current_count < inter.capacity) {
            sem_wait(&inter.semaphore);
            inter.current_count++;
            add_holding_train(inter, train);
            log_event("SERVER: GRANTED " + intersectionName + " to " + train + " (semaphore slot)");
            send_response(pid, "GRANT");
        } else {
            log_event("SERVER: " + intersectionName + " full. " + train + " must WAIT.");
            send_response(pid, "WAIT");
        }
    }
}

/*
  This function handles when a train is done with an intersection.
  It frees up the resource—unlocking a mutex or posting a semaphore—
  updates the shared state, and logs the release event.
*/
void handle_release(const std::string& train, pid_t pid, const std::string& intersectionName) {
    int index = get_intersection_index(intersectionName);
    if (index == -1) {
        log_event("SERVER: Invalid intersection release - " + intersectionName);
        return;
    }

    Intersection& inter = resourceTable->intersections[index];

    if (inter.type == 0) {
        pthread_mutex_unlock(&inter.mutex);
        inter.available = true;
        inter.occupiedBy = -1;
        remove_holding_train(inter, train);
        log_event("SERVER: " + train + " released " + intersectionName + " (mutex)");
    } else {
        sem_post(&inter.semaphore);
        inter.current_count--;
        remove_holding_train(inter, train);
        log_event("SERVER: " + train + " released " + intersectionName + " (semaphore)");
    }
}


/* This function is responsible for starting up several train processes.
Each train will follow its own route that goes through multiple intersections. */
void launch_trains() {
    // Here, we define a list of routes. Each route is a list of strings:
    // the name of the train followed by the intersections it needs to pass through.
    std::vector<std::vector<std::string>> trainRoutes = {
        {"Train1", "IntersectionA", "IntersectionB", "IntersectionC"},
        {"Train2", "IntersectionB", "IntersectionD", "IntersectionE"},
        {"Train3", "IntersectionC", "IntersectionD", "IntersectionA"},
        {"Train4", "IntersectionE", "IntersectionB", "IntersectionD"}
    };

    // We loop through each route to create a separate process for each train.
    for (const auto& route : trainRoutes) {
        pid_t pid = fork();

        // If we're in the child process, we prepare to launch the train
        if (pid == 0) {
            std::vector<char*> args;
            args.push_back((char*)"./train_process");

            // Then we add all the route details (train name + intersections)
            for (const auto& r : route)
                args.push_back(const_cast<char*>(r.c_str()));

            // Null pointer at the end to mark the end of arguments (required by execv)
            args.push_back(nullptr);

            // Replace this process image with a new program ("train_process")
            execv("./train_process", args.data());

            // If something goes wrong with execv, print an error message
            perror("execv failed");
            exit(1); // Exit child process if execv fails
        }
    }
}

/* Summary: This file implements the main server loop which launches train processes,
logs the event, and continually listens for messages on a message queue.  
Upon receiving a message, it parses the message to extract the message type, 
train name, process ID, and intersection. Based on the type (ACQUIRE or RELEASE), 
the corresponding handler function is invoked to process the request */
void run_server() {
    launch_trains();
    log_event("SERVER: Train processes launched");

    while (true) {
        TrainMessage msg;
        if (msgrcv(msgid, &msg, sizeof(msg.mtext), 1, 0) != -1) {
            std::string raw = msg.mtext;
            std::vector<std::string> tokens = split_message(raw);

            if (tokens.size() < 4) continue;

            std::string type = tokens[0];
            std::string trainName = tokens[1];
            pid_t pid = std::stoi(tokens[2]);
            std::string intersection = tokens[3];

            if (type == "ACQUIRE")
                handle_acquire(trainName, pid, intersection);
            else if (type == "RELEASE")
                handle_release(trainName, pid, intersection);
        }
    }
}


/**
 * This function serves as a Entry point, initializes inter-process communication (IPC),
 * starts the server, and ensures proper cleanup by closing the logger
 * before terminating. It provides a structured flow for the parent
 * process's lifecycle.
 *
 */
int main() {
    std::cout << "Parent Process Starting...\n";
    initialize_ipc();
    run_server();
    logger_close();
    std::cout << "Parent Process Terminated.\n";
    return 0;
}
