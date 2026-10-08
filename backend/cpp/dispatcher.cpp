#include "dispatcher.h"
#include <iostream>
#include <climits>

using namespace std;

// ─────────────────────────────────────────
// DispatchResult default constructor
// Sets success = false so caller can check
// ─────────────────────────────────────────
DispatchResult::DispatchResult()
    : success(false),
      emergencyNode(-1),
      severity(-1),
      unitStartNode(-1),
      totalDistance(-1) {}

// ─────────────────────────────────────────
// Dispatcher Constructor
// Binds to live objects — no copying
// ─────────────────────────────────────────
Dispatcher::Dispatcher(Graph& g, EmergencyManager& em, ResourceManager& rm)
    : graph(g), emergencyMgr(em), resourceMgr(rm) {}

// ─────────────────────────────────────────
// addEmergency
// Creates a new emergency and pushes it
// into the max-heap priority queue
// Called when dispatcher fills in the form
// ─────────────────────────────────────────
void Dispatcher::addEmergency(string type, int locationNode, int severity) {
    // Validate emergency type
    if (type != "MEDICAL" && type != "FIRE" && type != "CRIME") {
        cerr << "Invalid emergency type: " << type
             << ". Use MEDICAL / FIRE / CRIME." << endl;
        return;
    }

    // Validate location node exists in graph
    if (locationNode < 0 || locationNode >= graph.numNodes) {
        cerr << "Invalid location node: " << locationNode << endl;
        return;
    }

    // Validate severity range
    if (severity < 1 || severity > 10) {
        cerr << "Severity must be between 1 and 10." << endl;
        return;
    }

    emergencyMgr.addEmergency(type, locationNode, severity);

    cerr << "Emergency queued at: "
         << graph.getLocationName(locationNode)
         << " | Severity: " << severity << "/10" << endl;
}

// ─────────────────────────────────────────
// processEmergency  ← THE MAIN FUNCTION
//
// This is where all 4 algorithms meet:
//
//  1. Priority Queue  → pop highest severity emergency
//  2. Dijkstra        → run from each candidate resource
//  3. Greedy          → pick resource with min distance
//  4. Path tracking   → reconstruct route for display
//
// Returns a DispatchResult with everything
// the frontend needs to display the outcome
// ─────────────────────────────────────────
DispatchResult Dispatcher::processEmergency() {
    DispatchResult result;

    // ── Step 1: Priority Queue pop ───────
    // Gets the highest severity PENDING emergency
    // O(log n) operation on the max-heap
    Emergency emergency = emergencyMgr.processNext();

    if (emergency.id.empty()) {
        result.message = "No pending emergencies in queue.";
        return result; // success = false
    }

    result.emergencyId   = emergency.id;
    result.emergencyType = emergency.type;
    result.emergencyNode = emergency.locationNode;
    result.severity      = emergency.severity;

    cerr << "\n── Processing Emergency ──" << endl;
    cerr << "ID       : " << emergency.id << endl;
    cerr << "Type     : " << emergency.type << endl;
    cerr << "Location : " << graph.getLocationName(emergency.locationNode)
         << " (node " << emergency.locationNode << ")" << endl;
    cerr << "Severity : " << emergency.severity << "/10" << endl;

    // ── Step 2 + 3: Dijkstra + Greedy ───
    // selectBestResource internally:
    //   - filters by type + AVAILABLE status
    //   - runs Dijkstra from each candidate's node
    //   - greedily picks the one with min distance
    string bestUnitId = resourceMgr.selectBestResource(
        emergency.type,
        emergency.locationNode,
        graph
    );

    if (bestUnitId.empty()) {
        // No available unit of the required type
        result.message = "No available unit for " + emergency.type
                       + " emergency. Emergency re-queued.";

        // Re-queue the emergency so it's not lost
        emergencyMgr.emergencyQueue.push(emergency);

        cerr << result.message << endl;
        return result; // success = false
    }

    // Get the full resource object for result details
    Resource* unit = resourceMgr.findById(bestUnitId);
    result.assignedUnitId   = unit->id;
    result.assignedUnitType = unit->type;
    result.unitStartNode    = unit->currentNode;

    // ── Step 4: Path reconstruction ─────
    // getPath() runs Dijkstra again with parent[]
    // tracking to reconstruct the exact route
    vector<int> path = graph.getPath(unit->currentNode,
                                      emergency.locationNode);

    // Compute total distance along the path
    int totalDist = 0;
    for (int i = 0; i + 1 < (int)path.size(); i++) {
        vector<int> d = graph.dijkstra(path[i]);
        totalDist += d[path[i + 1]];
    }

    result.pathNodes     = path;
    result.totalDistance = totalDist;

    // Convert node ids to location names for display
    for (int nodeId : path) {
        result.pathNames.push_back(graph.getLocationName(nodeId));
    }

    // ── Step 5: Update system state ─────
    // Mark emergency as ASSIGNED
    emergencyMgr.markAssigned(emergency.id, bestUnitId);

    // Mark unit as EN_ROUTE (locks it from reassignment)
    resourceMgr.updateStatus(bestUnitId, "EN_ROUTE");

    // ── Build result summary ─────────────
    result.success = true;

    result.message = "Dispatched " + unit->id
                   + " to " + graph.getLocationName(emergency.locationNode)
                   + " via ";

    for (int i = 0; i < (int)result.pathNames.size(); i++) {
        result.message += result.pathNames[i];
        if (i < (int)result.pathNames.size() - 1)
            result.message += " → ";
    }

    result.message += " | ETA: " + to_string(totalDist) + " min";

    // ── Console output ───────────────────
    cerr << "\n✅ DISPATCH RESULT" << endl;
    cerr << "Unit     : " << unit->id << " (" << unit->type << ")" << endl;
    cerr << "Route    : ";
    for (int i = 0; i < (int)result.pathNames.size(); i++) {
        cerr << result.pathNames[i];
        if (i < (int)result.pathNames.size() - 1) cerr << " → ";
    }
    cerr << endl;
    cerr << "Distance : " << totalDist << " min" << endl;
    cerr << "─────────────────────────" << endl;

    return result;
}

