#pragma once

#include <cmath>
#include <sstream>
#include <string>
#include <vector>

#include "SymbolTable.hpp"
#include "lexer.hpp"
#include "Utils.hpp"
enum Type
  {
    ASSIGNMENT, // Contains a variable in the left child and expression in the right child
    VARIABLE, // Contains information about the name of a variable in "token" field
    NUMBER, // Contains a literal number in "value" field
    BINARY_OPERATION, // Contains a binary operation + or - in a "token" field, and expressions in both left and right children
    UNARY_OPERATION, // Contains no useful information in its fields. It will have only a left child, the value of which we must negate 
    UPDATE, // Updates a variable that already exists, for example a = 5;
    STATEMENT_BLOCK, // Block node, not implemented
    PRINT // Print node. Prints expression in its left child. TO DO: printing strings with {}
  };
class ASTNode {

private:
  
  Type type;
  emplex::Token token;
  int var_unique_id;
  double value = 0;
  ASTNode* left = nullptr;
  ASTNode* right = nullptr;

public:
  ASTNode(Type type)
  {
    this->type = type;
  }
  ASTNode(Type type, emplex::Token token) : ASTNode(type)
  {
    this->token = token;
  }
  ASTNode(Type type, double value) : ASTNode(type)
  {
    if (type == Type::VARIABLE)
      var_unique_id = (int) value;
    else
      this->value = value;
    //std::cout << "Created NUMBER with a value " << value << std::endl;
    
  }
  ASTNode(Type type, ASTNode* left) : ASTNode(type)
  {
    this->left = left;
  }

  ASTNode(Type type, VarData varData) : ASTNode(type)
  {
    var_unique_id = varData.unique_id;
  }
  // CONSTRUCTORS, ETC HERE.
  // CAN SPECIFY NODE TYPE AND ANY NEEDED VALUES HERE OR USING OTHER FUNCTIONS.

  // CODE TO ADD CHILDREN AND SETUP AST NODE HERE.
  void AddChild(ASTNode node) { ; }
  void SetLeft(ASTNode* node)
  {
    left = node;
  }
  void SetRight(ASTNode* node)
  {
    right = node;
  }
  
  // CODE TO EXECUTE THIS NODE (AND ITS CHILDREN, AS NEEDED).
  double Run(SymbolTable & symbols) 
  { 
    double lvalue = 0, rvalue = 0;
    int unique_id;
    std::string var_identifier{};
    switch (type)
    {
      case NUMBER:
        //std::cout << "RUN NUMBER, value = " << value << std::endl;
        return value;
        break;
      case VARIABLE:
        return symbols.GetValue(var_unique_id);
        break;
      case ASSIGNMENT:
        unique_id = left->getVarId();
        if (right != nullptr)
        {
          rvalue = right->Run(symbols);
        }
        //std::cout << "Running node ASSIGNMENT: var " << var_identifier << " = " << rvalue << std::endl;
        symbols.UpdateVar(unique_id, rvalue);
        return rvalue;
      case UNARY_OPERATION:
        // UNARY node will has only one left child - value it has to negate
        value = left->Run(symbols);
        //std::cout << "Unary token id is " << token.id << "\n";
        if (token.id == emplex::Lexer::ID_negation)
          return -value;
        else if (token.id == emplex::Lexer::ID_not)
          return value == 0 ? 1 : 0;  // Logical NOT
        Utils::error("Expected unary -", token);
        return 0;
        break;
      case BINARY_OPERATION:
        lvalue = left->Run(symbols);
        rvalue = right->Run(symbols);
        //std::cout << "Executing BINARY_OPERATION: left = " << lvalue << ", right = " << rvalue << std::endl;
        switch (token.id)
        {
          case emplex::Lexer::ID_add:
            value = lvalue + rvalue;
            return lvalue + rvalue;
          case emplex::Lexer::ID_negation:
            value = lvalue - rvalue;
            return lvalue - rvalue;
          case emplex::Lexer::ID_multiply:
            return lvalue * rvalue;
          case emplex::Lexer::ID_divide:
            if (rvalue == 0) Utils::error("Division by zero", token);
              return lvalue / rvalue;
          case emplex::Lexer::ID_exponent:
            return pow(lvalue, rvalue);
          default:
            Utils::error("Unknown binary operation", token);
        }
      case PRINT:
        lvalue = left->Run(symbols);
        std::cout << lvalue << std::endl;
        break;
      default:
        Utils::error("Unknown token encountered during execution of a tree",token);
        break;
      
        
    }

    return 0;
  }

  int getVarId()
  {
    return this->var_unique_id;
  }
  ASTNode* getRight()
  {
    return this->right;
  }
};
