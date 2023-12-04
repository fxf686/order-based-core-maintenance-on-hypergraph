#include <algorithm>
#include <cstdio>
#include <set>
#include <iostream>
#include <unordered_map>
#include<stack>
#include<queue>
#include "Hypergraph.hpp"
#include "GraphScheduler.hpp"
#include "HypergraphCoreDecomp.hpp"
using namespace std;

class Mcd {
public:
	Mcd(char fileName[]): scheduler(fileName) {}
    void run() {
        unsigned cnt = 0;
        while (scheduler.hasNext()) {
            EdgeUpdate edgeUpdate = scheduler.nextUpdate();
            if (edgeUpdate.updType == INS)
                h.insertEdge(edgeUpdate.e);
            else
                h.deleteEdge(edgeUpdate.id);
            ++cnt;
            if (cnt % 100000 == 0) {
                fprintf(stderr, "%d...\n", cnt);
            }
        }
        HypergraphCoreDecomp a(h);
        a.solve();
        cv = a.cv;
        ce = a.ce;
        cnt = 0;
        for(Node node : h.nodeSet){
            for(auto id : h.eList[node]) {
                if(cv[node] == ce[id]) {
                    mcd[node] += 1;
                }
            }
        }
        time_t t = clock();
        while (scheduler.testNext()) {
            EdgeUpdate edgeUpdate = scheduler.nextUpdate();
            if (edgeUpdate.updType == INS)
                insertEdge(edgeUpdate.e);
            else
                deleteEdge(edgeUpdate.id);
            ++cnt;
            if (cnt % 20 == 0) {
                time_t t0 = clock() - t;
                cerr << t0/1000 << " ms." << endl;
                fprintf(stderr, "%d...\n", cnt);
            }
            if(cnt % 1000 == 0) {
                t = clock();
            }
        }
    }

    void debug() {
        for (int i = 1; i <= 1000; ++i)
            cerr << cv[i] << ' ';
        cerr << endl;
    }
    Hypergraph h;
private:
	GraphScheduler scheduler;
	unordered_map<Node, int> cv;
    unordered_map<unsigned, int> ce;
    unordered_map<Node, int> mcd;

