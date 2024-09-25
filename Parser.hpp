#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include "lexer.hpp"
class Parser
{
private:
        std::vector<emplex::Token> tokens;
        int token_id = 0;
public:
    Parser(std::ifstream& in_file)
    {
        emplex::Lexer lexer;
        tokens = lexer.Tokenize(in_file);
    }



    //for debugging purposes
    void print_tokens()
    {
        for (auto token : tokens)
        {
            std::cout << "Token id: " << token.id << ", lexeme: " << token.lexeme << "\n";
        }
    }

    void Parse()
    {
        return;
    }
};