/*
 * Copyright (c) 2015, Ondrej Slamecka <ondrej@slamecka.cz>
 * See the LICENSE file in the root folder of this repository.
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <set>
#include <algorithm>

using namespace std;

void readFile(const char *filename, set<set<int>> &S)
{
	if (filename == string("~")) {
		return;
	}

	ifstream is(filename);
	if (!is.is_open()) {
		cerr << filename << " not found";
		exit(2);
	}

	string line;
	for (; getline(is, line);) {
		istringstream line_s(line);
		set<int> cut;
		string edgeIndex;
		while (getline(line_s, edgeIndex, ',')) {
			cut.insert(stoi(edgeIndex));
		}

		S.insert(cut);
	}
}

ostream & operator<<(std::ostream &os, const set<int>& S)
{
	int i = 0, ss = S.size();
	for (auto e : S) {
		os << e;
		if (i < ss - 1) os << ",";
		i++;
	}
	return os;
}

int main(int argc, char *argv[])
{
	if (argc != 3 || argv[1] == string("-h")
		|| argv[1] == string("--help")) {
		cerr << "Usage: " << argv[0] << " <file A with cuts> " \
		   "[<file B with cuts> | ~]" << endl;

		cerr << "Output:" << endl << \
	   		"\t* Set difference A\\B (note that B\\A is not computed)\n" \
		   	"\t* Each cut of A and B is considered a set\n" \
			"\t* '~' replaces second file with empty set" \
			<< endl;
		exit(1);
	}

	set<set<int>> A, B, AmB;
	readFile(argv[1], A);
	readFile(argv[2], B);

	set_difference(A.begin(), A.end(), B.begin(), B.end(),
			inserter(AmB, AmB.end()));

	for (set<int> c : AmB) {
		cout << c << endl;
	}

	return 0;
}
