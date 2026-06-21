// *******************
// Program: hashTable_search_step.cpp
// Course: CCP6214 Algorithm Design and Analysis
// Lecture Class: TC2L
// Tutorial Class: TT8L
// Trimester: 2610
// Member_1: 242UC244S9 | GOH WEI JING | goh.wei.jing@student.mmu.edu.my | 01110872022
// Member_2: 243UC247DJ | WONG KAI SHEN | wong.kai.shen@student.mmu.edu.my | 0167129682
// Member_3: 251UC2517Z | JAYAVARMAN THIYAGU | Jayavarman.thiyagu@student.mmu.edu.my | 0169441376
// Member_4: ID | SAMIEON NGIAM TUN SHEN | SAMIEON.NGIAM.TUN@student.mmu.edu.my | 0169515810
// *******************
// Task Distribution
// Member_1: RADIX SORT
// Member_2: HEAP
// Member_3: DATA GENERATION
// Member_4: HASH TABLE
// *******************
//
// Build:  g++ -O2 -std=c++17 hashTable_search_step.cpp -o hash_table_search_step
// Run:    ./hash_table_search_step "../Dataset Generator/dataset_1000.csv" <target>
//   e.g.  ./hash_table_search_step "../Dataset Generator/dataset_1000.csv" 2008864030
//
// This is the STEP-TRACE program required by the assignment. It builds the same
// chaining hash table (each bucket is an AVL tree) used by the benchmark, then
// searches for ONE target key and prints the path the search takes:
//   1. the bucket the key hashes to, then
//   2. every AVL node visited inside that bucket and which way it branches.
//
// Output file: dataset_<n>_hash_table_search_step_<target>.txt
// Final line format (graded):
//   found     ->  2008864030 = 2008864030/rdiea
//   not found ->  -1 != 123456789
//
// The AVL tree is array-based (one pooled vector<Node>, 4-byte indices instead of
// pointers) so this program can also build the table for the largest datasets
// (e.g. 200,000,000 rows) on a 16 GB machine without being OOM-killed.
//
// Constraints honoured: no library search structure (hand-written hash table +
// AVL). The step trace is descriptive output, so there is no timed region here.

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdint>
#include <algorithm>

using namespace std;

static const uint32_t NIL = 0xFFFFFFFFu;

// Pooled AVL node (24 bytes): key(8)+left(4)+right(4)+value(5)+height(1).
struct Node {
    long long key;
    uint32_t  left;
    uint32_t  right;
    char      value[5];
    uint8_t   height;
};

// ==========================================
// HASH TABLE: chaining, each bucket an array-based AVL tree
// ==========================================
class HashTableArray {
private:
    vector<Node>&    pool;
    vector<uint32_t> buckets;
    long long        tableSize;

    int  h(uint32_t n)  { return n == NIL ? 0 : pool[n].height; }
    int  bf(uint32_t n) { return n == NIL ? 0 : h(pool[n].left) - h(pool[n].right); }
    void upd(uint32_t n){ pool[n].height = (uint8_t)(1 + max(h(pool[n].left), h(pool[n].right))); }

    uint32_t rotateRight(uint32_t y) {
        uint32_t x = pool[y].left, t = pool[x].right;
        pool[x].right = y; pool[y].left = t;
        upd(y); upd(x); return x;
    }
    uint32_t rotateLeft(uint32_t x) {
        uint32_t y = pool[x].right, t = pool[y].left;
        pool[y].left = x; pool[x].right = t;
        upd(x); upd(y); return y;
    }
    uint32_t insertNode(uint32_t node, uint32_t z) {
        if (node == NIL) return z;
        if (pool[z].key < pool[node].key)      pool[node].left  = insertNode(pool[node].left, z);
        else if (pool[z].key > pool[node].key) pool[node].right = insertNode(pool[node].right, z);
        else return node;
        upd(node);
        int b = bf(node);
        if (b > 1 && pool[z].key < pool[pool[node].left].key)  return rotateRight(node);
        if (b < -1 && pool[z].key > pool[pool[node].right].key) return rotateLeft(node);
        if (b > 1 && pool[z].key > pool[pool[node].left].key)  { pool[node].left  = rotateLeft(pool[node].left);   return rotateRight(node); }
        if (b < -1 && pool[z].key < pool[pool[node].right].key){ pool[node].right = rotateRight(pool[node].right); return rotateLeft(node); }
        return node;
    }

public:
    HashTableArray(vector<Node>& p, long long size)
        : pool(p), buckets(size, NIL), tableSize(size) {}

    long long size() const { return tableSize; }

    long long hashFunction(long long key) {
        long long index = key % tableSize;
        return index < 0 ? index + tableSize : index;
    }

    void insert(uint32_t z) {
        long long idx = hashFunction(pool[z].key);
        buckets[idx] = insertNode(buckets[idx], z);
    }

