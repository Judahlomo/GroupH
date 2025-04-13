#include <iostream>
#include <unistd.h>
#include <semaphore.h>      
#include <pthread.h>
#include <vector>
#include <unordered_map>
#include <set>
#include <tuple>
#include <logger.cpp>

using namespace std;
int count = 0;
set<std::tuple<const string&, const string&>> callHistory;
bool hasCalled = false;

// Structure for the RAG
class ResourceAllocationGraph {
public:
    unordered_map<string, set<string>> graph;

    // Add a request edge: Train -> Intersection
    void addRequest(const string& train, const string& intersection) {
        hasCalled = hasBeenCalledBefore(train, intersection);
        if (hasCalled == true) {
            count++;
            if (count > 3) {
                // Circular Wait
                log_event("Deadlock Detected: Circular Wait");
                count = 0;
            }
            else {
                graph[train].insert(intersection); 
            }
        }
        else {
            graph[train].insert(intersection); 
        }
    }

    // Add an allocation edge: Intersection -> Train
    void addAllocation(const string& station, const string& train) {
        graph[station].insert(train);
    }

    // Print the graph
    void printGraph() {
        cout << "Resource Allocation Graph:\n";
        for (const auto& node : graph) {
            for (const auto& neighbor : node.second) {
                cout << node.first << " --> " << neighbor << "\n";
            }
        }
    }
};

bool hasBeenCalledBefore(const string& train, const string& intersection) {
    std::tuple<const string&, const string&> key = std::make_tuple(train, intersection);
    if (callHistory.find(key) != callHistory.end()) {
        return true;
    } else {
        callHistory.insert(key);
        return false;
    }
}

int main() {
    ResourceAllocationGraph rag;

    // Intersection capacities from File 1: intersections.txt
    unordered_map<string, int> intersectionCapacity = {
        {"IntersectionA", 1},
        {"IntersectionB", 2},
        {"IntersectionC", 1},
        {"IntersectionD", 3},
        {"IntersectionE", 1}
    };

    // Train paths from File 2: trains.txt
    unordered_map<string, vector<string>> trainPaths = {
        {"Train1", {"IntersectionA", "IntersectionB", "IntersectionC"}},
        {"Train2", {"IntersectionB", "IntersectionD", "IntersectionE"}},
        {"Train3", {"IntersectionC", "IntersectionD", "IntersectionA"}},
        {"Train4", {"IntersectionE", "IntersectionB", "IntersectionD"}}
    };

    // Simulate RAG (will be removed in the future)
    for (size_t j = 0; j < 3; j++){
        for (const auto& train : trainPaths) {
            const string& trainName = train.first;
            const vector<string>& path = train.second;
            
            if (j == 0) {
                rag.addRequest(trainName, path[j]);
            } else {
                rag.addAllocation(path[j - 1], trainName);
                rag.addRequest(trainName, path[j]);
            }
            rag.addAllocation(path.back(), trainName);
        }
    }

    rag.printGraph();

    return 0;
}
