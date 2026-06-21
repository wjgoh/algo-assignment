#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <chrono>

using namespace std;

// ==========================================
// 1. DATA RECORD STRUCTURE
// ==========================================
struct Record {
    long long key;
    string value;
};

// ==========================================
// 2. LINKED-LIST-BASED AVL TREE
// ==========================================
struct AVLNode {
    Record data;
    AVLNode* left;
    AVLNode* right;
    int height;

    AVLNode(Record r) : data(r), left(nullptr), right(nullptr), height(1) {}
};

int getHeight(AVLNode* node) {
    if (node != nullptr) {
        return node->height;
    } else {
        return 0;
    }
}

int getBalanceFactor(AVLNode* node) {
    if (node != nullptr) {
        return getHeight(node->left) - getHeight(node->right);
    } else {
        return 0;
    }
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
    if (node == nullptr) {
        return new AVLNode(r);
    }

    if (r.key < node->data.key) {
        node->left = insertAVLNode(node->left, r);
    } else if (r.key > node->data.key) {
        node->right = insertAVLNode(node->right, r);
    } else {
        return node; // Duplicates are ignored
    }

    node->height = 1 + max(getHeight(node->left), getHeight(node->right));
    int balance = getBalanceFactor(node);

    // Left Left Case
    if (balance > 1 && r.key < node->left->data.key) {
        return rotateRight(node);
    }
    // Right Right Case
    if (balance < -1 && r.key > node->right->data.key) {
        return rotateLeft(node);
    }
    // Left Right Case
    if (balance > 1 && r.key > node->left->data.key) {
        node->left = rotateLeft(node->left);
        return rotateRight(node);
    }
    // Right Left Case
    if (balance < -1 && r.key < node->right->data.key) {
        node->right = rotateRight(node->right);
        return rotateLeft(node);
    }

    return node;
}

// Memory cleanup helper for AVL Tree to prevent memory leaks
void destroyTree(AVLNode* node) {
    if (node != nullptr) {
        destroyTree(node->left);
        destroyTree(node->right);
        delete node;
    }
}

// ==========================================
// 3. MASTER HASH TABLE OBJECT
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
        for (int i = 0; i < tableSize; ++i) {
            table[i] = nullptr;
        }
    }

    ~HashTableLL() {
        for (int i = 0; i < tableSize; ++i) {
            destroyTree(table[i]);
        }
        delete[] table;
    }

    void insert(Record r) {
        int index = hashFunction(r.key);
        table[index] = insertAVLNode(table[index], r);
    }

    // Step verification tracker requirement (Outputs path to file)
    bool searchStep(long long targetKey, ofstream& outFile) {
        int index = hashFunction(targetKey);
        AVLNode* current = table[index];

        while (current != nullptr) {
            outFile << current->data.key << " ";
            if (current->data.key == targetKey) {
                outFile << "= " << current->data.key << "/" << current->data.value << "\n";
                return true;
            }
            if (targetKey < current->data.key) {
                current = current->left;
            } else {
                current = current->right;
            }
        }
        outFile << "-1 != " << targetKey << "\n";
        return false;
    }

    // Silent search for benchmarking (No I/O overhead as requested in PDF)
    bool searchSilent(long long targetKey) {
        int index = hashFunction(targetKey);
        AVLNode* current = table[index];

        while (current != nullptr) {
            if (current->data.key == targetKey) {
                return true;
            }
            if (targetKey < current->data.key) {
                current = current->left;
            } else {
                current = current->right;
            }
        }
        return false;
    }
};

