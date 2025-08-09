#include "header.h"
#include <iostream>
#include <math.h>

using enum logic::SequentialCircuit::Gate::Mode;


uint64_t triangleNumber(uint64_t dims, uint64_t length)
{
    if (dims <= 1 or length <= 1) return length;
    return triangleNumber(dims, length - 1) + triangleNumber(dims - 1, length);
}

uint64_t combinationsWReplacements(uint64_t n, uint64_t k)
{
    uint64_t x = 1;
    for (uint64_t i = 1; i < n; i++)
        x += x * k / i;

    return x;
}


void printModeCombos(
    std::vector<std::vector<uint64_t>> combos, 
    std::vector<logic::SequentialCircuit::Gate::Mode> modes
) {
    int i = 0;
    for (auto c : combos)
    {
        std::cout << ++i << ": \t";
        for (auto g : c)
            switch (modes[g])
            {
                case AND:   std::cout << "AND  \t"; break;
                case OR:    std::cout << "OR   \t"; break;
                case XOR:   std::cout << "XOR  \t"; break;
                case NAND:  std::cout << "NAND \t"; break;
                case NOR:   std::cout << "NOR  \t"; break;
                case XNOR:  std::cout << "XNOR \t"; break;
            };
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void printConnectionCombos(std::vector<std::vector<uint64_t>> combos)
{
    int i = 0;
    for (auto c : combos)
    {
        std::cout << ++i << ": \t";
        for (auto m : c)
            std::cout << m + 1 << " \t";
        std::cout << std::endl;
    }
    std::cout << std::endl;
}


int main()
{
    auto f1 = [](int m, int n) 
    {
        auto t = triangleNumber(m, n);
        auto p = powl(n, m);
        auto c = combinationsWReplacements(n, m);
        std::cout << m << ", " << n << ": " 
                  << t << " / " << p << " = " 
                  << round(1000.0 * t / p) / 10.0 << "% " 
                  << c << std::endl;
    };

    auto f2 = [](int m, int n)
    {
        auto c = logic::uniqueCombinationsOI(m, n, false).size();
        auto p = powl(n, m);
        std::cout << m << ", " << n << ": "
                  << c << " / " << p << " = "
                  << round(1000.0 * c / p) / 10.0 << "% "
                  << std::endl;
    };

    f1(1, 6);
    f1(2, 6);
    f1(3, 6);
    f1(4, 6);
    f1(5, 6);
    f1(6, 6);
    std::cout << std::endl;

    f1(1, 5);
    f1(2, 5);
    f1(3, 5);
    f1(4, 5);
    f1(5, 5);
    std::cout << std::endl;


    f1(3, 6);
    std::vector<logic::SequentialCircuit::Gate::Mode> modes = { AND, OR, XOR, NAND, NOR, XNOR };
    printModeCombos(logic::uniqueCombinationsOI(3, 6), modes);

    f2(3, 3);
    f2(2, 7);
    f2(3, 7);
    f2(4, 7);
    f2(2, 15);
    f2(3, 15);
    f2(4, 15);
    std::cout << std::endl;

    logic::TruthTable table;
    logic::SequentialCircuit::solve({ 4, 4, 4 }, table, modes);

    return 0;
}
