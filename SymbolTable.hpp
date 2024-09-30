#pragma once

#include <assert.h>
#include <string>
#include <unordered_map>
#include <vector>
#include "Utils.hpp"
struct VarData {
  double value;
  int unique_id;

  VarData(int unique_id)
  {
    this->unique_id = unique_id;
  }
  VarData(int unique_id, double value) : VarData(unique_id)
  {
    this->value = value;
  }
};
class SymbolTable {
private:
  std::vector<std::unordered_map<std::string, int>> scopes; // a stack of scopes - unordered maps that have a variable name as a key and unique id as a value
  std::vector<VarData> variables; // array of ALL variables defined throughout the program. VarData contains a unique id of the variable and the value itself
  int scopes_count = 0;
  int unique_id_increment = 0; // increment for unique ids

  std::unordered_map<std::string,int> GetCurrentScope()
  {
    return scopes[scopes_count-1];
  }
public:
  
  SymbolTable()
  {
    PushScope(); // initialize global scope
  }

  bool HasVarInCurrentScope(std::string name)
  {
    // checks if variable exists only in the CURRENT scope
    auto current_scope = GetCurrentScope();
    if (current_scope.find(name) != current_scope.end())
      return true;
    return false;
  }
  
  // this function will be called when we enter a new scope, i.e. encounter "{"
  void PushScope() 
  { 
    std::unordered_map<std::string, int> scope;
    scopes.push_back(scope);

    scopes_count++;
  }

  // this function will be called when we leave a scope, i.e. encounter "}"
  void PopScope() 
  { 
    if (scopes_count > 1) // > 1 and not > 0 because we can't pop the global scope
    {
      scopes.pop_back();
      scopes_count--;
    }
    else
    {
      std::cout << "Error: no scope to pop\n";
      exit(1);
    }
  }

  // checks if a variable with the "name" was defined
  bool HasVar(std::string& name) const 
  { 
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it)
    {
      auto found = it->find(name);
      if (found != it->end())
        return true;
    }

    return false;
  }
  
  // get value of a variable by its ID
  double GetValue(int unique_id) const {
    for (auto varData : variables)
    {
      if (unique_id == varData.unique_id)
        return varData.value;
    }

    // this return should be unreachable, it's just here to prevent compiler warnings
    return -99999;
  }

  // get unique ID of a variable by its name.
  int GetUniqueId(std::string& name)
  {
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it)
    {
      auto found = it->find(name);
      if (found != it->end())
        return found->second;
    }
    Utils::error("Variable not defined: " + name);
    return -1;
  }

  // initializes a new variable and returns its unique id
  int InitializeVar(std::string name)
  { 
    scopes[scopes_count-1].insert({name, unique_id_increment});
    variables.push_back(VarData(unique_id_increment, 0));

    return unique_id_increment++;
  }

  // updates existing variable by its unique id
  size_t UpdateVar(int unique_id, double value)
  { 
    for (auto &var : variables)
    {
      if (unique_id == var.unique_id)
        var.value = value;
    }
    return 0;
  }

  // prints table (for debug)
  void PrintTable()
  {
    int i =0;
    std::cout << "DEBUG: printing SymbolTable:\n";
    for (VarData var : variables)
    {
      std::cout << "    id = " << var.unique_id << ", value = " << var.value << std::endl;
    }
  }
};
