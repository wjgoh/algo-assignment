// dataset_generator.cpp
// CCP6214 Assignment - Dataset generator.
//
// Build:  g++ -O2 -std=c++17 "dataset_generator.cpp" -o dataset_generator
// Run:    ./dataset_generator 1000        -> writes dataset_1000.csv
//
// Produces n rows of:  <unique random 10-digit integer>,<5 lowercase letters>
//   - integers are UNIQUE, random, positive, in [1,000,000,000 .. 9,999,999,999]
//   - elements are emitted in random order (no sorting)
//
// Seed: derived from the group leader's student ID 242UC244S9 using the
// assignment's letter->digit table (U=1, C=3, S=9):
//       2 4 2 U C 2 4 4 S 9  ->  2 4 2 1 3 2 4 4 9 9  ->  2421324499
// The seed is set right after main() enters, per the assignment instruction.

#include <iostream>
#include <fstream>
#include <string>
#include <random>
#include <unordered_set>   // used ONLY to guarantee uniqueness of generated keys,
                           // not as part of any sorting/searching algorithm.

int main(int argc, char** argv) {
    // ---- Group-leader seed (student ID 242UC244S9) ----
    std::mt19937_64 rng(2421324499);

    long long n = (argc > 1) ? std::stoll(argv[1]) : 1000;
    if (n <= 0) {
        std::cerr << "Error: size must be a positive integer.\n";
        return 1;
    }

    const long long LO = 1000000000LL;   // smallest 10-digit number
    const long long HI = 9999999999LL;   // largest 10-digit number
    if (n > (HI - LO + 1)) {
        std::cerr << "Error: n exceeds the count of available unique 10-digit integers.\n";
        return 1;
    }

    std::uniform_int_distribution<long long> keyDist(LO, HI);
    std::uniform_int_distribution<int> letterDist(0, 25);

    std::string outputFile = "dataset_" + std::to_string(n) + ".csv";
    std::ofstream out(outputFile);
    if (!out) {
        std::cerr << "Error: cannot write output file '" << outputFile << "'\n";
        return 1;
    }

    std::unordered_set<long long> seen;
    seen.reserve(static_cast<std::size_t>(n) * 2);

    for (long long i = 0; i < n; ++i) {
        long long key;
        do {
            key = keyDist(rng);
        } while (!seen.insert(key).second);   // re-draw on collision -> uniqueness

        std::string s(5, 'a');
        for (int j = 0; j < 5; ++j)
            s[j] = static_cast<char>('a' + letterDist(rng));

        out << key << ',' << s << '\n';
    }
    out.close();

    std::cout << "Generated " << n << " unique rows -> " << outputFile << '\n';
    return 0;
}
