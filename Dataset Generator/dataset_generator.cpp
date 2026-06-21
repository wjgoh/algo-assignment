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
#include <vector>          // raw contiguous storage only (no sorting/searching API used)

// ----------------------------------------------------------------------------
// Hand-built open-addressing hash set (linear probing).
//
// The assignment forbids any standard-library container that searches/sorts
// internally (e.g. std::unordered_set). To guarantee that the generated keys
// are unique we therefore implement our own hash set from scratch, backed only
// by a plain array of slots. std::vector is used purely as raw memory.
// ----------------------------------------------------------------------------
class UniqueKeySet {
public:
    explicit UniqueKeySet(long long expected) {
        // pick a power-of-two capacity at least 2x the expected element count
        std::size_t cap = 16;
        while (cap < static_cast<std::size_t>(expected) * 2 + 1) cap <<= 1;
        slots_.assign(cap, EMPTY);
        mask_ = cap - 1;
    }

    // returns true if 'key' was newly inserted, false if it was already present
    bool insert(long long key) {
        std::size_t i = hash(key) & mask_;
        while (slots_[i] != EMPTY) {
            if (slots_[i] == key) return false;   // already present
            i = (i + 1) & mask_;                   // linear probe
        }
        slots_[i] = key;
        return true;
    }

private:
    static constexpr long long EMPTY = -1;        // keys are always positive

    static std::size_t hash(long long key) {
        // 64-bit fibonacci-style mix
        unsigned long long x = static_cast<unsigned long long>(key);
        x ^= x >> 33;
        x *= 0xff51afd7ed558ccdULL;
        x ^= x >> 33;
        return static_cast<std::size_t>(x);
    }

    std::vector<long long> slots_;
    std::size_t mask_;
};

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

    UniqueKeySet seen(n);

    for (long long i = 0; i < n; ++i) {
        long long key;
        do {
            key = keyDist(rng);
        } while (!seen.insert(key));   // re-draw on collision -> uniqueness

        std::string s(5, 'a');
        for (int j = 0; j < 5; ++j)
            s[j] = static_cast<char>('a' + letterDist(rng));

        out << key << ',' << s << '\n';
    }
    out.close();

    std::cout << "Generated " << n << " unique rows -> " << outputFile << '\n';
    return 0;
}
