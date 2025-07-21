#include <algorithm>
#include <numeric>
#include <sstream>
#include <iterator>
#include <stdexcept>
#include "expression_evaluator.h"

expression_evaluator::expression_evaluator(random_number_generator* rng) : rng_{ rng }
{
}

int expression_evaluator::get_precedence(const std::string& op)
{
    auto it = operators.find(op);
    if (it == operators.end())
    {
        throw std::runtime_error("Unknown operator precedence: " + op);
    }
    return it->second.precedence;
}
expression_evaluator::assocativity expression_evaluator::get_associativity(const std::string& op)
{
    auto it = operators.find(op);
    if (it == operators.end())
    {
        throw std::runtime_error("Unknown operator associativity: " + op);
    }
    return it->second.associativity;
}

int expression_evaluator::evaluate(const std::string expression, std::string* description)
{
    std::stack<int> stack;
    std::vector<std::string> rolls;

    auto tokens = parse(expression);
    auto prefix = convert_infix_to_prefix(tokens);

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
        std::stringstream result{};
        std::copy(rolls.begin(), rolls.end(), std::ostream_iterator<std::string>(result, " "));
        *description = result.str();
        if (!rolls.empty())
        {
            (*description).pop_back();
        }
    }

    return stack.top();
}

int expression_evaluator::evaluate_dice_expression(const std::string& token, std::vector<std::string>& rolls)
{
    const std::regex expr{ "(\\d*)[dD](\\d+)([bBwW]?)(\\d*)" };
    std::smatch match;
    if (!std::regex_match(token, match, expr))
    {
        throw std::runtime_error("Improper dice expression: " + token);
    }

    auto num_rolls = match[1].str().empty() ? 1 : std::stoi(match[1].str());
    auto dice_size = std::stoi(match[2].str());
    auto keeping_mode = get_keeping_mode(match[3].str());
    auto keeping_count = match[4].str().empty() ? 0 : std::stoi(match[4].str());

    std::vector<int> dice_rolls;
    for (auto i = 0; i < num_rolls; ++i)
    {
        dice_rolls.emplace_back(rng_->generate(1, dice_size));
    }

    int result{};
    std::vector<int> dropped_dice_rolls;
    switch (keeping_mode)
    {
    case keeping_mode::all:
        break;

    case keeping_mode::best:
        while (dice_rolls.size() > keeping_count)
        {
            auto smallest = std::min_element(dice_rolls.begin(), dice_rolls.end());
            dropped_dice_rolls.push_back(*smallest);
            dice_rolls.erase(smallest);
        }
        break;

    case keeping_mode::worst:
        while (dice_rolls.size() > keeping_count)
        {
            auto largest = std::max_element(dice_rolls.begin(), dice_rolls.end());
            dropped_dice_rolls.push_back(*largest);
            dice_rolls.erase(largest);
        }
        break;

    default:
        throw std::runtime_error("Invalid dice modifier: " + match[3].str());
    }

    result = std::accumulate(dice_rolls.begin(), dice_rolls.end(), 0);

    std::stringstream roll_description_stream;
    roll_description_stream << '(';
    std::copy(dice_rolls.begin(), dice_rolls.end(), std::ostream_iterator<int>(roll_description_stream, ", "));
    std::copy(dropped_dice_rolls.begin(), dropped_dice_rolls.end(),
              std::ostream_iterator<int>(roll_description_stream, ", "));

    auto roll_description = roll_description_stream.str();
    roll_description.pop_back();
    roll_description.pop_back();
    roll_description.push_back(')');

    rolls.emplace_back(roll_description);

    return result;
}

void expression_evaluator::evaluate_operation(std::stack<int>& stack, const std::string& token)
{
    int op2 = stack.top();
    stack.pop();

    int op1 = stack.top();
    stack.pop();

    switch (token[0])
    {
    case '+':
        stack.push(op1 + op2);
        break;

    case '-':
        stack.push(op1 - op2);
        break;

    case '*':
        stack.push(op1 * op2);
        break;

    default:
        throw std::runtime_error("Unexpected operator: " + token);
    }
}

expression_evaluator::token_type expression_evaluator::get_token_type(const std::string& token)
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
    else if (token == "+" || token == "-" || token == "*")
    {
        return token_type::operation;
    }
    else if (token.find_first_of("dD") != -1)
    {
        return token_type::dice_expression;
    }

    throw std::runtime_error("Unexpected token type: " + token);
}

expression_evaluator::keeping_mode expression_evaluator::get_keeping_mode(const std::string& m)
{
    if (m.empty())
    {
        return keeping_mode::all;
    }

    switch (std::tolower(m[0]))
    {
    case 'b':
        return keeping_mode::best;

    case 'w':
        return keeping_mode::worst;

    default:
        throw std::runtime_error("Invalid dice modifier: " + m);
    }
}

std::vector<std::string> expression_evaluator::parse(const std::string expression)
{
    const std::regex expr{ R"(([0-9dbw]+|[\+\(\)\*-]))" };
    const std::sregex_iterator rend;
    std::sregex_iterator rit(expression.begin(), expression.end(), expr);
    std::vector<std::string> tokens;
    while (rit != rend)
    {
        tokens.emplace_back((rit++)->str());
    }
    return tokens;
}

std::vector<std::string> expression_evaluator::convert_infix_to_prefix(const std::vector<std::string>& tokens)
{
    std::vector<std::string> result;
    std::stack<std::string> operator_stack;

    for (const auto& token : tokens)
    {
        auto token_type = get_token_type(token);
        switch (token_type)
        {
        case token_type::number:
        case token_type::dice_expression:
            result.push_back(token);
            break;

        case token_type::left_parenthesis:
            operator_stack.push(token);
            break;

        case token_type::right_parenthesis: {
            while (!operator_stack.empty() && get_token_type(operator_stack.top()) != token_type::left_parenthesis)
            {
                result.push_back(operator_stack.top());
                operator_stack.pop();
            }

            if (get_token_type(operator_stack.top()) != token_type::left_parenthesis)
            {
                throw std::runtime_error("No matching parenthesis");
            }
            operator_stack.pop();
        }
        break;

        case token_type::operation:
            while (!operator_stack.empty())
            {
                auto top_token = operator_stack.top();
                auto top_token_precedence = get_precedence(top_token);
                auto token_precedence = get_precedence(token);

                if (top_token_precedence > token_precedence ||
                    (top_token_precedence == token_precedence &&
                     get_associativity(token) == assocativity::left_to_right))
                {
                    result.push_back(top_token);
                    operator_stack.pop();
                }
                else
                {
                    break;
                }
            }
            operator_stack.push(token);
            break;

        default:
            throw std::runtime_error("Unexpected token: " + token);
        }
    }

    while (!operator_stack.empty())
    {
        result.emplace_back(operator_stack.top());
        operator_stack.pop();
    }

    return result;
}