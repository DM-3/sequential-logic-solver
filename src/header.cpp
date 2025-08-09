#include "header.h"
#include <iostream>
#include <algorithm>
#include <math.h>

using namespace logic;
using LayerCombinations = std::vector<SequentialCircuit::Layer>;


struct LayerBuilder
{
    uint8_t size;
    uint8_t inputOffset;
    uint8_t gateOffset;
    LayerCombinations combinations;
};


SequentialCircuit SequentialCircuit::solve(
    std::vector<uint8_t> layerSizes,
    TruthTable& truthTable,
    std::vector<Gate::Mode> modes,
    bool balanced
) {
    if (layerSizes.size() < 2)
        throw std::invalid_argument("Solver expects at least input and output layer sizes.");
    if (std::find_if(layerSizes.begin(), layerSizes.end(), [](uint8_t s){ return s == 0; }) != layerSizes.end())
        throw std::invalid_argument("Solver expects layer sizes to be greater 0");    


    // prepare layer builders
    
    std::vector<LayerBuilder> layerBuilders(layerSizes.size() - 2);
    for (uint8_t i = 1, g = 0; i < layerSizes.size() - 1; i++)
    {
        LayerBuilder& builder = layerBuilders[i - 1];
        builder.size = layerSizes[i];
        builder.inputOffset = balanced ? g : 0;
        g += layerSizes[i - 1];
        builder.gateOffset = g;
    }

    // prepare layer combinations
    
    for (auto& builder : layerBuilders)
    {
        // create layer gate mode combinations

        auto modeCombos = uniqueCombinationsOI(builder.size, modes.size());


        // create layer gate connection combinations

        uint64_t gateCons = (1ul << (builder.gateOffset - builder.inputOffset)) - 1;
        uint64_t totalCons = powl(gateCons, builder.size);
        builder.combinations.reserve(totalCons * modeCombos.size());

        for (auto modeCombo : modeCombos)
        {
            std::vector<std::vector<uint64_t>> masks = {{}};

            // optimize connections for gates of the same mode
            uint8_t positions = 1;
            for (int i = 0; i < modeCombo.size(); i++)
            {
                if (i == modeCombo.size() - 1 or modeCombo[i] != modeCombo[i+1])
                {
                    auto gateMasks = uniqueCombinationsOI(positions, gateCons, false);
                    masks = cartesianProduct(masks, gateMasks);
                    positions = 0;
                }
                positions++;
            }

            for (auto maskSet : masks)
            {
                Layer layer;
                layer.inputOffset = builder.inputOffset;
                layer.gateOffset = builder.gateOffset;
                layer.gates.resize(builder.size);

                for (int i = 0; i < builder.size; i++)
                {
                    layer.gates[i].inputMask = (maskSet[i] + 1) << builder.inputOffset;
                    layer.gates[i].mode = modes[modeCombo[i]];
                }

                builder.combinations.push_back(layer);
            }
        }

        builder.combinations.shrink_to_fit();




        std::cout << "layer combinations: " << builder.combinations.size()
                  << " (" << round(1000.0 * builder.combinations.size() / powl(modes.size() * gateCons, builder.size)) / 10.0 << "%) " 
                  << std::endl;
    }


    uint64_t nCircuitCombos = 1;
    for (auto& builder : layerBuilders)
        nCircuitCombos *= builder.combinations.size();
    std::cout << "circuit combinations: " << nCircuitCombos << std::endl << std::endl;


    // create circuit combinations

    // optimize circuit combinations

    // search circuit combinations
    // -> check constructability of output layer against truth table

    return {};
}








std::vector<std::vector<uint64_t>> logic::uniqueCombinationsOI(uint8_t positions, uint64_t types, bool typeDuplicates)
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



