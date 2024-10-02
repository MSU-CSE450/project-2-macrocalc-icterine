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

    // parses a statement that begins with var - either just initialization or assignment
    // i.e. var x; OR var x = expression;
    ASTNode* parseAssignment()
    {
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
        if (table.HasVarInCurrentScope(identifier))
            Utils::error("Tried to redefine varialbe", tokens[token_id]);

        int unique_id = table.InitializeVar(identifier);
        
        node->SetLeft(new ASTNode(Type::VARIABLE, unique_id));
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
        using namespace emplex;
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
    ASTNode* node = parseFactor(); 
    while (token_id < tokens.size() && (tokens[token_id].id == emplex::Lexer::ID_multiply || tokens[token_id].id == emplex::Lexer::ID_divide))
    {
        emplex::Token binary_op = tokens[token_id];
        ++token_id;
        ASTNode* right_node = parseFactor();
        ASTNode* binary_op_node = new ASTNode(BINARY_OPERATION, binary_op);
        binary_op_node->SetLeft(node);
        binary_op_node->SetRight(right_node);
        node = binary_op_node;
    }
    return node;
}

ASTNode* parseFactor()
{
    using namespace emplex;
    ASTNode* node = parsePrimary();
    if (token_id < tokens.size() && tokens[token_id].id == emplex::Lexer::ID_exponent)
    {
        emplex::Token binary_op = tokens[token_id];
        ++token_id;
        ASTNode* right_node = parseFactor();  // Right-associative behavior: parse the right side recursively
        ASTNode* binary_op_node = new ASTNode(BINARY_OPERATION, binary_op);
        binary_op_node->SetLeft(node);
        binary_op_node->SetRight(right_node);
        node = binary_op_node;
    }
    return node;
}

// parsePrimary handles numbers, variables, and parenthesized expressions
ASTNode* parsePrimary()
{
    // Handle negation (unary minus)
    if (token_id < tokens.size() && tokens[token_id].id == emplex::Lexer::ID_negation)
    {
        emplex::Token negation_token = tokens[token_id];
        ++token_id;
        ASTNode* operand = parsePrimary();  // Recursively parse the operand
        ASTNode* negation_node = new ASTNode(UNARY_OPERATION, negation_token);
        negation_node->SetLeft(operand);
        return negation_node;
    }

        // Handle logical NOT (!)
    if (token_id < tokens.size() && tokens[token_id].id == emplex::Lexer::ID_not)
    {
        emplex::Token not_token = tokens[token_id];
        ++token_id;
        ASTNode* operand = parsePrimary();  // Recursively parse the operand
        ASTNode* not_node = new ASTNode(UNARY_OPERATION, not_token);
        not_node->SetLeft(operand);
        return not_node;
    }

        // Handle assignment in expressions
    if (token_id < tokens.size() && tokens[token_id].id == emplex::Lexer::ID_identifier)
    {
        int unique_id = table.GetUniqueId(tokens[token_id].lexeme);
        ++token_id;

        // Handle assignment inside expressions
        if (token_id < tokens.size() && tokens[token_id].id == emplex::Lexer::ID_assignment)
        {
            ++token_id;
            ASTNode* assignment_expr = parseExpression(); // Right-hand side of the assignment
            ASTNode* assignment_node = new ASTNode(ASSIGNMENT);
            assignment_node->SetLeft(new ASTNode(VARIABLE, unique_id));
            assignment_node->SetRight(assignment_expr);
            return assignment_node;  // Return the assignment node as part of the expression
        }

        return new ASTNode(VARIABLE, unique_id);
    }

    // Handle floating-point numbers and integers
    if (token_id < tokens.size() && (tokens[token_id].id == emplex::Lexer::ID_integer || tokens[token_id].id == emplex::Lexer::ID_float))
    {
        ++token_id;
        return new ASTNode(NUMBER, std::stod(tokens[token_id-1].lexeme)); // Convert the lexeme to double
    }
    else if (token_id < tokens.size() && tokens[token_id].id == emplex::Lexer::ID_identifier)
    {
        int unique_id = table.GetUniqueId(tokens[token_id].lexeme);
        ++token_id;
        return new ASTNode(VARIABLE, unique_id);
    }
    else if (token_id < tokens.size() && tokens[token_id].id == emplex::Lexer::ID_open_parenthesis)
    {
        ++token_id;
        ASTNode* node = parseExpression();
        if (tokens[token_id].id != emplex::Lexer::ID_close_parenthesis)
        {
            Utils::error("Expected closing parenthesis", tokens[token_id]);
        }
        ++token_id;
        return node;
    }

    Utils::error("Unexpected token", tokens[token_id]);
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

    SymbolTable getSymbolTable()
    {
        return this->table;
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
                case Lexer::ID_identifier:
                    node = parseIdentifier();
                    nodes.push_back(node);
                    break;
                case Lexer::ID_print:
                    node = parsePrint();
                    nodes.push_back(node);
                    break;
                default:
                    Utils::error("[Parse loop] Unexpected identifier, ", tokens[token_id-1]);
                    break;
            }
        }

        // execute nodes after we parse all of them
        for (auto node: nodes)
            node->Run(table);


    }

    ASTNode* parseIdentifier()
    // parses statement that begins with an identifier (variable name)
    {
        std::string identifier = tokens[token_id].lexeme; //extract variable name
        

        int unique_id = table.GetUniqueId(identifier); // get a unique id of this variable in a SymbolTable
        
        

        // expect "=" after an identifier
        if (tokens[++token_id] != emplex::Lexer::ID_assignment)
            Utils::error("Expected = after identifier", tokens[token_id]);

        token_id++;

        ASTNode* assignmentNode = new ASTNode(ASSIGNMENT); // create an assignment node
        ASTNode* variableNode = new ASTNode(VARIABLE, unique_id); // create a variable node (left child)
        ASTNode* expressionNode = parseExpression(); // create an expression node (right child)

        assignmentNode->SetLeft(variableNode);
        assignmentNode->SetRight(expressionNode);
        
        // expect a semicolon at the end of expression
        if (tokens[token_id] != emplex::Lexer::ID_semicolon)
            Utils::error("Expected a semicolon at the end of expression (parsing =)", tokens[token_id]);
        token_id++;
        return assignmentNode;
    }

    ASTNode* parsePrint()
    // parses expressions ONLY. Not fully implemented.
    {
        ++token_id;
        
        // expect that the next lexeme after print is "("
        if (tokens[token_id] != emplex::Lexer::ID_open_parenthesis)
            Utils::error("Expected ( after print keyword", tokens[token_id]);

        ++token_id;

        // parse whatever expression there is
        ASTNode* expression = parseExpression();

        // expect ) and ; at the end
        if (tokens[token_id] != emplex::Lexer::ID_close_parenthesis)
            Utils::error("Expected ) at the end of print", tokens[token_id]);
        ++token_id;
        if (tokens[token_id] != emplex::Lexer::ID_semicolon)
            Utils::error("Expected ; at the end of print");
        ++token_id;
        
        ASTNode* printNode = new ASTNode(PRINT);

        printNode->SetLeft(expression);

        return printNode;
    }
    void print_table()
    {
        table.PrintTable();
    }
};