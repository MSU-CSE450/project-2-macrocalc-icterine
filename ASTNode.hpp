#pragma once

#include <cmath>
#include <sstream>
#include <string>
#include <vector>

#include "SymbolTable.hpp"
#include "lexer.hpp"
#include "Utils.hpp"

enum Type {
  ASSIGNMENT,        // Variable assignment (left = variable, right = expression)
  VARIABLE,          // Variable node
  NUMBER,            // Numeric literal
  BINARY_OPERATION,  // Binary operations (+, -, *, etc.)
  UNARY_OPERATION,   // Unary operations (negation, logical NOT)
  UPDATE,            // Update variable value
  STATEMENT_BLOCK,   // Block of statements
  PRINT,             // Print statements
  STRING,            // String literals
  IF_STATEMENT,      // If statements
  ELSE_STATEMENT     // Else statements
};

class ASTNode {
private:
  Type type;
  emplex::Token token;
  int var_unique_id;
  double value = 0;
  std::string lexeme;
  ASTNode* left = nullptr;
  ASTNode* right = nullptr;
  ASTNode* elseBlock = nullptr; // optional 3rd child for 'if' nodes
  std::vector<ASTNode*> blockStatements;

  // Helper function to process strings with variable interpolation
  std::string processString(const std::string& str, SymbolTable& symbols) {
    std::ostringstream result;
    size_t i = 0;
    
    while (i < str.length()) {
      if (str[i] == '{') {
        size_t j = i + 1;
        while (j < str.length() && str[j] != '}') ++j;
        if (j < str.length() && str[j] == '}') {
          std::string var_name = str.substr(i + 1, j - i - 1);
          int unique_id = symbols.GetUniqueId(var_name);
          double var_value = symbols.GetValue(unique_id);

          result << (var_value == static_cast<int>(var_value) ? static_cast<int>(var_value) : var_value);
          i = j + 1;
        }
      } else {
        result << str[i];
        ++i;
      }
    }
    
    return result.str();
  }

public:
  // Constructor for STRING nodes
  ASTNode(Type type, const std::string& string_val) : type(type), lexeme(string_val) {}

  // Constructor for other types
  ASTNode(Type type) : type(type) {}

  // Constructor with token
  ASTNode(Type type, emplex::Token token) : ASTNode(type) { this->token = token; }

  // Constructor for NUMBER or VARIABLE nodes
  ASTNode(Type type, double value) : ASTNode(type) {
    if (type == Type::VARIABLE) var_unique_id = static_cast<int>(value);
    else this->value = value;
  }

  // Set block statements for a block node
  void SetBlockStatements(std::vector<ASTNode*> blockStatements) { this->blockStatements = blockStatements; }

  // Set left and right child nodes
  void SetLeft(ASTNode* node) { left = node; }
  void SetRight(ASTNode* node) { right = node; }
  void SetElseBlock(ASTNode* node) { elseBlock = node; }

  // Main run function to evaluate the ASTNode
  double Run(SymbolTable& symbols) { 
    double lvalue = 0, rvalue = 0;
    int unique_id;

    switch (type) {
      case NUMBER:
        return value;

      case STRING: {
        std::string stripped_string = lexeme.substr(1, lexeme.length() - 2);
        std::cout << processString(stripped_string, symbols) << std::endl;
        return 0;
      }

      case VARIABLE:
        return symbols.GetValue(var_unique_id);

      case ASSIGNMENT:
        unique_id = left->var_unique_id;
        if (right != nullptr) {
          rvalue = right->Run(symbols);
        }
        symbols.UpdateVar(unique_id, rvalue);
        return rvalue;

      case UNARY_OPERATION:
        value = left->Run(symbols);
        if (token.id == emplex::Lexer::ID_negation)
          return -value;
        else if (token.id == emplex::Lexer::ID_not)
          return value == 0 ? 1 : 0;
        Utils::error("Expected unary operation", token);
        return 0;

      case BINARY_OPERATION:
        lvalue = left->Run(symbols);
        if (token.id == emplex::Lexer::ID_and)
          return (lvalue != 0) && (right->Run(symbols) != 0) ? 1 : 0;
        if (token.id == emplex::Lexer::ID_or)
          return (lvalue != 0) || (right->Run(symbols) != 0) ? 1 : 0;

        rvalue = right->Run(symbols);
        switch (token.id) {
          case emplex::Lexer::ID_add:
            return lvalue + rvalue;
          case emplex::Lexer::ID_negation:
            return lvalue - rvalue;
          case emplex::Lexer::ID_multiply:
            return lvalue * rvalue;
          case emplex::Lexer::ID_divide:
            if (rvalue == 0) Utils::error("Division by zero", token);
            return lvalue / rvalue;
          case emplex::Lexer::ID_modulus:
          {
            int lvalue_int = round(lvalue);
            int rvalue_int = round(rvalue);
            auto result = lvalue_int % rvalue_int;
            return (double)result;
          }
          case emplex::Lexer::ID_exponent:
            return pow(lvalue, rvalue);
          case emplex::Lexer::ID_equality:
            return lvalue == rvalue ? 1 : 0;
          case emplex::Lexer::ID_not_eq:
            return lvalue != rvalue ? 1 : 0;
          case emplex::Lexer::ID_greater_than:
            return lvalue > rvalue ? 1 : 0;
          case emplex::Lexer::ID_greater_or_eq:
            return lvalue >= rvalue ? 1 : 0;
          case emplex::Lexer::ID_less_than:
            return lvalue < rvalue ? 1 : 0;
          case emplex::Lexer::ID_less_or_eq:
            return lvalue <= rvalue ? 1 : 0;
          default:
            Utils::error("Unknown binary operation", token);
        }

      case PRINT:
        lvalue = left->Run(symbols);
        if (left->type != STRING) {
          std::cout << lvalue << std::endl;
        }
        return 0;

      case STATEMENT_BLOCK:
        for (ASTNode* statement : blockStatements) {
          statement->Run(symbols);
        }
        return 0;

      case IF_STATEMENT:
        lvalue = left->Run(symbols);
        if (lvalue != 0) {
          rvalue = right->Run(symbols);
        }
        else if (elseBlock != nullptr)
        {
          elseBlock->Run(symbols);
        }
        return 0;

      case ELSE_STATEMENT:
        rvalue = right->Run(symbols);
        return 0;

      default:
        Utils::error("Unknown node type encountered during execution", token);
    }

    return 0;
  }
};
