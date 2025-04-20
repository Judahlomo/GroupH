/*File: structures.h
Author: Ryeleigh Avila
Course: CS 4323 Operating Systems
Project 4 (Group H)
 // structures.h contributes to shared memory structures for intersections as well as the resource allocation table.
 // Crucial as it processes to mange and synchronize train access to track intersections.
*/

#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <pthread.h>
#include <semaphore.h>

#define MAX_TRAINS 10
#define MAX_NAME 20

// Describes intersection on the track
struct Intersection {
    int type;      // 0 = mutex, 1 = semaphore
    int capacity;  // semaphores
    bool available;  // Available or not
    int occupiedBy;     // Train ID -1 if available
    pthread_mutex_t mutex;       // Mutex for type 0
    sem_t semaphore;      // Semaphore for type 1
    int current_count;               // Number of trains inside
    char holding_trains[MAX_TRAINS][MAX_NAME]; // List of train names
};

// Resource Allocation Table shared between processes
struct ResourceAllocationTable {
    int num_intersections;           // Total intersections
    pthread_mutex_t table_mutex;     // Global lock
    Intersection intersections[10]; // Up to 10 intersections
};

#endif
