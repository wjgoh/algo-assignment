// *******************
// Program: hashTable_search_array.cpp
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
// Build:  g++ -O2 -std=c++17 hashTable_search_array.cpp -o hash_table_search_array
// Run:    ./hash_table_search_array "../Dataset Generator/dataset_200000000.csv"
//
// This is the ARRAY-BASED AVL variant of the hash-table search, written to
// contrast with the linked-list (pointer) based version in hashTable_search.cpp.
// Both are chaining hash tables whose buckets are AVL trees; the only difference
// is how the AVL nodes are stored:
//
//   * Pointer version : every node is a separate `new AVLNode`, children are
//                       raw pointers (8 B each) and each allocation carries
//                       ~16 B of malloc bookkeeping.
//   * Array  version  : all nodes live in ONE contiguous pool (vector<Node>);
//                       children are 4-byte indices into that pool. No per-node
//                       malloc overhead, far better cache locality.
//
// Memory is why this file exists. On a 16 GB machine the pointer version is
// killed by the OS while building the table for n = 200,000,000 (it needs ~27 GB);
// this array version needs ~6.4 GB and completes:
//     pool   : 200M nodes x 24 B  ~= 4.8 GB
//     buckets: ~400M indices x 4 B ~= 1.6 GB
// The separate `dataset` vector is also gone: every key already sits in the pool,
// so the average-case loop just walks the pool instead of a second copy.
//
// Constraints honoured:
//  - No library search / no std::map / std::set / std::unordered_map: the hash
//    table and per-bucket AVL tree are hand-written. (std::vector is only a
//    dynamic array, used as the node pool and the bucket array.)
//  - The measured region excludes all file I/O.

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <algorithm>

using namespace std;

// Sentinel meaning "no node" (replaces nullptr for the index-based tree).
static const uint32_t NIL = 0xFFFFFFFFu;

// ==========================================
// 1. POOLED AVL NODE  (24 bytes)
// ==========================================
// key(8) + left(4) + right(4) + value(5) + height(1) -> 22, padded to 24.
struct Node {
    long long key;     // the 10-digit integer search key
    uint32_t  left;    // pool index of left child  (NIL if none)
    uint32_t  right;   // pool index of right child (NIL if none)
    char      value[5];// the 5-letter attached string (no NUL needed)
    uint8_t   height;  // AVL height
};

// ==========================================
// 2. HASH TABLE: chaining, each bucket is an array-based AVL tree
// ==========================================
class HashTableArray {
private:
    vector<Node>&     pool;     // shared node pool (owns all the data)
    vector<uint32_t>  buckets;  // one AVL root index per bucket (NIL = empty)
    long long         tableSize;

    long long hashFunction(long long key) {
        long long index = key % tableSize;
        return index < 0 ? index + tableSize : index;   // guard negatives
    }

    int  h(uint32_t n)  { return n == NIL ? 0 : pool[n].height; }
    int  bf(uint32_t n) { return n == NIL ? 0 : h(pool[n].left) - h(pool[n].right); }
    void upd(uint32_t n){ pool[n].height = (uint8_t)(1 + max(h(pool[n].left), h(pool[n].right))); }

    uint32_t rotateRight(uint32_t y) {
        uint32_t x = pool[y].left;
        uint32_t t = pool[x].right;
        pool[x].right = y;
        pool[y].left  = t;
        upd(y); upd(x);
        return x;
    }

    uint32_t rotateLeft(uint32_t x) {
        uint32_t y = pool[x].right;
        uint32_t t = pool[y].left;
        pool[y].left  = x;
        pool[x].right = t;
        upd(x); upd(y);
        return y;
    }

    // Insert the already-created pool node `z` into the subtree rooted at `node`.
    // Returns the (possibly new) subtree root index. Pure index arithmetic, so a
    // pool reallocation would not invalidate it -- though we reserve() to avoid one.
    uint32_t insertNode(uint32_t node, uint32_t z) {
        if (node == NIL) return z;

        if (pool[z].key < pool[node].key)
            pool[node].left = insertNode(pool[node].left, z);
        else if (pool[z].key > pool[node].key)
            pool[node].right = insertNode(pool[node].right, z);
        else
            return node;  // duplicate key (keys are unique in the dataset)

        upd(node);
        int balance = bf(node);

        // Left Left
        if (balance > 1 && pool[z].key < pool[pool[node].left].key)
            return rotateRight(node);
        // Right Right
        if (balance < -1 && pool[z].key > pool[pool[node].right].key)
            return rotateLeft(node);
        // Left Right
        if (balance > 1 && pool[z].key > pool[pool[node].left].key) {
            pool[node].left = rotateLeft(pool[node].left);
            return rotateRight(node);
        }
        // Right Left
        if (balance < -1 && pool[z].key < pool[pool[node].right].key) {
            pool[node].right = rotateRight(pool[node].right);
            return rotateLeft(node);
        }
        return node;
    }

public:
    HashTableArray(vector<Node>& p, long long size)
        : pool(p), buckets(size, NIL), tableSize(size) {}

    void insert(uint32_t z) {
        long long idx = hashFunction(pool[z].key);
        buckets[idx] = insertNode(buckets[idx], z);
    }

    // Silent search for benchmarking (no I/O inside the timed region).
    bool searchSilent(long long targetKey) {
        uint32_t cur = buckets[hashFunction(targetKey)];
        while (cur != NIL) {
            if (pool[cur].key == targetKey) return true;
            cur = (targetKey < pool[cur].key) ? pool[cur].left : pool[cur].right;
        }
        return false;
    }

