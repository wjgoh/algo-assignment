// ***
// Program: heap_sort_step.cpp
// Course: CCP6214 Algorithm Design and Analysis
// Lecture Class: TC2L
// Tutorial Class: TT8L
// Trimester: 2610
// Member_1: 242UC244S9 | GOH WEI JING | goh.wei.jing@student.mmu.edu.my | 01110872022
// Member_2: 243UC247DJ | WONG KAI SHEN | wong.kai.shen@student.mmu.edu.my | 0167129682
// Member_3: 251UC2517Z | JAYAVARMAN THIYAGU | Jayavarman.thiyagu@student.mmu.edu.my | 0169441376
// Member_4: ID | SAMIEON NGIAM TUN SHEN | SAMIEON.NGIAM.TUN@student.mmu.edu.my | 0169515810
// ***
// Task Distribution
// Member_1: RADIX SORT
// Member_2: HEAP
// Member_3: DATA GENERATION
// Member_4: HASH TABLE
// ***
// 
// // heap_sort_step.cpp
// CCP6214 Assignment - Heap Sort step tracer (using maxheap).
//
// Build:  g++ -O2 -std=c++17 "heap_sort_step.cpp" -o heap_sort_step
// Run:    ./heap_sort_step dataset_1000.csv
//
// The tutor specifies start row and end row IN THE CODE FILE (see the two
// constants below). The program heap-sorts the rows from START_ROW to END_ROW
// (1-indexed, inclusive) and writes every sorting step.
//
// Output: dataset_<n>_heap_sorted_step_<start>_<end>.txt
//
// Step format (matches the assignment sample):
//   [int/str, int/str, ...] initial      <- array right after building the maxheap
//   [int/str, ...] i = 6                  <- array after each extraction, i = heap size
//   ...
//   [...] i = 1

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

// ---- Tutor-specified rows (1-indexed, inclusive) ----
static const int START_ROW = 1;
static const int END_ROW   = 7;

struct Record {
    long long key;
    std::string str;
};

static void siftDown(std::vector<Record>& a, int root, int size) {
    while (true) {
        int largest = root;
        int left = 2 * root + 1;
        int right = 2 * root + 2;
        if (left < size && a[left].key > a[largest].key) largest = left;
        if (right < size && a[right].key > a[largest].key) largest = right;
        if (largest == root) break;
        std::swap(a[root], a[largest]);
        root = largest;
    }
}

// Print the whole array as  [key/str, key/str, ...] <label>
static void printStep(std::ofstream& out, const std::vector<Record>& a,
                      const std::string& label) {
    out << '[';
    for (std::size_t i = 0; i < a.size(); ++i) {
        out << a[i].key << '/' << a[i].str;
        if (i + 1 < a.size()) out << ", ";
    }
    out << "] " << label << '\n';
}

int main(int argc, char** argv) {
    std::string inputFile = (argc > 1) ? argv[1] : "dataset_1000.csv";

    std::ifstream in(inputFile);
    if (!in) {
        std::cerr << "Error: cannot open input file '" << inputFile << "'\n";
        return 1;
    }

    // Read all rows, then keep only [START_ROW .. END_ROW].
    std::vector<Record> all;
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line);
        std::string keyStr, str;
        if (!std::getline(ss, keyStr, ',')) continue;
        std::getline(ss, str);
        all.push_back({std::stoll(keyStr), str});
    }
    in.close();

    if (START_ROW < 1 || END_ROW > static_cast<int>(all.size()) || START_ROW > END_ROW) {
        std::cerr << "Error: invalid START_ROW/END_ROW for a file of "
                  << all.size() << " rows.\n";
        return 1;
    }

    std::vector<Record> a(all.begin() + (START_ROW - 1), all.begin() + END_ROW);

    // Derive dataset size n from the input filename pattern dataset_<n>.csv,
    // falling back to the total row count.
    std::size_t n = all.size();

    std::string outputFile = "dataset_" + std::to_string(n) +
                             "_heap_sorted_step_" + std::to_string(START_ROW) +
                             "_" + std::to_string(END_ROW) + ".txt";
    std::ofstream out(outputFile);
    if (!out) {
        std::cerr << "Error: cannot write output file '" << outputFile << "'\n";
        return 1;
    }

    int size = static_cast<int>(a.size());

    // Build the max-heap, then print the initial heap.
    for (int i = size / 2 - 1; i >= 0; --i)
        siftDown(a, i, size);
    printStep(out, a, "initial");

    // Extraction phase: after each extraction, i = current heap size.
    for (int end = size - 1; end >= 1; --end) {
        std::swap(a[0], a[end]);
        siftDown(a, 0, end);
        printStep(out, a, "i = " + std::to_string(end));
    }
    out.close();

    std::cout << "Heap-sort steps for rows " << START_ROW << ".." << END_ROW
              << " -> " << outputFile << '\n';
    return 0;
}
