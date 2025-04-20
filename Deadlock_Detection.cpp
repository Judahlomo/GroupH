// Made by Yosep Lazar

#include <iostream>
#include <unordered_map>
#include <set>
#include <string>
#include <vector>
#include "logger.h"

using namespace std;

class ResourceAllocationGraph {
public:
    unordered_map<string, set<string>> graph;

    // Add edge: train → resource
    void addRequest(const string& train, const string& inter) {
        graph[train].insert(inter);
    }

    // Add edge: resource → train
    void addAllocation(const string& inter, const string& train) {
        graph[inter].insert(train);
    }

    // Naive cycle detector
    bool detectCycleUtil(const string& node, set<string>& visited, set<string>& recStack) {
        if (!visited.count(node)) {
            visited.insert(node);
            recStack.insert(node);

            for (const auto& neighbor : graph[node]) {
                if (!visited.count(neighbor) && detectCycleUtil(neighbor, visited, recStack))
                    return true;
                else if (recStack.count(neighbor))
                    return true;
            }
        }
        recStack.erase(node);
        return false;
    }

    bool detectCycle() {
        set<string> visited;
        set<string> recStack;
        for (const auto& pair : graph)
            if (detectCycleUtil(pair.first, visited, recStack))
                return true;
        return false;
    }

    // Function for printing Resource Allocation Graph (used for testing purposes)
    void printGraph() {
        for (const auto& p : graph)
            for (const auto& dest : p.second)
                cout << p.first << " -> " << dest << endl;
    }
};
