#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H

#include <fstream> 
#include <string>
#include "graph.h"
#include "emergency.h"
#include "resource.h"
#include "user.h"

using namespace std;

// Replace the 5 path constants at the top of file_handler.h with these:



// Get the directory of the running exe and go up one level to find data/
// This makes paths work whether you run from backend/ or backend/cpp/
inline string getDataPath(const string& filename) {
    // Try backend/data/ first (when called by Node.js)
    ifstream test1("data/" + filename);
    if (test1.is_open()) return "data/" + filename;

    // Try ../data/ (when run directly from backend/cpp/)
    ifstream test2("../data/" + filename);
    if (test2.is_open()) return "../data/" + filename;

    // Default fallback
    return "../data/" + filename;
}

const string PATH_GRAPH       = getDataPath("graph.txt");
const string PATH_LOCATIONS   = getDataPath("locations.txt");
const string PATH_USERS       = getDataPath("users.txt");
const string PATH_RESOURCES   = getDataPath("resources.txt");
const string PATH_EMERGENCIES = getDataPath("emergencies.txt");

// ─────────────────────────────────────────
// FileHandler class
// Central coordinator for all file I/O
// Holds references to all manager objects
// ─────────────────────────────────────────
class FileHandler {
public:
    // References to all managers
    // These are set once at startup in main.cpp
    Graph&            graph;
    EmergencyManager& emergencyMgr;
    ResourceManager&  resourceMgr;
    UserManager&      userMgr;

    // Constructor — binds to the actual manager objects
    FileHandler(Graph& g,
                EmergencyManager& em,
                ResourceManager&  rm,
                UserManager&      um);

    // ── Bulk operations ──────────────────
    // Load everything at startup (call once in main.cpp)
    void loadAll();

    // Save everything (call after any write operation)
    void saveAll();

    // ── Individual loaders ───────────────
    void loadGraph();
    void loadLocations();
    void loadUsers();
    void loadResources();
    void loadEmergencies();

    // ── Individual savers ────────────────
    void saveUsers();
    void saveResources();
    void saveEmergencies();

    // ── Utility ──────────────────────────
    // Checks all data files exist, creates empty ones if missing
    void ensureFilesExist();

    // Prints a startup summary to console
    void printSystemStatus();
};

#endif