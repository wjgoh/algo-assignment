// heap_sort.cpp
// CCP6214 Assignment - Heap Sort (using maxheap), sorting by the 10-digit integer key.
//
// Build:  g++ -O2 -std=c++17 "heap_sort.cpp" -o heap_sort
// Run:    ./heap_sort dataset_1000.csv
//         (defaults to dataset_1000.csv if no argument is given)
//
// Input  : a CSV where each row is  <10-digit integer>,<5-letter string>
// Output : heap_sort_dataset_<n>.csv  with every element in ascending order
//          (rows kept as integer,string). Running time is printed to the
//          console and appended into the output file.
//
// Constraints honoured:
//  - No library sort / no priority_queue: the maxheap is implemented by hand.
//  - Sorting is by the integer key; the attached string travels with it.
//  - The measured region excludes all file I/O.

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <chrono>

struct Record {
    long long key;      // the 10-digit integer (sort key)
    std::string str;    // the 5-letter attached string
};

// Sift the element at index `root` down through a heap of logical size `size`,
// restoring the max-heap property (largest key at the top).
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

// Ascending heap sort using a max-heap.
static void heapSort(std::vector<Record>& a) {
    int n = static_cast<int>(a.size());

    // Build the max-heap (heapify from the last internal node upward).
    for (int i = n / 2 - 1; i >= 0; --i)
        siftDown(a, i, n);

    // Repeatedly move the current max to the end, then re-heapify the rest.
    for (int end = n - 1; end >= 1; --end) {
        std::swap(a[0], a[end]);
        siftDown(a, 0, end);
    }
}

int main(int argc, char** argv) {
    std::string inputFile = (argc > 1) ? argv[1] : "dataset_1000.csv";

    // ---- Read input (NOT timed) ----
    std::ifstream in(inputFile);
    if (!in) {
        std::cerr << "Error: cannot open input file '" << inputFile << "'\n";
        return 1;
    }

    std::vector<Record> data;
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line);
        std::string keyStr, str;
        if (!std::getline(ss, keyStr, ',')) continue;
        std::getline(ss, str);
        data.push_back({std::stoll(keyStr), str});
    }
    in.close();

    // ---- Sort (TIMED region: no I/O inside) ----
    auto start = std::chrono::high_resolution_clock::now();
    heapSort(data);
    auto finish = std::chrono::high_resolution_clock::now();

    double seconds = std::chrono::duration<double>(finish - start).count();

    // ---- Write output (NOT timed) ----
    std::size_t n = data.size();
    std::string outputFile = "heap_sort_dataset_" + std::to_string(n) + ".csv";
    std::ofstream out(outputFile);
    if (!out) {
        std::cerr << "Error: cannot write output file '" << outputFile << "'\n";
        return 1;
    }
    for (const auto& r : data)
        out << r.key << ',' << r.str << '\n';

    // Running time goes to console AND into the output file (per spec).
    out << "Running time: " << seconds << " seconds\n";
    out.close();

    std::cout << "Sorted " << n << " elements -> " << outputFile << '\n';
    std::cout << "Running time: " << seconds << " seconds\n";
    return 0;
}
