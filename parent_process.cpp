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

#define SHM_KEY 1234
#define MSG_Q_KEY 5678
#define CLOCK_SHM_KEY 2468

struct TrainMessage {
    long mtype;
    char mtext[100];
};

ResourceAllocationTable* resourceTable = nullptr;

// Helper: split message by spaces
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

// Handle ACQUIRE request
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

// Handle RELEASE request
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

// Launch 4 child train processes
void launch_trains() {
    std::vector<std::vector<std::string>> trainRoutes = {
        {"Train1", "IntersectionA", "IntersectionB", "IntersectionC"},
        {"Train2", "IntersectionB", "IntersectionD", "IntersectionE"},
        {"Train3", "IntersectionC", "IntersectionD", "IntersectionA"},
        {"Train4", "IntersectionE", "IntersectionB", "IntersectionD"}
    };

    for (const auto& route : trainRoutes) {
        pid_t pid = fork();
        if (pid == 0) {
            std::vector<char*> args;
            args.push_back((char*)"./train_process");
            for (const auto& r : route)
                args.push_back(const_cast<char*>(r.c_str()));
            args.push_back(nullptr);

            execv("./train_process", args.data());
            perror("execv failed");
            exit(1);
        }
    }
}

// Main server loop
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

// Entry point
int main() {
    std::cout << "Parent Process Starting...\n";
    initialize_ipc();
    run_server();
    logger_close();
    std::cout << "Parent Process Terminated.\n";
    return 0;
}
