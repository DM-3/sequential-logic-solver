#include "header.h"
#include <iostream>
#include <algorithm>
#include <numeric>
#include <math.h>
#include <iomanip>
#include <fstream>

using namespace logic;
using LayerCombinations = std::vector<SequentialCircuit::Layer>;




logic::TruthTable logic::TruthTable::readCSV(std::string filename)
{
    std::ifstream file(filename);

    if (!file.is_open()) return {};

    logic::TruthTable table;
    std::string line;
    std::getline(file, line);
    while (std::getline(file, line))
    {
        if (line.empty()) continue;

        table.entries.push_back({});

        size_t begin = 0, end;
        end = line.find(',', begin);
        table.entries.back().inputBits = std::stoull(line.substr(begin, end - begin));
        if (end == std::string::npos) continue;

        begin = end + 1;
        end = line.find(',', begin);
        table.entries.back().outputBits = std::stoull(line.substr(begin, end - begin));
        if (end == std::string::npos) continue;

        begin = end + 1;
        end = std::string::npos;
        table.entries.back().dontCareBits = std::stoull(line.substr(begin, end - begin));
    }

    file.close();

    return table;
}




struct LayerBuilder
{
    uint8_t size;
    uint8_t inputOffset;
    uint8_t gateOffset;
    LayerCombinations combinations;
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

    Layer inputLayer;
    inputLayer.gateOffset = 0;
    inputLayer.inputOffset = 0;
    inputLayer.gates.resize(layerSizes.front(), { 0, Gate::Mode::IN });
    
    Layer outputLayer;
    outputLayer.gateOffset = std::reduce(layerSizes.begin(), layerSizes.end()) - layerSizes.back();
    outputLayer.inputOffset = balanced ? outputLayer.gateOffset - layerSizes[layerSizes.size() - 2] : 0;
    outputLayer.gates.resize(layerSizes.back());
    
    
    
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
        
        if (tryConstructOutputLayer(circuit, truthTable, modes))
        {
            std::cout << std::endl;
            return circuit;
        }
    }
    
    std::cout << std::endl;
    return {};
}




uint64_t logic::SequentialCircuit::Gate::getActivation(uint64_t activation)
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




bool logic::tryConstructOutputLayer(
    SequentialCircuit& circuit, 
    TruthTable& truthTable, 
    std::vector<SequentialCircuit::Gate::Mode> modes
) {
    // compute full circuit activations for all truth table inputs 
    // pair: (input | activations | output << n, dontCareBits)
    std::vector<std::pair<uint64_t, uint64_t>> tt(truthTable.entries.size());
    for (uint64_t i = 0; i < tt.size(); i++)
    {
        // write input bits
        tt[i].first = truthTable.entries[i].inputBits;
        
        // write gate activation bits
        for (uint8_t l = 1; l < circuit.layers.size() - 1; l++)
        {
            SequentialCircuit::Layer& layer = circuit.layers[l];
            for (uint8_t g = 0; g < layer.gates.size(); g++)
                tt[i].first |= layer.gates[g].getActivation(tt[i].first) << (layer.gateOffset + g);
        }
        
        // write output bits
        tt[i].first |= truthTable.entries[i].outputBits << circuit.layers.back().gateOffset;
    
        // write dont care bits
        tt[i].second = truthTable.entries[i].dontCareBits << circuit.layers.back().gateOffset;
    }


    // try construction
    
    uint64_t maskInc = 1ul << circuit.layers.back().inputOffset;
    uint64_t maskTop = 1ul << circuit.layers.back().gateOffset;
    uint64_t pos     = 1ul << circuit.layers.back().gateOffset;
    for (auto& gate : circuit.layers.back().gates)
    {
        for (gate.inputMask = maskInc; gate.inputMask < maskTop; gate.inputMask += maskInc)
        {
            for (auto mode : modes)
            {
                gate.mode = mode;

                for (auto [activation, dontCare] : tt)
                {
                    if (dontCare & pos != 0) 
                        continue;

                    if (
                        gate.getActivation(activation) * pos  // gate activation shifted to gate position
                        != 
                        (activation & pos)                    // truth table at gate position
                    )
                        goto next_mode;                     // truth table fail -> try next gate mode
                }

                // getting here means the truth table could be matched
                goto next_pos;
                
            next_mode: ;
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




namespace std
{
    std::string to_string(const logic::SequentialCircuit::Gate::Mode& mode)
    {
        using enum logic::SequentialCircuit::Gate::Mode;

        switch (mode)
        {
            case IN:    return "IN  ";
            case AND:   return "AND ";
            case OR:    return "OR  ";
            case XOR:   return "XOR ";
            case NAND:  return "NAND";
            case NOR:   return "NOR ";
            case XNOR:  return "XNOR";
        }
        return {};
    }

    std::string to_string(const logic::SequentialCircuit::Gate& gate)
    {
        std::stringstream ss;
        ss << setw(5) << gate.inputMask << "_" << gate.mode;
        return ss.str();
    }

    std::string to_string(const logic::SequentialCircuit::Layer& layer)
    {
        std::stringstream ss;
        ss << "[ ";
        for (auto& gate : layer.gates)
            ss << gate << "\t";
        ss << "]";
        return ss.str();
    }

    std::string to_string(const logic::SequentialCircuit& circuit)
    {
        std::stringstream ss;
        ss << "circuit:\n";
        for (int i = 0; i < circuit.layers.size(); i++)
            ss << " layer " << i << ": \t" << circuit.layers[i] << "\n";
        ss << "\n";
        return ss.str();
    }

    std::ostream& operator<<(std::ostream& out, const logic::SequentialCircuit::Gate::Mode& mode)
    { return out << to_string(mode); }

    std::ostream& operator<<(std::ostream& out, const logic::SequentialCircuit::Gate& gate)
    { return out << to_string(gate); }

    std::ostream& operator<<(std::ostream& out, const logic::SequentialCircuit::Layer& layer)
    { return out << to_string(layer); }

    std::ostream& operator<<(std::ostream& out, const logic::SequentialCircuit& circuit)
    { return out << to_string(circuit); }
};
