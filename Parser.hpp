#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include "lexer.hpp"
#include "ASTNode.hpp"
#include "Utils.hpp"

using namespace emplex;

class Parser {
private:
  std::vector<emplex::Token> tokens;
  int token_id = 0;
  SymbolTable table;

  // Parses an assignment statement (e.g., var x = expr;)
  ASTNode* parseAssignment() {
    ++token_id;
    ASTNode* node = new ASTNode(Type::ASSIGNMENT);

    // Ensure the current token is an identifier
    if (token_id >= tokens.size() || tokens[token_id] != Lexer::ID_identifier) {
      Utils::error("Expected identifier", tokens[token_id]);
    }
    std::string identifier = tokens[token_id].lexeme;

    // Check if the variable is already defined in the current scope
    if (table.HasVarInCurrentScope(identifier)) {
      Utils::error("Tried to redefine variable", tokens[token_id]);
    }

    int unique_id = table.InitializeVar(identifier);
    node->SetLeft(new ASTNode(Type::VARIABLE, unique_id));
    ++token_id;

    // Handle variable declaration without assignment (e.g., var x;)
    if (token_id < tokens.size() && tokens[token_id] == Lexer::ID_semicolon) {
      ++token_id;
      return node;
    }

    // Ensure the next token is the assignment operator '='
    if (token_id >= tokens.size() || tokens[token_id] != Lexer::ID_assignment) {
      Utils::error("Expected assignment operator", tokens[token_id]);
    }
    ++token_id;

    // Parse the right-hand side expression
    ASTNode* right = parseExpression();

    // Ensure the statement ends with a semicolon
    if (token_id >= tokens.size() || tokens[token_id] != Lexer::ID_semicolon) {
      Utils::error("Expected semicolon at end of statement", tokens[token_id]);
    }
    ++token_id;

    node->SetRight(right);
    return node;
  }

  // Parses logical expressions (e.g., a && b || c)
  ASTNode* parseLogical() {
    ASTNode* node = parseComparison();
    while (token_id < tokens.size() && 
          (tokens[token_id].id == Lexer::ID_and || 
           tokens[token_id].id == Lexer::ID_or)) {
      emplex::Token logical_op = tokens[token_id];
      ++token_id;
      ASTNode* right_node = parseComparison();
      ASTNode* logical_op_node = new ASTNode(BINARY_OPERATION, logical_op);
      logical_op_node->SetLeft(node);
      logical_op_node->SetRight(right_node);
      node = logical_op_node;
    }
    return node;
  }

  // Parses comparison expressions (e.g., a == b, a < b)
  ASTNode* parseComparison(bool singleLineStatement = false) {
    int binary_node_count = 0;

    ASTNode* node;
    if (singleLineStatement) {
      token_id++;
      auto varNode = parseIdentifier(true);
    }
    else {
      node = parseExpression();
    }

    while (token_id < tokens.size() &&
          (tokens[token_id].id == Lexer::ID_equality || 
           tokens[token_id].id == Lexer::ID_not_eq || 
           tokens[token_id].id == Lexer::ID_greater_than || 
           tokens[token_id].id == Lexer::ID_greater_or_eq || 
           tokens[token_id].id == Lexer::ID_less_than || 
           tokens[token_id].id == Lexer::ID_less_or_eq)) {
      emplex::Token binary_op = tokens[token_id];
      ++token_id;
      ASTNode* right_node = parseExpression();
      
      ASTNode* binary_op_node = new ASTNode(BINARY_OPERATION, binary_op);
      binary_node_count++;

      if (binary_node_count > 1) {
        Utils::error("Comparisons should be non-associative.", tokens[token_id]);
      }

      binary_op_node->SetLeft(node);
      binary_op_node->SetRight(right_node);
      node = binary_op_node;
    }
    return node;
  }

