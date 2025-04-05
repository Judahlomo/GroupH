/*
 File: logger.cpp
 Author: Ryleigh Avila
 Course: CS 4323 – Operating Systems
 Project: Multi-Train Railway Intersection Control System (Group H)
*/

#include "input_parser.h"
#include <fstream>
#include <sstream>
#include <iostream>

// Parses intersections.txt and creates a map of intersection info
// Each line: IntersectionName:Capacity
// e.g., IntersectionA:1 → Mutex, IntersectionD:3 → Semaphore
std::unordered_map<std::string, IntersectionInfo> parse_intersections(const std::string& filename) {
    std::unordered_map<std::string, IntersectionInfo> intersections;
    std::ifstream infile(filename);

    // Check if file opened correctly
    if (!infile.is_open()) {
        std::cerr << "Error opening " << filename << std::endl;
        return intersections;
    }

    std::string line;
    // Read file line by line
    while (std::getline(infile, line)) {
        std::stringstream ss(line);
        std::string name;
        int capacity;

        // Split by colon to get name and capacity
        if (std::getline(ss, name, ':') && ss >> capacity) {
            IntersectionInfo info;
            info.capacity = capacity;

            // Decide whether it's a mutex or semaphore
            info.type = (capacity == 1) ? "MUTEX" : "SEMAPHORE";

            intersections[name] = info;
        }
    }

    return intersections;
}

// Parses trains.txt and returns a list of TrainRoute objects
// Each line: TrainName:IntersectionA,IntersectionB,...
std::vector<TrainRoute> parse_trains(const std::string& filename) {
    std::vector<TrainRoute> trains;
    std::ifstream infile(filename);

    // Check if file opened correctly
    if (!infile.is_open()) {
        std::cerr << "Error opening " << filename << std::endl;
        return trains;
    }

    std::string line;
    // Read file line by line
    while (std::getline(infile, line)) {
        std::stringstream ss(line);
        std::string trainName, routeStr;

        // Split by colon to get train name and route
        if (std::getline(ss, trainName, ':') && std::getline(ss, routeStr)) {
            TrainRoute route;
            route.trainName = trainName;

            // Split the route string by commas into intersections
            std::stringstream routeSS(routeStr);
            std::string intersection;
            while (std::getline(routeSS, intersection, ',')) {
                route.route.push_back(intersection);
            }

            trains.push_back(route);
        }
    }

    return trains;
}
