#include <gtest\gtest.h>
#include <gmock\gmock.h>
#include "expression_evaluator_test.h"

using ::testing::_;
using ::testing::Eq;
using ::testing::Return;
using ::testing::StrEq;

struct evaluate_test : public expression_evaluator_test
{
    std::string description;
};

TEST_F(evaluate_test, simple_number)
{
    EXPECT_CALL(rng, generate(_, _)).Times(0);
    auto result = eval.evaluate("4", &description);
    EXPECT_THAT(result, Eq(4));
    EXPECT_THAT(description, StrEq(""));
}

TEST_F(evaluate_test, simple_number_redux)
{
    EXPECT_CALL(rng, generate(_, _)).Times(0);
    auto result = eval.evaluate("6", &description);
    EXPECT_THAT(result, Eq(6));
    EXPECT_THAT(description, StrEq(""));
}

TEST_F(evaluate_test, simple_addition)
{
    EXPECT_CALL(rng, generate(_, _)).Times(0);
    auto result = eval.evaluate("1+1", &description);
    EXPECT_THAT(result, Eq(2));
    EXPECT_THAT(description, StrEq(""));
}

TEST_F(evaluate_test, simple_subtraction)
{
    EXPECT_CALL(rng, generate(_, _)).Times(0);
    auto result = eval.evaluate("3-1", &description);
    EXPECT_THAT(result, Eq(2));
    EXPECT_THAT(description, StrEq(""));
}

TEST_F(evaluate_test, simple_multiplication)
{
    EXPECT_CALL(rng, generate(_, _)).Times(0);
    auto result = eval.evaluate("3*2", &description);
    EXPECT_THAT(result, Eq(6));
    EXPECT_THAT(description, StrEq(""));
}

TEST_F(evaluate_test, order_of_operations_1)
{
    EXPECT_CALL(rng, generate(_, _)).Times(0);
    auto result = eval.evaluate("4*8+3", &description);
    EXPECT_THAT(result, Eq(35));
    EXPECT_THAT(description, StrEq(""));
}

TEST_F(evaluate_test, order_of_operations_2)
{
    EXPECT_CALL(rng, generate(_, _)).Times(0);
    auto result = eval.evaluate("3+4*8", &description); // Bad L2R math will do 7*8 == 56
    EXPECT_THAT(result, Eq(35));
    EXPECT_THAT(description, StrEq(""));
}

TEST_F(evaluate_test, simple_d6_roll)
{
    EXPECT_CALL(rng, generate(1, 6)).Times(1).WillOnce(Return(4));
    auto result = eval.evaluate("d6", &description);
    EXPECT_THAT(result, Eq(4));
    EXPECT_THAT(description, StrEq("(4)"));
}

TEST_F(evaluate_test, simple_d6_roll_redux)
{
    EXPECT_CALL(rng, generate(1, 6)).Times(1).WillOnce(Return(4));
    auto result = eval.evaluate("1d6", &description);
    EXPECT_THAT(result, Eq(4));
    EXPECT_THAT(description, StrEq("(4)"));
}

TEST_F(evaluate_test, multiple_d6_roll)
{
    EXPECT_CALL(rng, generate(1, 6)).Times(2).WillOnce(Return(4)).WillOnce(Return(3));
    auto result = eval.evaluate("2d6", &description);
    EXPECT_THAT(result, Eq(7));
    EXPECT_THAT(description, StrEq("(4, 3)"));
}

TEST_F(evaluate_test, roll_with_advantage)
{
    EXPECT_CALL(rng, generate(1, 20)).Times(2).WillOnce(Return(3)).WillOnce(Return(17));
    auto result = eval.evaluate("2d20b1", &description);
    EXPECT_THAT(result, Eq(17));
    EXPECT_THAT(description,
                StrEq("(17, 3)"));   // Note that this will always print the dropped dice after the good ones
}

TEST_F(evaluate_test, roll_with_disadvantage)
{
    EXPECT_CALL(rng, generate(1, 20)).Times(2).WillOnce(Return(3)).WillOnce(Return(17));
    auto result = eval.evaluate("2d20w1", &description);
    EXPECT_THAT(result, Eq(3));
    EXPECT_THAT(description,
                StrEq("(3, 17)"));   // Note that this will always print the dropped dice after the good ones
}

TEST_F(evaluate_test, roll_4d6_keep_best_3)
{
    EXPECT_CALL(rng, generate(1, 6))
        .Times(4)
        .WillOnce(Return(3))
        .WillOnce(Return(3))
        .WillOnce(Return(5))
        .WillOnce(Return(6));
    auto result = eval.evaluate("4d6b3", &description);
    EXPECT_THAT(result, Eq(14));
    EXPECT_THAT(description,
                StrEq("(3, 5, 6, 3)"));   // Note that this will always print the dropped dice after the good ones
}

TEST_F(evaluate_test, year_zero_table_d66)
{
    EXPECT_CALL(rng, generate(1, 6)).Times(2).WillOnce(Return(4)).WillOnce(Return(2));
    auto result = eval.evaluate("(1d6*10)+1d6", &description);
    EXPECT_THAT(result, Eq(42));
    EXPECT_THAT(description, StrEq("(4) (2)"));
}

TEST_F(evaluate_test, year_zero_table_d666)
{
    EXPECT_CALL(rng, generate(1, 6)).Times(3).WillOnce(Return(4)).WillOnce(Return(2)).WillOnce(Return(6));
    auto result = eval.evaluate("(1d6*100)+(1d6*10)+1d6", &description);
    EXPECT_THAT(result, Eq(426));
    EXPECT_THAT(description, StrEq("(4) (2) (6)"));
}