	void insertEdge(Hyperedge &e) {
        int id = h.insertEdge(e);
        int min_Core = INT_MAX;
        for (auto v: e){
            min_Core = min(min_Core, cv[v]);
        }
        ce[id] = min_Core;
        unordered_set<Node> visitV;
        unordered_set<Node> preV;
        unordered_set<int> visitE;
        stack<Node> stk;
        for(auto v: e) {
            if(cv[v] == min_Core) {
                mcd[v]++;
                if(mcd[v] > min_Core)
                    stk.push(v);
                visitV.insert(v);
            }
        }
        while(!stk.empty()) {
             Node u = stk.top();
             stk.pop();
             if(mcd[u] > min_Core) {
                 for(auto ids : h.eList[u]) {
                     if(ce[ids] == min_Core) {
                         for(auto w : h.edgePool[ids]) {
                             if(cv[w] == min_Core && !visitV.count(w)) {
                                 visitV.insert(w);
                                 stk.push(w);
                             }
                         }
                     }
                 }
             }else {
                 if(!preV.count(u)) {
                     stack<Node> st;
                     st.push(u);
                     preV.insert(u);
                     while(!st.empty()) {
                         Node y = st.top();
                         st.pop();
                         for(auto ids : h.eList[y]) {
                             if(ce[ids] == min_Core && !visitE.count(ids)) {
                                 visitE.insert(ids);
                                 for(auto z : h.edgePool[ids]) {
                                     if(cv[z] == min_Core) {
                                         mcd[z]--;
                                         if(!preV.count(z) && mcd[z] == min_Core) {
                                             preV.insert(z);
                                             st.push(z);
                                         }
                                     }
                                 }
                             }
                         }
                     }
                 }
             }
        }

        unordered_set<unsigned int> tmp;
        for(auto & u : visitV) {
            if(!preV.count(u)) {
                cv[u] = min_Core + 1;
                for(auto es:h.eList[u]){
                    if(ce[es] == min_Core){
                        int  min_Core1 = INT_MAX;
                        for (auto v:h.edgePool[es]) {
                            min_Core1 = min(min_Core1, cv[v]);
                        }
                        ce[es] = min_Core1;
                        if(min_Core1 > min_Core){
                            tmp.insert(es);
                        }
                    }
                }
            }
        }
        for(auto ids : visitE) {
            for(auto v: h.edgePool[ids]) {
                if(cv[v] == ce[ids]) {
                    mcd[v]++;
                }
            }
        }
        for(auto ids : tmp) {
            if(ce[ids] == min_Core + 1) {
                for(auto v: h.edgePool[ids]) {
                    if(cv[v] == ce[ids]) {
                        mcd[v]++;
                    }
                }
            }
        }
        for(auto u: visitV) {
            if (!preV.count(u)) {
                mcd[u] = 0;
                for (auto ids: h.eList[u]) {
                    if (ce[ids] == cv[u]) {
                        mcd[u]++;
                    }
                }
            }
        }
	}
    void deleteEdge(const unsigned & id) {
        unordered_set<Node> exclude;
        list<Node> removed;
        queue<Node> Support;
        unordered_set<unsigned  int> visit;
        unordered_set<Node> visitV;
        int min_Core = ce[id];
        Hyperedge e0 = h.edgePool[id];
        h.deleteEdge(id);
        for(auto node: e0) {
            if(cv[node] == min_Core) {
                mcd[node] --;
                if(mcd[node] < min_Core) {
                    Support.push(node);
                    visitV.insert(node);
                }
            }
        }
        while(!Support.empty()) {
            Node w = Support.front();
            Support.pop();
            exclude.insert(w);
            removed.push_back(w);
//            degPlus[w] = 0;
//            O[min_Core].erase(iterToO[w]);
//            for(auto ids : h.eList[w]) {
//                if(ce[ids] == min_Core) {
//                    for(auto nodes: h.edgePool[ids]) {
//                        if(edge_node[ids] == nodes && exclude.count(nodes)) {
//                            degPlus[nodes] ++;
//                        } else if(cv[nodes] == min_Core && !exclude.count(nodes) && A[min_Core].rank(pointerToA[w]) > A[min_Core].rank(pointerToA[nodes])) {
//                            degPlus[nodes]--;
//                            degPlus[w]++;
//                            edge_node[ids] = w;
//                        }
//                    }
//                }
//            }
            for(auto ids : h.eList[w]) {
                if(ce[ids] == min_Core && !visit.count(ids)) {
                    visit.insert(ids);
                    for(auto nodes: h.edgePool[ids]) {
                        if(cv[nodes] == min_Core) {
                            mcd[nodes]--;
                            if(mcd[nodes] < min_Core && !visitV.count(nodes)) {
                                Support.push(nodes);
                                visitV.insert(nodes);
                            }
                        }
                    }
                }
            }
        }

        for(auto v: removed) {
            cv[v]--;
            for(auto es: h.eList[v]) {
                if(ce[es] == min_Core) {
                    ce[es] = min_Core - 1;
                }
            }
        }
        for(auto v: exclude) {
            for(auto es: h.eList[v]) {
                if(ce[es] == min_Core - 1) {
                    mcd[v]++;
                }
            }
        }
        visitV.clear();
        visit.clear();
        exclude.clear();
    }
    unordered_map<Node, int> ComputeSupport(Hyperedge & e0){
        unsigned int max_value = 0;
        for(auto & node :h.nodeSet) {
            max_value = max(max_value, node);
        }
        unordered_map<Node, int> support;
        vector<bool> visited(max_value + 1, false);
        int k = INT_MAX;
        stack<Node> stk;
        for(auto v :  e0) {
            k = min(cv[v], k);
        }
        support.clear();
        for(auto v: e0){
            if(cv[v] == k) {
                stk.push(v);
                visited[v] = true;
            }
        }
        while(!stk.empty()){
            Node v = stk.top();
            stk.pop();
            support[v] = 0;
            for(auto item: h.eList[v]){
                if(ce[item] == cv[v]){
                    support[v] = support[v] + 1;
                }
                if(ce[item] == k){
                    for(auto n: h.edgePool[item]){
                        if(!visited[n] && cv[n] == k){
                            stk.push(n);
                            visited[n] = true;
                        }
                    }
                }
            }
        }
        return support;
    }
};

int main(int argc, char **argv) {
	char *fileName = argv[1];
	Mcd mcd(fileName);
	time_t t0 = clock();
	mcd.run();
    t0 = clock() - t0;
    cout << t0/1000 << " ms." << endl;
    mcd.debug();
	return 0;
}
