#ifndef EMERGENCY_H
#define EMERGENCY_H

#include <string>
#include <queue> 
#include <vector>
using namespace std;

// ─────────────────────────────────────────
// Emergency struct
// One object = one reported emergency
// ─────────────────────────────────────────
struct Emergency {
    string id;           // E001, E002 ...
    string type;         // MEDICAL / FIRE / CRIME
    int    locationNode; // node id from graph (e.g. 5 = MarketSquare)
    int    severity;     // 1–10 (10 = most critical)
    string status;       // PENDING / ASSIGNED / COMPLETED
    string assignedUnit; // resource id assigned (empty if PENDING)

    // Constructor
    Emergency(string id, string type, int locationNode,
              int severity, string status = "PENDING",
              string assignedUnit = "");
};

// ─────────────────────────────────────────
// Comparator for max-heap
// Higher severity = higher priority
// ─────────────────────────────────────────
struct CompareEmergency {
    bool operator()(Emergency& a, Emergency& b) {
        return a.severity < b.severity;
    }
};

// ─────────────────────────────────────────
// EmergencyManager class
// Owns the priority queue + all operations
// ─────────────────────────────────────────
class EmergencyManager {
public:
    // Max-heap ordered by severity
    priority_queue<Emergency,
                   vector<Emergency>,
                   CompareEmergency> emergencyQueue;

    // All emergencies ever created (for history/display)
    vector<Emergency> allEmergencies;

    // Counter to generate unique IDs like E001, E002
    int emergencyCounter;

    // Constructor
    EmergencyManager();

    // Generates next id → E001, E002 ...
    string generateId();

    // Creates emergency + pushes to priority queue
    void addEmergency(string type, int locationNode, int severity);

    // Pops highest priority emergency from queue
    // Returns the emergency that was popped
    // Returns empty emergency if queue is empty
    Emergency processNext();

    // Marks an emergency as ASSIGNED
    // Stores which unit was assigned
    void markAssigned(string emergencyId, string unitId);

    // Marks an emergency as COMPLETED
    void markCompleted(string emergencyId);

    // Returns all emergencies with a given status
    vector<Emergency> getByStatus(string status);

    // Loads emergencies.txt into allEmergencies
    // Also rebuilds priority queue from PENDING ones
    void loadFromFile(string filename);

    // Saves all emergencies back to emergencies.txt
    void saveToFile(string filename);

    // Prints all emergencies to console (for testing)
    void displayAll();
};

#endif