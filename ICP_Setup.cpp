#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <cstring>
#include <unistd.h>

// Message Queues
struct TrainMessage {
    long mtype; //use trainID
    char mtext[100]; //messages
};

struct Intersection {
    bool available;
    int occupiedBy; //use trainID int
};

//Example IPC keys can be changed
const key_t SHM_KEY = 1234;
const key_t MSG_Q_KEY = 5678;


void setupIPC() {
    //shared memory sections for intersections
    int shmid = shmget(SHM_KEY, sizeof(Intersection), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget failed");
        exit(1);
    }

    Intersection* intersection = (Intersection*)shmat(shmid, NULL, 0);
    if (intersection == (void*)-1) {
        perror("shmat failed");
        exit(1);
    }

    //Checking for intersections availability
    intersection->available = true;
    intersection->occupiedBy = -1;

    //Makes message queue
    int msgid = msgget(MSG_Q_KEY, IPC_CREAT | 0666);
    if (msgid == -1) {
        perror("msgget failed");
        exit(1);
    }

    std::cout << "IPC setup complete:\n";
    std::cout << "- Shared Memory ID: " << shmid << "\n";
    std::cout << "- Message Queue ID: " << msgid << "\n";
}

// Cleanup IPC components
void cleanupIPC() {
    // Remove shared memory
    int shmid = shmget(SHM_KEY, sizeof(Intersection), 0666);
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl remove failed");
    }

    // Remove message queue
    int msgid = msgget(MSG_Q_KEY, 0666);
    if (msgctl(msgid, IPC_RMID, NULL) == -1) {
        perror("msgctl remove failed");
    }

    std::cout << "IPC components cleaned up\n";
}

int main() {
    std::cout << "Setting up mock train system IPC...\n";
    
    setupIPC();

    /* Still have to fork server and train processes 
    and have trains request intersections from message queue 
    and let parent server manages intersection access via shared memory
    */
    std::cout << "Press Enter to shutdown";
    std::cin.ignore();
    
    cleanupIPC();
    return 0;
}