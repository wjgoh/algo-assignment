#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <chrono>
#include <iomanip>

using namespace std;

// Location of the dataset_<n>.csv files, relative to where you run the program.
static const string DATASET_DIR = "../Dataset Generator/";

// The experiment sizes to run automatically
static const vector<long long> DEFAULT_SIZES = {
    1000, 10000, 100000, 500000, 1000000,
    5000000, 10000000, 50000000, 100000000, 200000000
};

// ==========================================
// DATA RECORD & AVL TREE (Collision Handling)
// ==========================================
struct Record {
    long long key;
    string str;
};

struct AVLNode {
    Record data;
    AVLNode* left;
    AVLNode* right;
    int height;
    AVLNode(Record r) : data(r), left(nullptr), right(nullptr), height(1) {}
};

int getHeight(AVLNode* node) {
    return node != nullptr ? node->height : 0;
}

int getBalanceFactor(AVLNode* node) {
    return node != nullptr ? getHeight(node->left) - getHeight(node->right) : 0;
}

AVLNode* rotateRight(AVLNode* y) {
    AVLNode* x = y->left;
    AVLNode* T2 = x->right;
    x->right = y;
    y->left = T2;
    y->height = 1 + max(getHeight(y->left), getHeight(y->right));
    x->height = 1 + max(getHeight(x->left), getHeight(x->right));
    return x;
}

AVLNode* rotateLeft(AVLNode* x) {
    AVLNode* y = x->right;
    AVLNode* T2 = y->left;
    y->left = x;
    x->right = T2;
    x->height = 1 + max(getHeight(x->left), getHeight(x->right));
    y->height = 1 + max(getHeight(y->left), getHeight(y->right));
    return y;
}

AVLNode* insertAVLNode(AVLNode* node, Record r) {
    if (node == nullptr) return new AVLNode(r);

    if (r.key < node->data.key) node->left = insertAVLNode(node->left, r);
    else if (r.key > node->data.key) node->right = insertAVLNode(node->right, r);
    else return node; // Ignore duplicates

    node->height = 1 + max(getHeight(node->left), getHeight(node->right));
    int balance = getBalanceFactor(node);

    if (balance > 1 && r.key < node->left->data.key) return rotateRight(node);
    if (balance < -1 && r.key > node->right->data.key) return rotateLeft(node);
    if (balance > 1 && r.key > node->left->data.key) {
        node->left = rotateLeft(node->left);
        return rotateRight(node);
    }
    if (balance < -1 && r.key < node->right->data.key) {
        node->right = rotateRight(node->right);
        return rotateLeft(node);
    }
    return node;
}

void destroyTree(AVLNode* node) {
    if (node != nullptr) {
        destroyTree(node->left);
        destroyTree(node->right);
        delete node;
    }
}

// ==========================================
// HASH TABLE OBJECT
// ==========================================
class HashTableLL {
private:
    AVLNode** table;
    int tableSize;

    int hashFunction(long long key) {
        int index = key % tableSize;
        // Safety check to ensure negative keys don't cause out-of-bounds array access
        return index < 0 ? index + tableSize : index;
    }

public:
    HashTableLL(int size) {
        tableSize = size;
        table = new AVLNode*[tableSize];
        for (int i = 0; i < tableSize; ++i) table[i] = nullptr;
    }

    ~HashTableLL() {
        for (int i = 0; i < tableSize; ++i) destroyTree(table[i]);
        delete[] table;
    }

    void insert(Record r) {
        int index = hashFunction(r.key);
        table[index] = insertAVLNode(table[index], r);
    }

    bool searchSilent(long long targetKey) {
        int index = hashFunction(targetKey);
        AVLNode* current = table[index];
        while (current != nullptr) {
            if (current->data.key == targetKey) return true;
            if (targetKey < current->data.key) current = current->left;
            else current = current->right;
        }
        return false;
    }
};

// ==========================================
// BENCHMARK RUNNER LOGIC
// ==========================================
struct Result {
    long long n;
    long long rows;
    double bestTime;
    double avgTime;
    double worstTime;
    bool ok;
};

