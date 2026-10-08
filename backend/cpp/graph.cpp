#include "graph.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <queue>
#include <climits>
using namespace std;

// ─────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────
Graph::Graph() {
    numNodes = 0;
}

// ─────────────────────────────────────────
// loadGraph
// Reads graph.txt line by line
// First line  → numNodes, numEdges
// Other lines → u v w  (edge from u to v with weight w)
// ─────────────────────────────────────────
void Graph::loadGraph(string filename) {
    ifstream file(filename);

    if (!file.is_open()) {
        cerr << "ERROR: Cannot open " << filename << endl;
        return;
    }

    int E;
    file >> numNodes >> E;      // read first line
    adj.resize(numNodes);       // resize adjacency list to numNodes

    int u, v, w;
    while (file >> u >> v >> w) {
        // undirected graph → add edge in both directions
        adj[u].push_back({v, w});
        adj[v].push_back({u, w});
    }

    file.close();
    cerr << "Graph loaded: " << numNodes << " nodes, " << E << " edges." << endl;
}

// ─────────────────────────────────────────
// loadLocations
// Reads locations.txt line by line
// Format → id name x y
// Example → 2 Hospital 100 200
// ─────────────────────────────────────────
void Graph::loadLocations(string filename) {
    ifstream file(filename);

    if (!file.is_open()) {
        cerr << "ERROR: Cannot open " << filename << endl;
        return;
    }

    int id, x, y;
    string name;

    while (file >> id >> name >> x >> y) {
        locationNames[id]   = name;
        locationCoords[id]  = {x, y};
    }

    file.close();
    cerr << "Locations loaded: " << locationNames.size() << " locations." << endl;
}

// ─────────────────────────────────────────
// dijkstra
// Returns dist[] — shortest distance from
// src to every other node in the graph
// Uses min-heap (priority_queue with greater<>)
// Time complexity: O((V + E) log V)
// ─────────────────────────────────────────
vector<int> Graph::dijkstra(int src) {
    // Step 1 — initialise all distances to infinity
    vector<int> dist(numNodes, INT_MAX);
    dist[src] = 0;

    // Step 2 — min-heap stores {distance, node}
    // smallest distance always at top
    priority_queue<pair<int,int>,
                   vector<pair<int,int>>,
                   greater<pair<int,int>>> minHeap;

    minHeap.push({0, src});

    // Step 3 — process until heap is empty
    while (!minHeap.empty()) {
        auto [d, u] = minHeap.top();
        minHeap.pop();

        // stale entry — a shorter path was already found
        if (d > dist[u]) continue;

        // relax all neighbours of u
        for (auto [v, w] : adj[u]) {
            if (dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                minHeap.push({dist[v], v});
            }
        }
    }

    return dist;
}

// ─────────────────────────────────────────
// getPath
// Runs Dijkstra again but also tracks parent[]
// Then backtracks from dest → src to build path
// Returns list of node ids from src to dest
// ─────────────────────────────────────────
vector<int> Graph::getPath(int src, int dest) {
    vector<int> dist(numNodes, INT_MAX);
    vector<int> parent(numNodes, -1);   // parent[i] = which node we came from
    dist[src] = 0;

    priority_queue<pair<int,int>,
                   vector<pair<int,int>>,
                   greater<pair<int,int>>> minHeap;

    minHeap.push({0, src});

    while (!minHeap.empty()) {
        auto [d, u] = minHeap.top();
        minHeap.pop();

        if (d > dist[u]) continue;

        for (auto [v, w] : adj[u]) {
            if (dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                parent[v] = u;          // track where we came from
                minHeap.push({dist[v], v});
            }
        }
    }

    // backtrack from dest to src using parent[]
    vector<int> path;
    for (int node = dest; node != -1; node = parent[node]) {
        path.push_back(node);
    }

    // path is built backwards → reverse it
    reverse(path.begin(), path.end());

    // if path doesn't start at src, no path exists
    if (path.front() != src) return {};

    return path;
}

// ─────────────────────────────────────────
// getLocationName
// Returns name of node id
// Returns "Unknown" if id not in map
// ─────────────────────────────────────────
string Graph::getLocationName(int nodeId) {
    if (locationNames.count(nodeId))
        return locationNames[nodeId];
    return "Unknown";
}

// ─────────────────────────────────────────
// getNodeId
// Returns node id for a given name
// Returns -1 if not found
// ─────────────────────────────────────────
int Graph::getNodeId(string name) {
    for (auto [id, n] : locationNames) {
        if (n == name) return id;
    }
    return -1;
}