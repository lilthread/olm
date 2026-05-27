#pragma once
#include "runtime_values.h"
#include <string_view>
// forward declaration


using NativeMethod = ValuePtr(*)(ValuePtr self, std::span<const ValuePtr> args);

struct NativeMethodDesc final {
  std::string_view name;
  std::size_t arity;
  bool variadic;
  NativeMethod fn;
};

using NativeFuncDesc = NativeMethod;

auto find_builtin(std::span<const NativeMethodDesc> methods, std::string_view name) -> const NativeMethodDesc*;


auto escribe( ValuePtr self, std::span<const ValuePtr> args) -> ValuePtr;
auto leer(ValuePtr self, std::span<const ValuePtr> args) -> ValuePtr;
auto aleatorio(ValuePtr self, std::span<const ValuePtr> args) -> ValuePtr;
auto entero(ValuePtr self, std::span<const ValuePtr> args) -> ValuePtr;
auto decimal(ValuePtr self, std::span<const ValuePtr> args) -> ValuePtr;
auto cadena(ValuePtr self, std::span<const ValuePtr> args) -> ValuePtr;
auto std_to_bool(ValuePtr self, std::span<const ValuePtr> args) -> ValuePtr;
auto longitud(ValuePtr self, std::span<const ValuePtr> args) -> ValuePtr;
auto std_abs(ValuePtr self, std::span<const ValuePtr> args) -> ValuePtr;
auto std_max (ValuePtr self, std::span<const ValuePtr> args) -> ValuePtr;
auto std_min(ValuePtr self, std::span<const ValuePtr> args) -> ValuePtr;
auto std_pow(ValuePtr self, std::span<const ValuePtr> args) -> ValuePtr;
auto std_sqrt(ValuePtr self, std::span<const ValuePtr> args) -> ValuePtr;
auto std_floor(ValuePtr self, std::span<const ValuePtr> args) -> ValuePtr;
auto std_ceil(ValuePtr self, std::span<const ValuePtr> args) -> ValuePtr;
auto std_round(ValuePtr self, std::span<const ValuePtr> args) -> ValuePtr;
auto std_exit(ValuePtr self, std::span<const ValuePtr> args) -> ValuePtr;
auto std_is_int(ValuePtr self, std::span<const ValuePtr> args) -> ValuePtr;
auto std_is_float(ValuePtr self, std::span<const ValuePtr> args) -> ValuePtr;
auto std_is_str(ValuePtr self, std::span<const ValuePtr> args) -> ValuePtr;
auto std_is_bool(ValuePtr self, std::span<const ValuePtr> args) -> ValuePtr;

// ARRAY
auto array_insertar(ValuePtr self, std::span<const ValuePtr> args) -> ValuePtr;
auto array_eliminar(ValuePtr self, std::span<const ValuePtr> args) -> ValuePtr;
auto array_contiene(ValuePtr self, std::span<const ValuePtr> args) -> ValuePtr;
auto array_insertar_en(ValuePtr self, std::span<const ValuePtr> args) -> ValuePtr;
auto array_encuentra_index(ValuePtr self, std::span<const ValuePtr> args) -> ValuePtr;

// STRING

auto string_separar(ValuePtr self, std::span<const ValuePtr> args) -> ValuePtr;
auto std_lower(ValuePtr self, std::span<const ValuePtr> args) -> ValuePtr;
auto std_upper(ValuePtr self, std::span<const ValuePtr> args) -> ValuePtr;
auto str_encuentra_index(ValuePtr self, std::span<const ValuePtr> args) -> ValuePtr;
auto std_is_digit(ValuePtr self, std::span<const ValuePtr> args) -> ValuePtr;
