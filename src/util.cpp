#include "sequentialCircuit.h"




uint64_t combinationsWReplacementsRec(uint64_t n, uint64_t k)
{
    if (k <= 1 or n <= 1) return n;
    return combinationsWReplacementsRec(n - 1, k) + combinationsWReplacementsRec(n, k - 1);
}

uint64_t combinationsWReplacements(uint64_t n, uint64_t k)
{
    uint64_t x = 1;
    for (uint64_t i = 1; i < n; i++)
        x += x * k / i;
    return x;
}




std::vector<std::vector<uint64_t>> uniqueCombinationsOI(uint8_t positions, uint64_t types, bool typeDuplicates)
{
    std::vector<std::vector<uint64_t>> combinations = {};

    if (!typeDuplicates and types < positions)
        return combinations;
    
    if (types <= 1)
    {
        combinations.push_back(std::vector<uint64_t>(positions, 0));
        return combinations;
    }
    
    if (positions <= 1)
    {
        for (uint64_t i = 0; i < types; i++) 
            combinations.push_back({ i });
        return combinations;
    }

    auto c2 = uniqueCombinationsOI(positions, types - 1, typeDuplicates);
    std::copy(c2.begin(), c2.end(), std::back_inserter(combinations));
    
    auto c1 = uniqueCombinationsOI(positions - 1, types - (typeDuplicates ? 0 : 1), typeDuplicates);
    for (auto& c : c1)
        c.push_back(types - 1);
    std::copy(c1.begin(), c1.end(), std::back_inserter(combinations));

    return combinations;
}
