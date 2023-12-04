#include <algorithm>
#include <cstdio>
#include <cassert>
#include <iostream>
#include <unordered_map>
#include <list>
#include <set>
#include <queue>
#include<stack>
#include "Hypergraph.hpp"
#include "GraphScheduler.hpp"
#include "HypergraphCoreDecomp.hpp"
using namespace std;




class Parallel {
public:
    Parallel(char fileName[]): scheduler(fileName), O(1), A(1) {}
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
        solve();
        cnt = 0;
        time_t t = clock();
        while (scheduler.testNext()) {
            EdgeUpdate edgeUpdate = scheduler.nextUpdate();
            if (edgeUpdate.updType == INS)
                insertEdge(edgeUpdate.e);
            else
                deleteEdge(edgeUpdate.id);
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

    void solve() {
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

    void debug() {
        for (int i = 1; i <= 200; ++i)
            cerr << cv[i] << ' ';
        cerr << endl;
    }
    Hypergraph h;
private:
    GraphScheduler scheduler;
    unordered_map<Node, int> cv;
    unordered_map<unsigned, unsigned> ce;
    vector<Node> edge_node;
    unordered_map<Node, int> deg;
    unordered_set<unsigned> erasedEdgeIds;
    vector<list<Node>> O;//order序列。
    unordered_map<Node, list<Node>::iterator> iterToO;
    vector<SplayTree<Node>> A;  //存储核值以及order；伸展树的形式。
    unordered_map<Node, SplayNode<Node> *> pointerToA;
    unordered_map<Node, unsigned> degPlus, degStar;
    vector<pair<Node, int>> changesInA;
    unordered_map<Node, int> mcd;

    void insertEdge(Hyperedge &e) {
        unsigned id = h.insertEdge(e);
        unordered_set<Node> removed;
        unordered_set<Node> visit;
        unordered_set<Node> remained;
        list<Node> VC;
        unordered_set<unsigned> visited_E;
        unsigned K = INT_MAX;
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
        ++degPlus[u];
        if (degPlus[u] <= K) return;
        struct cmp{
            bool operator()(pair<int, Node>&a,pair<int, Node>&b){
                return a.first > b.first ;
            }
        };
        priority_queue<pair<int, Node>,vector<pair<int, Node>>,cmp> stk;//小顶堆
        visit.insert(u);
        remained.insert(u);
        stk.push(make_pair(A[K].rank(pointerToA[u]),u));
        list<Node>::iterator listIterator = iterToO[u];
        while(!stk.empty() && !remained.empty()) {
            Node v = stk.top().second;
            stk.pop();
            if(degPlus[v] + degStar[v] > K) {
                VC.push_back(v);
                remained.insert(u);
                O[K].erase(iterToO[v]);
                for(auto edge: h.eList[v]) {
                    if(ce[edge] == K && edge_node[edge] == v) {
                        int tmp = 0;
                        for(auto w: h.edgePool[edge]) {
                            if(cv[w] == K && w!= v) {
                                tmp++;
                                degStar[w]++;
                                if(!visit.count(w)){
                                    stk.emplace(A[K].rank(pointerToA[w]),w);
                                    visit.insert(w);
                                }
                            }
                        }
                        if(tmp > 0)
                            visited_E.insert(edge);
                    }
                }
            } else {
                queue<Node> st;
                st.push(v);
                removed.insert(v);
                remained.erase(v);
                listIterator = O[K].erase(iterToO[v]);
                while(!st.empty()){
                    Node x = st.front();
                    st.pop();
                    degPlus[x] += degStar[x];
                    degStar[x] = 0;
                    iterToO[x] = O[K].emplace(listIterator, x);
                    if(iterToO[x] == O[K].begin()) {
                        changesInA.push_back(make_pair(x, 0));
                    } else{
                        changesInA.push_back(make_pair(x, *prev(iterToO[x])));
                    }
                    for(auto edge: h.eList[x]) {
                        if(visited_E.count(edge)) {
                            auto bian = h.edgePool[edge];
                            for(auto w: bian) {
                                if(cv[w] == K && w!= x) {
                                    if (edge_node[edge] == w) {
                                        --degPlus[w];
                                    }else
                                        --degStar[w];
                                    if (degPlus[w] + degStar[w] <= K && A[K].rank(pointerToA[w]) < A[K].rank(pointerToA[v])  && !removed.count(w)) {
                                            st.push(w);
                                            removed.insert(w);
                                    }
                                }
                            }
                            edge_node[edge] = x;
                            visited_E.erase(edge);
                        }
                    }
                }

            }
        }


        for (auto iter = VC.rbegin(); iter != VC.rend(); ++iter) {
            const Node w = *iter;
            if(!removed.count(w)) {
                if (O.size() <= K + 1){
                    O.push_back(list<Node>());
                }
                A[K].del(pointerToA[w]);
                iterToO[w] = O[K + 1].emplace(O[K + 1].begin(), w);
                // Modify data structure A
                if (A.size() <= K + 1) A.push_back(SplayTree<Node>());
                A[K + 1].insertAfter(pointerToA[w], A[K + 1].begindummy);
            }
        }
        for(auto w: VC) {
            for(auto edge: h.eList[w]) {
                if(!removed.count(w) &&edge_node[edge] == w) {
                    for(auto node : h.edgePool[edge]) {
                        if(cv[node] == K + 1){
                            mcd[node]++;
                        }
                    }
                    ce[edge]++;
                }
            }
        }

        for(auto node: VC) {
            if(!removed.count(node)) {
                degStar[node] = 0;
                cv[node] = K + 1;
                for(auto edge: h.eList[node]) {
                    if(ce[edge] == K) {
                        mcd[node]--;
                    }
                }
            }
        }


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
        visit.clear();
        removed.clear();
        remained.clear();

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


    unordered_map<Node, int> ComputeSupport(Hypergraph& HG, unsigned& e){
        unsigned int max_value = 0;
        for(auto & node :HG.nodeSet) {
            max_value = max(max_value, node);
        }
        unordered_map<Node, int> support;
        vector<bool> visited(max_value + 1, false);
        unsigned int k;
        stack<Node> stk;
        k = ce[e];
        support.clear();
        for(auto v: HG.edgePool[e]){
            if(cv[v] == k) {
                stk.push(v);
                visited[v] = true;
            }
        }
        while(!stk.empty()){
            Node v = stk.top();
            stk.pop();
            for(auto item: HG.eList[v]){
                if(ce[item] == cv[v]){
                    support[v] = support.count(v) ? support[v] + 1: 1;
                }
                if(ce[item] == k){
                    for(auto n: HG.edgePool[item]){
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
    Parallel parallel(fileName);
    time_t t0 = clock();
    parallel.run();
    t0 = clock() - t0;
    cout << t0/1000 << " ms." << endl;
    parallel.debug();
    return 0;
}
