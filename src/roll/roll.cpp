#include <algorithm>
#include <iostream>
#include <unordered_map>
#include <numeric>
#include <random>
#include <regex>
#include <stack>
#include <sstream>
#include <vector>
#include "rpgtools/random_number_generator.h"
#include "rpgtools/expression_evaluator.h"

auto main(int argc, char* argv[]) -> int
{
    if (argc < 2)
    {
        std::cout << "Usage:\n"
                  << "   [expression] (... [expression])\n"
                  << "\n"
                  << "   Simple dice rolls: 1d4 1d4+3\n"
                  << "   Keep best/worst: 4d6b3 2d20b1+3\n"
                  << "\n";
        return 0;
    }

    try
    {
        for (int x = 1; x < argc; x++)
        {
            std::string roll_description{};
            random_number_generator rng{};
            expression_evaluator parser{ &rng };

            auto result = parser.evaluate(argv[x], &roll_description);

            std::cout << argv[x] << ": " << roll_description << " = " << result << "\n";
        }
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << "\n";
    }
}
