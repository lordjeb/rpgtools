#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "expression_evaluator_test.h"

using ::testing::ContainerEq;

struct expression_parsing_test_params
{
    std::string expression;
    std::vector<std::string> infix_notation;
    std::vector<std::string> prefix_notation;
};

void PrintTo(const expression_parsing_test_params& param, ::std::ostream* os)
{
    *os << param.expression;
}

struct expression_parsing_test
    : public expression_evaluator_test,
                                 public ::testing::WithParamInterface<expression_parsing_test_params>
{
};

TEST_P(expression_parsing_test, parse)
{
    const auto& param = GetParam();
    auto result = eval.parse(param.expression);
    EXPECT_THAT(result, ContainerEq(param.infix_notation));
}

TEST_P(expression_parsing_test, convert_infix_to_prefix)
{
    const auto& param = GetParam();
    auto result = eval.convert_infix_to_prefix(param.infix_notation);
    EXPECT_THAT(result, ContainerEq(param.prefix_notation));
}

INSTANTIATE_TEST_SUITE_P(
    parse_test, expression_parsing_test,
    ::testing::ValuesIn(std::vector<expression_parsing_test_params>{
        // clang-format off
        { "", std::vector<std::string>{}, std::vector<std::string>{} },
        { "1+1", std::vector<std::string>{ "1", "+", "1" }, std::vector<std::string>{ "1", "1", "+" } },
        { "1-1", std::vector<std::string>{ "1", "-", "1" }, std::vector<std::string>{ "1", "1", "-" } },
        { "1*1", std::vector<std::string>{ "1", "*", "1" }, std::vector<std::string>{ "1", "1", "*" } },
        { "(1*1)", std::vector<std::string>{ "(", "1", "*", "1", ")" }, std::vector<std::string>{ "1", "1", "*" } },
        { "(1*1)+1", std::vector<std::string>{ "(", "1", "*", "1", ")", "+", "1" }, std::vector<std::string>{ "1", "1", "*", "1", "+" } },
        { "(1*1)+(1*1)", std::vector<std::string>{ "(", "1", "*", "1", ")", "+", "(", "1", "*", "1", ")" }, std::vector<std::string>{ "1", "1", "*", "1", "1", "*", "+" } },
        { "4*8+3", std::vector<std::string>{ "4", "*", "8", "+", "3" }, std::vector<std::string>{ "4", "8", "*", "3", "+" } },
        { "3+4*8", std::vector<std::string>{ "3", "+", "4", "*", "8" }, std::vector<std::string>{ "3", "4", "8", "*", "+" } },
        { "1d20b1+7", std::vector<std::string>{ "1d20b1", "+", "7" }, std::vector<std::string>{ "1d20b1", "7", "+" } },
        { "(1d20b1+7)-3", std::vector<std::string>{ "(", "1d20b1", "+", "7", ")", "-", "3" }, std::vector<std::string>{ "1d20b1", "7", "+", "3", "-" } },
        { "(1*1d10)+(1*1d10)+1", std::vector<std::string>{ "(", "1", "*", "1d10", ")", "+", "(", "1", "*", "1d10", ")", "+", "1" }, std::vector<std::string>{ "1", "1d10", "*", "1", "1d10", "*", "+", "1", "+" } },
        // clang-format on
    }
));