#include <cstdio>
#include <ctime>
#include <cassert>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <list>
#include <queue>
#include <set>
#include "Hypergraph.hpp"
#include "GraphScheduler.hpp"
#include "HypergraphCoreDecomp.hpp"
using namespace std;

class OrderDynamic {
public:
    OrderDynamic(char fileName[]): scheduler(fileName), O(1), A(1) {}
    void run() {
        unsigned cnt = 0;
        while (scheduler.hasNext()) {
            EdgeUpdate edgeUpdate = scheduler.nextUpdate();
            if (edgeUpdate.updType == INS) {
                h.insertEdge(edgeUpdate.e);
            }else if (edgeUpdate.updType == DEL) {
                h.deleteEdge(edgeUpdate.id);
            }
            ++cnt;
            if (cnt % 100000 == 0) {
                fprintf(stderr, "%d...\n", cnt);
            }
        }
        solve();//core_decomposition;
        for(Node node : h.nodeSet){
            for(auto id : h.eList[node]) {
                if(cv[node] == ce[id]) {
                    mcd[node] += 1;
                }
            }
        }
        cnt = 0;
        time_t t = clock();
        while (scheduler.testNext()) {
            EdgeUpdate edgeUpdate = scheduler.nextUpdate();
            if (edgeUpdate.updType == INS) {
                insertEdge(edgeUpdate.e);
            }else{
                deleteEdge(edgeUpdate.id);
            }
            ++cnt;
            if (cnt % 200 == 0) {
                time_t t0 = clock() - t;
                cerr << t0/1000 << " ms." << endl;
                fprintf(stderr, "%d...\n", cnt);
            }
            if(cnt % 100000 == 0) {
                t = clock();
            }
        }
    }
    
    void solve() {//core_decomposition_while_updating_PD_and_order;
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
            PD[p.second] = number;
        }
    }

    void debug() {
        for (int i = 1; i <= 300; ++i)
            cerr << cv[i] << ' ';
        cerr << endl;
    }
    Hypergraph h;
