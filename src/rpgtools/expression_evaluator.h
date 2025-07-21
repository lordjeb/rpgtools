#pragma once
#include <string>
#include <vector>
#include <stack>
#include <unordered_map>
#include <regex>
#include "random_number_generator.h"

class expression_evaluator
{
    random_number_generator* rng_;

    enum class assocativity { left_to_right, right_to_left };

    struct operator_info
    {
        int precedence;
        assocativity associativity;
    };
    
    // PEMDAS...
    const std::unordered_map<std::string, operator_info> operators = {
        { "(", { 0, assocativity::left_to_right } },
        { "*", { 4, assocativity::left_to_right } },
        // { "/", { 4, assocativity::left_to_right } },
        { "+", { 2, assocativity::left_to_right } },
        { "-", { 2, assocativity::left_to_right } },
    };

    int get_precedence(const std::string& op);
    assocativity get_associativity(const std::string& op);

public:
    enum class token_type { number, operation, left_parenthesis, right_parenthesis, dice_expression };
    enum class keeping_mode { all, best, worst };

    expression_evaluator(random_number_generator* rng);

    int evaluate(const std::string expression, std::string* description = nullptr);
    int evaluate_dice_expression(const std::string& token, std::vector<std::string>& rolls);
    void evaluate_operation(std::stack<int>& stack, const std::string& token);
    token_type get_token_type(const std::string& token);
    keeping_mode get_keeping_mode(const std::string& m);
    std::vector<std::string> parse(const std::string expression);
    std::vector<std::string> convert_infix_to_prefix(const std::vector<std::string>& tokens);
};
