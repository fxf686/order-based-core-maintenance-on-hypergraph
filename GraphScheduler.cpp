#include "GraphScheduler.hpp"
#include <cassert>
#include <string>
#include <iostream>
#include <algorithm>
#include <unordered_map>
using namespace std;

GraphScheduler::GraphScheduler(const char fileName[]) {
	fin.open(fileName);
	load();
	fin.close();
	position = 0;
}

void GraphScheduler::load() {
	string delimiter = " ";
	string line;
	vector<string> tokens;

	while (getline(fin, line)) {
		tokens.clear();
		size_t pos = 0;
		std::string token;
		while (pos != std::string::npos) {
			pos = line.find(delimiter);
			token = line.substr(0, pos);
			tokens.push_back(token);
			line.erase(0, pos + delimiter.length());
		}

		assert(tokens.size() >= 2);

		EdgeUpdate edgeUpdate;
		if (tokens[0][0] == '+') {
			edgeUpdate.updType = INS;
		} else if (tokens[0][0] == '-') {
            edgeUpdate.updType = DEL;
        } else if (tokens[0][0] == '*') {
            edgeUpdate.updType = ADD;
        } else if (tokens[0][0] == '/') {
            edgeUpdate.updType = REM;
		} else if (tokens[0][0] == '!') {
            edgeUpdate.updType = PADD;
        } else if (tokens[0][0] == '@') {
            edgeUpdate.updType = PREM;
        } else if (tokens[0][0] == '%') {
            edgeUpdate.updType = UPDATE;
        } else {
			assert(false);
		}

		for (size_t i = 1; i < tokens.size(); i++) {
            edgeUpdate.e.push_back(atoi(tokens[i].c_str()));
        };
		if (edgeUpdate.updType == INS) {
			edgeUpdate.timestamp = edgeUpdate.e.back();
			edgeUpdate.e.pop_back();
            if(edgeUpdate.e.back() ==0)
                edgeUpdate.e.pop_back();
        }else if (edgeUpdate.updType == ADD || edgeUpdate.updType == REM || edgeUpdate.updType == PREM || edgeUpdate.updType == PADD) {
            edgeUpdate.timestamp = edgeUpdate.e.back();
            edgeUpdate.e.pop_back();
            edgeUpdate.id = edgeUpdate.e.back();
            edgeUpdate.e.pop_back();

        } else{
            edgeUpdate.id = edgeUpdate.e.back();
            edgeUpdate.e.pop_back();
        }
        if(edgeUpdate.updType != DEL) {
            sort(edgeUpdate.e.begin(), edgeUpdate.e.end());
            edgeUpdate.e.shrink_to_fit();
        }
		updates.push_back(edgeUpdate);
		const int MOD = 1000000; // 100000000
		if (updates.size() % MOD == MOD - 1) {
			cerr << "READ #: " << updates.size() + 1 << endl;
		}
	}
	// Only in C++11
	updates.shrink_to_fit();
	cerr << "Finished. " << updates.size() << " updates." << endl;
}

EdgeUpdate GraphScheduler::nextUpdate() {
	return updates[position++];
	
}
