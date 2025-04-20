/*
  File: Train_process.cpp
  Author: Judah Lomo
  Course: CS 4323 Operating Systems
  Project Group H

  Simulates a train's behavior in the railway system by requesting, waiting for, 
  and releasing intersections via message queues.
*/

#include <iostream>
#include <cstring>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define MSG_Q_KEY 5678

// Message structure used for communication with the parent
struct TrainMessage {
    long mtype;          // Train's PID or 1 for server
    char mtext[100];     // Request = ACQUIRE or RELEASE
};

// Globals to track message queue, name, and route
int msgid;
std::string trainName;
std::vector<std::string> route;

// Send ACQUIRE request
void request_intersection(const std::string& intersectionName, pid_t pid) {
    TrainMessage msg;
    msg.mtype = 1; // Server listens on 1
    snprintf(msg.mtext, sizeof(msg.mtext), "ACQUIRE %s %d %s", trainName.c_str(), pid, intersectionName.c_str());
    msgsnd(msgid, &msg, sizeof(msg.mtext), 0);
    std::cout << trainName << " requesting " << intersectionName << std::endl;
}

// Send RELEASE request
void release_intersection(const std::string& intersectionName, pid_t pid) {
    TrainMessage msg;
    msg.mtype = 1;
    snprintf(msg.mtext, sizeof(msg.mtext), "RELEASE %s %d %s", trainName.c_str(), pid, intersectionName.c_str());
    msgsnd(msgid, &msg, sizeof(msg.mtext), 0);
    std::cout << trainName << " releasing " << intersectionName << std::endl;
}

// Wait for GRANT or WAIT
void wait_for_grant(pid_t pid) {
    TrainMessage response;
    while (true) {
        if (msgrcv(msgid, &response, sizeof(response.mtext), pid, 0) != -1) {
            std::string text(response.mtext);
            std::cout << trainName << " received: " << text << std::endl;
            if (text.find("GRANT") != std::string::npos) break;
            else if (text.find("WAIT") != std::string::npos) sleep(1);
            else if (text.find("DENY") != std::string::npos) exit(1);
        }
    }
}

// Main function expect to see at least train name + 1 intersection
int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: ./Train_process <TrainName> <Intersection1> [Intersection2] ...\n";
        return 1;
    }

    // Fill route vector with its given intersections
    trainName = argv[1];
    for (int i = 2; i < argc; ++i)
        route.push_back(argv[i]);

    pid_t pid = getpid();
    msgid = msgget(MSG_Q_KEY, 0666);

    // Process each intersection in the train's route
    std::cout << trainName << ": Starting route...\n";
    for (const auto& intersection : route) {
        request_intersection(intersection, pid);
        wait_for_grant(pid);
        sleep(2); // Simulated travel time
        release_intersection(intersection, pid);
    }

    std::cout << trainName << ": Finished route.\n";
    return 0;
}

