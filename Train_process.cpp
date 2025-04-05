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
void request_intersection(const std::string& intersectionName) {
    TrainMessage msg;
    msg.mtype = 1; // Can be train ID in future
    snprintf(msg.mtext, sizeof(msg.mtext), "ACQUIRE %s %s", trainName.c_str(), intersectionName.c_str());

    msgsnd(msgid, &msg, sizeof(msg.mtext), 0);
}

// Sends RELEASE request to parent
void release_intersection(const std::string& intersectionName) {
    TrainMessage msg;
    msg.mtype = 1;
    snprintf(msg.mtext, sizeof(msg.mtext), "RELEASE %s %s", trainName.c_str(), intersectionName.c_str());

    msgsnd(msgid, &msg, sizeof(msg.mtext), 0);
}

// Waits for GRANT response from parent
void wait_for_grant() {
    TrainMessage response;
    msgrcv(msgid, &response, sizeof(response.mtext), 1, 0);
    std::cout << trainName << ": " << response.mtext << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <TrainName>\n";
        return 1;
    }

    trainName = argv[1];

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
        request_intersection(intersection);
        wait_for_grant();

        // TODO: Simulate traversal delay (e.g., sleep(2))
        sleep(2);

        release_intersection(intersection);
    }

    std::cout << trainName << ": Finished route.\n";
    return 0;
}