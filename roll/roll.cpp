#include <algorithm>
#include <iostream>
#include <numeric>
#include <random>
#include <regex>
#include <stack>
#include <vector>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

std::random_device random_seed;
std::default_random_engine random_engine(random_seed());

class random_number_generator
{
public:
    random_number_generator() = default;
    ~random_number_generator() = default;
    virtual auto generate(int min, int max) -> int
    {
        std::uniform_int_distribution<int> uniform_dist{ min, max };
        return uniform_dist(random_engine);
    }
};

class expression_evaluator
{
    random_number_generator* rng_;

    enum class operator_assocation { none, left_to_right, right_to_left };

    struct operator_info
    {
        int precedence;
        operator_assocation associativity;
    };

    std::unordered_map<std::string, operator_info> operators = {
        // clang-format off
        { "+", { 2, operator_assocation::left_to_right } },
        { "-", { 2, operator_assocation::left_to_right } },
        // clang-format on
    };

    int precedence(const std::string& op)
    {
        return operators[op].precedence;
    }
    operator_assocation associativity(const std::string& op)
    {
        return operators[op].associativity;
    }

public:
    enum class token_type { number, operation, left_parenthesis, right_parenthesis, dice_expression };

    expression_evaluator(random_number_generator* rng) : rng_{ rng }
    {
    }

    auto evaluate(const std::string expression, std::string* description = nullptr) -> int
    {
        std::stack<int> stack;
        std::vector<std::string> rolls;

        auto tokens = parse(expression);

        auto prefix = convert_infix_to_prefix(tokens);

        // Evaluate expression in prefix order
        for (const auto& token : prefix)
        {
            switch (get_token_type(token))
            {
            case token_type::number:
                stack.push(std::stoi(token));
                break;

            case token_type::dice_expression:
                stack.push(evaluate_dice_expression(token, rolls));
                break;

            case token_type::operation:
                evaluate_operation(stack, token);
                break;

            default:
                throw std::runtime_error("Unexpected token: " + token);
            }
        }

        if (stack.size() > 1)
        {
            throw std::runtime_error("Parse error");
        }

        if (description)
        {
            std::stringstream result;
            std::copy(rolls.begin(), rolls.end(), std::ostream_iterator<std::string>(result, " "));

            *description = result.str();

            if (!rolls.empty())
            {
                (*description).pop_back();
            }
        }

        return stack.top();
    }

    auto evaluate_dice_expression(const std::string& token, std::vector<std::string>& rolls) -> int
    {
        const std::regex expr{ "(\\d*)[dD](\\d+)" };
        std::smatch match;
        if (!std::regex_match(token, match, expr))
        {
            throw std::runtime_error("Improper dice expression: " + token);
        }

        auto num_rolls = match[1].str().empty() ? 1 : std::stoi(match[1].str());
        auto dice_size = std::stoi(match[2].str());

        std::vector<int> dice_rolls;
        for (auto i = 0; i < num_rolls; ++i)
        {
            dice_rolls.emplace_back(rng_->generate(1, dice_size));
        }

        auto result = std::accumulate(dice_rolls.begin(), dice_rolls.end(), 0);

        std::stringstream roll_description_stream;
        roll_description_stream << '(';
        std::copy(dice_rolls.begin(), dice_rolls.end(), std::ostream_iterator<int>(roll_description_stream, ", "));
        
        auto roll_description = roll_description_stream.str();
        roll_description.pop_back();
        roll_description.pop_back();
        roll_description.push_back(')');

        rolls.emplace_back(roll_description);

        return result;
    }

    auto evaluate_operation(std::stack<int>& stack, const std::string& token) -> void
    {
        int op2 = stack.top();
        stack.pop();

        int op1 = stack.top();
        stack.pop();

        if (token == "+")
        {
            stack.push(op1 + op2);
        }
        else if (token == "-")
        {
            stack.push(op1 - op2);
        }
        else
        {
            throw std::runtime_error("Unexpected operator: " + token);
        }
    }

    auto get_token_type(const std::string token) -> token_type
    {
        if (!token.empty() &&
            std::find_if(token.begin(), token.end(), [](char c) { return !std::isdigit(c); }) == token.end())
        {
            return token_type::number;
        }
        else if (token == "(")
        {
            return token_type::left_parenthesis;
        }
        else if (token == ")")
        {
            return token_type::right_parenthesis;
        }
        else if (token == "+" || token == "-")
        {
            return token_type::operation;
        }
        else if (token.find_first_of("dD") != -1)
        {
            return token_type::dice_expression;
        }

        throw std::runtime_error("Unexpected token type: " + token);
    }

    auto parse(const std::string expression) -> std::vector<std::string>
    {
        // Split expression on parenthesis and mathematical operators
        const std::regex expr{ R"(([0-9dbw]+|[\+\(\)-]))" };
        const std::sregex_iterator rend;
        std::sregex_iterator rit(expression.begin(), expression.end(), expr);
        std::vector<std::string> tokens;
        while (rit != rend)
        {
            tokens.emplace_back((rit++)->str());
        }
        return tokens;
    }

