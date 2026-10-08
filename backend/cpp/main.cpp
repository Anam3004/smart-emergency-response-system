#include <iostream>
#include <string>
#include <sstream>
#include "graph.h"
#include "emergency.h"
#include "resource.h"
#include "user.h"
#include "file_handler.h"
#include "dispatcher.h"
using namespace std;

// ─────────────────────────────────────────
// JSON helpers
// ─────────────────────────────────────────
string jsonStr(string key, string val) {
    return "\"" + key + "\":\"" + val + "\"";
}
string jsonInt(string key, int val) {
    return "\"" + key + "\":" + to_string(val);
}
string jsonBool(string key, bool val) {
    return "\"" + key + "\":" + (val ? "true" : "false");
}
string jsonStrArray(string key, vector<string>& arr) {
    string out = "\"" + key + "\":[";
    for (int i = 0; i < (int)arr.size(); i++) {
        out += "\"" + arr[i] + "\"";
        if (i < (int)arr.size() - 1) out += ",";
    }
    return out + "]";
}
string jsonIntArray(string key, vector<int>& arr) {
    string out = "\"" + key + "\":[";
    for (int i = 0; i < (int)arr.size(); i++) {
        out += to_string(arr[i]);
        if (i < (int)arr.size() - 1) out += ",";
    }
    return out + "]";
}

// ─────────────────────────────────────────
// DispatchResult → JSON
// ─────────────────────────────────────────
string dispatchResultToJson(DispatchResult& r) {
    string out = "{";
    out += jsonBool("success", r.success)           + ",";
    out += jsonStr("emergencyId", r.emergencyId)    + ",";
    out += jsonStr("emergencyType", r.emergencyType)+ ",";
    out += jsonInt("emergencyNode", r.emergencyNode)+ ",";
    out += jsonInt("severity", r.severity)          + ",";
    out += jsonStr("assignedUnitId", r.assignedUnitId)   + ",";
    out += jsonStr("assignedUnitType", r.assignedUnitType)+ ",";
    out += jsonInt("unitStartNode", r.unitStartNode)+ ",";
    out += jsonIntArray("pathNodes", r.pathNodes)   + ",";
    out += jsonStrArray("pathNames", r.pathNames)   + ",";
    out += jsonInt("totalDistance", r.totalDistance)+ ",";
    out += jsonStr("message", r.message);
    return out + "}";
}

// ─────────────────────────────────────────
// Emergency → JSON
// FIX: now includes locationName (resolved from graph)
// This fixes the e.locationName bug in the frontend
// ─────────────────────────────────────────
string emergencyToJson(Emergency& e, Graph& g) {
    string out = "{";
    out += jsonStr("id", e.id)                                     + ",";
    out += jsonStr("type", e.type)                                 + ",";
    out += jsonInt("locationNode", e.locationNode)                 + ",";
    out += jsonStr("locationName", g.getLocationName(e.locationNode)) + ",";  // ← ADDED
    out += jsonInt("severity", e.severity)                         + ",";
    out += jsonStr("status", e.status)                             + ",";
    out += jsonStr("assignedUnit", e.assignedUnit);
    return out + "}";
}

// ─────────────────────────────────────────
// Resource → JSON
// FIX: now includes owner field
// ─────────────────────────────────────────
string resourceToJson(Resource& r, Graph& g) {
    string out = "{";
    out += jsonStr("id", r.id)                                         + ",";
    out += jsonStr("type", r.type)                                     + ",";
    out += jsonInt("currentNode", r.currentNode)                       + ",";
    out += jsonStr("currentLocation", g.getLocationName(r.currentNode))+ ",";  // ← location name
    out += jsonStr("status", r.status)                                 + ",";
    out += jsonStr("owner", r.owner);
    return out + "}";
}

