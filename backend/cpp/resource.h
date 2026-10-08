#ifndef RESOURCE_H
#define RESOURCE_H

#include <string>
#include <vector>
#include "graph.h"

using namespace std;

// ─────────────────────────────────────────
// Resource struct
// One object = one vehicle/unit
// ─────────────────────────────────────────
struct Resource {
    string id;          // AMB_01, TRUCK_01, COP_01 ...
    string type;        // AMBULANCE / FIRETRUCK / POLICE
    int currentNode;    // which graph node it is currently at
    string status;      // AVAILABLE / EN_ROUTE / REACHED / COMPLETED
    string owner;       // HospitalA / HospitalB / FireStationA / PolicePrecinct1

    // Constructor
    Resource(string id, string type, int currentNode, string status, string owner = "");
};

// ─────────────────────────────────────────
// ResourceManager class
// Owns all resources + greedy selection logic
// ─────────────────────────────────────────
class ResourceManager {
public:
    vector<Resource> resources;

    // Constructor
    ResourceManager();

    // Add a new resource (used by Hospital/FireDept/Police to add units)
    void addResource(string id, string type, int currentNode,string owner);

    // Update the current node of a resource (relocation)
    void updateNode(string resourceId, int newNode);

    // Update status of a resource
    void updateStatus(string resourceId, string newStatus);

    // Returns pointer to resource by id, nullptr if not found
    Resource* findById(string resourceId);

    // Returns all resources of a given type
    vector<Resource*> getByType(string type);

    // ─────────────────────────────────────
    // GREEDY SELECTOR
    // Given:
    //   - emergencyType → determines which resource type to use
    //   - emergencyNode → where the emergency is
    //   - graph         → to run Dijkstra from each resource
    // Returns:
    //   - id of the best (nearest available) resource
    //   - empty string if none available
    // ─────────────────────────────────────
    string selectBestResource(string emergencyType, int emergencyNode, Graph& graph);

    // Loads resources.txt into resources vector
    void loadFromFile(string filename);

    // Saves all resources back to resources.txt
    void saveToFile(string filename);

    // Prints all resources to console (for testing)
    void displayAll();
};

#endif