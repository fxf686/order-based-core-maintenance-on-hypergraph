#ifndef __HYPERGRAPH__
#define __HYPERGRAPH__

#include <vector>
#include <unordered_set>
#include <unordered_map>

struct vectorHash {
	std::size_t operator()(std::vector<unsigned> const& vec) const {
		std::size_t seed = vec.size();
		for(auto& i: vec) {
			seed ^= i + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		}
		return seed;
	}
};

typedef unsigned Node;
typedef std::vector<Node> Hyperedge;

class Hypergraph{
public:
	Hypergraph();
	unsigned nNodes;
	unsigned nEdges;
	unsigned edgeIdCounter;
	std::vector<Hyperedge> edgePool;
    std::unordered_set<Node> nodeSet;
//	  std::unordered_multimap<Hyperedge, unsigned, vectorHash> edge2id;
//    std::unordered_map<unsigned, Hyperedge> id2edge;
	std::unordered_map<Node, std::unordered_set<unsigned> > eList;
    std::unordered_map<Node, unsigned> degree;
	unsigned insertEdge(const Hyperedge&);
	void deleteEdge(unsigned int);
    void addNode(const Hyperedge&, unsigned int id);
    void removeNode(const Hyperedge&, unsigned  int id);
};
#endif