// ─────────────────────────────────────────
// main
// ─────────────────────────────────────────
int main(int argc, char* argv[]) {

    // Boot — silence loading messages
    Graph g; EmergencyManager em; ResourceManager rm; UserManager um;
    FileHandler fh(g, em, rm, um);

    streambuf* oldCout = cout.rdbuf();
    ostringstream devNull;
    cout.rdbuf(devNull.rdbuf());
    fh.loadAll();
    cout.rdbuf(oldCout);

    Dispatcher d(g, em, rm);

    if (argc < 2) {
        cout << "{\"status\":\"ok\",\"nodes\":" << g.numNodes << "}" << endl;
        return 0;
    }

    string cmd = argv[1];

    // ── ping ─────────────────────────────
    if (cmd == "ping") {
        cout << "{\"status\":\"ok\"}" << endl;
    }

    // ── login ────────────────────────────
    // Usage: emergency.exe login <username> <password>
    // Returns: {success, role, unitName, username}
    else if (cmd == "login" && argc == 4) {
        auto [role, unitName] = um.login(argv[2], argv[3]);

        if (role.empty()) {
            cout << "{\"success\":false}" << endl;
        } else {
            cout << "{"
                 << jsonBool("success", true)      << ","
                 << jsonStr("role", role)           << ","
                 << jsonStr("unitName", unitName)   << ","
                 << jsonStr("username", argv[2])
                 << "}" << endl;
        }
    }

    // ── register ─────────────────────────
    // Usage: emergency.exe register <user> <role> <unitName> <pass>
    else if (cmd == "register" && argc == 6) {
        bool ok = um.registerUser(argv[2], argv[3], argv[4], argv[5]);
        if (ok) fh.saveUsers();
        cout << "{" << jsonBool("success", ok) << "}" << endl;
    }

    // ── add emergency ────────────────────
    else if (cmd == "add" && argc == 5) {
        d.addEmergency(argv[2], stoi(argv[3]), stoi(argv[4]));
        fh.saveEmergencies();
        cout << "{\"success\":true}" << endl;
    }

    // ── process ──────────────────────────
    else if (cmd == "process") {
        DispatchResult result = d.processEmergency();
        fh.saveResources();
        fh.saveEmergencies();
        cout << dispatchResultToJson(result) << endl;
    }

    // ── status update ────────────────────
    else if (cmd == "status" && argc == 5) {
        string unitId      = argv[2];
        string newStatus   = argv[3];
        string emergencyId = argv[4];

        if      (newStatus == "EN_ROUTE")  d.markEnRoute(unitId);
        else if (newStatus == "REACHED")   d.markReached(unitId);
        else if (newStatus == "COMPLETED") d.markCompleted(unitId, emergencyId);

        fh.saveResources();
        fh.saveEmergencies();
        cout << "{\"success\":true}" << endl;
    }

    // ── emergencies list ─────────────────
    // Returns locationName in each object (fixes frontend bug)
    else if (cmd == "emergencies") {
        vector<Emergency> list = (argc == 3)
            ? d.getEmergenciesByStatus(argv[2])
            : d.getAllEmergencies();

        cout << "[";
        for (int i = 0; i < (int)list.size(); i++) {
            cout << emergencyToJson(list[i], g);      // pass g for name lookup
            if (i < (int)list.size() - 1) cout << ",";
        }
        cout << "]" << endl;
    }

    // ── resources list ───────────────────
    // Optional filter by type AND/OR owner
    // Usage: emergency.exe resources
    //        emergency.exe resources AMBULANCE
    //        emergency.exe resources AMBULANCE HospitalA
    else if (cmd == "resources") {
        string filterType  = (argc >= 3) ? argv[2] : "";
        string filterOwner = (argc >= 4) ? argv[3] : "";

        vector<Resource> list;
        for (auto& r : rm.resources) {
            bool typeOk  = filterType.empty()  || r.type  == filterType;
            bool ownerOk = filterOwner.empty() || r.owner == filterOwner;
            if (typeOk && ownerOk) list.push_back(r);
        }

        cout << "[";
        for (int i = 0; i < (int)list.size(); i++) {
            cout << resourceToJson(list[i], g);
            if (i < (int)list.size() - 1) cout << ",";
        }
        cout << "]" << endl;
    }

    // ── add unit ─────────────────────────
    // Usage: emergency.exe addunit AMB_03 AMBULANCE 2 HospitalA
    else if (cmd == "addunit" && argc == 6) {
        rm.addResource(argv[2], argv[3], stoi(argv[4]), argv[5]);
        fh.saveResources();
        cout << "{\"success\":true}" << endl;
    }

    // ── update node ──────────────────────
    else if (cmd == "updatenode" && argc == 4) {
        rm.updateNode(argv[2], stoi(argv[3]));
        fh.saveResources();
        cout << "{\"success\":true}" << endl;
    }

    // ── path between nodes ───────────────
    // Returns pathNames, pathNodes (int array), distance
    // pathNodes added so frontend can highlight edges
    else if (cmd == "path" && argc == 4) {
        int from = stoi(argv[2]);
        int to   = stoi(argv[3]);

        vector<int>    pathNodes = g.getPath(from, to);
        vector<string> pathNames;
        for (int n : pathNodes) pathNames.push_back(g.getLocationName(n));
        int dist = d.getDistance(from, to);

        cout << "{";
        cout << jsonIntArray("pathNodes", pathNodes) << ",";
        cout << jsonStrArray("pathNames", pathNames) << ",";
        cout << jsonInt("distance", dist);
        cout << "}" << endl;
    }

    // ── graph nodes ──────────────────────
    else if (cmd == "graph") {
        cout << "[";
        bool first = true;
        for (auto& [id, name] : g.locationNames) {
            if (!first) cout << ",";
            first = false;
            auto [x, y] = g.locationCoords[id];
            cout << "{"
                 << jsonInt("id", id)     << ","
                 << jsonStr("name", name) << ","
                 << jsonInt("x", x)       << ","
                 << jsonInt("y", y)
                 << "}";
        }
        cout << "]" << endl;
    }

    // ── graph edges ──────────────────────
    else if (cmd == "edges") {
        cout << "[";
        bool first = true;
        for (int u = 0; u < g.numNodes; u++) {
            for (auto [v, w] : g.adj[u]) {
                if (u < v) {
                    if (!first) cout << ",";
                    first = false;
                    cout << "{"
                         << jsonInt("from", u)   << ","
                         << jsonInt("to", v)     << ","
                         << jsonInt("weight", w)
                         << "}";
                }
            }
        }
        cout << "]" << endl;
    }

    else {
        cout << "{\"error\":\"unknown command: " << cmd << "\"}" << endl;
        return 1;
    }

    return 0;
}
