// *******************
// Program: radix_sort_step.cpp
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
 
using namespace std;
 
const int DIGITS = 10; // every integer in the dataset has exactly 10 digits
 
struct Record {
    long long id;   // 10-digit unique positive integer
    string code;    // 5-letter lowercase string
};
 
// ---- extract the digit at "position" places from the right (0 = units) ----
int getDigitAt(long long number, int position) {
    long long divisor = 1;
    for (int k = 0; k < position; k++) divisor *= 10;
    return (int)((number / divisor) % 10);
}
 
// ---- format a vector<Record> exactly like the sample output ----
// e.g. [1000000038/uoren, 1000000009/igerk, ...] original
string formatLine(const vector<Record>& data, const string& label) {
    ostringstream oss;
    oss << "[";
    for (size_t i = 0; i < data.size(); i++) {
        oss << data[i].id << "/" << data[i].code;
        if (i + 1 < data.size()) oss << ", ";
    }
    oss << "] " << label;
    return oss.str();
}
 
// ---- bucketSort(S, 10): stable sort of "data" by the digit at "position" ----
void bucketSortByDigit(vector<Record>& data, int position) {
    vector<vector<Record> > bucket(10); // N = 10 buckets, one per digit 0-9
 
    // distribute
    for (size_t i = 0; i < data.size(); i++) {
        int d = getDigitAt(data[i].id, position);
        bucket[d].push_back(data[i]);
    }
 
    // collect back in order, preserving relative order within each bucket
    // (this is what makes the sort stable)
    data.clear();
    for (int b = 0; b < 10; b++) {
        for (size_t j = 0; j < bucket[b].size(); j++) {
            data.push_back(bucket[b][j]);
        }
    }
}
 
// ---- radixSort(S, d), writing each pass's resulting state to outFile ----
void radixSortWithSteps(vector<Record>& data, int d, ofstream& outFile) {
    outFile << formatLine(data, "original") << "\n";
 
    for (int i = d; i >= 1; i--) {
        // i = d on the first pass -> rightmost digit (position 0)
        // i = 1 on the last pass  -> leftmost digit  (position d-1)
        int digitPosition = d - i;
        bucketSortByDigit(data, digitPosition);
        outFile << formatLine(data, "d=" + to_string(i)) << "\n";
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
 
// ---- pull n out of a filename like "dataset_1000.csv" -> "1000" ----
string extractN(const string& filename) {
    size_t start = filename.find('_');
    size_t end = filename.find(".csv");
    if (start == string::npos || end == string::npos) return "0";
    return filename.substr(start + 1, end - start - 1);
}
 
int main(int argc, char** argv) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <dataset_n.csv>" << endl;
        return 1;
    }
    string datasetFile = argv[1]; int startRow = 1; int endRow = 7;
 
    // ----------------------------------------------------------------
 
    vector<Record> fullData;
    if (!readDataset(datasetFile, fullData)) {
        return 1;
    }
 
    if (startRow < 1 || endRow > (int)fullData.size() || startRow > endRow) {
        cerr << "Error: invalid start/end row for a dataset of size "
             << fullData.size() << endl;
        return 1;
    }
 
    // extract the requested sub-range (1-indexed, inclusive)
    vector<Record> subset(fullData.begin() + (startRow - 1), fullData.begin() + endRow);
 
    string n = extractN(datasetFile);
    string outName = "dataset_" + n + "_radix_sorted_step_" +
                      to_string(startRow) + "_" + to_string(endRow) + ".txt";
 
    ofstream outFile(outName);
    if (!outFile.is_open()) {
        cerr << "Error: cannot create output file " << outName << endl;
        return 1;
    }
 
    radixSortWithSteps(subset, DIGITS, outFile);
    outFile.close();
 
    cout << "Done. Steps written to " << outName << endl;
    return 0;
}