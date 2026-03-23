#include "sequentialCircuit.h"
#include <fstream>
#include <iostream>
#include <bitset>




logic::TruthTable logic::TruthTable::readCSV(std::string filename)
{
    std::ifstream file(filename);

    if (!file.is_open())
        throw std::runtime_error("failed to open file: " + filename);

    logic::TruthTable table;
    std::string line;
    std::getline(file, line);
    while (std::getline(file, line))
    {
        if (line.empty()) continue;

        table.entries.push_back({});

        auto fReadField = [line](size_t& begin, size_t& end)
        {
            end = line.find(',', begin);
            size_t bbegin = line.find('b', begin);
            begin = bbegin < end ? bbegin + 1 : begin;
            auto val = std::stoull(line.substr(begin, end - begin), 0, bbegin < end ? 2 : 10);
            begin = end + 1;
            return val;
        };

        size_t begin = 0, end;
        table.entries.back().inputBits = fReadField(begin, end);
        if (end == std::string::npos) continue;
        table.entries.back().outputBits = fReadField(begin, end);
        if (end == std::string::npos) continue;
        table.entries.back().dontCareBits = fReadField(begin, end);
    }

    file.close();

    return table;
}
