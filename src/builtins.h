#pragma once
#include "std.h"
#include "utilities.h"

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
  { "pow", 2, false, std_pow},
  { "floor", 1, false, std_floor},
  { "ceil", 1, false, std_ceil},
  { "raiz", 1, false, std_sqrt},
  { "redondear", 1, false, std_round},
  { "presicion", 1, false, std_round},
  { "max", 2, false, std_max},
  { "min", 2, false, std_min},
};

constexpr static inline auto is_builtin(std::string_view name) -> bool {
  for (auto i{0uz}; i < countof(FREE_FUNCTIONS); i++) {
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

