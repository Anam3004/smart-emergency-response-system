#ifndef DISPATCHER_H
#define DISPATCHER_H

#include <string>
#include <vector>
#include "graph.h"
#include "emergency.h"
#include "resource.h"
#include "user.h"

using namespace std;

// ─────────────────────────────────────────
// DispatchResult struct
// Returned by processEmergency()
// Contains everything the frontend needs
// to display the result of one dispatch
// ─────────────────────────────────────────
struct DispatchResult {
    bool success;            // false if no unit available or queue empty

    // Emergency details
    string emergencyId;
    string emergencyType;
    int    emergencyNode;
    int    severity;

    // Assigned unit details
    string assignedUnitId;
    string assignedUnitType;
    int    unitStartNode;    // where the unit was when assigned

    // Route computed by Dijkstra + path reconstruction
    vector<int>    pathNodes;      // node ids: [1, 4, 5]
    vector<string> pathNames;      // names:    ["FireStation", "CentralPark", "MarketSquare"]
    int            totalDistance;  // total travel time/weight

    string message; // human-readable summary for frontend display

    DispatchResult(); // default: success = false
};

// ─────────────────────────────────────────
// Dispatcher class
// Orchestrates the full emergency response
// workflow using all other components
// ─────────────────────────────────────────
class Dispatcher {
public:
    // References to all system components
    Graph&            graph;
    EmergencyManager& emergencyMgr;
    ResourceManager&  resourceMgr;

    // Constructor — binds to live component objects
    Dispatcher(Graph& g, EmergencyManager& em, ResourceManager& rm);

    // ── Core workflow ────────────────────

    // Add a new emergency to the priority queue
    // Called when dispatcher enters emergency details
    void addEmergency(string type, int locationNode, int severity);

    // THE MAIN FUNCTION
    // Pops highest priority emergency → runs Dijkstra
    // → greedy selects unit → assigns → returns full result
    DispatchResult processEmergency();

    // ── Unit status updates ──────────────
    // Called by hospital/firedept/police dashboards
    // as their unit progresses through the response

    void markEnRoute(string resourceId);
    void markReached(string resourceId);

    // Marks unit COMPLETED + frees it back to AVAILABLE
    // Also marks the linked emergency as COMPLETED
    void markCompleted(string resourceId, string emergencyId);

    // ── View helpers ─────────────────────

    // Returns all emergencies (for dispatcher dashboard)
    vector<Emergency> getAllEmergencies();

    // Returns emergencies filtered by status
    vector<Emergency> getEmergenciesByStatus(string status);

    // Returns all resources (for dispatcher overview)
    vector<Resource>  getAllResources();

    // Returns resources filtered by type
    // Used by each department to see only their units
    vector<Resource>  getResourcesByType(string type);

    // Returns the shortest path between two nodes as names
    // Used by any role to display their route
    vector<string> getPathNames(int fromNode, int toNode);

    // Returns total distance between two nodes
    int getDistance(int fromNode, int toNode);
};

#endif