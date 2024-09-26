#pragma once
#include <iostream>
#include <string>
#include "lexer.hpp"
class Utils
{
public:
    static void error(std::string message, emplex::Token token)
    {
    
        std::cerr << "Error at line " << token.line_id << ": " << message << ", lexeme: " << token.lexeme << " (id " << token.id << ")" << std::endl;
        exit(1);
    }
};