// deadlock_resolution.cpp
// Mohammed Omar
// Description: This file handles the deadlock resolution logic for the train simulation and figures out a soloution for them.
#include "deadlock_resolution.h"
#include <iostream>

// Logs the trains that are in the dealock and picks one to preempt 
void resolve_deadlock(const std::vector<std::string>& deadlockedTrains) {
    std::cout << "Deadlock detected among: ";
	// Prints all the trains in the deadlock.
    for (const auto& t : deadlockedTrains)
        std::cout << t << " ";
    std::cout << std::endl;

    if (!deadlockedTrains.empty()) {
	// Log the selected victim.
        std::string victim = deadlockedTrains.back(); // It picks the last train.
        std::cout << "Preempting: " << victim << std::endl;
        // TODO: Sends a message to the victim train for instruction.
    }
}

