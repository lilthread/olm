#pragma once
#include <string>
#include <cstddef>
#include <unordered_map>
#include <vector>
#include <functional>
#include "runtime_values.h"

// ─────────────────────────────────────────────────────────────
//  Built-in function descriptor
//  Both Sema and Interpreter read from this table.
//  To add a new built-in:
//    1. Add an entry here.
//    2. Impl in builtins.cpp
// ─────────────────────────────────────────────────────────────

struct BuiltinDesc final {
  const char*  name{};
  std::size_t  arity{};    // exact argument count; if variadic ==
  bool         variadic{}; // true → any number of args accepted
};

using BuiltinFnImpl = std::function<ValuePtr(std::vector<ValuePtr>)>;

struct Builtins final {
  static constexpr BuiltinDesc INFO[] = {
    { "escribe",    1, true },
    { "leer",       0, false},
    { "aleatorio",  2, false},
    { "entero",    1, false},
    { "decimal",    1, false},
    { "cadena",     1, false},
    { "bool",     1, false},
    { "salir",     0, false},
    { "longitud",  1, false},
    // MATH
    { "abs", 1, false},
    { "max", 2, false},
    { "min", 2, false},
    { "pow", 2, false},
    { "sqrt", 1, false},
    { "floor", 1, false},
    { "ceil", 1, false},
    { "round", 1, false},
  };
  static auto is_builtin(const std::string& name) noexcept -> const BuiltinDesc*;
  static auto load_builtins(std::unordered_map<std::string, BuiltinFnImpl>&) -> void;
};