  // Parses addition and subtraction expressions (e.g., a + b - c)
  ASTNode* parseExpression() {
    ASTNode* node = parseTerm();
    while (token_id < tokens.size() && 
          (tokens[token_id].id == Lexer::ID_add || 
           tokens[token_id].id == Lexer::ID_negation)) {
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

  // Parses multiplication, division, and modulus expressions (e.g., a * b / c % d)
  ASTNode* parseTerm() {
    ASTNode* node = parseFactor();
    while (token_id < tokens.size() && 
          (tokens[token_id].id == Lexer::ID_multiply || 
           tokens[token_id].id == Lexer::ID_divide ||
           tokens[token_id].id == Lexer::ID_modulus)) {
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

  // Parses exponentiation expressions (e.g., a ^ b)
  ASTNode* parseFactor() {
    ASTNode* node = parsePrimary();
    if (token_id < tokens.size() && tokens[token_id].id == Lexer::ID_exponent) {
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

  // Parses primary expressions such as literals, variables, and parentheses
  ASTNode* parsePrimary() {
    // Handle unary negation
    if (token_id < tokens.size() && tokens[token_id].id == Lexer::ID_negation) {
      emplex::Token negation_token = tokens[token_id];
      ++token_id;
      ASTNode* operand = parsePrimary();
      ASTNode* negation_node = new ASTNode(UNARY_OPERATION, negation_token);
      negation_node->SetLeft(operand);
      return negation_node;
    }

    // Handle logical NOT
    if (token_id < tokens.size() && tokens[token_id].id == Lexer::ID_not) {
      emplex::Token not_token = tokens[token_id];
      ++token_id;
      ASTNode* operand = parsePrimary();
      ASTNode* not_node = new ASTNode(UNARY_OPERATION, not_token);
      not_node->SetLeft(operand);
      return not_node;
    }

    // Handle string literals
    if (token_id < tokens.size() && tokens[token_id].id == Lexer::ID_string) {
      emplex::Token string_token = tokens[token_id];
      ++token_id;
      std::string stripped_string = string_token.lexeme.substr(1, string_token.lexeme.size() - 2);
      return new ASTNode(STRING, stripped_string);
    }

    // Handle variables and assignments
    if (token_id < tokens.size() && tokens[token_id].id == Lexer::ID_identifier) {
      int unique_id = table.GetUniqueId(tokens[token_id].lexeme);
      ++token_id;

      // Handle assignment within expressions (e.g., x = expr)
      if (token_id < tokens.size() && tokens[token_id].id == Lexer::ID_assignment) {
        ++token_id;
        ASTNode* assignment_expr = parseExpression();
        ASTNode* assignment_node = new ASTNode(ASSIGNMENT);
        assignment_node->SetLeft(new ASTNode(VARIABLE, unique_id));
        assignment_node->SetRight(assignment_expr);
        return assignment_node;
      }
      return new ASTNode(VARIABLE, unique_id);
    }

    // Handle numeric literals
    if (token_id < tokens.size() && 
       (tokens[token_id].id == Lexer::ID_integer || tokens[token_id].id == Lexer::ID_float)) {
      ++token_id;
      return new ASTNode(NUMBER, std::stod(tokens[token_id - 1].lexeme));
    }

    // Handle parentheses (e.g., (expr))
    if (token_id < tokens.size() && tokens[token_id].id == Lexer::ID_open_parenthesis) {
      ++token_id;
      ASTNode* node = parseExpression();
      if (tokens[token_id].id != Lexer::ID_close_parenthesis) {
        Utils::error("Expected closing parenthesis", tokens[token_id]);
      }
      ++token_id;
      return node;
    }

    Utils::error("Unexpected token", tokens[token_id]);
    return nullptr;
  }

  // Parses a block of statements inside braces (e.g., { statements })
  ASTNode* parseBlock() {
    if (tokens[token_id].id != Lexer::ID_open_brace) {
      Utils::error("Expected { at the start of block", tokens[token_id]);
    }
    ++token_id;

    table.PushScope();

    std::vector<ASTNode*> blockStatements;
    while (token_id < tokens.size() && tokens[token_id].id != Lexer::ID_close_brace) {
      switch (tokens[token_id].id) {
        case Lexer::ID_var:
          blockStatements.push_back(parseAssignment());
          break;
        case Lexer::ID_identifier:
          blockStatements.push_back(parseIdentifier());
          break;
        case Lexer::ID_print:
          blockStatements.push_back(parsePrint());
          break;
        case Lexer::ID_if:
          blockStatements.push_back(parseIf());
          break;
        case Lexer::ID_while:
          blockStatements.push_back(parseWhile());
          break;
        default:
          Utils::error("Unexpected token in block", tokens[token_id]);
          break;
      }
    }

    if (tokens[token_id].id != Lexer::ID_close_brace) {
      Utils::error("Expected } at the end of block", tokens[token_id]);
    }
    ++token_id;

    ASTNode* blockNode = new ASTNode(STATEMENT_BLOCK);
    blockNode->SetBlockStatements(blockStatements);

    table.PopScope();
    return blockNode;
  }

  ASTNode* parseSingleLine() {

    ASTNode* statement;
    if (token_id < tokens.size() && tokens[token_id].id != Lexer::ID_semicolon) {
      switch (tokens[token_id].id) {
        case Lexer::ID_var:
          statement = parseAssignment();
          break;
        case Lexer::ID_identifier:
          statement = parseIdentifier();
          break;
        case Lexer::ID_print:
          statement = parsePrint();
          break;
        default:
          Utils::error("Unexpected token in block", tokens[token_id]);
          break;
      }
    }

    return statement;
  }

  ASTNode* parseIf() {
    if (tokens[token_id + 1].id != Lexer::ID_open_parenthesis) {
      Utils::error("Expected ( at the start of condition", tokens[token_id]);
    }
    auto if_token = tokens[token_id];
    ASTNode* if_node = new ASTNode(IF_STATEMENT, if_token);

    token_id += 2; // move onto the start of the condition block

    auto conditional = parseLogical();
    if_node->SetLeft(conditional);

    token_id++; // go to beginning of statement token
    ASTNode* statement_node;

    if (tokens[token_id].id != Lexer::ID_open_brace) {
      statement_node = parseSingleLine(); // no block, just single line
    }
    else {
      statement_node = parseBlock(); // typical block parsing
    }

    if_node->SetRight(statement_node);

    if (tokens[token_id].id == Lexer::ID_else) // possible 'else' condition
    {
      auto else_node = parseElse();
      if_node->SetElseBlock(else_node);
    }

    return if_node;
  }

  ASTNode* parseElse() {
      token_id++;
      ASTNode* statement_node;
      
      if (tokens[token_id].id != Lexer::ID_open_brace) {
        statement_node = parseSingleLine(); // no block, just single line
      }
      else {
        statement_node = parseBlock(); // typical block parsing
      }

      return statement_node;
  }

  ASTNode* parseSingleLineLoop() {

    ASTNode* statement;
    if (token_id < tokens.size() && tokens[token_id].id != Lexer::ID_open_parenthesis) {
      switch (tokens[token_id].id) {
        case Lexer::ID_var:
          statement = parseAssignment();
          break;
        case Lexer::ID_identifier:
          statement = parseIdentifier();
          break;
        case Lexer::ID_print:
          statement = parsePrint();
          break;
        default:
          Utils::error("Unexpected token in block", tokens[token_id]);
          break;
      }
    }

    return statement;
  }

  ASTNode* parseWhile() {
    if (tokens[token_id + 1].id != Lexer::ID_open_parenthesis) {
      Utils::error("Expected ( at the start of condition", tokens[token_id]);
    }

    auto while_token = tokens[token_id];
    ASTNode* while_node = new ASTNode(WHILE_LOOP, while_token);

    token_id += 2; // move onto the start of the condition block

    ASTNode* conditional;
    // single line while loop
    // my idea is to evaluate the assignment first, run it and get its result right here, and then do the comparison
    // now that I'm writing this down, I don't think it'll work because the comparison operator depends on two nodes to compare with
    // I would have to change the comparison function to accomodate that
    // there must be an easier way besides tweaking nearly every single parsing function to consider this one case
    if (tokens[token_id].id == Lexer::ID_open_parenthesis) {
      int start = token_id;
      token_id++;
      auto varNode = parseIdentifier(true);

    }
    // normal setup of while loop
    else {
      conditional = parseLogical();
      while_node->SetLeft(conditional); // left child node will be conditional to evaluate every iteration

      token_id++; // go to beginning of statement token
      ASTNode* statement_node;

      // parse single line or statement block?
      if (tokens[token_id].id != Lexer::ID_open_brace) {
        statement_node = parseSingleLine();
      }
      else {
        statement_node = parseBlock();
      }
      

      while_node->SetRight(statement_node);
    }
    
    // while_node->SetLeft(conditional); // left child node will be conditional to evaluate every iteration

    // token_id++; // go to beginning of statement token
    // ASTNode* statement_node;

    // // single-line while-loop
    // if (tokens[token_id].id == Lexer::ID_open_parenthesis) {
    //   token_id++;
    //   statement_node = parseSingleLineLoop();
    // }
    // else {
    //   // parse single line or statement block?
    //   if (tokens[token_id].id != Lexer::ID_open_brace) {
    //     statement_node = parseSingleLine();
    //   }
    //   else {
    //     statement_node = parseBlock();
    //   }
    // }

    // while_node->SetRight(statement_node);

    return while_node;
  }

public:
  // Constructor: initialize the parser with tokens from an input file
  Parser(std::ifstream& in_file) {
    Lexer lexer;
    tokens = lexer.Tokenize(in_file);
  }
  void print_tokens()
  {
    for (auto token : tokens)
      std::cout << token.lexeme << " ";
  }
  // Main parsing function that builds and executes the AST
  void Parse() {
    std::vector<ASTNode*> nodes;
    while (token_id < tokens.size()) {
      switch (tokens[token_id].id) {
        case Lexer::ID_var:
          nodes.push_back(parseAssignment());
          break;
        case Lexer::ID_identifier:
          nodes.push_back(parseIdentifier());
          break;
        case Lexer::ID_print:
          nodes.push_back(parsePrint());
          break;
        case Lexer::ID_open_brace:
          nodes.push_back(parseBlock());
          break;
        case Lexer::ID_if:
          nodes.push_back(parseIf());
          break;
        case Lexer::ID_while:
          nodes.push_back(parseWhile());
          break;
        default:
          Utils::error("[Parse loop] Unexpected token", tokens[token_id]);
          break;
      }
    }

    // Execute parsed nodes
    for (auto node : nodes) {
      node->Run(table);
    }
  }

  // Parses an identifier assignment statement (e.g., x = expr;)
  ASTNode* parseIdentifier(bool singleLineStatement = false) {
    std::string identifier = tokens[token_id].lexeme;
    int unique_id = table.GetUniqueId(identifier);

    if (tokens[++token_id] != Lexer::ID_assignment) {
      Utils::error("Expected = after identifier", tokens[token_id]);
    }
    ++token_id;

    ASTNode* assignmentNode = new ASTNode(ASSIGNMENT);
    ASTNode* variableNode = new ASTNode(VARIABLE, unique_id);
    ASTNode* expressionNode = parseExpression();

    assignmentNode->SetLeft(variableNode);
    assignmentNode->SetRight(expressionNode);

    if (tokens[token_id] != Lexer::ID_semicolon && !singleLineStatement) {
      Utils::error("Expected semicolon at end of expression", tokens[token_id]);
    }
    ++token_id;
    return assignmentNode;
  }

  // Parses a print statement (e.g., print(expr);)
  ASTNode* parsePrint() {
    ++token_id;
    if (tokens[token_id] != Lexer::ID_open_parenthesis) {
      Utils::error("Expected ( after print keyword", tokens[token_id]);
    }
    ++token_id;


    ASTNode* expression;
    if (tokens[token_id].id == Lexer::ID_string) {
      std::string str = tokens[token_id++].lexeme;
      str = str.substr(1, str.length() - 2);
      auto entries = getVariableEntriesInString(str);
      expression = new ASTNode(STRING, str, entries);
    }
    else 
      expression = parseLogical();

    if (tokens[token_id] != Lexer::ID_close_parenthesis) {
      Utils::error("Expected closing parenthesis at the end of print expression", tokens[token_id]);
    }
    ++token_id;

    if (tokens[token_id] != Lexer::ID_semicolon) {
      Utils::error("Expected semicolon at end of print statement", tokens[token_id]);
    }
    ++token_id;

    ASTNode* printNode = new ASTNode(PRINT);
    printNode->SetLeft(expression);
    return printNode;
  }

  std::vector<std::pair<int, int>> getVariableEntriesInString(std::string& str)
  {
    size_t i = 0, index = 0;
    std::vector<std::pair<int, int>> entries{};
    while (i < str.length()) {
      if (str[i] == '{') {
        size_t j = i + 1;
        while (j < str.length() && str[j] != '}') ++j;
        if (j < str.length() && str[j] == '}') {
          std::string var_name = str.substr(i + 1, j - i - 1); // extract variable name inside {}
          int unique_id = table.GetUniqueId(var_name); // get unique id
          entries.push_back(std::make_pair(i, unique_id)); // append index of a variable in a string with its unique id
          str.erase(i, j - i + 1); // erase {} from a string

        }
        else if (i >= str.length())
          Utils::error("Expected } inside the string");
      }
      i++;
    }
    return entries;
  }
};
