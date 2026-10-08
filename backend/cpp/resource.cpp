#include "resource.h"
#include <fstream>
#include <iostream>
#include <climits>

using namespace std;

// ─────────────────────────────────────────
// Resource Constructor
// ─────────────────────────────────────────
Resource::Resource(string id, string type, int currentNode, string status, string owner)
    : id(id), type(type), currentNode(currentNode), status(status), owner(owner) {}

// ─────────────────────────────────────────
// ResourceManager Constructor
// ─────────────────────────────────────────
ResourceManager::ResourceManager() {}

// ─────────────────────────────────────────
// addResource
// Appends a new resource to the vector
// Called when Hospital/FireDept/Police adds a unit
// ─────────────────────────────────────────
// addResource — add owner param
void ResourceManager::addResource(string id, string type,
                                  int currentNode, string owner) {
    if (findById(id) != nullptr) {
        cerr << "Resource " << id << " already exists." << endl;
        return;
    }
    resources.emplace_back(id, type, currentNode, "AVAILABLE", owner);
    cerr << "Added: " << id << " | " << type
         << " | node " << currentNode
         << " | owner: " << owner << endl;
}

// ─────────────────────────────────────────
// updateNode
// Relocates a resource to a new graph node
// This affects Dijkstra distances for future assignments
// ─────────────────────────────────────────
void ResourceManager::updateNode(string resourceId, int newNode) {
    Resource* r = findById(resourceId);
    if (r == nullptr) {
        // cerr << "Resource " << resourceId << " not found." << endl;
        return;
    }
    int oldNode = r->currentNode;
    r->currentNode = newNode;
    // cerr << resourceId << " relocated: node " << oldNode
        //  << " → node " << newNode << endl;
}

// ─────────────────────────────────────────
// updateStatus
// Changes status of a resource
// Called when unit marks EN_ROUTE / REACHED / COMPLETED
// ─────────────────────────────────────────
void ResourceManager::updateStatus(string resourceId, string newStatus) {
    Resource* r = findById(resourceId);
    if (r == nullptr) {
        // cerr << "Resource " << resourceId << " not found." << endl;
        return;
    }
    string oldStatus = r->status;
    r->status = newStatus;

    // When COMPLETED → reset to AVAILABLE for next assignment
    if (newStatus == "COMPLETED") {
        r->status = "AVAILABLE";
        // cerr << resourceId << " completed mission → now AVAILABLE." << endl;
    } else {
        // cerr << resourceId << " status: " << oldStatus
            //  << " → " << newStatus << endl;
    }
}

// ─────────────────────────────────────────
// findById
// Returns pointer to resource with given id
// Returns nullptr if not found
// ─────────────────────────────────────────
Resource* ResourceManager::findById(string resourceId) {
    for (auto& r : resources) {
        if (r.id == resourceId) return &r;
    }
    return nullptr;
}

// ─────────────────────────────────────────
// getByType
// Returns pointers to all resources of a given type
// e.g. getByType("AMBULANCE") → all ambulances
// ─────────────────────────────────────────
vector<Resource*> ResourceManager::getByType(string type) {
    vector<Resource*> result;
    for (auto& r : resources) {
        if (r.type == type) result.push_back(&r);
    }
    return result;
}

// ─────────────────────────────────────────
// selectBestResource  ← THE GREEDY ALGORITHM
//
// Step 1: Map emergency type → resource type
// Step 2: Filter to AVAILABLE resources of that type
// Step 3: For each candidate, run Dijkstra from its node
//         to get distance to emergencyNode
// Step 4: Greedy pick = resource with minimum distance
//
// Returns resource id, or "" if none available
// ─────────────────────────────────────────
string ResourceManager::selectBestResource(string emergencyType,
                                            int emergencyNode,
                                            Graph& graph) {
    // Step 1: map emergency type → required resource type
    string requiredType;
    if      (emergencyType == "MEDICAL") requiredType = "AMBULANCE";
    else if (emergencyType == "FIRE")    requiredType = "FIRETRUCK";
    else if (emergencyType == "CRIME")   requiredType = "POLICE";
    else {
        cerr << "Unknown emergency type: " << emergencyType << endl;
        return "";
    }

    // Step 2: collect AVAILABLE resources of the right type
    vector<Resource*> candidates;
    for (auto& r : resources) {
        if (r.type == requiredType && r.status == "AVAILABLE") {
            candidates.push_back(&r);
        }
    }

    if (candidates.empty()) {
        cerr << "No available " << requiredType << " units." << endl;
        return "";
    }

    // Step 3 + 4: Greedy — find minimum distance
    string bestId = "";
    int bestDist  = INT_MAX;

    for (Resource* r : candidates) {
        // Run Dijkstra from this resource's current node
        vector<int> dist = graph.dijkstra(r->currentNode);

        int d = dist[emergencyNode]; // distance to emergency location

        cerr << r->id << " at node " << r->currentNode
             << " → distance to emergency: " << d << endl;

        // Greedy choice: pick the one with smallest distance
        if (d < bestDist) {
            bestDist = d;
            bestId   = r->id;
        }
    }

    cerr << "✅ Greedy selected: " << bestId
         << " (distance = " << bestDist << ")" << endl;

    return bestId;
}

// ─────────────────────────────────────────
// loadFromFile — 5-field format
// id type currentNode status owner
// AMB_01 AMBULANCE 2 AVAILABLE HospitalA
// ─────────────────────────────────────────
void ResourceManager::loadFromFile(string filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "No resources file — starting fresh." << endl;
        return;
    }
    resources.clear();
    string id, type, status, owner;
    int currentNode;
    while (file >> id >> type >> currentNode >> status >> owner) {
        resources.emplace_back(id, type, currentNode, status, owner);
    }
    file.close();
    cerr << "Loaded " << resources.size() << " resources." << endl;
}

// ─────────────────────────────────────────
// saveToFile
// Writes all resources to resources.txt
// Overwrites the entire file each time
// ─────────────────────────────────────────
void ResourceManager::saveToFile(string filename) {
    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "ERROR: Cannot write to " << filename << endl;
        return;
    }

    for (auto& r : resources) {
        file << r.id          << " "
             << r.type        << " "
             << r.currentNode << " "
             << r.status      << " "
             << r.owner       << "\n";
    }

    file.close();
}

// ─────────────────────────────────────────
// displayAll
// Prints all resources to console (for testing)
// ─────────────────────────────────────────
void ResourceManager::displayAll() {
    cerr << "\n--- All Resources ---" << endl;
    for (auto& r : resources) {
        cerr << r.id
             << " | " << r.type
             << " | Node " << r.currentNode
             << " | " << r.status
             << " | " << r.owner << endl;
    }
    cerr << "---------------------" << endl;
}