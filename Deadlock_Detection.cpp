#include <iostream>
#include <unistd.h>
#include <semaphore.h>      
#include <pthread.h>
#include <vector>
#include <unordered_map>
#include <set>

using namespace std;
int countA = 0;
int countB = 0;
int countC = 0;
int countD = 0;
int countE = 0;



// Structure for the RAG
class ResourceAllocationGraph {
public:
    unordered_map<string, set<string>> graph;

    // Add a request edge: Train -> Intersection
    void addRequest(const string& train, const string& intersection) {
        graph[train].insert(intersection);   
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
    for (const auto& train : trainPaths) {
        const string& trainName = train.first;
        const vector<string>& path = train.second;

        for (size_t i = 0; i < path.size(); ++i) {
            if (i == 0) {
                rag.addRequest(trainName, path[i]);
            } else {
                rag.addAllocation(path[i - 1], trainName);
                rag.addRequest(trainName, path[i]);
            }
        }
        rag.addAllocation(path.back(), trainName);
    }
    rag.printGraph();

    return 0;
}