// ==========================================
// 4. BENCHMARK & EXECUTION LOGIC
// ==========================================
void runBenchmark(HashTableLL& ht, Record* dataset, long long size) {
    string outFilename = "hash_table_search_dataset_" + to_string(size) + ".txt";
    ofstream outFile(outFilename);

    if(!outFile.is_open()) {
        cout << "Error opening benchmark output file." << endl;
        return;
    }

    // BEST CASE: Search for a known root element N times
    long long bestTarget = dataset[0].key;
    auto startBest = chrono::high_resolution_clock::now();
    for (int i = 0; i < size; ++i) ht.searchSilent(bestTarget);
    auto endBest = chrono::high_resolution_clock::now();
    chrono::duration<double> diffBest = endBest - startBest;

    // AVERAGE CASE: Search for randomly existing elements N times
    auto startAvg = chrono::high_resolution_clock::now();
    for (int i = 0; i < size; ++i) ht.searchSilent(dataset[i].key);
    auto endAvg = chrono::high_resolution_clock::now();
    chrono::duration<double> diffAvg = endAvg - startAvg;

    // WORST CASE: Search for an element guaranteed to not exist N times
    long long worstTarget = 123456789; // Positive dummy value so modulo doesn't crash
    auto startWorst = chrono::high_resolution_clock::now();
    for (int i = 0; i < size; ++i) ht.searchSilent(worstTarget);
    auto endWorst = chrono::high_resolution_clock::now();
    chrono::duration<double> diffWorst = endWorst - startWorst;

    // Output to File
    outFile << "Best case time: " << diffBest.count() << " seconds\n";
    outFile << "Average case time: " << diffAvg.count() << " seconds\n";
    outFile << "Worst case time: " << diffWorst.count() << " seconds\n";
    outFile.close();

    // Output to Command Prompt (Required for Screenshots)
    cout << "\n--- BENCHMARK RESULTS (" << size << " records) ---" << endl;
    cout << "Best case time: " << diffBest.count() << " seconds" << endl;
    cout << "Average case time: " << diffAvg.count() << " seconds" << endl;
    cout << "Worst case time: " << diffWorst.count() << " seconds" << endl;
    cout << "---------------------------------------" << endl;
    cout << "Benchmark results written to: " << outFilename << endl;
}

int main() {
    long long dataSize;
    cout << "Enter the dataset size (e.g., 1000): ";
    cin >> dataSize;

    // Point the code to the Dataset Generator folder
    string filename = "../Dataset Generator/dataset_" + to_string(dataSize) + ".csv";

    // Dynamic array to hold records (avoids banned standard libraries like vector)
    Record* dataset = new Record[dataSize];

    // Hash table size (Using a prime number close to size is best practice)
    int primeTableSize = dataSize + 9;
    HashTableLL ht(primeTableSize);

    // 1. Read CSV File
    ifstream inFile(filename);
    if (!inFile.is_open()) {
        cout << "\nERROR: Could not open " << filename << "." << endl;
        cout << "Please ensure the CSV file is in the same folder as this program." << endl;
        delete[] dataset;
        return 1;
    }

    string line, keyStr, valueStr;
    int count = 0;
    while (getline(inFile, line) && count < dataSize) {
        stringstream ss(line);
        getline(ss, keyStr, ',');
        getline(ss, valueStr, ',');

        dataset[count].key = stoll(keyStr);
        dataset[count].value = valueStr;

        // Populate the Hash Table
        ht.insert(dataset[count]);
        count++;
    }
    inFile.close();
    cout << "Successfully loaded " << count << " records into Hash Table." << endl;

    // 2. Perform Step Search
    // We grab a target we know exists, and a dummy target that doesn't
    long long targetFound = dataset[dataSize/2].key;
    long long targetNotFound = 123456789;

    string stepFilename = "dataset_" + to_string(dataSize) + "_hash_table_search_step_target.txt";
    ofstream stepFile(stepFilename);

    stepFile << "Target " << targetFound << " (Expected to find):\n";
    ht.searchStep(targetFound, stepFile);

    stepFile << "\nTarget " << targetNotFound << " (Expected NOT to find):\n";
    ht.searchStep(targetNotFound, stepFile);

    stepFile.close();
    cout << "Step search logic written to: " << stepFilename << endl;

    // 3. Run Benchmarks
    runBenchmark(ht, dataset, dataSize);

    // Clean up dynamic memory
    delete[] dataset;

    cout << "\nProgram complete. Press Enter to exit.";
    cin.ignore();
    cin.get();

    return 0;
}
