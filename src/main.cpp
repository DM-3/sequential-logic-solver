#include "sequentialCircuit.h"
#include <iostream>
#include <bitset>

using Mode = logic::SequentialCircuit::Gate::Mode;
using enum Mode;




int main()
{
    //std::vector<Mode> modes = { AND, OR, XOR, NAND, NOR, XNOR };
    //auto table = logic::TruthTable::readCSV("ttables/greater_4_add_3.csv");
    //auto circuit = logic::SequentialCircuit::solve({ 4, 4, 4 }, table, modes, false);
    
    std::vector<Mode> modes = { AND, XOR };
    auto table = logic::TruthTable::readCSV("ttables/4bit_popcount.csv");
    auto circuit = logic::SequentialCircuit::solve({ 4, 3, 1, 3 }, table, modes, false);
    
    if (circuit)
        std::cout << circuit.value();
    else
        std::cout << "no circuit solution found";

    return 0;
}
