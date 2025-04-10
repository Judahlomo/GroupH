// Train_process.cpp - Judah Lomo
#include <iostream>
#include <cstring>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>

#define SHM_KEY 1234
#define MSG_Q_KEY 5678

struct TrainMessage {
    long mtype;
    char mtext[100];
};

struct Intersection {
    bool available;
    int occupiedBy;
};

// Global message queue ID
int msgid;

// Placeholder for route data (to be populated via shared memory or arguments)
std::vector<std::string> route;
std::string trainName;

// Sends ACQUIRE request to parent
void request_intersection(const std::string& intersectionName, pid_t pid) {
    TrainMessage msg;
    msg.mtype = 1; // server listens on 1
    snprintf(msg.mtext, sizeof(msg.mtext), "ACQUIRE %s %d %s", trainName.c_str(), pid, intersectionName.c_str());

    msgsnd(msgid, &msg, sizeof(msg.mtext), 0);
}

// Sends RELEASE request to parent
void release_intersection(const std::string& intersectionName, pid_t pid) {
    TrainMessage msg;
    msg.mtype = 1;
    snprintf(msg.mtext, sizeof(msg.mtext), "RELEASE %s %d %s", trainName.c_str(), pid, intersectionName.c_str());

    msgsnd(msgid, &msg, sizeof(msg.mtext), 0);
}

// Waits for GRANT response from parent (filtered by this train's PID)
void wait_for_grant(pid_t pid) {
    TrainMessage response;
    while (true) {
        msgrcv(msgid, &response, sizeof(response.mtext), pid, 0);
        std::string responseText(response.mtext);
        std::cout << trainName << ": " << responseText << std::endl;

        if (responseText.find("GRANT") != std::string::npos) {
            break;
        } else if (responseText.find("WAIT") != std::string::npos) {
            sleep(1); // optional backoff before retrying
        } else if (responseText.find("DENY") != std::string::npos) {
            std::cerr << trainName << ": Access denied. Exiting.\n";
            exit(1);
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <TrainName>\n";
        return 1;
    }

    trainName = argv[1];
    pid_t pid = getpid();

    // TODO: Pull actual route info from shared memory or arguments
    // Temporary hardcoded example
    route = {"IntersectionA", "IntersectionB", "IntersectionC"};

    // Connect to message queue
    msgid = msgget(MSG_Q_KEY, 0666);
    if (msgid == -1) {
        perror("msgget failed in train process");
        return 1;
    }

    std::cout << trainName << ": Starting route...\n";

    for (const auto& intersection : route) {
        request_intersection(intersection, pid);
        wait_for_grant(pid);

        sleep(2); // Simulate traversal

        release_intersection(intersection, pid);
    }
    
    std::cout << trainName << ": Finished route.\n";
    return 0;
}