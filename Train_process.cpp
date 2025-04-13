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

int msgid;
std::vector<std::string> route;
std::string trainName;

// Sends ACQUIRE request to parent
void request_intersection(const std::string& intersectionName, pid_t pid) {
    TrainMessage msg;
    msg.mtype = 1; // Parent listens on type 1
    snprintf(msg.mtext, sizeof(msg.mtext), "ACQUIRE %s %d %s", trainName.c_str(), pid, intersectionName.c_str());

    if (msgsnd(msgid, &msg, sizeof(msg.mtext), 0) == -1) {
        perror("msgsnd failed (ACQUIRE)");
        exit(1);
    }
    std::cout << trainName << ": Requested " << intersectionName << std::endl;
}

// Sends RELEASE request to parent
void release_intersection(const std::string& intersectionName, pid_t pid) {
    TrainMessage msg;
    msg.mtype = 1;
    snprintf(msg.mtext, sizeof(msg.mtext), "RELEASE %s %d %s", trainName.c_str(), pid, intersectionName.c_str());

    if (msgsnd(msgid, &msg, sizeof(msg.mtext), 0) == -1) {
        perror("msgsnd failed (RELEASE)");
        exit(1);
    }
    std::cout << trainName << ": Released " << intersectionName << std::endl;
}

// Waits for GRANT response from parent (filtered by this train's PID)
void wait_for_grant(pid_t pid) {
    TrainMessage response;
    while (true) {
        if (msgrcv(msgid, &response, sizeof(response.mtext), pid, 0) == -1) {
            perror("msgrcv failed");
            exit(1);
        }

        std::string responseText(response.mtext);
        std::cout << trainName << ": " << responseText << std::endl;

        if (responseText.find("GRANT") != std::string::npos) {
            break;
        } else if (responseText.find("WAIT") != std::string::npos) {
            sleep(1); // Retry after delay
        } else if (responseText.find("DENY") != std::string::npos) {
            std::cerr << trainName << ": Access denied. Exiting.\n";
            exit(1);
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <TrainName> <Intersection1> [Intersection2] ...\n";
        return 1;
    }

    trainName = argv[1];
    pid_t pid = getpid();

    // Populate route from command-line args
    for (int i = 2; i < argc; ++i) {
        route.emplace_back(argv[i]);
    }

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