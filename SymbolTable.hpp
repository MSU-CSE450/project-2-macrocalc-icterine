#pragma once

#include <assert.h>
#include <string>
#include <unordered_map>
#include <vector>
#include "Utils.hpp"

struct VarData {
  double value;
  int unique_id;

  VarData(int unique_id) : unique_id(unique_id), value(0) {}
  VarData(int unique_id, double value) : unique_id(unique_id), value(value) {}
};

class SymbolTable {
private:
  std::vector<std::unordered_map<std::string, int>> scopes;
  std::vector<VarData> variables;
  int unique_id_increment = 0;

  std::unordered_map<std::string, int>& GetCurrentScope() {
    return scopes.back();
  }

public:
  SymbolTable() {
    PushScope(); // Initialize with the global scope
  }

  bool HasVarInCurrentScope(const std::string& name) {
    return GetCurrentScope().find(name) != GetCurrentScope().end();
  }

  bool HasVar(const std::string& name) {
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
      if (it->find(name) != it->end())
        return true;
    }
    return false;
  }

  double GetValue(int unique_id) const {
    for (const auto& varData : variables) {
      if (varData.unique_id == unique_id)
        return varData.value;
    }
    return -99999; // Error case
  }

  int GetUniqueId(const std::string& name) {
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
      auto found = it->find(name);
      if (found != it->end())
        return found->second;
    }
    Utils::error("Variable not defined: " + name);
    return -1;
  }

  int InitializeVar(const std::string& name) {
    if (HasVarInCurrentScope(name)) {
      Utils::error("Variable already defined in this scope: " + name);
    }

    GetCurrentScope()[name] = unique_id_increment;
    variables.emplace_back(unique_id_increment, 0); // New variable with default value 0
    return unique_id_increment++;
  }

  void UpdateVar(int unique_id, double value) {
    for (auto& var : variables) {
      if (var.unique_id == unique_id) {
        var.value = value;
        return;
      }
    }
  }

  void PushScope() {
    scopes.emplace_back();
  }

  void PopScope() {
    if (scopes.size() > 1) {
      scopes.pop_back();
    } else {
      Utils::error("No scope to pop");
    }
  }
};