    // Key at the root of the first non-empty bucket: a guaranteed single-comparison
    // (root) hit, i.e. the true best case.
    bool bestCaseKey(long long& out) {
        for (long long i = 0; i < tableSize; ++i) {
            if (buckets[i] != NIL) {
                out = pool[buckets[i]].key;
                return true;
            }
        }
        return false;
    }
};

// ==========================================
// 3. HELPERS
// ==========================================
// Pull n out of a filename like ".../dataset_1000.csv" -> "1000".
string extractN(const string& filename) {
    size_t start = filename.rfind('_');
    size_t end = filename.rfind(".csv");
    if (start == string::npos || end == string::npos || end < start) return "0";
    return filename.substr(start + 1, end - start - 1);
}

// Smallest prime >= n, so the table size is genuinely prime (good distribution).
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
    string filename = (argc > 1) ? argv[1] : "../Dataset Generator/dataset_1000.csv";

    // ---- Read input (NOT timed) ----
    // Manual line parsing (no stringstream) keeps the huge-file load tolerable.
    ios::sync_with_stdio(false);
    ifstream inFile(filename);
    if (!inFile.is_open()) {
        cerr << "Error: cannot open input file '" << filename << "'\n";
        return 1;
    }

    vector<Node> pool;
    // Reserve up front so the pool never reallocates mid-load (avoids a transient
    // 1.5x memory spike on the biggest datasets).
    {
        string nStr = extractN(filename);
        try {
            long long hint = stoll(nStr);
            if (hint > 0) pool.reserve((size_t)hint);
        } catch (...) { /* unparseable -> let the vector grow on its own */ }
    }

    string line;
    while (getline(inFile, line)) {
        if (line.empty()) continue;

        // Parse the integer key up to the comma.
        long long key = 0;
        size_t i = 0;
        size_t len = line.size();
        while (i < len && line[i] != ',') {
            key = key * 10 + (line[i] - '0');
            ++i;
        }

        Node node;
        node.key = key;
        node.left = NIL;
        node.right = NIL;
        node.height = 1;
        // Copy up to 5 value characters (skip the comma at line[i]).
        size_t v = i + 1;
        for (int k = 0; k < 5; ++k)
            node.value[k] = (v + k < len) ? line[v + k] : '\0';

        pool.push_back(node);
    }
    inFile.close();

    long long size = static_cast<long long>(pool.size());
    if (size == 0) {
        cerr << "Error: no records read from '" << filename << "'\n";
        return 1;
    }

    // Build the hash table (a prime table size keeps the load factor sensible).
    HashTableArray ht(pool, nextPrime(size * 2 + 1));
    for (long long i = 0; i < size; ++i)
        ht.insert((uint32_t)i);

    cout << "Loaded " << size << " records into the hash table (array-based AVL)." << endl;

    // ---- Prepare search targets ----
    long long bestTarget;
    ht.bestCaseKey(bestTarget);          // guaranteed root hit
    long long worstTarget = 123456789;   // 9 digits => cannot exist in a 10-digit dataset

    // Sinks stop the optimiser from deleting the (otherwise unused) search loops.
    volatile bool sink = false;

    // ---- BEST CASE: n searches of a key at a bucket root ----
    auto startBest = chrono::high_resolution_clock::now();
    for (long long i = 0; i < size; ++i) sink ^= ht.searchSilent(bestTarget);
    auto endBest = chrono::high_resolution_clock::now();
    double diffBest = chrono::duration<double>(endBest - startBest).count();

    // ---- AVERAGE CASE: n searches across every existing key (read from the pool) ----
    auto startAvg = chrono::high_resolution_clock::now();
    for (long long i = 0; i < size; ++i) sink ^= ht.searchSilent(pool[i].key);
    auto endAvg = chrono::high_resolution_clock::now();
    double diffAvg = chrono::duration<double>(endAvg - startAvg).count();

    // ---- WORST CASE: n searches of a key that does not exist ----
    auto startWorst = chrono::high_resolution_clock::now();
    for (long long i = 0; i < size; ++i) sink ^= ht.searchSilent(worstTarget);
    auto endWorst = chrono::high_resolution_clock::now();
    double diffWorst = chrono::duration<double>(endWorst - startWorst).count();

    (void)sink;

    // ---- Write output (NOT timed) ----
    string n = extractN(filename);
    if (n == "0") n = to_string(size);
    string outFilename = "hash_table_search_dataset_" + n + ".txt";

    ofstream outFile(outFilename);
    if (!outFile.is_open()) {
        cerr << "Error: cannot write output file '" << outFilename << "'\n";
        return 1;
    }
    outFile << "Number of searches per case: " << size << "\n";
    outFile << "Best case time: " << diffBest << " seconds\n";
    outFile << "Average case time: " << diffAvg << " seconds\n";
    outFile << "Worst case time: " << diffWorst << " seconds\n";
    outFile.close();

    // ---- Console (for the report screenshots) ----
    cout << "\n--- BENCHMARK RESULTS (" << size << " searches per case) ---" << endl;
    cout << "Best case time   : " << diffBest << " seconds" << endl;
    cout << "Average case time: " << diffAvg << " seconds" << endl;
    cout << "Worst case time  : " << diffWorst << " seconds" << endl;
    cout << "Results written to: " << outFilename << endl;

    return 0;
}
