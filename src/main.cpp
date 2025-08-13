#include "header.h"
#include <iostream>
#include <math.h>

using enum logic::SequentialCircuit::Gate::Mode;




int main()
{
    std::vector<logic::SequentialCircuit::Gate::Mode> modes = { AND, OR, XOR, NAND, NOR, XNOR };

    
    // truth table for double dabble, with 3rd input bit inverted

    const uint64_t inv = 0b0100;

    logic::TruthTable table;
    table.entries = 
    {
        { 0 ^ inv,    0,     0 },
        { 1 ^ inv,    1,     0 },
        { 2 ^ inv,    2,     0 },
        { 3 ^ inv,    3,     0 },
        { 4 ^ inv,    4,     0 },
        { 5 ^ inv,    8,     0 },
        { 6 ^ inv,    9,     0 },
        { 7 ^ inv,   10,     0 },
        { 8 ^ inv,   11,     0 },
        { 9 ^ inv,   12,     0 }
    };


    auto circuit = logic::SequentialCircuit::solve({ 4, 3, 4 }, table, modes, false);
    if (circuit)
        std::cout << circuit.value();
    else
        std::cout << "no circuit solution found";

    return 0;
}
