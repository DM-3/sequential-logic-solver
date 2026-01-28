#include "sequentialCircuit.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <bitset>




namespace logic
{
    std::string SequentialCircuit::toLDG()
    {
        std::vector<std::pair<uint8_t, uint8_t>> edges;
        logic::SequentialCircuit::Gate::Mode modes[64];

        // iterate over gates in circuit
        for (const auto& layer : layers)
            for (int g = 0; g < layer.gates.size(); g++)
            {
                const auto& gate = layer.gates[g];
                uint8_t index = layer.gateOffset + g;
                modes[index] = gate.mode;

                // iterate over gate's input connections
                uint64_t inputMask = gate.inputMask;
                while (inputMask)
                {
                    uint64_t bit = (~inputMask + 1) & inputMask;
                    inputMask &= ~bit;
                    uint8_t pos = std::bitset<64>(bit - 1).count();
                    edges.push_back({ pos, index });
                }
            }

        // write edges to string
        std::stringstream ss;
        for (auto [from, to] : edges)
            ss << std::to_string(modes[from], false) << int(from) 
               << " -> " 
               << std::to_string(modes[to], false) << int(to) 
               << "\n";
        
        return ss.str();
    }
}



namespace std
{
    std::string to_string(const logic::SequentialCircuit::Gate::Mode& mode, bool pad)
    {
        using enum logic::SequentialCircuit::Gate::Mode;

        if (pad)
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
        else
            switch (mode)
            {
                case IN:    return "IN";
                case AND:   return "AND";
                case OR:    return "OR";
                case XOR:   return "XOR";
                case NAND:  return "NAND";
                case NOR:   return "NOR";
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
