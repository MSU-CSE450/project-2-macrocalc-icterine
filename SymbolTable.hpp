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
  // CODE TO STORE SCOPES AND VARIABLES HERE.
  
  // HINT: YOU CAN CONVERT EACH VARIABLE NAME TO A UNIQUE ID TO CLEANLY DEAL
  //       WITH SHADOWING AND LOOKING UP VARIABLES LATER.
  std::vector<std::unordered_map<std::string, int>> scopes;
  std::vector<VarData> variables;
  int scopes_count = 0;
  int unique_id_increment = 0;

  std::unordered_map<std::string,int> GetCurrentScope()
  {
    return scopes[scopes_count-1];
  }
public:
  // CONSTRUCTOR, ETC HERE
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
  // FUNCTIONS TO MANAGE SCOPES (NEED BODIES FOR THESE IF YOU WANT TO USE THEM)
  void PushScope() 
  { 
    std::unordered_map<std::string, int> scope;
    scopes.push_back(scope);

    scopes_count++;
  }
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

  // FUNCTIONS TO MANAGE VARIABLES - LOTS MORE NEEDED
  // (NEED REAL FUNCTION BODIES FOR THESE IF YOU WANT TO USE THEM)
  bool HasVar(std::string name) const 
  { 
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it)
    {
      auto found = it->find(name);
      if (found != it->end())
        return true;
    }

    return false;
  }
  
  double GetValue(int unique_id) const {
    for (auto varData : variables)
    {
      if (unique_id == varData.unique_id)
        return varData.value;
    }

    // this block should be unreachable
    return -99999;
  }

  int GetUniqueId(std::string& name)
  {
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it)
    {
      auto found = it->find(name);
      if (found != it->end())
        return found->second;
    }
    Utils::error("Could not find a variable " + name);
    return -1;
  }
  int InitializeVar(std::string name)
  { 
    //std::cout << "Inserting " << name << " = " << value << " in scope " << scopes_count-1 << std::endl;
    scopes[scopes_count-1].insert({name, unique_id_increment});
    variables.push_back(VarData(unique_id_increment, 0));

    return unique_id_increment++;
  }

  size_t UpdateVar(int unique_id, double value)
  { 
    for (auto &var : variables)
    {
      if (unique_id == var.unique_id)
        var.value = value;
    }
    return 0;
  }

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