static Result runOne(long long n) {
    Result r{n, 0, 0.0, 0.0, 0.0, false};
    string inputFile = DATASET_DIR + "dataset_" + to_string(n) + ".csv";

    // ---- Read Data (NOT timed) ----
    ifstream in(inputFile);
    if (!in) {
        cerr << "  [skip] cannot open " << inputFile << '\n';
        return r;
    }

    vector<Record> data;
    string line;
    while (getline(in, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        string keyStr, str;
        if (!getline(ss, keyStr, ',')) continue;
        getline(ss, str);
        data.push_back({stoll(keyStr), str});
    }
    in.close();
    r.rows = static_cast<long long>(data.size());

    // ---- Build Hash Table (NOT timed) ----
    HashTableLL ht(r.rows);
    for (const auto& rec : data) {
        ht.insert(rec);
    }

    // ---- Perform Searches (TIMED region) ----
    long long size = r.rows;
    long long bestTarget = data[0].key;         // Best Case: Root of an AVL tree
    long long worstTarget = 123456789;          // Worst Case: Missing target (Positive so modulo doesn't crash)

    // 1. BEST CASE (Search N times)
    auto startBest = chrono::high_resolution_clock::now();
    for (int i = 0; i < size; ++i) ht.searchSilent(bestTarget);
    auto endBest = chrono::high_resolution_clock::now();
    r.bestTime = chrono::duration<double>(endBest - startBest).count();

    // 2. AVERAGE CASE (Search N random existing targets)
    auto startAvg = chrono::high_resolution_clock::now();
    for (int i = 0; i < size; ++i) ht.searchSilent(data[i].key);
    auto endAvg = chrono::high_resolution_clock::now();
    r.avgTime = chrono::duration<double>(endAvg - startAvg).count();

    // 3. WORST CASE (Search N missing targets)
    auto startWorst = chrono::high_resolution_clock::now();
    for (int i = 0; i < size; ++i) ht.searchSilent(worstTarget);
    auto endWorst = chrono::high_resolution_clock::now();
    r.worstTime = chrono::duration<double>(endWorst - startWorst).count();
    r.ok = true;

    // ---- Write Output File (NOT timed) ----
    string outputFile = "hash_table_search_dataset_" + to_string(r.rows) + ".txt";
    ofstream out(outputFile);
    if (out) {
        out << "Best case time: " << r.bestTime << " seconds\n";
        out << "Average case time: " << r.avgTime << " seconds\n";
        out << "Worst case time: " << r.worstTime << " seconds\n";
        out.close();
    }

    cout << "  Searched " << r.rows << " rows -> " << outputFile << '\n';
    return r;
}

static void printTable(ostream& os, const vector<Result>& results) {
    os << '\n';
    os << "+----------------+----------------+----------------+----------------+----------------+\n";
    os << "|  Dataset size  |    Elements    |  Best (secs)   |  Avg (secs)    |  Worst (secs)  |\n";
    os << "+----------------+----------------+----------------+----------------+----------------+\n";
    for (const auto& r : results) {
        os << "| " << setw(14) << r.n << " | ";
        if (r.ok) {
            os << setw(14) << r.rows << " | "
               << setw(14) << fixed << setprecision(6) << r.bestTime << " | "
               << setw(14) << fixed << setprecision(6) << r.avgTime << " | "
               << setw(14) << fixed << setprecision(6) << r.worstTime << " |\n";
        } else {
            os << setw(14) << "-" << " | "
               << setw(14) << "MISSING" << " | "
               << setw(14) << "MISSING" << " | "
               << setw(14) << "MISSING" << " |\n";
        }
    }
    os << "+----------------+----------------+----------------+----------------+----------------+\n";
}

int main(int argc, char** argv) {
    vector<long long> sizes;
    if (argc > 1) {
        for (int i = 1; i < argc; ++i) sizes.push_back(stoll(argv[i]));
    } else {
        sizes = DEFAULT_SIZES;
    }

    vector<Result> results;
    for (long long n : sizes) {
        cout << "Running dataset_" << n << ".csv ...\n";
        results.push_back(runOne(n));
    }

    // Print summary table to console
    printTable(cout, results);

    return 0;
}
