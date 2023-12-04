#ifndef __GRAPHSCHEDULER__
#define __GRAPHSCHEDULER__

#include <vector>
#include <fstream>
#include "Hypergraph.hpp"

enum Update {INS, DEL, ADD, REM, PADD, PREM, UPDATE};

struct EdgeUpdate {
	Hyperedge e;
	unsigned  int timestamp;
	Update updType;
    int id;
};

class GraphScheduler {
public:
	GraphScheduler(const char fileName[]);
	EdgeUpdate nextUpdate();
    std::vector<EdgeUpdate> updates;
    unsigned position;
	inline bool hasNext() {
		return position < updates.size()- 200;
	}
    inline bool testNext() {
        return position < updates.size();
    }
private:
	void load();
	std::ifstream fin;
};
#endif
