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
    auto result = eval.evaluate("3+4*8", &description);   // Bad L2R math will do 7*8 == 56
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

TEST_F(evaluate_test, year_zero_table_d66_manual)
{
    EXPECT_CALL(rng, generate(1, 6)).Times(2).WillOnce(Return(4)).WillOnce(Return(2));
    auto result = eval.evaluate("(1d6*10)+1d6", &description);
    EXPECT_THAT(result, Eq(42));
    EXPECT_THAT(description, StrEq("(4) (2)"));
}

TEST_F(evaluate_test, year_zero_table_d666_manual)
{
    EXPECT_CALL(rng, generate(1, 6)).Times(3).WillOnce(Return(4)).WillOnce(Return(2)).WillOnce(Return(6));
    auto result = eval.evaluate("(1d6*100)+(1d6*10)+1d6", &description);
    EXPECT_THAT(result, Eq(426));
    EXPECT_THAT(description, StrEq("(4) (2) (6)"));
}

TEST_F(evaluate_test, year_zero_table_d66)
{
    EXPECT_CALL(rng, generate(1, 6)).Times(2).WillOnce(Return(4)).WillOnce(Return(2));
    auto result = eval.evaluate("d66", &description);
    EXPECT_THAT(result, Eq(42));
    EXPECT_THAT(description, StrEq("(42)"));
}

TEST_F(evaluate_test, year_zero_table_d666)
{
    EXPECT_CALL(rng, generate(1, 6)).Times(3).WillOnce(Return(4)).WillOnce(Return(2)).WillOnce(Return(6));
    auto result = eval.evaluate("d666", &description);
    EXPECT_THAT(result, Eq(426));
    EXPECT_THAT(description, StrEq("(426)"));
}

// Exploding dice tests
TEST_F(evaluate_test, simple_exploding_d6_no_explosion)
{
    EXPECT_CALL(rng, generate(1, 6)).Times(1).WillOnce(Return(4));
    auto result = eval.evaluate("d6!", &description);
    EXPECT_THAT(result, Eq(4));
    EXPECT_THAT(description, StrEq("(4)"));
}

TEST_F(evaluate_test, simple_exploding_d6_single_explosion)
{
    EXPECT_CALL(rng, generate(1, 6)).Times(2).WillOnce(Return(6)).WillOnce(Return(3));
    auto result = eval.evaluate("d6!", &description);
    EXPECT_THAT(result, Eq(9));
    EXPECT_THAT(description, StrEq("([6+3])"));
}

TEST_F(evaluate_test, simple_exploding_d6_multiple_explosions)
{
    EXPECT_CALL(rng, generate(1, 6))
        .Times(4)
        .WillOnce(Return(6))
        .WillOnce(Return(6))
        .WillOnce(Return(6))
        .WillOnce(Return(2));
    auto result = eval.evaluate("d6!", &description);
    EXPECT_THAT(result, Eq(20));
    EXPECT_THAT(description, StrEq("([6+6+6+2])"));
}

TEST_F(evaluate_test, multiple_exploding_d6_mixed_results)
{
    EXPECT_CALL(rng, generate(1, 6)).Times(3).WillOnce(Return(3)).WillOnce(Return(6)).WillOnce(Return(4));
    auto result = eval.evaluate("2d6!", &description);
    EXPECT_THAT(result, Eq(13));
    EXPECT_THAT(description, StrEq("(3, [6+4])"));
}

TEST_F(evaluate_test, exploding_d20_single_explosion)
{
    EXPECT_CALL(rng, generate(1, 20)).Times(2).WillOnce(Return(20)).WillOnce(Return(15));
    auto result = eval.evaluate("d20!", &description);
    EXPECT_THAT(result, Eq(35));
    EXPECT_THAT(description, StrEq("([20+15])"));
}

TEST_F(evaluate_test, exploding_dice_with_advantage)
{
    EXPECT_CALL(rng, generate(1, 20)).Times(3).WillOnce(Return(20)).WillOnce(Return(5)).WillOnce(Return(18));
    auto result = eval.evaluate("2d20!b1", &description);
    EXPECT_THAT(result, Eq(25));   // Best of [20+5]=25 and 18
    EXPECT_THAT(description, StrEq("([20+5], 18)"));
}

TEST_F(evaluate_test, non_exploding_vs_exploding_comparison)
{
    {
        EXPECT_CALL(rng, generate(1, 6)).Times(1).WillOnce(Return(4));
        auto result1 = eval.evaluate("d6", &description);
        EXPECT_THAT(result1, Eq(4));
        EXPECT_THAT(description, StrEq("(4)"));
    }

    {
        EXPECT_CALL(rng, generate(1, 6)).Times(2).WillOnce(Return(6)).WillOnce(Return(3));
        auto result2 = eval.evaluate("d6!", &description);
        EXPECT_THAT(result2, Eq(9));
        EXPECT_THAT(description, StrEq("([6+3])"));
    }
}

TEST_F(evaluate_test, exploding_dice_dropped_show_explosions)
{
    EXPECT_CALL(rng, generate(1, 6))
        .Times(4)
        .WillOnce(Return(6))
        .WillOnce(Return(3))
        .WillOnce(Return(2))
        .WillOnce(Return(4));
    auto result = eval.evaluate("3d6!b1", &description);
    EXPECT_THAT(result, Eq(9));   // Best of [6+3]=9, 2, and 4
    EXPECT_THAT(description, StrEq("([6+3], 2, 4)"));
}

// TODO: Evaluate with a target number with the total
// TODO: Division with a round down ala raises in Savage Worlds
// TODO: Count results higher than a certain value, ala 6 is success in year zero
// TODO: Support different kinds of dice, like 2d6[attribute]3d6[skill]4d6[stress]
// TODO: Support no sums
// TODO: Make aliases for common rolls (similar to what d66/d666 are)
// TODO: Support loading an alias list from a json file so you could do roll.exe fighting or roll.exe stress
