#pragma once
#include <vector>
#include <inttypes.h>
#include <optional>
#include <string>




namespace logic
{

    struct TruthTable
    {
        struct Entry
        {
            uint64_t inputBits;
            uint64_t outputBits;
            uint64_t dontCareBits;
        };

        std::vector<Entry> entries;

        static TruthTable readCSV(std::string filename);
    };


    struct SequentialCircuit
    {
        struct Gate
        {
            enum class Mode : uint8_t
            {
                IN   = 0b000,

                AND  = 0b001,
                OR   = 0b010,
                XOR  = 0b011,
                
                NAND = 0b101,
                NOR  = 0b110,
                XNOR = 0b111
            };

            uint64_t inputMask;
            Mode mode;

            // gate activation 0 or 1
            uint64_t getActivation(uint64_t activation);
        };

        struct Layer
        {
            std::vector<Gate> gates;
            uint8_t inputOffset;
            uint8_t gateOffset;
        };

        static std::optional<SequentialCircuit> solve(
            std::vector<uint8_t> layerSizes,
            TruthTable& truthTable,
            std::vector<Gate::Mode> modes,
            bool balanced = true);

        std::vector<Layer> layers;
    };

    
    // return true if an output layer can be constructed,
    // which satisfies the truth table
    bool tryConstructOutputLayer(
        SequentialCircuit& circuit, 
        TruthTable& truthTable, 
        std::vector<SequentialCircuit::Gate::Mode> modes);
};




uint64_t combinationsWReplacementsRec(uint64_t n, uint64_t k);
uint64_t combinationsWReplacements(uint64_t n, uint64_t k);


// unique order-independent combinations for
// placing items of m types at k positions
// optionally allow for type duplicates 
std::vector<std::vector<uint64_t>> uniqueCombinationsOI(uint8_t positions, uint64_t types, bool typeDuplicates = true);


template<typename T>
std::vector<std::vector<T>> cartesianProduct(
    std::vector<std::vector<T>>& v1,
    std::vector<std::vector<T>>& v2
) {
    std::vector<std::vector<T>> v;
    v.reserve(v1.size() * v2.size());

    for (auto& e1 : v1)
    {
        for (auto& e2 : v2)
        {
            v.push_back({});
            v.back().reserve(e1.size() + e2.size());

            std::copy(e1.begin(), e1.end(), std::back_inserter(v.back()));
            std::copy(e2.begin(), e2.end(), std::back_inserter(v.back()));
        }
    }

    return v;
}



namespace std
{
    std::string to_string(const logic::SequentialCircuit::Gate::Mode& mode);
    std::string to_string(const logic::SequentialCircuit::Gate& gate);
    std::string to_string(const logic::SequentialCircuit::Layer& layer);
    std::string to_string(const logic::SequentialCircuit& circuit);
    
    std::ostream& operator<<(std::ostream &out, const logic::SequentialCircuit::Gate::Mode& mode);
    std::ostream& operator<<(std::ostream &out, const logic::SequentialCircuit::Gate& gate);
    std::ostream& operator<<(std::ostream &out, const logic::SequentialCircuit::Layer& layer);
    std::ostream& operator<<(std::ostream &out, const logic::SequentialCircuit& circuit);
};
