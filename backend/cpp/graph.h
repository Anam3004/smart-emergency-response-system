#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
#include <map>
#include <string>
using namespace std;

class Graph {
public:
    int numNodes;

    // adj[i] = list of {neighbourNode, weight} pairs
    vector<vector<pair<int,int>>> adj;

    // node id → location name  (loaded from locations.txt)
    map<int, string> locationNames;

    // node id → {x, y} pixel coords for frontend
    map<int, pair<int,int>> locationCoords;

    // Constructor — initializes an empty graph
    Graph();

    // Reads graph.txt → builds adjacency list
    void loadGraph(string filename);

    // Reads locations.txt → fills locationNames + locationCoords
    void loadLocations(string filename);

    // Core algorithm — returns dist[] from src to every node
    vector<int> dijkstra(int src);

    // Returns the actual path (list of node ids) from src to dest
    // Uses parent[] array tracked during Dijkstra
    vector<int> getPath(int src, int dest);

    // Helper — converts node id to location name
    string getLocationName(int nodeId);

    // Helper — converts location name to node id
    // Returns -1 if not found
    int getNodeId(string name);
};

#endif