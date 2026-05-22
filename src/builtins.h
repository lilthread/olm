#pragma once
#include "std.h"

static constexpr NativeMethodDesc FREE_FUNCTIONS[] {
  { "escribe", 1, true, escribe},
  { "leer", 0, false, leer},
  { "aleatorio", 2, false, aleatorio},
  { "entero", 1, false, entero},
  { "decimal", 1, false, decimal},
  { "cadena", 1, false, cadena},
  { "salir", 0, false, std_exit},
  { "longitud", 1, false, longitud},
  // MATH
  { "abs", 1, false, std_abs},
  { "max", 2, false, std_max},
  { "min", 2, false, std_min},
  { "pow", 2, false, std_pow},
  { "sqrt", 1, false, std_sqrt},
  { "floor", 1, false, std_floor},
  { "ceil", 1, false, std_ceil},
  { "round", 1, false, std_round},
};

static inline auto is_builtin(std::string_view name) -> bool {
  for (auto i{0}; i < 10; i++) {
    if (name == FREE_FUNCTIONS[i].name)
      return true;
  }
  return false;
}

static constexpr NativeMethodDesc ARRAY_METHODS[] {
  { "insertar", 1, false, array_insertar },
  { "insertar_en", 2, false, array_insertar_en },
  { "eliminar", 1, false, array_eliminar },
  { "contiene", 1, false, array_contiene },
  { "encuentra_index", 1, false, array_encuentra_index }
};

static constexpr NativeMethodDesc STRING_METHODS[] {
  { "separar", 1, false, string_separar }
};

