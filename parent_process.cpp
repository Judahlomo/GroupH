// parent_process.cpp - Roberts Kovalonoks

#include <iostream>
#include <vector>
#include <string>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <pthread.h>
#include "logger.h"

#define SHM_KEY 1234
#define MSG_Q_KEY 5678
#define CLOCK_SHM_KEY 2468 // SimClock key - unique

struct Intersection {
    bool available;
    int occupiedBy;
};

struct TrainMessage {
    long mtype;
    char mtext[100];
};

// Globals (defined only here)
Intersection* intersectionShm = nullptr;
int msgid;  // OK to keep this global

// Attach & setup shared memory (Intersections + SimClock)
void initialize_ipc() {
    // Attach Intersections shared memory
    int shmid = shmget(SHM_KEY, sizeof(Intersection), 0666);
    if (shmid == -1) {
        perror("shmget failed for Intersection");
        exit(1);
    }

    intersectionShm = (Intersection*)shmat(shmid, NULL, 0);
    if (intersectionShm == (void*)-1) {
        perror("shmat failed for Intersection");
        exit(1);
    }

    // Attach SimClock shared memory
    int clock_shmid = shmget(CLOCK_SHM_KEY, sizeof(SimClock), IPC_CREAT | 0666);
    if (clock_shmid == -1) {
        perror("shmget failed for SimClock");
        exit(1);
    }

    SimClock* shared_clock = (SimClock*)shmat(clock_shmid, NULL, 0);
    if (shared_clock == (void*)-1) {
        perror("shmat failed for SimClock");
        exit(1);
    }

    // Initialize SimClock
    shared_clock->sim_time = 0;
    pthread_mutexattr_t mattr;
    pthread_mutexattr_init(&mattr);
    pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&shared_clock->time_mutex, &mattr);

    // Init logger with shared SimClock
    logger_init(shared_clock);
    log_event("SERVER: Logger initialized and SimClock attached");

    // Attach Message Queue
    msgid = msgget(MSG_Q_KEY, 0666);
    if (msgid == -1) {
        perror("msgget failed");
        exit(1);
    }

    log_event("SERVER: IPC resources attached");
}

// Main event loop for listening to Train Messages
void run_server() {
    log_event("SERVER: Entering server loop");

    while (true) {
        TrainMessage msg;
        if (msgrcv(msgid, &msg, sizeof(msg.mtext), 1, 0) == -1) {
            perror("msgrcv failed");
            exit(1);
        }

        std::string message(msg.mtext);
        log_event("SERVER: Received - " + message);

        if (message == "EXIT") {
            log_event("SERVER: Exit command received, shutting down");
            break;
        }
    }
}

// Entry point
int main() {
    std::cout << "Parent Process Starting...\n";

    initialize_ipc();
    run_server();

    logger_close();

    std::cout << "Parent Process Terminated.\n";
    return 0;
}