    // Search for `target`, appending a human-readable line for every step.
    // On a hit, copies the 5-char value into `foundValue`.
    bool searchTrace(long long target, string& foundValue, vector<string>& trace) {
        long long idx = hashFunction(target);
        trace.push_back("Hash index: " + to_string(target) + " % " +
                        to_string(tableSize) + " = " + to_string(idx));

        uint32_t cur = buckets[idx];
        if (cur == NIL) {
            trace.push_back("Bucket " + to_string(idx) + " is empty -> not found");
            return false;
        }

        int step = 1;
        while (cur != NIL) {
            long long k = pool[cur].key;
            if (target == k) {
                trace.push_back("Step " + to_string(step) + ": compare " +
                                to_string(target) + " == " + to_string(k) + " -> MATCH");
                foundValue.assign(pool[cur].value, 5);
                return true;
            } else if (target < k) {
                trace.push_back("Step " + to_string(step) + ": compare " +
                                to_string(target) + " < " + to_string(k) + " -> go left");
                cur = pool[cur].left;
            } else {
                trace.push_back("Step " + to_string(step) + ": compare " +
                                to_string(target) + " > " + to_string(k) + " -> go right");
                cur = pool[cur].right;
            }
            ++step;
        }
        trace.push_back("Reached empty subtree -> not found");
        return false;
    }
};

// Pull n out of a filename like ".../dataset_1000.csv" -> "1000".
string extractN(const string& filename) {
    size_t start = filename.rfind('_');
    size_t end = filename.rfind(".csv");
    if (start == string::npos || end == string::npos || end < start) return "0";
    return filename.substr(start + 1, end - start - 1);
}

long long nextPrime(long long n) {
    if (n < 2) return 2;
    auto isPrime = [](long long x) {
        if (x < 2) return false;
        for (long long d = 2; d * d <= x; ++d)
            if (x % d == 0) return false;
        return true;
    };
    while (!isPrime(n)) ++n;
    return n;
}

int main(int argc, char** argv) {
    if (argc < 3) {
        cerr << "Usage: " << argv[0] << " <dataset_n.csv> <target_key>\n";
        cerr << "  e.g. " << argv[0]
             << " \"../Dataset Generator/dataset_1000.csv\" 2008864030\n";
        return 1;
    }
    string filename = argv[1];
    long long target = stoll(argv[2]);

    // ---- Read input ----
    ios::sync_with_stdio(false);
    ifstream inFile(filename);
    if (!inFile.is_open()) {
        cerr << "Error: cannot open input file '" << filename << "'\n";
        return 1;
    }

    vector<Node> pool;
    {
        string nStr = extractN(filename);
        try { long long hint = stoll(nStr); if (hint > 0) pool.reserve((size_t)hint); }
        catch (...) {}
    }

    string line;
    while (getline(inFile, line)) {
        if (line.empty()) continue;
        long long key = 0;
        size_t i = 0, len = line.size();
        while (i < len && line[i] != ',') { key = key * 10 + (line[i] - '0'); ++i; }
        Node node;
        node.key = key; node.left = NIL; node.right = NIL; node.height = 1;
        size_t v = i + 1;
        for (int k = 0; k < 5; ++k) node.value[k] = (v + k < len) ? line[v + k] : '\0';
        pool.push_back(node);
    }
    inFile.close();

    long long size = (long long)pool.size();
    if (size == 0) {
        cerr << "Error: no records read from '" << filename << "'\n";
        return 1;
    }

    // Build the table (same sizing rule as the benchmark for a representative path).
    HashTableArray ht(pool, nextPrime(size * 2 + 1));
    for (long long i = 0; i < size; ++i) ht.insert((uint32_t)i);

    // ---- Trace one search ----
    vector<string> trace;
    string foundValue;
    bool found = ht.searchTrace(target, foundValue, trace);

    // Final, graded result line.
    string resultLine;
    if (found)
        resultLine = to_string(target) + " = " + to_string(target) + "/" + foundValue;
    else
        resultLine = "-1 != " + to_string(target);

    // ---- Write output file ----
    string n = extractN(filename);
    if (n == "0") n = to_string(size);
    string outFilename = "dataset_" + n + "_hash_table_search_step_" +
                         to_string(target) + ".txt";

    ofstream outFile(outFilename);
    if (!outFile.is_open()) {
        cerr << "Error: cannot write output file '" << outFilename << "'\n";
        return 1;
    }
    outFile << "Dataset: " << filename << "  (" << size << " records)\n";
    outFile << "Table size (prime): " << ht.size() << "\n";
    outFile << "Search target: " << target << "\n";
    outFile << "--- search path ---\n";
    for (const string& s : trace) outFile << s << "\n";
    outFile << "--- result ---\n";
    outFile << resultLine << "\n";
    outFile.close();

    // ---- Console (for the report screenshots) ----
    cout << "Search target: " << target << "\n";
    cout << "--- search path ---\n";
    for (const string& s : trace) cout << s << "\n";
    cout << "--- result ---\n";
    cout << resultLine << "\n";
    cout << "Trace written to: " << outFilename << "\n";

    return 0;
}
