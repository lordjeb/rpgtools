#pragma once
#include <random>

class random_number_generator
{
public:
    random_number_generator();
    virtual ~random_number_generator();
    virtual int generate(int min, int max);
protected:
    static std::default_random_engine& get_engine();
};
