#pragma once
#include <gtest\gtest.h>
#include <gmock\gmock.h>
#include "rpgtools\random_number_generator.h"
#include "rpgtools\expression_evaluator.h"

class mock_random_number_generator : public random_number_generator
{
public:
    MOCK_METHOD(int, generate, (int min, int max), (override));
};

struct expression_evaluator_test : public ::testing::Test
{
    mock_random_number_generator rng;
    expression_evaluator eval{ &rng };
};
