#include "random_number_generator.h"
#include <random>

std::random_device random_seed;
std::default_random_engine random_engine(random_seed());

std::default_random_engine& random_number_generator::get_engine()
{
    return random_engine;
}

random_number_generator::random_number_generator() = default;
random_number_generator::~random_number_generator() = default;

int random_number_generator::generate(int min, int max)
{
    std::uniform_int_distribution<int> uniform_dist{ min, max };
    return uniform_dist(get_engine());
}