private:
    GraphScheduler scheduler;
    unordered_map<unsigned , int> ce;
    vector<Node> edge_node;
    unordered_map<Node, int> deg;
    unordered_set<unsigned> erasedEdgeIds;
    unordered_map<Node, int> cv;//存储核值。
    vector<list<Node>> O;//order序列。
    unordered_map<Node, list<Node>::iterator> iterToO;
    list<Node> VC;//核值增加的节点。
    unordered_map<Node, list<Node>::iterator> iterToVC;
    vector<SplayTree<Node>> A; // Data structure A //存储核值以及order；伸展树的形式。
    unordered_map<Node, SplayNode<Node> *> pointerToA;
    set<pair<int, Node>> B; // Data structure B//remove序列
    unordered_map<Node, unsigned> PD, AD;
    unordered_map<Node, int> mcd;
    vector<pair<Node, int>> changesInA; // Record how we should modify A to make it consistent with the new order
    void insertEdge(const Hyperedge& e) { // OrderInsert
        //preprocessing_phase
        unsigned  id = h.insertEdge(e);
        unsigned K = INT_MAX;
        unordered_set<unsigned> removed;
        unordered_set<unsigned> visited;
        edge_node.push_back(e[0]);
        for (const Node node: e) {
            if (pointerToA[node] == NULL) {
                iterToO[node] = O[0].emplace(O[0].begin(), node);
                pointerToA[node] = new SplayNode<Node>(node);
                A[0].insertAfter(pointerToA[node], A[0].begindummy);
                cv[node] = 0;
            }
            if (cv[node] < K) {
                K = cv[node];
                edge_node[id] = node;
            } else if (cv[node] == K && A[K].rank(pointerToA[edge_node[id]]) > A[K].rank(pointerToA[node])) {
                edge_node[id] = node;
            }
        }
        for(auto nodes:e){
            if(cv[nodes] == K){
                mcd[nodes]++;
            }
        }
        ce[id] = K;
        Node u = edge_node[id];
        ++PD[u];
        if (PD[u] <= K) return;
        B.insert(make_pair(A[K].rank(pointerToA[u]), u));
        //compute_phase
        for (list<Node>::iterator iter = O[K].begin(); iter != O[K].end();) {
            Node v = *iter;
            if (AD[v] + PD[v] > K) { // Case 1
                iter = O[K].erase(iter);
                iterToVC[v] = VC.emplace(VC.end(), v);
                for (const unsigned eId: h.eList[v]) {
                    if(ce[eId] == K && !removed.count(eId) && edge_node[eId] == v) {
                        visited.insert(eId);
                        removed.insert(eId);
                        const Hyperedge &vector = h.edgePool[eId];
                        for (const Node w: vector) {
                            if (cv[w] == K && w != v) {
                                if (AD[w] == 0) {
                                    B.insert(make_pair(A[K].rank(pointerToA[w]), w));
                                }
                                ++AD[w];
                            }
                        }
                    }
                }
            } else if (AD[v] == 0) { // Case 2
                if (B.empty())
                    break;
                Node w = B.begin()->second;
                iter = iterToO[w];
            } else { // Case 3
                PD[v] += AD[v];
                AD[v] = 0;
                ++iter;
                removeCandidates(iter, v, K, visited);
            }
            //end_phase
            while (!B.empty() && A[K].rank(pointerToA[v]) >= B.begin()->first) {
                B.erase(B.begin());
            }
        }
        for (auto iter = VC.rbegin(); iter != VC.rend(); ++iter) {
            const Node w = *iter;
            if (O.size() <= K + 1) O.push_back(list<Node>());
            iterToO[w] = O[K + 1].emplace(O[K + 1].begin(), w);
            // Modify data structure A
            A[K].del(pointerToA[w]);
            if (A.size() <= K + 1) A.push_back(SplayTree<Node>());
            A[K + 1].insertAfter(pointerToA[w], A[K + 1].begindummy);
        }
        for(auto w: VC) {
            for(auto edge: h.eList[w]) {
                if(edge_node[edge] == w) {
                    for(auto node : h.edgePool[edge]) {
                        if(cv[node] == K + 1){
                            mcd[node]++;
                        }
                    }
                    ce[edge]++;
                }
            }
        }
        for (const Node w: VC) {
            AD[w] = 0;
            cv[w] = K + 1;
            for(auto edge: h.eList[w]) {
                if(ce[edge] == K) {
                    mcd[w]--;
                }
            }
        }
        // Modify A to make it consistent with the new order
        for (auto iter = changesInA.begin(); iter != changesInA.end(); ++iter) {
            A[K].del(pointerToA[iter->first]);
            if(iter->second == 0) {
                A[K].insertAfter(pointerToA[iter->first], A[K].begindummy);
            }else {
                A[K].insertAfter(pointerToA[iter->first], pointerToA[iter->second]);
            }
        }
        changesInA.clear();
        VC.clear();
        iterToVC.clear();
        removed.clear();
        B.clear();
    }

    void deleteEdge(const unsigned & id) { //OrderRemove
        unordered_set<Node> exclude;
        list<Node> removed;
        queue<Node> Support;
        unordered_set<unsigned  int> visit;
        unordered_set<Node> visitV;
        int min_Core = ce[id];
        PD[edge_node[id]]--;
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
            PD[w] = 0;
            O[min_Core].erase(iterToO[w]);
            for(auto ids : h.eList[w]) {
                if(ce[ids] == min_Core) {
                    for(auto nodes: h.edgePool[ids]) {
                        if(edge_node[ids] == nodes && exclude.count(nodes)) {
                            PD[nodes] ++;
                        } else if(cv[nodes] == min_Core && !exclude.count(nodes) && A[min_Core].rank(pointerToA[w]) > A[min_Core].rank(pointerToA[nodes])) {
                            PD[nodes]--;
                            PD[w]++;
                            edge_node[ids] = w;
                        }
                    }
                }
            }
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

        int a =  *O[min_Core - 1].end();
        for(auto v: removed) {
            cv[v]--;
//            O[min_Core].erase(iterToO[v]);
            iterToO[v] = O[min_Core - 1].emplace(O[min_Core -1].end(), v);
            A[min_Core].del(pointerToA[v]);
            if(a == 0)
                A[min_Core - 1].insertAfter(pointerToA[v], A[min_Core - 1].begindummy);
            else
                A[min_Core - 1].insertAfter(pointerToA[v], pointerToA[a]);
            a = v;
//            PD[v] = 0;
//            for(auto ids : h.eList[v]) {
//                if(ce[ids] == min_Core) {
//                    for(auto nodes: h.edgePool[ids]) {
//                        if(edge_node[ids] == nodes &&  cv[nodes] == min_Core - 1) {
//                            PD[nodes] ++;
//                        } else if(edge_node[ids] == nodes && cv[nodes] == min_Core && !exclude.count(nodes)) {
//                            PD[nodes]--;
//                            PD[v]++;
//                            edge_node[ids] = v;
//                        }
//                    }
//                }
//            }
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

    void removeCandidates(const list<Node>::iterator &iter, const Node w, const unsigned K, unordered_set<unsigned> & visited) { // Algorithm 3: RomoveCandidates
        queue<Node> Q;
        unordered_set<Node> Qnodes;
        for (const unsigned eId: h.eList[w]) {
            if(visited.count(eId)) {
                const Hyperedge &e = h.edgePool[eId];
                for(Node w2: e) {
                    if (iterToVC.count(w2) && edge_node[eId] == w2) {
                        --PD[w2];
                        if (PD[w2] + AD[w2] <= K) {
                            if(!Qnodes.count(w2))
                                Q.push(w2);
                            Qnodes.insert(w2);
                        }
                    } else if (iterToVC.count(w2)){
                        --AD[w2];
                        if (PD[w2] + AD[w2] <= K) {
                            if(!Qnodes.count(w2))
                                Q.push(w2);
                            Qnodes.insert(w2);
                        }
                    } else if(w2!= w && cv[w2] == K) {
                        --AD[w2];
                    }
                }
                edge_node[eId] = w;
                visited.erase(eId);
            }
        }
        while (!Q.empty()) {
            const Node w2 = Q.front();
            Q.pop();
            Qnodes.erase(w2);
            PD[w2] += AD[w2];
            AD[w2] = 0;
            VC.erase(iterToVC[w2]);
            iterToVC.erase(w2);
            iterToO[w2] = O[K].emplace(iter, w2);
            // Modify data structure A
            if(iterToO[w2] == O[K].begin()) {
                changesInA.push_back(make_pair(w2, 0));
            } else{
                changesInA.push_back(make_pair(w2, *prev(iterToO[w2])));
            }
            for (const unsigned eId: h.eList[w2]) {
                if(visited.count(eId)) {
                    const Hyperedge &e = h.edgePool[eId];
                    for(auto w3: e) {
                        if (cv[w3] == K) {
                            if (A[K].rank(pointerToA[w]) < A[K].rank(pointerToA[w3])) {
                                --AD[w3];
                                if (AD[w3] == 0) {
                                    B.erase(make_pair(A[K].rank(pointerToA[w3]), w3));
                                }
                            } else if (iterToVC.count(w3)) {
                                if (edge_node[eId] == w2)
                                    --PD[w3];
                                else
                                    --AD[w3];
                                if (PD[w3] + AD[w3] <= K && !Qnodes.count(w3)) {
                                    if(!Qnodes.count(w3))
                                        Q.push(w3);
                                    Qnodes.insert(w3);
                                }
                            }
                        }
                    }
                    edge_node[eId] = w2;
                    visited.erase(eId);
                }
            }
        }
    }
};

int main(int argc, char **argv) {
    char *fileName = argv[1];
    OrderDynamic fullyDynamic(fileName);
    time_t t0 = clock();
    fullyDynamic.run();
    t0 = clock() - t0;
    cout << t0/1000 << " ms." << endl;
    fullyDynamic.debug();
	return 0;
}
