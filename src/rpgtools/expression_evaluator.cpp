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
    const std::regex expr{ "(\\d*)[dD](\\d+)(!)?(([bBwW])(\\d*))?" };
    std::smatch match;
    if (!std::regex_match(token, match, expr))
    {
        throw std::runtime_error("Improper dice expression: " + token);
    }

    auto num_rolls = match[1].str().empty() ? 1 : std::stoi(match[1].str());
    auto dice_size = std::stoi(match[2].str());
    auto is_exploding = !match[3].str().empty();
    auto selection_mode = get_keeping_mode(match[5].str());
    auto selection_count = match[6].str().empty() ? 0 : std::stoi(match[6].str());

    std::vector<int> dice_rolls;
    std::vector<std::vector<int>> exploded_rolls; // To track individual explosions for description
    
    for (auto i = 0; i < num_rolls; ++i)
    {
        std::vector<int> individual_exploded_rolls;
        int total_result = 0;
        
        switch (dice_size)
        {
        case 666:
            {
                int result = rng_->generate(1, 6) * 100;
                result += rng_->generate(1, 6) * 10;
                result += rng_->generate(1, 6);
                total_result = result;
                individual_exploded_rolls.push_back(result);
                // Note: Exploding dice logic doesn't apply to special dice like d666/d66
            }
            break;

        case 66:
            {
                int result = rng_->generate(1, 6) * 10;
                result += rng_->generate(1, 6);
                total_result = result;
                individual_exploded_rolls.push_back(result);
                // Note: Exploding dice logic doesn't apply to special dice like d666/d66
            }
            break;

        default:
            {
                int roll = rng_->generate(1, dice_size);
                total_result = roll;
                individual_exploded_rolls.push_back(roll);
                
                // Handle exploding dice
                if (is_exploding)
                {
                    while (roll == dice_size)
                    {
                        roll = rng_->generate(1, dice_size);
                        total_result += roll;
                        individual_exploded_rolls.push_back(roll);
                    }
                }
            }
            break;
        }
        
        dice_rolls.emplace_back(total_result);
        exploded_rolls.emplace_back(individual_exploded_rolls);
    }

    int result{};
    std::vector<int> dropped_dice_rolls;
    std::vector<std::vector<int>> dropped_exploded_rolls; // Track explosion details for dropped dice
    
    switch (selection_mode)
    {
    case dice_selection_mode::all:
        break;

    case dice_selection_mode::best:
        while (dice_rolls.size() > selection_count)
        {
            auto smallest = std::min_element(dice_rolls.begin(), dice_rolls.end());
            auto smallest_index = std::distance(dice_rolls.begin(), smallest);
            
            dropped_dice_rolls.push_back(*smallest);
            dropped_exploded_rolls.push_back(exploded_rolls[smallest_index]);
            
            dice_rolls.erase(smallest);
            exploded_rolls.erase(exploded_rolls.begin() + smallest_index);
        }
        break;

    case dice_selection_mode::worst:
        while (dice_rolls.size() > selection_count)
        {
            auto largest = std::max_element(dice_rolls.begin(), dice_rolls.end());
            auto largest_index = std::distance(dice_rolls.begin(), largest);
            
            dropped_dice_rolls.push_back(*largest);
            dropped_exploded_rolls.push_back(exploded_rolls[largest_index]);
            
            dice_rolls.erase(largest);
            exploded_rolls.erase(exploded_rolls.begin() + largest_index);
        }
        break;

    default:
        throw std::runtime_error("Invalid dice modifier: " + match[3].str());
    }

    result = std::accumulate(dice_rolls.begin(), dice_rolls.end(), 0);

    std::stringstream roll_description_stream;
    roll_description_stream << '(';
    
    for (size_t i = 0; i < dice_rolls.size(); ++i)
    {
        if (i > 0) roll_description_stream << ", ";
        
        if (is_exploding && exploded_rolls[i].size() > 1)
        {
            // Show exploding dice as [roll1+roll2+...]
            roll_description_stream << '[';
            for (size_t j = 0; j < exploded_rolls[i].size(); ++j)
            {
                if (j > 0) roll_description_stream << '+';
                roll_description_stream << exploded_rolls[i][j];
            }
            roll_description_stream << ']';
        }
        else
        {
            roll_description_stream << dice_rolls[i];
        }
    }
    
    // Add dropped dice to description
    for (size_t i = 0; i < dropped_dice_rolls.size(); ++i)
    {
        roll_description_stream << ", ";
        
        if (is_exploding && dropped_exploded_rolls[i].size() > 1)
        {
            // Show exploding dice as [roll1+roll2+...]
            roll_description_stream << '[';
            for (size_t j = 0; j < dropped_exploded_rolls[i].size(); ++j)
            {
                if (j > 0) roll_description_stream << '+';
                roll_description_stream << dropped_exploded_rolls[i][j];
            }
            roll_description_stream << ']';
        }
        else
        {
            roll_description_stream << dropped_dice_rolls[i];
        }
    }

    roll_description_stream << ')';
    auto roll_description = roll_description_stream.str();

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

expression_evaluator::dice_selection_mode expression_evaluator::get_keeping_mode(const std::string& m)
{
    if (m.empty())
    {
        return dice_selection_mode::all;
    }

    switch (std::tolower(m[0]))
    {
    case 'b':
        return dice_selection_mode::best;

    case 'w':
        return dice_selection_mode::worst;

    default:
        throw std::runtime_error("Invalid dice modifier: " + m);
    }
}

std::vector<std::string> expression_evaluator::parse(const std::string expression)
{
    const std::regex expr{ R"(([0-9dbw!]+|[\+\(\)\*-]))" };
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