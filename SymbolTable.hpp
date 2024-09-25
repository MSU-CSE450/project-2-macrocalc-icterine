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
  size_t AddVar(std::string name, size_t line_num) // not implemented
  { 
    return 0;
  }
  double GetValue(std::string name) const {
    assert(HasVar(name));
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it)
    {
      auto found = it->find(name);
      if (found != it->end())
        return found->second;
    }
  }
  void SetValue(std::string name, double value) // not implemented
  { 
    assert(HasVar);
  }
};
