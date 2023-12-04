#include "Hypergraph.hpp"
#include <iostream>
#include<list>
#include<algorithm>
using namespace std;

Hypergraph::Hypergraph() {
	nNodes = nEdges = edgeIdCounter = 0;
}

unsigned Hypergraph::insertEdge(const Hyperedge& e) {
	// Insert an edge to the hypergraph
	for (const Node u: e) {
        nodeSet.insert(u);
        eList[u].insert(edgeIdCounter);
        degree[u]++;
    }
	edgePool.push_back(e);
	++nEdges;
	return edgeIdCounter++;
}

void Hypergraph::deleteEdge(const unsigned int id) {
	// Delete an edge from the hypergraph
	for (const Node u: edgePool[id]) {
        eList[u].erase(id);
        degree[u]--;
    }
	edgePool[id].clear();
	--nEdges;
}

void Hypergraph::addNode(const Hyperedge &e, unsigned int id) {
    // Delete an edge from the hypergraph
    if(id >= edgePool.size()) {
        edgePool.push_back(e);
        for (const Node u: e) {
            nodeSet.insert(u);
            eList[u].insert(id);
            degree[u]++;
        }
        ++nEdges;
        edgeIdCounter++;
    } else {
        for (const Node u: e) {
            if (!nodeSet.count(u))
                nodeSet.insert(u);
            unsigned int v1 = eList[u].size();
            eList[u].insert(id);
            unsigned int v2 = eList[u].size();
            if (v2 - v1)
                degree[u]++;
            edgePool[id].push_back(u);
        }
    }
}

void Hypergraph::removeNode(const Hyperedge &e, unsigned int id) {
    // Delete an edge from the hypergraph
    for (const Node u: e) {
        eList[u].erase(id);
        degree[u]--;
        std::remove(edgePool[id].begin(), edgePool[id].end(),u);
    }
    if(edgePool[id].size() == 0){
        --nEdges;
        edgePool[id].clear();
    }
}