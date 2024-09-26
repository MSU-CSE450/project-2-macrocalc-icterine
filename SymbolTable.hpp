#pragma once

#include <assert.h>
#include <string>
#include <unordered_map>
#include <vector>

class SymbolTable {
private:
  // CODE TO STORE SCOPES AND VARIABLES HERE.
  
  // HINT: YOU CAN CONVERT EACH VARIABLE NAME TO A UNIQUE ID TO CLEANLY DEAL
  //       WITH SHADOWING AND LOOKING UP VARIABLES LATER.
  std::vector<std::unordered_map<std::string, double>> scopes;
  int scopes_count = 0;
  bool HasVarCurrentScope(std::string name)
  {
    // checks if variable exists only in the CURRENT scope
    auto current_scope = GetCurrentScope();
    if (current_scope.find(name) != current_scope.end())
      return true;
    return false;
  }
  std::unordered_map<std::string,double> GetCurrentScope()
  {
    return scopes[scopes_count-1];
  }
public:
  // CONSTRUCTOR, ETC HERE
  SymbolTable()
  {
    PushScope(); // initialize global scope
  }
  // FUNCTIONS TO MANAGE SCOPES (NEED BODIES FOR THESE IF YOU WANT TO USE THEM)
  void PushScope() 
  { 
    std::unordered_map<std::string, double> scope;
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
  
  double GetValue(std::string name) const {
    if (!HasVar(name))
    {
      std::cerr << "Error: tried to access undefined variable " << name << std::endl;
      exit(1);
    }
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it)
    {
      auto found = it->find(name);
      if (found != it->end())
        return found->second;
    }

    // this block should be unreachable
    return -99999;
  }
  void InitializeVar(std::string name, double value = 0)
  { 
    if (HasVarCurrentScope(name))
    {
      // check if variable already defined in the current scope. Example:
      /*
      Valid initialization:
      {
        var x = 20;
        {
          var x = 20; // ok - can init variable with the same name in inner scopes
        }
      }

      Invalid initialization:
      {
        var x = 20;
        var x = 50; // cannot initialize same variable in the same scope
      }
      */
      std::cerr << "Error: tried to redefine variable " << name << std::endl;
      exit(1);
    }
    //std::cout << "Inserting " << name << " = " << value << " in scope " << scopes_count-1 << std::endl;
    scopes[scopes_count-1].insert({name, value});
  }

  size_t UpdateVar(std::string name, double value)
  { 
    if (!HasVar(name))
    {
      std::cerr << "Error: tried accessing undefined variable " << name << std::endl;
    }

    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it)
    {
      auto found = it->find(name);
      if (found != it->end())
        (*found).second = value;
    }
    return 0;
  }

  void PrintTable()
  {
    int i =0;
    std::cout << "DEBUG: printing SymbolTable by scopes:\n";
    for (auto scope : scopes)
    {
      std::cout << "Printing scope " << i << " \n";
      for (auto pair : scope)
      {
        std::cout << "  " << pair.first << " = " << pair.second << std::endl;
      }
      i++;
    }
  }
};