    auto convert_infix_to_prefix(const std::vector<std::string>& tokens) -> std::vector<std::string>
    {
        std::vector<std::string> result;
        std::stack<std::string> stack;

        for (const auto& token : tokens)
        {
            // If number push onto stack.
            auto token_type = get_token_type(token);
            switch (token_type)
            {
            case token_type::number:
            case token_type::dice_expression:
                result.emplace_back(token);
                break;

            case token_type::left_parenthesis:
            case token_type::right_parenthesis:
            case token_type::operation:
                if (token_type != token_type::left_parenthesis)
                {
                    // clang-format off
                    while (!stack.empty() &&
                           (
                               (token_type == token_type::right_parenthesis && get_token_type(stack.top()) != token_type::left_parenthesis) ||
                               precedence(stack.top()) > precedence(token) ||
                               (precedence(stack.top()) == precedence(token) && associativity(token) == operator_assocation::left_to_right)
                           )
                        )
                    // clang-format on
                    {
                        result.emplace_back(stack.top());
                        stack.pop();
                    }

                    if (token_type == token_type::right_parenthesis)
                    {
                        stack.pop();
                    }
                }

                if (token_type != token_type::right_parenthesis)
                {
                    stack.push(token);
                }
                break;

            default:
                throw std::runtime_error("Unexpected token: " + token);
            }
        }

        while (!stack.empty())
        {
            result.emplace_back(stack.top());
            stack.pop();
        }

        return result;
    }
};

auto main(int argc, char* argv[]) -> int
{
    if (argc < 2)
    {
        std::cout << "Usage:\n"
                  << "   1d4\n"
                  << "\n";
        return 0;
    }

    if (_stricmp(argv[1], "test") == 0)
    {
        ::testing::InitGoogleTest(&argc, argv);
        return RUN_ALL_TESTS();
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

// TESTING

using ::testing::_;
using ::testing::ContainerEq;
using ::testing::Eq;
using ::testing::Return;
using ::testing::StrEq;
using ::testing::Test;

class mock_random_number_generator : public random_number_generator
{
public:
    MOCK_METHOD(int, generate, (int min, int max), (override));
};

struct expression_evaluator_test : public Test
{
    std::string description;
    mock_random_number_generator rng;
    expression_evaluator eval{ &rng };
};

TEST_F(expression_evaluator_test, parse_1)
{
    auto result = eval.parse("1d20b1+7");
    EXPECT_THAT(result, ContainerEq(std::vector<std::string>{ "1d20b1", "+", "7" }));
}

TEST_F(expression_evaluator_test, parse_2)
{
    auto result = eval.parse("(1d20b1+7)-3");
    EXPECT_THAT(result, ContainerEq(std::vector<std::string>{ "(", "1d20b1", "+", "7", ")", "-", "3" }));
}

TEST_F(expression_evaluator_test, parse_3)
{
    auto result = eval.parse("1+1");
    EXPECT_THAT(result, ContainerEq(std::vector<std::string>{ "1", "+", "1" }));
}

TEST_F(expression_evaluator_test, convert_infix_to_prefix_1)
{
    auto result = eval.convert_infix_to_prefix(std::vector<std::string>{ "1", "+", "1" });
    EXPECT_THAT(result, ContainerEq(std::vector<std::string>{ "1", "1", "+" }));
}

TEST_F(expression_evaluator_test, convert_infix_to_prefix_2)
{
    EXPECT_THROW(eval.convert_infix_to_prefix(std::vector<std::string>{ "1", "+", "a" }), std::runtime_error);
}

TEST_F(expression_evaluator_test, convert_infix_to_prefix_3)
{
    auto result = eval.convert_infix_to_prefix(std::vector<std::string>{ "(", "1", "+", "1", ")" });
    EXPECT_THAT(result, ContainerEq(std::vector<std::string>{ "1", "1", "+" }));
}

TEST_F(expression_evaluator_test, convert_infix_to_prefix_4)
{
    auto result = eval.convert_infix_to_prefix(std::vector<std::string>{ "(", "2d10b1", "+", "1", ")" });
    EXPECT_THAT(result, ContainerEq(std::vector<std::string>{ "2d10b1", "1", "+" }));
}

TEST_F(expression_evaluator_test, simple_number)
{
    EXPECT_CALL(rng, generate(_, _)).Times(0);
    auto result = eval.evaluate("4", &description);
    EXPECT_THAT(result, Eq(4));
    EXPECT_THAT(description, StrEq(""));
}

TEST_F(expression_evaluator_test, simple_number_redux)
{
    EXPECT_CALL(rng, generate(_, _)).Times(0);
    auto result = eval.evaluate("6", &description);
    EXPECT_THAT(result, Eq(6));
    EXPECT_THAT(description, StrEq(""));
}

TEST_F(expression_evaluator_test, simple_addition)
{
    EXPECT_CALL(rng, generate(_, _)).Times(0);
    auto result = eval.evaluate("1+1", &description);
    EXPECT_THAT(result, Eq(2));
    EXPECT_THAT(description, StrEq(""));
}

TEST_F(expression_evaluator_test, simple_d6_roll)
{
    EXPECT_CALL(rng, generate(1, 6)).Times(1).WillOnce(Return(4));
    auto result = eval.evaluate("d6", &description);
    EXPECT_THAT(result, Eq(4));
    EXPECT_THAT(description, StrEq("(4)"));
}

TEST_F(expression_evaluator_test, simple_d6_roll_redux)
{
    EXPECT_CALL(rng, generate(1, 6)).Times(1).WillOnce(Return(4));
    auto result = eval.evaluate("1d6", &description);
    EXPECT_THAT(result, Eq(4));
    EXPECT_THAT(description, StrEq("(4)"));
}

TEST_F(expression_evaluator_test, multiple_d6_roll)
{
    EXPECT_CALL(rng, generate(1, 6)).Times(2).WillOnce(Return(4)).WillOnce(Return(3));
    auto result = eval.evaluate("2d6", &description);
    EXPECT_THAT(result, Eq(7));
    EXPECT_THAT(description, StrEq("(4, 3)"));
}
