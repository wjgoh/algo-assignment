// *********************************************************
// Program: dataset_generator.cpp
// Course: CCP6214 Algorithm Design and Analysis
// Lecture Class: TC2L
// Tutorial Class: TT8L
// Trimester: 2610
// Member_1: 242UC244S9 | GOH WEI JING | EMAIL | PHONE
// Member_2: 243UC247DJ | WONG KAI SHEN | EMAIL | PHONE
// Member_3: 251UC2517Z | JAYAVARMAN THIYAGU | Jayavarman.thiyagu@student.mmu.edu.my | 0169441376
// Member_4: ID | NAME | EMAIL | PHONE
// *********************************************************
// Task Distribution
// Member_1: 
// Member_2: HEAP
// Member_3: DATA GENERATION
// Member_4: 
// *********************************************************

#include <iostream>
#include <fstream>
#include <string>
#include <random>

using namespace std;

int main(int argc, char** argv) {
    // Seed derived from Group Leader ID (242UC244S9 -> 2421324499)
    std::mt19937_64 rng(2421324499ULL);

    // Retrieve dataset size from command line arguments
    long long n = (argc > 1) ? std::stoll(argv[1]) : 1000;
    if (n <= 0) {
        std::cerr << "Error: size must be a positive integer.\n";
        return 1;
    }

    // Generate sequential unique 10-digit numbers
    long long* numbers = new long long[n];
    long long current = 1000000000LL; 
    
    // Dynamically scale the gap so numbers span the entire 9-billion range
    long long maxGap = (8999999999LL / n); 
    if (maxGap < 1) maxGap = 1; // Safety fallback
    std::uniform_int_distribution<long long> stepDist(1, maxGap); 

    for(long long i = 0; i < n; ++i) {
        current += stepDist(rng);
        numbers[i] = current;
    }

    // Fisher-Yates shuffle to ensure random distribution of unique integers
    for(long long i = n - 1; i > 0; --i) {
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
        return 1;
    }

    // Append a 5-letter randomized lowercase string to each row
    std::uniform_int_distribution<int> letterDist(0, 25);

    for(long long i = 0; i < n; ++i) {
        std::string randomStr = "";
        for(int c = 0; c < 5; ++c) {
            randomStr += static_cast<char>('a' + letterDist(rng));
        }
        out << numbers[i] << "," << randomStr << "\n";
    }

    out.close();
    delete[] numbers;

    std::cout << "Generated " << n << " unique rows -> " << outputFile << '\n';
    return 0;
}
