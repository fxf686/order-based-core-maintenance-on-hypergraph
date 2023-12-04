#include <algorithm>
#include <cstdio>
#include <set>
#include <iostream>
#include <unordered_map>
#include<stack>
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
        cerr << 1 << endl;
        for(auto i = 0; i < h.edgePool.size();++i) {
            int cur_Min = INT_MAX;
            for(auto j:h.edgePool[i]) {
                cur_Min = min(cur_Min, cv[j]);
            }
            ce[i] = cur_Min;
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
    unordered_set<unsigned  int> visit;

	void insertEdge(Hyperedge &e) {
        int id = h.insertEdge(e);
        visit.clear();
        int min_Core = INT_MAX;
        for (auto v: e){
            min_Core = min(min_Core, cv[v]);
        }
        ce[id] = min_Core;
        unordered_map<Node, int> Support = ComputeSupport(e);
        set<pair<unsigned, Node>> S;
        for(auto i : Support) {
            S.insert(make_pair(i.second,i.first));
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
                            if (S.erase(make_pair(Support[v], v))) {
                                --Support[v];
                                S.insert(make_pair(Support[v], v));
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
	void deleteEdge(int e) { // Algorithm 9
        unordered_set<Node> exclude;
        visit.clear();
        int min_Core = INT_MAX;
        for (auto v: h.edgePool[e]){
            min_Core = min(min_Core, cv[v]);
        }
        Hyperedge e0 = h.edgePool[e];
        h.deleteEdge(e);
        unordered_map<Node, int> Support = ComputeSupport(e0);
//        set<pair<unsigned, Node>> S;
//        for(auto i : Support) {
//            S.insert(make_pair(i.second,i.first));
//        }
//
//        while (!S.empty()) {
//            pair<unsigned, Node> p = *S.begin();
//            if(p.first >= min_Core) {
//                break;
//            } else {
//                S.erase(S.begin());
//                exclude.insert(p.second);
//                cv[p.second]--;
//                for(auto item:h.eList[p.second]){
//                    if(ce[item]==min_Core && !visit.count(item)) {
//                        visit.insert(item);
//                        for (auto v: h.edgePool[item]) {
//                            if (S.erase(make_pair(Support[v], v))) {
//                                --Support[v];
//                                S.insert(make_pair(Support[v], v));
//                            }
//                        }
//                    }
//                }
//            }
//
//        }
        int flag = true;
        while(flag) {
            flag = false;
            for(auto it = Support.begin(); it != Support.end();) {
                if (it->second < min_Core) {
                    exclude.insert(it->first);
                    cv[it->first]--;
                    for (auto item: h.eList[it->first]) {
                        if (ce[item] == min_Core && !visit.count(item)) {
                            visit.insert(item);
                            for (auto v: h.edgePool[item]) {
                                if (!exclude.count(v) && Support.count(v)) {
                                    Support[v]--;
                                    flag = true;
                                }
                            }
                        }
                    }
                    it = Support.erase(it);
                } else {
                    it++;
                }
            }
        }
//        for(auto it = Support.begin(); it != Support.end();) {
//            if (it->second < min_Core) {
//                bool signal = false;
//                exclude.insert(it->first);
//                cv[it->first]--;
//                for(auto item:h.eList[it->first]){
//                    if(ce[item]==min_Core && !visit.count(item)) {
//                        visit.insert(item);
//                        for (auto v: h.edgePool[item]) {
//                            if (!exclude.count(v) && Support.count(v)) {
//                                Support[v]--;
//                                signal = true;
//                            }
//                        }
//                    }
//                }
//                it = Support.erase(it);
//                if(signal)
//                    it = Support.begin();
//            }else{
//                it++;
//            }
//        }
        for(auto v: exclude) {
            for(auto es: h.eList[v]) {
                if(ce[es] == min_Core) {
                    ce[es] = min_Core - 1;
                }
            }
        }
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
