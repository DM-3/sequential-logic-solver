#pragma once
#include <vector>
#include <inttypes.h>



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
    };


    struct SequentialCircuit
    {
        struct Gate
        {
            enum class Mode : uint8_t
            {
                IN   = 0,
                AND  = 1,
                OR   = 2,
                XOR  = 3,
                NAND = 4,
                NOR  = 5,
                XNOR = 6
            };

            uint64_t inputMask;
            Mode mode;
        };

        struct Layer
        {
            std::vector<Gate> gates;
            uint8_t inputOffset;
            uint8_t gateOffset;
        };

        static SequentialCircuit solve(
            std::vector<uint8_t> layerSizes,
            TruthTable& truthTable,
            std::vector<Gate::Mode> modes,
            bool balanced = true);

        std::vector<Layer> layers;
    };



    // unique order-independent combinations for
    // placing items of m types at k positions
    // optionally allow for type duplicates 
    std::vector<std::vector<uint64_t>> uniqueCombinationsOI(uint8_t positions, uint64_t types, bool typeDuplicates = true);
};




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