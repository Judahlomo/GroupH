// deadlock_resolution.cpp
#include "deadlock_resolution.h"
#include <iostream>
#include <vector>
#include <string>

void resolve_deadlock(const std::vector<std::string>& deadlockedTrains) {
    std::cout << "?? Deadlock detected among trains: ";
    for (const auto& train : deadlockedTrains) {
        std::cout << train << " ";
    }
    std::cout << std::endl;

    if (!deadlockedTrains.empty()) {
        std::string victim = deadlockedTrains.back(); // naive strategy
        std::cout << "?? Preempting train: " << victim << " to resolve deadlock." << std::endl;

        // TODO: notify victim (via message queue or flag)
        // TODO: log this action to simulation.log
    } else {
        std::cout << "No trains provided for resolution." << std::endl;
    }
}
