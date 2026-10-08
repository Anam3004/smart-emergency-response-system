#include "file_handler.h"
#include <iostream>
#include <fstream>

using namespace std;

// ─────────────────────────────────────────
// Constructor
// Binds FileHandler to the live manager objects
// Uses initializer list — no copying, direct binding
// ─────────────────────────────────────────
FileHandler::FileHandler(Graph& g,
                         EmergencyManager& em,
                         ResourceManager&  rm,
                         UserManager&      um)
    : graph(g), emergencyMgr(em), resourceMgr(rm), userMgr(um) {}

// ─────────────────────────────────────────
// ensureFilesExist
// Creates empty data files if they don't exist yet
// Prevents crashes on first run when no data files exist
// Called before loadAll()
// ─────────────────────────────────────────
void FileHandler::ensureFilesExist() {
    // List of all files that must exist
    // graph.txt and locations.txt must be pre-filled — we don't create them
    // but we do create the others as empty files so loaders don't crash
    auto touch = [](const string& path) {
        ifstream check(path);
        if (!check.is_open()) {
            ofstream create(path); // creates empty file
            cerr << "Created empty file: " << path << endl;
        }
    };

    touch(PATH_USERS);
    touch(PATH_RESOURCES);
    touch(PATH_EMERGENCIES);

    // graph.txt and locations.txt must already exist with data
    // warn the user if they're missing
    ifstream gcheck(PATH_GRAPH);
    if (!gcheck.is_open()) {
        cerr << "WARNING: " << PATH_GRAPH
             << " not found! Graph will be empty." << endl;
    }

    ifstream lcheck(PATH_LOCATIONS);
    if (!lcheck.is_open()) {
        cerr << "WARNING: " << PATH_LOCATIONS
             << " not found! Location names will be missing." << endl;
    }
}

// ─────────────────────────────────────────
// loadAll
// Boots the entire system from disk
// Call once at the start of main.cpp
// Order matters: graph must load before anything
// that depends on node ids
// ─────────────────────────────────────────
void FileHandler::loadAll() {
    cerr << "\n========== Loading System Data ==========" << endl;

    ensureFilesExist();

    loadGraph();        // must be first — everything uses node ids
    loadLocations();    // names + coords for graph nodes
    loadUsers();        // credentials for login
    loadResources();    // all units and their status
    loadEmergencies();  // pending + historical emergencies

    cerr << "========== System Ready ==========" << endl;

    printSystemStatus();
}

// ─────────────────────────────────────────
// saveAll
// Persists entire system state to disk
// Call after any write operation if you want
// to save everything at once
// Note: graph and locations are never written
// back — they are read-only after startup
// ─────────────────────────────────────────
void FileHandler::saveAll() {
    cerr << "\n========== Saving System Data ==========" << endl;
    saveUsers();
    saveResources();
    saveEmergencies();
    cerr << "========== Save Complete ==========" << endl;
}

// ─────────────────────────────────────────
// Individual loaders
// Each delegates to the correct manager
// ─────────────────────────────────────────
void FileHandler::loadGraph() {
    graph.loadGraph(PATH_GRAPH);
}

void FileHandler::loadLocations() {
    graph.loadLocations(PATH_LOCATIONS);
}

void FileHandler::loadUsers() {
    userMgr.loadFromFile(PATH_USERS);
}

void FileHandler::loadResources() {
    resourceMgr.loadFromFile(PATH_RESOURCES);
}

void FileHandler::loadEmergencies() {
    emergencyMgr.loadFromFile(PATH_EMERGENCIES);
}

// ─────────────────────────────────────────
// Individual savers
// Each delegates to the correct manager
// Graph and locations are never saved back
// — they are static city data
// ─────────────────────────────────────────
void FileHandler::saveUsers() {
    userMgr.saveToFile(PATH_USERS);
}

void FileHandler::saveResources() {
    resourceMgr.saveToFile(PATH_RESOURCES);
}

void FileHandler::saveEmergencies() {
    emergencyMgr.saveToFile(PATH_EMERGENCIES);
}

// ─────────────────────────────────────────
// printSystemStatus
// Prints a clean startup summary to console
// Shows counts of loaded data — useful for
// verifying everything loaded correctly
// ─────────────────────────────────────────
void FileHandler::printSystemStatus() {
    cerr << "\n┌─────────────────────────────────┐" << endl;
    cerr << "│    SMART EMERGENCY RESPONSE     │" << endl;
    cerr << "│         SYSTEM STATUS           │" << endl;
    cerr << "├─────────────────────────────────┤" << endl;
    cerr << "│ Graph nodes  : "
         << graph.numNodes
         << string(18 - to_string(graph.numNodes).length(), ' ')
         << "│" << endl;
    cerr << "│ Locations    : "
         << graph.locationNames.size()
         << string(18 - to_string(graph.locationNames.size()).length(), ' ')
         << "│" << endl;
    cerr << "│ Users        : "
         << userMgr.users.size()
         << string(18 - to_string(userMgr.users.size()).length(), ' ')
         << "│" << endl;
    cerr << "│ Resources    : "
         << resourceMgr.resources.size()
         << string(18 - to_string(resourceMgr.resources.size()).length(), ' ')
         << "│" << endl;
    cerr << "│ Emergencies  : "
         << emergencyMgr.allEmergencies.size()
         << string(18 - to_string(emergencyMgr.allEmergencies.size()).length(), ' ')
         << "│" << endl;
    cerr << "└─────────────────────────────────┘" << endl;
}