// ─────────────────────────────────────────
// markEnRoute
// Called by unit's dashboard when they
// confirm they are moving to the scene
// ─────────────────────────────────────────
void Dispatcher::markEnRoute(string resourceId) {
    Resource* r = resourceMgr.findById(resourceId);
    if (r == nullptr) {
        cerr << "Unit " << resourceId << " not found." << endl;
        return;
    }
    if (r->status != "EN_ROUTE") {
        // Allow re-confirmation or late mark
        resourceMgr.updateStatus(resourceId, "EN_ROUTE");
    }
    cerr << resourceId << " is EN_ROUTE to the scene." << endl;
}

// ─────────────────────────────────────────
// markReached
// Called by unit's dashboard when they
// arrive at the emergency location
// ─────────────────────────────────────────
void Dispatcher::markReached(string resourceId) {
    Resource* r = resourceMgr.findById(resourceId);
    if (r == nullptr) {
        cerr << "Unit " << resourceId << " not found." << endl;
        return;
    }
    resourceMgr.updateStatus(resourceId, "REACHED");
    cerr << resourceId << " has REACHED the scene." << endl;
}

// ─────────────────────────────────────────
// markCompleted
// Called when unit finishes the response
// Frees the unit back to AVAILABLE
// Also closes the linked emergency
// ─────────────────────────────────────────
void Dispatcher::markCompleted(string resourceId, string emergencyId) {
    // Free the unit — updateStatus sets COMPLETED → AVAILABLE
    resourceMgr.updateStatus(resourceId, "COMPLETED");

    // Close the emergency
    emergencyMgr.markCompleted(emergencyId);

    cerr << "Mission complete: " << resourceId
         << " handled " << emergencyId
         << " → unit now AVAILABLE." << endl;
}

// ─────────────────────────────────────────
// getAllEmergencies
// Returns full list for dispatcher dashboard
// ─────────────────────────────────────────
vector<Emergency> Dispatcher::getAllEmergencies() {
    return emergencyMgr.allEmergencies;
}

// ─────────────────────────────────────────
// getEmergenciesByStatus
// Returns filtered list by status string
// e.g. "PENDING" / "ASSIGNED" / "COMPLETED"
// ─────────────────────────────────────────
vector<Emergency> Dispatcher::getEmergenciesByStatus(string status) {
    return emergencyMgr.getByStatus(status);
}

// ─────────────────────────────────────────
// getAllResources
// Returns full resource list
// Used by dispatcher overview panel
// ─────────────────────────────────────────
vector<Resource> Dispatcher::getAllResources() {
    return resourceMgr.resources;
}

// ─────────────────────────────────────────
// getResourcesByType
// Returns only resources of a given type
// Hospital calls this with "AMBULANCE"
// FireDept with "FIRETRUCK", Police with "POLICE"
// ─────────────────────────────────────────
vector<Resource> Dispatcher::getResourcesByType(string type) {
    vector<Resource> result;
    for (auto& r : resourceMgr.resources) {
        if (r.type == type) result.push_back(r);
    }
    return result;
}

// ─────────────────────────────────────────
// getPathNames
// Returns route as location name strings
// e.g. ["FireStation", "CentralPark", "MarketSquare"]
// Used by any role's "View Route" button
// ─────────────────────────────────────────
vector<string> Dispatcher::getPathNames(int fromNode, int toNode) {
    vector<int> path = graph.getPath(fromNode, toNode);
    vector<string> names;
    for (int nodeId : path) {
        names.push_back(graph.getLocationName(nodeId));
    }
    return names;
}

// ─────────────────────────────────────────
// getDistance
// Returns shortest distance between two nodes
// Uses Dijkstra — O((V+E) log V)
// ─────────────────────────────────────────
int Dispatcher::getDistance(int fromNode, int toNode) {
    vector<int> dist = graph.dijkstra(fromNode);
    if (toNode < 0 || toNode >= (int)dist.size()) return -1;
    return dist[toNode];
}