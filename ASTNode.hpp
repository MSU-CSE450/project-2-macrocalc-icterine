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
  ELSE_STATEMENT,    // Else statements
  WHILE_LOOP         // While loops
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
  std::vector<std::pair<int, int>> variableEntries; // first pair.first - index in string, pair.second - unique id

public:
  // Constructor for STRING nodes
  ASTNode(Type type, const std::string& string_val) : type(type), lexeme(string_val) {}
  ASTNode(Type type, const std::string& string_val, std::vector<std::pair<int,int>> entries) : type(type), lexeme(string_val) {
    this->variableEntries = entries;
  }

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
        std::ostringstream result;
        int var_index = 0, i = 0;

        // move through a string character-by-character
        while (var_index < variableEntries.size() || i < lexeme.length()) {

          // if current index = index of a variable, look it up in a symbol table and append to the output
          if (var_index < variableEntries.size() && i == variableEntries[var_index].first) {
            result << symbols.GetValue(variableEntries[var_index++].second);  
          } else  {
            result << lexeme[i++];  
          }
        }
        std::cout << result.str() << "\n";
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
        else if (elseBlock != nullptr) {
          elseBlock->Run(symbols);
        }

        return 0;

      case ELSE_STATEMENT:
        rvalue = right->Run(symbols);
        return 0;

      case WHILE_LOOP:
        lvalue = left->Run(symbols);

        while (lvalue != 0) {
          rvalue = right->Run(symbols);
          lvalue = left->Run(symbols);
        }

        return 0;

      default:
        Utils::error("Unknown node type encountered during execution", token);
    }

    return 0;
  }
};
