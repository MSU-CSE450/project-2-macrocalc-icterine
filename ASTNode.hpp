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
    PRINT, // Print node. Prints expression in its left child. TO DO: printing strings with {}
    STRING // Contains a string field
  };
class ASTNode {

private:
  
  Type type;
  emplex::Token token;
  int var_unique_id;
  double value = 0;
  std::string string_value = "";
  ASTNode* left = nullptr;
  ASTNode* right = nullptr;

public:

// Constructor for STRING nodes
ASTNode(Type type, const std::string& string_val) : type(type), string_value(string_val) {}

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
      case STRING:
        std::cout << string_value << std::endl; // implement string printing
        return 0;
      case VARIABLE:
        return symbols.GetValue(var_unique_id);
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
      case BINARY_OPERATION:
        lvalue = left->Run(symbols);

        // Short-circuit for AND (&&) and OR (||)
        if (token.id == emplex::Lexer::ID_and)
          return (lvalue != 0) && (right->Run(symbols) != 0) ? 1 : 0;  // Logical AND
        if (token.id == emplex::Lexer::ID_or)
          return (lvalue != 0) || (right->Run(symbols) != 0) ? 1 : 0;  // Logical OR
        //std::cout << "Executing BINARY_OPERATION: left = " << lvalue << ", right = " << rvalue << std::endl;
        rvalue = right->Run(symbols);
        switch (token.id)
        {
          case emplex::Lexer::ID_add:
            return lvalue + rvalue;
          case emplex::Lexer::ID_negation:
            return lvalue - rvalue;
          case emplex::Lexer::ID_multiply:
            return lvalue * rvalue;
          case emplex::Lexer::ID_divide:
            if (rvalue == 0) Utils::error("Division by zero", token);
              return lvalue / rvalue;
          case emplex::Lexer::ID_exponent:
            return pow(lvalue, rvalue);
          case emplex::Lexer::ID_equality:
            return lvalue == rvalue ? 1 : 0;  // Equality (==)
          case emplex::Lexer::ID_not_eq:
            return lvalue != rvalue ? 1 : 0;  // Not equal (!=)
          case emplex::Lexer::ID_greater_than:
            return lvalue > rvalue ? 1 : 0;   // Greater than (>)
          case emplex::Lexer::ID_greater_or_eq:
            return lvalue >= rvalue ? 1 : 0;  // Greater than or equal (>=)
          case emplex::Lexer::ID_less_than:
            return lvalue < rvalue ? 1 : 0;   // Less than (<)
          case emplex::Lexer::ID_less_or_eq:
            return lvalue <= rvalue ? 1 : 0;  // Less than or equal (<=)
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
