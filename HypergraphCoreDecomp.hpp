#ifndef __HYPERGRAPHCOREDECOMP__
#define __HYPERGRAPHCOREDECOMP__

#include "Hypergraph.hpp"
#include <unordered_map>
#include <list>
#include <set>
#include "Splay.cpp"

class HypergraphCoreDecomp {
public:
	HypergraphCoreDecomp(const Hypergraph&);
	void solve();
	std::unordered_map<Node, int> cv;
    std::vector<Node> edge_node;
    std::vector<std::list<Node>>  O;//order序列。
    std::unordered_map<Node, std::list<Node>::iterator> iterToO;
    std::vector<SplayTree<Node>> A; // Data structure A //存储核值以及order；伸展树的形式。
    std::unordered_map<Node, SplayNode<Node> *> pointerToA;
    std::unordered_map<Node, unsigned> degPlus, degStar;
    std::unordered_map<unsigned, int> ce;
private:
	const Hypergraph& h;
	std::unordered_map<Node, int> deg;
	std::unordered_set<unsigned> erasedEdgeIds;

//    std::vector<std::pair<Node, int>> changesInA;
};

#endif // __HYPERGRAPHCOREDECOMP__
