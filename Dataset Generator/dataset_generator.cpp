// ******
// Program: dataset_generator.cpp
// Course: CCP6214 Algorithm Design and Analysis
// Lecture Class: TC4L
// Tutorial Class: T13L
// Trimester: 2610
// ******
//
// Produces n rows of:  <unique random 10-digit integer>,<5 lowercase letters>
//   - integers are UNIQUE, random, positive, in [1,000,000,000 .. 9,999,999,999]
//   - elements are emitted in random order (shuffled before writing)
//
// Uniqueness is guaranteed by construction (a strictly increasing sequence),
// so NO data structure that searches internally is used anywhere. The order is
// then randomized with a hand-written Fisher-Yates shuffle.
//
// Seed: derived from the group leader's student ID 242UC244S9 using the
// assignment's letter->digit table (U=1, C=3, S=9):
//       2 4 2 U C 2 4 4 S 9  ->  2 4 2 1 3 2 4 4 9 9  ->  2421324499

#include <iostream>
#include <fstream>
#include <string>
#include <random>

using namespace std;

int main(int argc, char** argv) {
    // ---- Group-leader seed (student ID 242UC244S9) ----
    std::mt19937_64 rng(2421324499ULL);

    // Grab the size from the command line argument
    long long n = (argc > 1) ? std::stoll(argv[1]) : 1000;
    if (n <= 0) {
        std::cerr << "Error: size must be a positive integer.\n";
        return 1;
    }

    const long long LO = 1000000000LL;   // smallest 10-digit number
    const long long HI = 9999999999LL;   // largest 10-digit number
    const long long SPAN = HI - LO;      // 8,999,999,999 values of head-room
    if (n > SPAN + 1) {
        std::cerr << "Error: n exceeds the count of available unique 10-digit integers.\n";
        return 1;
    }

    // Dynamic step: largest jump that still keeps ALL n values within
    // [LO, HI]. Since n * maxStep <= SPAN, the final value never exceeds HI,
    // so every integer stays exactly 10 digits. For small n the steps are
    // large (values spread across the whole range); for large n they shrink.
    long long maxStep = SPAN / n;
    if (maxStep < 1) maxStep = 1;
    std::uniform_int_distribution<long long> stepDist(1, maxStep);

    // Generate sequential unique 10-digit numbers with random jumps.
    long long* numbers = new long long[n];
    long long current = LO;
    for (long long i = 0; i < n; ++i) {
        current += stepDist(rng);
        numbers[i] = current;
    }

    // Shuffle the integers (Fisher-Yates) so they are in random order before sorting.
    for (long long i = n - 1; i > 0; --i) {
        std::uniform_int_distribution<long long> indexDist(0, i);
        long long j = indexDist(rng);
        long long temp = numbers[i];
        numbers[i] = numbers[j];
        numbers[j] = temp;
    }

    std::string outputFile = "dataset_" + std::to_string(n) + ".csv";
    std::ofstream out(outputFile);
    if (!out) {
        std::cerr << "Error: cannot write output file '" << outputFile << "'\n";
        delete[] numbers;
        return 1;
    }

    // Generate the 5-letter lowercase strings and write to CSV.
    std::uniform_int_distribution<int> letterDist(0, 25);
    for (long long i = 0; i < n; ++i) {
        std::string randomStr = "";
        for (int c = 0; c < 5; ++c) {
            randomStr += static_cast<char>('a' + letterDist(rng));
        }
        out << numbers[i] << "," << randomStr << "\n";
    }

    out.close();
    delete[] numbers;

    std::cout << "Generated " << n << " unique rows -> " << outputFile << '\n';
    return 0;
}
