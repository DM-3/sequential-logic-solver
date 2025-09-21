#include "sequentialCircuit.h"
#include <iostream>
#include <iomanip>
#include <sstream>




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
