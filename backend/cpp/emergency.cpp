#include "emergency.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
using namespace std;

// ─────────────────────────────────────────
// Emergency Constructor
// ─────────────────────────────────────────
Emergency::Emergency(string id, string type, int locationNode,
                     int severity, string status, string assignedUnit)
    : id(id), type(type), locationNode(locationNode),
      severity(severity), status(status), assignedUnit(assignedUnit) {}

// ─────────────────────────────────────────
// EmergencyManager Constructor
// ─────────────────────────────────────────
EmergencyManager::EmergencyManager() {
    emergencyCounter = 0;
}

// ─────────────────────────────────────────
// generateId
// Produces E001, E002, E003 ...
// Uses counter + setw/setfill for zero padding
// ─────────────────────────────────────────
string EmergencyManager::generateId() {
    emergencyCounter++;
    ostringstream ss;
    ss << "E" << setw(3) << setfill('0') << emergencyCounter;
    return ss.str(); // E001, E002 ...
}

// ─────────────────────────────────────────
// addEmergency
// Creates a new Emergency object
// Pushes it into the max-heap
// Also stores it in allEmergencies for history
// ─────────────────────────────────────────
void EmergencyManager::addEmergency(string type, int locationNode, int severity) {
    string id = generateId();
    Emergency e(id, type, locationNode, severity, "PENDING", "");

    emergencyQueue.push(e);      // goes into max-heap → sorted by severity
    allEmergencies.push_back(e); // goes into history list

    cerr << "Emergency added: " << id
         << " | Type: " << type
         << " | Severity: " << severity << "/10"
         << " | Status: PENDING" << endl;
}

// ─────────────────────────────────────────
// processNext
// Pops the highest severity emergency
// Returns it so dispatcher can act on it
// Returns a dummy empty emergency if queue empty
// ─────────────────────────────────────────
Emergency EmergencyManager::processNext() {
    if (emergencyQueue.empty()) {
        cerr << "No pending emergencies in queue." << endl;
        // return a dummy emergency with empty id
        return Emergency("", "", -1, -1, "NONE", "");
    }

    Emergency top = emergencyQueue.top();
    emergencyQueue.pop();

    cerr << "Processing: " << top.id
         << " | Type: " << top.type
         << " | Severity: " << top.severity << "/10"
         << " | Location node: " << top.locationNode << endl;

    return top;
}

// ─────────────────────────────────────────
// markAssigned
// Finds emergency by id in allEmergencies
// Updates status → ASSIGNED
// Stores which unit was assigned
// ─────────────────────────────────────────
void EmergencyManager::markAssigned(string emergencyId, string unitId) {
    for (auto& e : allEmergencies) {
        if (e.id == emergencyId) {
            e.status       = "ASSIGNED";
            e.assignedUnit = unitId;
            cerr << emergencyId << " assigned to " << unitId << endl;
            return;
        }
    }
    cerr << "Emergency " << emergencyId << " not found." << endl;
}

// ─────────────────────────────────────────
// markCompleted
// Finds emergency by id
// Updates status → COMPLETED
// ─────────────────────────────────────────
void EmergencyManager::markCompleted(string emergencyId) {
    for (auto& e : allEmergencies) {
        if (e.id == emergencyId) {
            e.status = "COMPLETED";
            cerr << emergencyId << " marked COMPLETED." << endl;
            return;
        }
    }
    cerr << "Emergency " << emergencyId << " not found." << endl;
}

// ─────────────────────────────────────────
// getByStatus
// Returns a filtered list of emergencies
// matching the given status string
// Used by dispatcher dashboard
// ─────────────────────────────────────────
vector<Emergency> EmergencyManager::getByStatus(string status) {
    vector<Emergency> result;
    for (auto& e : allEmergencies) {
        if (e.status == status) {
            result.push_back(e);
        }
    }
    return result;
}

// ─────────────────────────────────────────
// loadFromFile
// Reads emergencies.txt
// Format → id type locationNode severity status assignedUnit
// Example → E001 FIRE 5 8 PENDING -
// "-" means no unit assigned yet
// Rebuilds priority queue from PENDING ones
// ─────────────────────────────────────────
void EmergencyManager::loadFromFile(string filename) {
    ifstream file(filename);

    if (!file.is_open()) {
        cerr << "No emergencies file found. Starting fresh." << endl;
        return;
    }

    // clear existing data before loading
    allEmergencies.clear();
    while (!emergencyQueue.empty()) emergencyQueue.pop();
    emergencyCounter = 0;

    string id, type, status, assignedUnit;
    int locationNode, severity;

    while (file >> id >> type >> locationNode >> severity >> status >> assignedUnit) {
        // "-" in file means no unit assigned yet → convert to empty string
        if (assignedUnit == "-") assignedUnit = "";

        Emergency e(id, type, locationNode, severity, status, assignedUnit);
        allEmergencies.push_back(e);

        // only PENDING emergencies go back into the queue
        if (status == "PENDING") {
            emergencyQueue.push(e);
        }

        // track highest id seen so new ids don't collide
        // E001 → extract 1, E012 → extract 12
        int num = stoi(id.substr(1));
        if (num > emergencyCounter) emergencyCounter = num;
    }

    file.close();
    cerr << "Loaded " << allEmergencies.size() << " emergencies." << endl;
}

// ─────────────────────────────────────────
// saveToFile
// Writes all emergencies back to file
// Uses "-" for empty assignedUnit
// Overwrites entire file each time
// ─────────────────────────────────────────
void EmergencyManager::saveToFile(string filename) {
    ofstream file(filename);

    if (!file.is_open()) {
        cerr << "ERROR: Cannot write to " << filename << endl;
        return;
    }

    for (auto& e : allEmergencies) {
        string unit = e.assignedUnit.empty() ? "-" : e.assignedUnit;
        file << e.id          << " "
             << e.type        << " "
             << e.locationNode << " "
             << e.severity    << " "
             << e.status      << " "
             << unit          << "\n";
    }

    file.close();
}

// ─────────────────────────────────────────
// displayAll
// Prints all emergencies to console
// Used during testing / debugging
// ─────────────────────────────────────────
void EmergencyManager::displayAll() {
    cerr << "\n--- All Emergencies ---" << endl;
    for (auto& e : allEmergencies) {
        string unit = e.assignedUnit.empty() ? "none" : e.assignedUnit;
        cerr << e.id
             << " | " << e.type
             << " | Node " << e.locationNode
             << " | Severity " << e.severity
             << " | " << e.status
             << " | Unit: " << unit << endl;
    }
    cerr << "----------------------" << endl;
}