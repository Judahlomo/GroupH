// Parent_process.cpp - Roberts Kovalonoks
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <unistd.h>
#include <cstring>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include "logger.h"              // Ryeleigh's module
#include "deadlock_detection.h"  // Yosep's module (future)
#include "deadlock_resolution.h" // Mohammed's module (future)

// Constants from ipc_setup.cpp (Joey's module)
// Defined locally for now (must match ipc_setup.cpp)
#define SHM_KEY 1234
#define MSG_Q_KEY 5678

#ifndef MSG_Q_KEY
#define MSG_Q_KEY 5678
#endif

struct Intersection {
    bool available;
    int occupiedBy;
};

struct TrainMessage {
    long mtype;
    char mtext[100];
};

// Globals
int sim_time = 0;
std::vector<std::string> trainList;
int msgid;
Intersection* intersectionShm = nullptr;

// 1. Load config files
void parse_config_files();

// 2. Setup IPC using Joey's setupIPC function
void setupIPC(); // from ipc_setup.cpp

void initialize_ipc() {
    setupIPC(); // Calls Joey's function to create shm & msg queue

    // Manually attach to shared memory
    int shmid = shmget(SHM_KEY, sizeof(Intersection), 0666);
    if (shmid == -1) {
        perror("shmget (read-only) failed");
        exit(1);
    }

    intersectionShm = (Intersection*)shmat(shmid, NULL, 0);
    if (intersectionShm == (void*)-1) {
        perror("shmat failed in parent");
        exit(1);
    }

    msgid = msgget(MSG_Q_KEY, 0666);
    if (msgid == -1) {
        perror("msgget failed in parent");
        exit(1);
    }

    log_event(sim_time++, "SERVER: IPC attached in parent_process.cpp");
}

// 3. Fork train processes (each executes train_process)
void fork_trains();

// 4. Main server loop
void run_server();

// 5. Handle ACQUIRE/RELEASE requests (message queue listener)
void handle_train_message(const TrainMessage& msg);

// 6. Manage shared memory state (update locks, semaphores)
void update_intersection_state();

// 7. Call deadlock detection + resolution if needed
void check_and_resolve_deadlock() {
    // Call Yosep's placeholder deadlock detection logic
    bool deadlock = false; // Default value

    // For now, simulate a basic call to a dummy function (to be replaced)
    if (deadlock == true) {
        log_event(sim_time++, "SERVER: Deadlock detected.");
        // Placeholder: eventually call resolve_deadlock() from Mohammed's module
    }
}

// Entry point
int main() {
    log_event(sim_time++, "SERVER: Starting Parent Process");

    parse_config_files();
    initialize_ipc();
    fork_trains();

    log_event(sim_time++, "SERVER: All trains forked. Entering event loop");
    run_server();

    return 0;
}
