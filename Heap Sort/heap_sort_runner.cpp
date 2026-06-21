// heap_sort_runner.cpp
// CCP6214 Assignment - Heap Sort experiment harness.
//
// Feeds every dataset into the heap sort (maxheap), times ONLY the sort
// (I/O excluded, as required), writes a sorted output file per dataset, and
// prints a summary table of running times to the console.
//
// Build:  g++ -O2 -std=c++17 "heap_sort_runner.cpp" -o heap_sort_runner
// Run:    ./heap_sort_runner                  -> runs the default 10 sizes
//         ./heap_sort_runner 1000 10000 ...    -> runs only the sizes you list
//
// Datasets are read from DATASET_DIR (the "Dataset Generator" folder) using the
// pattern dataset_<n>.csv. For each, it writes heap_sort_dataset_<n>.csv and
// records the sort time. A copy of the table is also saved to
// heap_sort_timings.txt for the report.

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <chrono>
#include <iomanip>

// Location of the dataset_<n>.csv files, relative to where you run the program.
static const std::string DATASET_DIR = "../Dataset Generator/";

// The 10 experiment sizes (used when no sizes are passed on the command line).
static const std::vector<long long> DEFAULT_SIZES = {
    1000, 10000, 100000, 500000, 1000000,
    5000000, 10000000, 50000000, 100000000, 200000000
};

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

// Ascending heap sort using a max-heap.
static void heapSort(std::vector<Record>& a) {
    int n = static_cast<int>(a.size());
    for (int i = n / 2 - 1; i >= 0; --i)
        siftDown(a, i, n);
    for (int end = n - 1; end >= 1; --end) {
        std::swap(a[0], a[end]);
        siftDown(a, 0, end);
    }
}

struct Result {
    long long n;        // requested size
    long long rows;     // rows actually read
    double seconds;     // sort time (I/O excluded)
    bool ok;            // false if the dataset was missing
};

static Result runOne(long long n) {
    Result r{n, 0, 0.0, false};
    std::string inputFile = DATASET_DIR + "dataset_" + std::to_string(n) + ".csv";

    // ---- Read (NOT timed) ----
    std::ifstream in(inputFile);
    if (!in) {
        std::cerr << "  [skip] cannot open " << inputFile << '\n';
        return r;
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
    r.rows = static_cast<long long>(data.size());

    // ---- Sort (TIMED region: no I/O inside) ----
    auto start = std::chrono::high_resolution_clock::now();
    heapSort(data);
    auto finish = std::chrono::high_resolution_clock::now();
    r.seconds = std::chrono::duration<double>(finish - start).count();
    r.ok = true;

    // ---- Write sorted output (NOT timed) ----
    std::string outputFile = "heap_sort_dataset_" + std::to_string(r.rows) + ".csv";
    std::ofstream out(outputFile);
    if (out) {
        for (const auto& rec : data)
            out << rec.key << ',' << rec.str << '\n';
        out << "Running time: " << r.seconds << " seconds\n";
        out.close();
    }

    std::cout << "  sorted " << r.rows << " rows in "
              << r.seconds << " s -> " << outputFile << '\n';
    return r;
}

// Print the results table to any output stream.
static void printTable(std::ostream& os, const std::vector<Result>& results) {
    os << '\n';
    os << "+----------------+----------------+--------------------+\n";
    os << "|  Dataset size  |   Elements     |   Sort time (s)    |\n";
    os << "+----------------+----------------+--------------------+\n";
    for (const auto& r : results) {
        os << "| " << std::setw(14) << r.n << " | ";
        if (r.ok) {
            os << std::setw(14) << r.rows << " | "
               << std::setw(18) << std::fixed << std::setprecision(6) << r.seconds << " |\n";
        } else {
            os << std::setw(14) << "-" << " | "
               << std::setw(18) << "MISSING" << " |\n";
        }
    }
    os << "+----------------+----------------+--------------------+\n";
}

int main(int argc, char** argv) {
    std::vector<long long> sizes;
    if (argc > 1) {
        for (int i = 1; i < argc; ++i) sizes.push_back(std::stoll(argv[i]));
    } else {
        sizes = DEFAULT_SIZES;
    }

    std::vector<Result> results;
    for (long long n : sizes) {
        std::cout << "Running dataset_" << n << ".csv ...\n";
        results.push_back(runOne(n));
    }

    // Print the summary table to the console.
    printTable(std::cout, results);

    // Save a copy for the report.
    std::ofstream tf("heap_sort_timings.txt");
    if (tf) {
        tf << "Heap Sort (maxheap) - running time per dataset (sort only, I/O excluded)\n";
        printTable(tf, results);
        tf.close();
        std::cout << "\nTable also saved to heap_sort_timings.txt\n";
    }
    return 0;
}
