// *******************
// Program: radix_sort.cpp
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

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <chrono>
 
using namespace std;
 
const int DIGITS = 10; // every integer in the dataset has exactly 10 digits
 
struct Record {
    long long id;
    string code;
};
 
// ---- extract the digit at "position" places from the right (0 = units) ----
int getDigitAt(long long number, int position) {
    long long divisor = 1;
    for (int k = 0; k < position; k++) divisor *= 10;
    return (int)((number / divisor) % 10);
}
 
// ---- bucketSort(S, 10): stable sort of "data" by the digit at "position" ----
void bucketSortByDigit(vector<Record>& data, int position) {
    vector<vector<Record> > bucket(10);
 
    for (size_t i = 0; i < data.size(); i++) {
        int d = getDigitAt(data[i].id, position);
        bucket[d].push_back(data[i]);
    }
 
    data.clear();
    for (int b = 0; b < 10; b++) {
        for (size_t j = 0; j < bucket[b].size(); j++) {
            data.push_back(bucket[b][j]);
        }
    }
}
 
// ---- radixSort(S, d): for i = d downto 1, bucketSort(S, 10) ----
void radixSort(vector<Record>& data, int d) {
    for (int i = d; i >= 1; i--) {
        int digitPosition = d - i;
        bucketSortByDigit(data, digitPosition);
    }
}
 
// ---- read the whole dataset csv into memory ----
bool readDataset(const string& filename, vector<Record>& out) {
    ifstream in(filename);
    if (!in.is_open()) {
        cerr << "Error: cannot open file " << filename << endl;
        return false;
    }
    string line;
    while (getline(in, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        string idStr, code;
        getline(ss, idStr, ',');
        getline(ss, code, ',');
        Record r;
        r.id = stoll(idStr);
        r.code = code;
        out.push_back(r);
    }
    in.close();
    return true;
}
 
// ---- pull n out of a filename like "dataset_1000000.csv" -> "1000000" ----
string extractN(const string& filename) {
    size_t start = filename.find('_');
    size_t end = filename.find(".csv");
    if (start == string::npos || end == string::npos) return "0";
    return filename.substr(start + 1, end - start - 1);
}
 
int main(int argc, char** argv) {

    // Pass the dataset path as argv[1]; falls back to the default below.
    string datasetFile = (argc > 1) ? argv[1] : "dataset_1000.csv";
 
    // ----------------------------------------------------------------
 
    vector<Record> data;
    if (!readDataset(datasetFile, data)) {   // file I/O - NOT timed
        return 1;
    }
 
    auto startTime = chrono::high_resolution_clock::now();
    radixSort(data, DIGITS);                  // sorting only - TIMED
    auto endTime = chrono::high_resolution_clock::now();
 
    double elapsedSeconds = chrono::duration<double>(endTime - startTime).count();
 
    string n = extractN(datasetFile);
    string outName = "radix_sorted_dataset_" + n + ".csv";
 
    ofstream outFile(outName);                // file I/O - NOT timed
    if (!outFile.is_open()) {
        cerr << "Error: cannot create output file " << outName << endl;
        return 1;
    }
    for (size_t i = 0; i < data.size(); i++) {
        outFile << data[i].id << "/" << data[i].code << "\n";
    }
    outFile << "# Radix sort running time (seconds): " << elapsedSeconds << "\n";
    outFile.close();
 
    cout << "Radix Sort complete." << endl;
    cout << "Dataset size      : " << data.size() << endl;
    cout << "Running time (sec): " << elapsedSeconds << endl;
    cout << "Sorted output     : " << outName << endl;
 
    return 0;
}
 