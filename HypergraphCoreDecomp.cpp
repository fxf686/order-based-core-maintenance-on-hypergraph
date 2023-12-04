#include "HypergraphCoreDecomp.hpp"
#include <set>
#include <algorithm>
#include <cassert>
#include <iostream>

using namespace std;

HypergraphCoreDecomp::HypergraphCoreDecomp(const Hypergraph& h): h(h), O(1), A(1) {}

void HypergraphCoreDecomp::solve() {
    edge_node.resize(h.nEdges);
	set<pair<int, Node>> S;
	int ans = 0;
	for (auto &  p: h.eList) {
		deg[p.first] = p.second.size();
		S.insert(make_pair(deg[p.first], p.first));
	}
    SplayNode<Node>* tmp;
	while (!S.empty()) {
		pair<int, Node> p = *S.begin();
		S.erase(S.begin());
		ans = max(ans, p.first);
		assert(p.first == deg[p.second]);
		cv[p.second] = ans;
        while(O.size() <= ans) {
            O.push_back(list<Node>());
        }
        iterToO[p.second] = O[ans].emplace(O[ans].end(), p.second);
        int flag = false;
        while (A.size() <= ans){
            A.push_back(SplayTree<Node>());
            flag = true;
        }
        if(flag)
            tmp = A[ans].begindummy;
        pointerToA[p.second] = new SplayNode<Node>(p.second);
        A[ans].insertAfter(pointerToA[p.second], tmp);
        tmp = pointerToA[p.second];

        int number = 0;
		for (const unsigned eId: h.eList.at(p.second)) {
			if (!erasedEdgeIds.count(eId)) {
                number++;
                edge_node[eId] = p.second;
                ce[eId] = ans;
				erasedEdgeIds.insert(eId);
				const Hyperedge& e = h.edgePool[eId];
				for (const Node v: e) {
					if (S.erase(make_pair(deg[v], v))) {
						--deg[v];
						S.insert(make_pair(deg[v], v));
					}
				}
			}
		}
        degPlus[p.second] = number;
	}
}
