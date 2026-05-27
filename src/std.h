#pragma once
#include "runtime_values.h"
#include <string_view>

using NativeMethod = ValuePtr(*)(ValuePtr self, std::span<const ValuePtr> args);

struct NativeMethodDesc final {
  std::string_view name;
  std::size_t arity;
  bool variadic;
  NativeMethod fn;
};

using NativeFuncDesc = NativeMethod;

auto find_builtin(std::span<const NativeMethodDesc> list, std::string_view name) -> const NativeMethodDesc*;

auto escribe(ValuePtr, std::span<const ValuePtr> args) -> ValuePtr;
auto leer(ValuePtr, std::span<const ValuePtr> args) -> ValuePtr;
auto aleatorio(ValuePtr, std::span<const ValuePtr> args) -> ValuePtr;
auto entero(ValuePtr, std::span<const ValuePtr> args) -> ValuePtr;
auto decimal(ValuePtr, std::span<const ValuePtr> args) -> ValuePtr;
auto cadena(ValuePtr, std::span<const ValuePtr> args) -> ValuePtr;
auto std_to_bool(ValuePtr, std::span<const ValuePtr> args) -> ValuePtr;
auto longitud(ValuePtr, std::span<const ValuePtr> args) -> ValuePtr;
auto std_abs(ValuePtr, std::span<const ValuePtr> args) -> ValuePtr;
auto std_max (ValuePtr, std::span<const ValuePtr> args) -> ValuePtr;
auto std_min(ValuePtr, std::span<const ValuePtr> args) -> ValuePtr;
auto std_pow(ValuePtr, std::span<const ValuePtr> args) -> ValuePtr;
auto std_sqrt(ValuePtr, std::span<const ValuePtr> args) -> ValuePtr;
auto std_floor(ValuePtr, std::span<const ValuePtr> args) -> ValuePtr;
auto std_ceil(ValuePtr, std::span<const ValuePtr> args) -> ValuePtr;
auto std_round(ValuePtr, std::span<const ValuePtr> args) -> ValuePtr;
auto std_exit(ValuePtr, std::span<const ValuePtr> args) -> ValuePtr;
auto std_is_int(ValuePtr, std::span<const ValuePtr> args) -> ValuePtr;
auto std_is_float(ValuePtr, std::span<const ValuePtr> args) -> ValuePtr;
auto std_is_str(ValuePtr, std::span<const ValuePtr> args) -> ValuePtr;
auto std_is_bool(ValuePtr, std::span<const ValuePtr> args) -> ValuePtr;

// ARRAY
auto array_insertar(ValuePtr self, std::span<const ValuePtr> args) -> ValuePtr;
auto array_eliminar(ValuePtr self, std::span<const ValuePtr> args) -> ValuePtr;
auto array_contiene(ValuePtr self, std::span<const ValuePtr> args) -> ValuePtr;
auto array_insertar_en(ValuePtr self, std::span<const ValuePtr> args) -> ValuePtr;
auto array_encuentra_index(ValuePtr self, std::span<const ValuePtr> args) -> ValuePtr;

// STRING
auto string_separar(ValuePtr self, std::span<const ValuePtr> args) -> ValuePtr;
auto std_lower(ValuePtr self, std::span<const ValuePtr>) -> ValuePtr;
auto std_upper(ValuePtr self, std::span<const ValuePtr>) -> ValuePtr;
auto str_encuentra_index(ValuePtr self, std::span<const ValuePtr> args) -> ValuePtr;
auto std_is_digit(ValuePtr self, std::span<const ValuePtr>) -> ValuePtr;
