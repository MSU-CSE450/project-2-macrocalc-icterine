#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include "lexer.hpp"
#include "ASTNode.hpp"
#include "Utils.hpp"
class Parser
{
private:
        std::vector<emplex::Token> tokens{};
        int token_id = 0;
        std::vector<ASTNode> nodes{};
        SymbolTable table{};
    

    bool isPlusOrMinusToken(emplex::Token token)
    {
        return token.id == emplex::Lexer::ID_add || token.id == emplex::Lexer::ID_negation;
    }

    ASTNode* parseAssignment()
    {
        // var x; OR var x = expression;
        using namespace emplex;
        ++token_id;
        ASTNode* node = new ASTNode(Type::ASSIGNMENT); // assignment node

        // check if current token is an identifier
        if (token_id < tokens.size() && tokens[token_id] != Lexer::ID_identifier)
        {
            Utils::error("Expected identifier", tokens[token_id]);
        }
        std::string identifier = tokens[token_id].lexeme;

        // left child of the assignment node will be a variable type
        node->SetLeft(new ASTNode(Type::VARIABLE, tokens[token_id]));
        ++token_id;
       

        // if there is a semicolor - return node
        if (token_id < tokens.size() && tokens[token_id] == Lexer::ID_semicolon)
        {
            ++token_id;
            return node;
        }
        
        // if not a semicolor - then it must be a "="
        if (token_id < tokens.size() && tokens[token_id] != Lexer::ID_assignment)
        {
            Utils::error("Expected assignment operator", tokens[token_id]);
        }
        ++token_id;
        // parse expression
        ASTNode* right = parseExpression();
        // expect that there will be a semicolon at the end of expression
        if (token_id < tokens.size() && tokens[token_id] != Lexer::ID_semicolon) 
        { Utils::error("Expect ; at the end of statement", tokens[token_id]); }

        ++token_id;
        // if everything is okay and there is a semicolon, assign an expression to the right subtree
        node->SetRight(right);

        // return a node of "ASSIGNMENT" type. Left child is a "VARIABLE", right child is the parsed expression subtree
        return node;
    }

    ASTNode* parseExpression()
    {
        ASTNode* node = parseTerm(); 
        while (token_id < tokens.size() && isPlusOrMinusToken(tokens[token_id]))
        {
            emplex::Token binary_op = tokens[token_id];
            ++token_id;
            ASTNode* right_node = parseTerm();
            ASTNode* binary_op_node = new ASTNode(BINARY_OPERATION, binary_op);
            binary_op_node->SetLeft(node);
            binary_op_node->SetRight(right_node); 

            node = binary_op_node;
        }
        return node;
    }

    ASTNode* parseTerm()
    {
        using namespace emplex;

        // handle negation aka unary minus. For example var a = -1;
        if (token_id < tokens.size() && tokens[token_id] == Lexer::ID_negation)
        {
            Token negation_token = tokens[token_id];
            token_id++;
            if (token_id >= tokens.size()) Utils::error("Expected a token, got end of the file", tokens[token_id-1]);

            ASTNode* operand = parseTerm(); // parse a term that we have to negate
            ASTNode* unary_node = new ASTNode(UNARY_OPERATION, negation_token);
            unary_node->SetLeft(operand); // put this term in the left child of the unary node
            return unary_node;
        }

        // numbers
        if (token_id < tokens.size() && ((tokens[token_id] == Lexer::ID_integer) || (tokens[token_id] == Lexer::ID_float)))
        {
            ++token_id;
            // create a "NUMBER" node and initialize field "value" with a literal number
            return new ASTNode(NUMBER, std::stod(tokens[token_id-1].lexeme));
        }

        //variables
        if (token_id < tokens.size() && tokens[token_id] == Lexer::ID_identifier)
        {
            // create a "VARIABLE" node and initialize "token" field with information about the variable
            return new ASTNode(VARIABLE, tokens[token_id++]);
        }


        // parentheses
        if (token_id < tokens.size() && tokens[token_id] == Lexer::ID_open_parenthesis)
        {
            token_id++; // skip the "(" and parse expression as usual
            ASTNode* node = parseExpression();

            // expect to have the ")" parenthesis at the end of expression
            if (token_id < tokens.size() && tokens[token_id] != Lexer::ID_close_parenthesis)
            {
                Utils::error("Expected ) at the end of expression", tokens[token_id]);
            }
            token_id++;
            return node;
        }

        // anything that is not implemented yet
        Utils::error("Unexpected token in the term", tokens[token_id]);

        return nullptr;
    }
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
        using namespace emplex;
        std::vector<ASTNode*> nodes;
        ASTNode* node = nullptr;
        while (token_id < tokens.size())
        {
            
            switch (tokens[token_id])
            {
                case Lexer::ID_var:
                    node = parseAssignment();
                    nodes.push_back(node);
                    break;
                default:
                    Utils::error("Unexpected identifier", tokens[token_id]);
                    break;
            }
        }

        // execute nodes after we parse all of them
        for (auto node: nodes)
            node->Run(table);


    }

    void print_table()
    {
        table.PrintTable();
    }
};