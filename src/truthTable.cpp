#include "sequentialCircuit.h"
#include <fstream>




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
