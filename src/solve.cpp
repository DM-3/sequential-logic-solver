#include "sequentialCircuit.h"
#include <iostream>
#include <algorithm>
#include <numeric>
#include <math.h>
#include <bitset>

using namespace logic;




struct LayerBuilder
{
    uint8_t size;
    uint8_t inputOffset;
    uint8_t gateOffset;
    std::vector<SequentialCircuit::Layer> combinations;
};


std::optional<SequentialCircuit> SequentialCircuit::solve(
    std::vector<uint8_t> layerSizes,
    TruthTable& truthTable,
    std::vector<Gate::Mode> modes,
    bool balanced
) {
    if (layerSizes.size() < 2)
        throw std::invalid_argument("Solver expects at least input and output layer sizes.");
    if (std::find_if(layerSizes.begin(), layerSizes.end(), [](uint8_t s){ return s == 0; }) != layerSizes.end())
        throw std::invalid_argument("Solver expects layer sizes to be greater 0");    

    
    std::sort(modes.begin(), modes.end());

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


            // remove redundant single-input connections
            if (balanced and modes.size() > 1 and (uint8_t)modes[1] < 4)
                masks.erase(std::remove_if(masks.begin(), masks.end(),
                [&](std::vector<uint64_t> maskCombo)
                {
                    for (int i = 0; i < modeCombo.size(); i++)
                        if (modeCombo[i] < 4 and
                            modeCombo[i] != (uint64_t)modes.front() and
                            maskCombo[i] & (maskCombo[i] - 1) == 0)
                            return true;
                    return false;
                }), masks.end());
            else
                masks.erase(std::remove_if(masks.begin(), masks.end(), 
                [&](std::vector<uint64_t> maskCombo)
                {
                    for (int i = 0; i < modeCombo.size(); i++)
                        if ((modeCombo[i] < 4 or 
                             modeCombo[i] > 4 and
                             modeCombo[i] != (uint64_t)modes.back()) and
                            maskCombo[i] & (maskCombo[i] - 1) == 0)
                            return true;
                    return false;
                }), masks.end());


            // write to builder
            for (auto maskCombo : masks)
            {
                Layer layer;
                layer.inputOffset = builder.inputOffset;
                layer.gateOffset = builder.gateOffset;
                layer.gates.resize(builder.size);

                for (int i = 0; i < builder.size; i++)
                {
                    layer.gates[i].inputMask = (maskCombo[i] + 1) << builder.inputOffset;
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

    Layer inputLayer;
    inputLayer.gateOffset = 0;
    inputLayer.inputOffset = 0;
    inputLayer.gates.resize(layerSizes.front(), { 0, Gate::Mode::IN });
    
    Layer outputLayer;
    outputLayer.gateOffset = std::reduce(layerSizes.begin(), layerSizes.end()) - layerSizes.back();
    outputLayer.inputOffset = balanced ? outputLayer.gateOffset - layerSizes[layerSizes.size() - 2] : 0;
    outputLayer.gates.resize(layerSizes.back());
    
    

    ActivationTruthTable att;

    // search circuit combinations
    // -> check constructability of output layer against truth table

    for (uint64_t circuitCombo = 0; circuitCombo < nCircuitCombos; circuitCombo++)
    {
        SequentialCircuit circuit;
        circuit.layers.push_back(inputLayer);
        
        uint64_t layerIdx = circuitCombo;
        for (auto b = layerBuilders.rbegin(); b != layerBuilders.rend(); b++)
        {
            circuit.layers.insert(
                circuit.layers.begin() + 1, 
                b->combinations[layerIdx % b->combinations.size()]);

            layerIdx /= b->combinations.size();
        }
        
        circuit.layers.push_back(outputLayer);

        std::cout << "\rcircuit combo: " << circuitCombo << " / " << nCircuitCombos;
        

        if (circuitCombo == 0)
            att = computeActivationTruthTable(circuit, truthTable);
        else
            updateActivationTruthTable(circuit, att, 
                circuitCombo % layerBuilders.back().combinations.size() == 0 ?
                1 : layerBuilders.size());

        if (tryConstructOutputLayer(circuit, att, modes))
        {
            std::cout << std::endl;
            return circuit;
        }
    }
    
    std::cout << std::endl;
    return {};
}




uint64_t logic::SequentialCircuit::Gate::getActivation(uint64_t activation) const
{
    uint64_t maskedActivation = activation & inputMask;

    switch (uint8_t(mode) & 3)
    {
        case uint8_t(Mode::AND):   return uint64_t(maskedActivation == inputMask) ^ (uint64_t(mode) >> 2);
        case uint8_t(Mode::OR):    return uint64_t(maskedActivation != 0)         ^ (uint64_t(mode) >> 2); 
        case uint8_t(Mode::XOR):   return __builtin_parityll(maskedActivation)    ^ (uint64_t(mode) >> 2);
    }
    return 0;
}




ActivationTruthTable logic::computeActivationTruthTable(
    const SequentialCircuit& circuit,
    const TruthTable& truthTable
) {
    std::vector<std::pair<uint64_t, uint64_t>> tt(truthTable.entries.size());
    for (uint64_t i = 0; i < tt.size(); i++)
    {
        // write input bits
        tt[i].first = truthTable.entries[i].inputBits;
        
        // write gate activation bits
        for (uint8_t l = 1; l < circuit.layers.size() - 1; l++)
        {
            const SequentialCircuit::Layer& layer = circuit.layers[l];
            for (uint8_t g = 0; g < layer.gates.size(); g++)
                tt[i].first |= layer.gates[g].getActivation(tt[i].first) << (layer.gateOffset + g);
        }
        
        // write output bits
        tt[i].first |= truthTable.entries[i].outputBits << circuit.layers.back().gateOffset;
    
        // write dont care bits
        tt[i].second = truthTable.entries[i].dontCareBits << circuit.layers.back().gateOffset;
    }

    return tt;
}




void logic::updateActivationTruthTable(
    const SequentialCircuit& circuit,
    ActivationTruthTable& activationTruthTable,
    uint8_t layerIndex
) {
    for (uint8_t l = layerIndex; l < circuit.layers.size() - 1; l++)
    {
        auto& layer = circuit.layers[l];
        for (auto& [activation, _] : activationTruthTable)
        {
            for (uint8_t g = 0; g < layer.gates.size(); g++)
            {
                activation &= ~(1ul << (layer.gateOffset + g));
                activation |= layer.gates[g].getActivation(activation) << (layer.gateOffset + g);
            }
        }
    }
}




bool logic::tryConstructOutputLayer(
    SequentialCircuit& circuit, 
    const ActivationTruthTable& activationTruthTable, 
    const std::vector<SequentialCircuit::Gate::Mode> modes
) {
    uint8_t allModes = 0;
    for (auto m : modes) allModes |= 1 << (uint8_t)m;

    const uint64_t maskInc = 1ul << circuit.layers.back().inputOffset;
    const uint64_t maskTop = 1ul << circuit.layers.back().gateOffset;
    uint64_t       pos     = 1ul << circuit.layers.back().gateOffset;
    for (auto& gate : circuit.layers.back().gates)
    {
        for (gate.inputMask = maskInc; gate.inputMask < maskTop; gate.inputMask += maskInc)
        {
            uint8_t modeOptions = allModes;
            for (auto [activation, dontCare] : activationTruthTable)
            {
                if (dontCare & pos) continue;

                using enum SequentialCircuit::Gate::Mode;

                // compute all mode activations simultaneously and 
                // keep track of wether one is suitable with the 
                // given input mask
                uint64_t maskedActivation = activation & gate.inputMask;
                uint8_t modeActivations = 
                    (uint8_t(maskedActivation == gate.inputMask)  << uint8_t(AND)) |
                    (uint8_t(maskedActivation > 0)                << uint8_t(OR))  |
                    (uint8_t(__builtin_parityll(maskedActivation) << uint8_t(XOR)));
                modeActivations |= (~modeActivations) << 4;
                
                // keep 1 in mode option if mode activation matches desired activation
                modeOptions &= (activation & pos) ? modeActivations : ~modeActivations;

                if (!modeOptions) break;
            }

            // getting here either there are no more mode options for this
            // position, or the truth table was fully traversed with mode
            // options remaining (position done)
            if (modeOptions)
            {
                // get first mode that persisted in mode options
                gate.mode = (SequentialCircuit::Gate::Mode)__builtin_popcountll(~modeOptions & (modeOptions - 1));
                goto next_pos;
            }
        }

        // getting here means all options for this position failed
        return false;

    next_pos:
        pos <<= 1;
    }

    // getting here means a gate was found for all output positions
    return true;
}
