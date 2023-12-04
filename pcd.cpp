#include <algorithm>
#include <cstdio>
#include <iostream>
#include <unordered_map>
#include<stack>
#include "Hypergraph.hpp"
#include "GraphScheduler.hpp"
#include "HypergraphCoreDecomp.hpp"
using namespace std;

class Pcd {
public:
	Pcd(char fileName[]): scheduler(fileName) {}
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
        for (int i = 1; i <= 200; ++i)
            cerr << cv[i] << ' ';
        cerr << endl;
    }
    Hypergraph h;
private:
	GraphScheduler scheduler;
	unordered_map<Node, int> cv;
    unordered_map<unsigned, int> ce;
    unordered_map<Node, int> mcd;
    unordered_map<unsigned, unsigned> pcd;
    unordered_map<unsigned, unordered_set<int>> pch;
	unordered_set<Node> visitedV;
    unordered_set<Node> exclude;
    unordered_set<unsigned  int> visit;

	void insertEdge(Hyperedge &e) {
        unsigned id = h.insertEdge(e);
        mcd.clear();
        pch.clear();
        int min_Core = INT_MAX;
        for (auto v: e){
            min_Core = min(min_Core, cv[v]);
        }
        ce[id] = min_Core;
        int K = ce[id];
        stack<Node> Q;
        unordered_set<Node> Mk = Computemcd(e);
        for(auto vs: Mk) {
            Q.push(vs);
            visitedV.insert(vs);
        }

        while (!Q.empty()) {
            Node x = Q.top();
            Q.pop();
            pcd[x] = 0;
            for(auto ids: h.eList[x]) {
                if(ce[ids] == cv[x]) {
                    bool flag = false;
                    for(auto u : h.edgePool[ids]) {
                        if (cv[u] == ce[ids] && u != x && mcd[u] <= K) {
                            flag = true;
                        }
                    }
                    if(!flag) {
                        pcd[x]++;
                        pch[x].insert(ids);
                        for(auto u : h.edgePool[ids]) {
                            if (!visitedV.count(u)) {
                                Q.push(u);
                                visitedV.insert(u);
                            }
                        }
                    }
                }
            }
        }


        set<pair<unsigned, Node>> S;
        for(auto i : Mk) {
            S.insert(make_pair(pcd[i],i));
        }
        while (!S.empty()) {
            pair<unsigned, Node> p = *S.begin();
            if(p.first > min_Core) {
                break;
            } else {
                S.erase(S.begin());
                for(auto item:h.eList[p.second]){
                    if(ce[item]==min_Core && !visit.count(item)) {
                        visit.insert(item);
                        for (auto v: h.edgePool[item]) {
                            if (S.erase(make_pair(pcd[v], v))) {
                                --pcd[v];
                                S.insert(make_pair(pcd[v], v));
                            }
                        }
                    }
                }
            }

        }

        for(auto & it : S) {
            cv[it.second] = min_Core + 1;
            for(auto es:h.eList[it.second]){
                if(ce[es] == min_Core){
                    int  min_Core1 = INT_MAX;
                    for (auto v:h.edgePool[es]) {
                        min_Core1 = min(min_Core1, cv[v]);
                    }
                    ce[es] = min_Core1;
                }
            }
        }

	}
    void deleteEdge(int e) {
        exclude.clear();
        visit.clear();
        int min_Core = INT_MAX;
        for (auto v: h.edgePool[e]){
            min_Core = min(min_Core, cv[v]);
        }
        Hyperedge e0 = h.edgePool[e];
        h.deleteEdge(e);
        Computemcd(e0);
        for(auto it = mcd.begin(); it != mcd.end();) {
            if (it->second < min_Core) {
                bool signal = false;
                exclude.insert(it->first);
                cv[it->first]--;
                for(auto item:h.eList[it->first]){
                    if(ce[item]==min_Core && !visit.count(item)) {
                        visit.insert(item);
                        for (auto v: h.edgePool[item]) {
                            if (!exclude.count(v) && mcd.count(v)) {
                                mcd[v]--;
                                signal = true;
                            }
                        }
                    }
                }
                it = mcd.erase(it);
                if(signal)
                    it = mcd.begin();
            }else{
                it++;
            }
        }
        for(auto v: exclude) {
            for(auto es: h.eList[v]) {
                if(ce[es] == min_Core) {
                    ce[es] = min_Core - 1;
                }
            }
        }
    }


    unordered_set<Node> Computemcd(Hyperedge & e0){
        unsigned int max_value = 0;
        for(auto & node :h.nodeSet) {
            max_value = max(max_value, node);
        }
        vector<bool> visited(max_value + 1, false);
        int k = INT_MAX;
        unordered_set<Node> Mk;
        stack<Node> stk;
        for(auto v :  e0) {
            k = min(cv[v], k);
        }
        mcd.clear();
        for(auto v: e0){
            if(cv[v] == k) {
                stk.push(v);
                visited[v] = true;
            }
        }
        while(!stk.empty()){
            Node v = stk.top();
            stk.pop();
            Mk.insert(v);
            mcd[v] = 0;
            for(auto item: h.eList[v]){
                if(ce[item] == cv[v]){
                    mcd[v] = mcd[v] + 1;
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
        return Mk;
    }
};

int main(int argc, char **argv) {
	char *fileName = argv[1];
	Pcd pcd(fileName);
	time_t t0 = clock();
	pcd.run();
    t0 = clock() - t0;
    cout << t0/1000 << " ms." << endl;
    pcd.debug();
	return 0;
}
