/*
File: IPC_setup
Author: Joseph (Joey) Mecoy
Course: CS 4323 Operating Systems
Project 4 (Group H)
IPC_Setup uses mutexes and semaphores through message queues and shared memory segments
*/

#include <fstream>
#include "structures.h"
//#include "ipc_setup.h"
#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <cstring>
#include <unistd.h>


//keys for shared memory and message queues
const key_t SHM_KEY = 1234;
const key_t MSG_Q_KEY = 5678;
//message queue ID
extern int msgid;

void setupIPC(ResourceAllocationTable*& table) {
    //creates memory segments for the resource allocation table
    int shmid = shmget(SHM_KEY, sizeof(ResourceAllocationTable), IPC_CREAT | 0666);
    if (shmid == -1) { perror("shmget failed"); exit(1); }

    table = (ResourceAllocationTable*)shmat(shmid, NULL, 0);
    if (table == (void*)-1) { perror("shmat failed"); exit(1); }

    //initializes mutex for shated mutexes
    pthread_mutexattr_t mattr;
    pthread_mutexattr_init(&mattr);
    pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&table->table_mutex, &mattr);

    //Reads intersection data file
    std::ifstream interFile("intersections.txt");
    std::string line;
    int count = 0;

    while (std::getline(interFile, line) && count < 10) {
        std::size_t colon = line.find(':');
        if (colon == std::string::npos) continue;

        //retrieves intersection name and capacity
        std::string name = line.substr(0, colon);
        int cap = std::stoi(line.substr(colon + 1));

        Intersection& inter = table->intersections[count];
        inter.capacity = cap;
        inter.available = true;
        inter.occupiedBy = -1;
        inter.current_count = 0;
        memset(inter.holding_trains, 0, sizeof(inter.holding_trains));
        pthread_mutex_init(&inter.mutex, &mattr);
        inter.type = (cap == 1) ? 0 : 1;
        //initializes semaphore if the intersection can hold more than 1 train
        if (inter.type == 1) {
            sem_init(&inter.semaphore, 1, cap);
        }

        count++;
    }

    table->num_intersections = count;

    //message queue creation
    msgid = msgget(MSG_Q_KEY, IPC_CREAT | 0666);
    if (msgid == -1) { perror("msgget failed"); exit(1); }
    std::cout << "IPC setup complete.\n";
}
