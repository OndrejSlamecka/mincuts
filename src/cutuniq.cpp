/*
 * Copyright (c) 2015, Ondrej Slamecka <ondrej@slamecka.cz>
 * See the LICENSE file in the root folder of this repository.
 *
 * Reads cuts, splits them into several files by CHUNK_SIZE lines,
 * sorts these files and then merges them, ignoring duplicates.
 *
 * If you are seeing memory leaks, it's because of
 * ios_base::sync_with_stdio(false); (see start of main) which
 * can be disabled (at the cost of slowing down the program)
 */

#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
#include <unistd.h>
#include <csignal>
#include <cstdlib>
#endif

#include <boost/filesystem.hpp>
#include <boost/heap/fibonacci_heap.hpp>
#include <cstdio>  // "remove" fnc
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <list>
#include <string>
#include <algorithm>

#define CHUNK_SIZE 1000000

using namespace std;

// Cut IO, comparison and equality
void parse_cut(const string &line, vector<int> &cut) {
    istringstream line_s(line);
    string index;
    while (getline(line_s, index, ',')) {
        cut.push_back(stoi(index));
    }
}

ostream & operator<<(std::ostream &os, const vector<int>& V) {
    int i = 0, vs = V.size();
    for (int e : V) {
        os << e;
        if (i < vs - 1) os << ",";
        i++;
    }
    return os;
}

bool cut_compare(const vector<int> &A, const vector<int> &B) {
    if (A.size() != B.size()) {
        int d = A.size() < B.size();
        return d;
    }

    auto b_it = B.begin();
    for (auto a : A) {
        if (a != *b_it) {
            return a < *b_it;
        }
        ++b_it;
    }

    return false;
}

bool cut_eq(const vector<int> &A, const vector<int> &B) {
    return !(cut_compare(A, B) | cut_compare(B, A));
}

/**
 * Stores ofstream and the cut which is to be read
 */
class Block {
    shared_ptr<fstream> file;  // unique would be better...
    string filepath;
    bool file_exhausted = false;  // For reading...

    vector<int> cut;

 public:
    Block(fstream *fs, const string &fp) : file(fs), filepath(fp) {}

    ~Block() {
        remove(filepath.c_str());
    }

    fstream & get_file() {
        return *file;
    }

    const string & get_filepath() const {
        return filepath;
    }

    void store_chunk(const vector<vector<int>> &chunk) {
        for (auto c : chunk) {
            *file << c << "\n";
        }

        file->flush();
        file->seekg(0);

        // Read the first line (prepare for read_cut)
        string line;
        if (getline(*file, line)) {
            parse_cut(line, cut);
            file_exhausted = false;
        } else {
            file_exhausted = true;
        }
    }

    /**
     * Peek at the current cut
     */
    const vector<int>& peek_cut() const {
        return cut;
    }

    /**
     * Pops the current cut and reads next into a buffer
     */
    void pop_cut() {
        cut.clear();

        string line;
        if (getline(*file, line)) {
            parse_cut(line, cut);
        } else {
            file_exhausted = true;
        }
    }

    bool is_exhausted() const {
        return file_exhausted;
    }
};

/**
 * Chunk storage and processing
 */
struct heap_node {
    Block *block;
    boost::heap::fibonacci_heap<heap_node>::handle_type handle;

    explicit heap_node(Block *b) : block(b) {}

    bool operator<(heap_node const& n2) const {
        return cut_eq(this->block->peek_cut(), n2.block->peek_cut())
            || !cut_compare(this->block->peek_cut(), n2.block->peek_cut());
    }
};

class ChunkStorage {
    // Each chunk is stored in a block
    list<Block> blocks;

 public:
    void read_and_split(ifstream &finput) {
        // Read chunks of CHUNK_SIZE
        vector<vector<int>> chunk;

        int i = 0;
        for (string line; getline(finput, line);) {
            vector<int> cut;
            parse_cut(line, cut);

            sort(cut.begin(), cut.end());
            chunk.push_back(cut);

            i++;

            // If chunk_size is reached, store it
            if (i == CHUNK_SIZE) {
                sort(chunk.begin(), chunk.end(), cut_compare);
                store(chunk);
                chunk.clear();
                i = 0;
            }
        }

        // Store leftover chunk
        if (i > 0 && i < CHUNK_SIZE) {
            sort(chunk.begin(), chunk.end(), cut_compare);
            store(chunk);
        }
    }

    void store(const vector<vector<int>> &chunk) {
        // Create new file
        string fp = "tmp/" + boost::filesystem::unique_path().native();

        constexpr auto mode = fstream::in | fstream::out | fstream::trunc;
        fstream *file = new fstream(fp, mode);
        if (!file->is_open()) {
            cerr << "Could not open file " << fp << endl;
            exit(3);
        }
        blocks.emplace_back(file, fp);

        // Store the chunk into a block
        blocks.back().store_chunk(chunk);
    }

    /**
     * Merges split chunks into a single output
     */
    void merge() {
        if (blocks.empty()) {
            return;
        }

        // Create a heap and merge by selecting from it
        boost::heap::fibonacci_heap<heap_node> heap;

        for (auto &b : blocks) {
            auto handle = heap.push(heap_node(&b));
            (*handle).handle = handle;
        }

        vector<int> last_cut;  // Remember last cut to avoid duplicates
        while (!heap.empty()) {
            const heap_node &top = heap.top();

            // If cut at the top of heap is not the same as previous cut, print
            if (!cut_eq(top.block->peek_cut(), last_cut)) {
                cout << top.block->peek_cut() << "\n";
                last_cut = top.block->peek_cut();
            }

            top.block->pop_cut();  // Cut was printed, read next from the file

            // If file isn't exhausted, fix block's position in queue, else
            // remove block from the heap
            if (!top.block->is_exhausted()) {
                heap.update(top.handle);
            } else {
                heap.pop();
            }
        }
    }

    /**
     * This should be called only on SIGINT
     */
    void remove_temp_files() {
        blocks.clear();
    }
};

unique_ptr<ChunkStorage> storage;

#ifdef _POSIX_C_SOURCE
void sigint_handler_fnc(int _) {
    storage->remove_temp_files();
    exit(1);
}
#endif

int main(int argc, char* argv[]) {
    ios_base::sync_with_stdio(false);  // This makes a serious difference

    if (argc != 2) {
        cerr << "Usage:\t" << argv[0] << " <file with cuts>\n" << endl;
        cerr << "\tOutputs to stdout the set of cuts without permutations." <<
            endl;
        exit(1);
    }

    // Open the input file
    ifstream finput(argv[1]);
    if (!finput.is_open()) {
        cerr << "File " << argv[1] << " doesn't exist or could not be " \
                "accessed. Terminating." << endl;
        exit(2);
    }

    storage = unique_ptr<ChunkStorage>(new ChunkStorage());

#ifdef _POSIX_C_SOURCE
    // Register sigint handler
    struct sigaction handler;
    handler.sa_handler = sigint_handler_fnc;
    sigemptyset(&handler.sa_mask);
    handler.sa_flags = 0;
    sigaction(SIGINT, &handler, NULL);
#endif

    // Read, split to chunks, sort chunks
    storage->read_and_split(finput);

    // Merge chunks, ignore duplicates
    storage->merge();

    return 0;
}

