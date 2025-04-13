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
    int occupiedBy; //use trainID int, -1 is available for occupation
};

//Example IPC keys can be changed, must match parent process
const key_t SHM_KEY = 1234;
const key_t MSG_Q_KEY = 5678;

//Sets global variables for the parent process
int msgid;
Intersection* intersectionShm = nullprt:

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

void cleanupIPC() {
    //Detachs shared memory
    if (intersectionShm != nullptr) {
        shmdt(intersectionShm);
    }
    
    //Removes shared memory segment
    int shmid = shmget(SHM_KEY, sizeof(Intersection), 0666);
    if (shmid != -1) {
        shmctl(shmid, IPC_RMID, NULL);
    }

    //Removes message queue
    if (msgid != -1) {
        msgctl(msgid, IPC_RMID, NULL);
    }
    
    std::cout << "Cleaned up IPC resources\n";
}

int main() {
    std::cout << "Setting up mock train system IPC...\n";
    
    setupIPC();
    //Shared memory segments
    int shmid = shmget(SHM_KEY, sizeof(ResourceAllocationTable), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget failed");
        exit(1);
    }
    //Shared Memory table
    ResourceAllocationTable* table = (ResourceAllocationTable*)shmat(shmid, NULL, 0);
    if (table == (void*)-1) {
        perror("shmat failed");
        exit(1);
    }
    //Initializes table mutexes
    pthread_mutexattr_t mattr;
    pthread_mutexattr_init(&mattr);
    pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&table->table_mutex, &mattr);
    //Creates intersections
    for (int i = 0; i < table->num_intersections; i++) {
        Intersection& inter = table->intersections[i];
        
        // Initialize the access control mutex
        pthread_mutex_init(&inter.mutex, &mattr);
        
        // Initialize semaphore if needed
        if (inter.type == 1) { // Semaphore type
            sem_init(&inter.semaphore, 1, inter.capacity);
        }
        
        inter.current_count = 0;
        memset(inter.holding_trains, 0, sizeof(inter.holding_trains));
    }
    //Message queues
    int msgid = msgget(MSG_Q_KEY, IPC_CREAT | 0666);
    if (msgid == -1) {
        perror("msgget failed");
        exit(1);
    }

    std::cout << "Press Enter to shutdown";
    std::cin.ignore();
    
    cleanupIPC();
    return 0;
}
