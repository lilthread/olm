#include "std.h"
#include "error_manager.h"
#include "runtime_values.h"
#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <print>
#include <sstream>
#include <string>
#include <iostream>
#include <random>

// Helper
namespace {
auto to_double(const ValuePtr& v) -> double {
  if (v->is_float())
    return v->as_float();
  else if (v->is_int())
    return static_cast<double>(v->as_int());

  throw RuntimeError("valor no numerico");
}

auto to_int(const ValuePtr& v) -> int64_t {
  if (v->is_float())
    return static_cast<int64_t>(v->as_float());
  else if (v->is_int())
    return v->as_int();

  throw RuntimeError("valor no numerico");
}

auto is_number(const ValuePtr& v) {
  return v->is_int() || v->is_float();
};

template<class... V> requires ((std::same_as<V, ValuePtr>) && ...)
auto any_float(const V&... v) -> bool {
  return ((v->is_float()) || ... || false);
}

/*auto any_float(std::span<const ValuePtr> values) -> bool {
  return std::ranges::any_of(values, [](const ValuePtr& v) {
    return v->is_float();
  });
}*/
}

auto find_builtin(std::span<const NativeMethodDesc> methods, std::string_view name) -> const NativeMethodDesc* {
  for (const auto& method : methods) {
    if (method.name == name)
      return &method;
  }
  return nullptr;
}

auto std_is_int(ValuePtr, std::span<const ValuePtr> args) -> ValuePtr {
  return make(args[0]->is_int());
}
auto std_is_float(ValuePtr, std::span<const ValuePtr> args) -> ValuePtr {
  return make(args[0]->is_float());
}
auto std_is_str(ValuePtr, std::span<const ValuePtr> args) -> ValuePtr {
  return make(args[0]->is_string());
}
auto std_is_bool(ValuePtr, std::span<const ValuePtr> args) -> ValuePtr {
  return make(args[0]->is_bool());
}

auto escribe(ValuePtr, std::span<const ValuePtr> args) -> ValuePtr {
  for (auto i{0uz}; i < args.size(); i++) {
    std::print("{}", args[i]->to_string());
    if (i + 1 < args.size())
      std::print(" ");
  }
  std::println("");
  return make_null();
}

auto leer(ValuePtr, std::span<const ValuePtr>) -> ValuePtr {
  std::string str;
  std::getline(std::cin, str);
  return std::make_shared<Value>(str);
}

auto aleatorio(ValuePtr, std::span<const ValuePtr> args) -> ValuePtr {
  if (!is_number(args[0]) || !is_number(args[1]))
    throw RuntimeError("aleatorio solo acepta numeros");

  static std::mt19937 gen(std::random_device{}());

  if (any_float(args[0], args[1])) {
    double begin = to_double(args[0]);
    double end   = to_double(args[1]);

    std::uniform_real_distribution<double> dist(begin, end);

    return make(dist(gen));
  }
  int64_t begin = args[0]->as_int();
  int64_t end   = args[1]->as_int();

  std::uniform_int_distribution<int64_t> dist(begin, end);

  return make(dist(gen));
}

auto entero(ValuePtr, std::span<const ValuePtr> args) -> ValuePtr {
  auto x = args[0];
  if (x->is_array())
    throw RuntimeError("No se puede convertir un array a numero");
  else if (x->is_string()) {
    auto str = args[0]->as_string();
    return make(static_cast<int64_t>(std::stoi(str)));
  } 
  return make(to_int(x));
}

auto std_to_bool(ValuePtr, std::span<const ValuePtr> args) -> ValuePtr {
  auto x = args[0];
  if(x->is_int())
    return make(x->as_int() != 0);
  else if(x->is_null())
    return make(false);
  else if(x->is_string())
    return make(!x->to_string().empty());
  else if(x->is_float())
    return make(x->as_float() != 0);
  else if(x->is_array())
    return make(!x->as_array().empty());
  return make(false);
}

auto decimal(ValuePtr, std::span<const ValuePtr> args) -> ValuePtr {
  auto x = args[0];
  if (x->is_array())
    throw RuntimeError("No se puede convertir un array a numero");
  else if (x->is_string()) {
    auto str = args[0]->as_string();
    return make(std::stod(str));
  } 
  return make(to_double(x));
}

auto cadena(ValuePtr, std::span<const ValuePtr> args) -> ValuePtr {
  return make(args[0]->to_string());
}

auto longitud(ValuePtr, std::span<const ValuePtr> args) -> ValuePtr {
  if (!args[0]->is_array())
    throw RuntimeError(std::format("'{}' no soporta longitud", args[0]->to_string()));
  return make(static_cast<int64_t>(args[0]->as_array().size()));
}

auto std_abs(ValuePtr, std::span<const ValuePtr> args) -> ValuePtr {
  auto v = args[0];

  if (v->is_float())
    return make(std::abs(v->as_float()));
  else if(v->is_int())
    return make(static_cast<int64_t>(std::llabs(v->as_int())));
  throw RuntimeError(std::format("abs espera un numeros, valor: {}", v->to_string()));
}

auto std_max(ValuePtr, std::span<const ValuePtr> args) -> ValuePtr {
  auto a = args[0];
  auto b = args[1];

  if (any_float(a, b)) {
    double x = to_double(a);
    double y = to_double(b);
    return std::make_shared<Value>(std::max(x, y));
  }
  return std::make_shared<Value>(std::max(a->as_int(), b->as_int()));
}

auto std_min(ValuePtr, std::span<const ValuePtr> args) -> ValuePtr {
  auto a = args[0];
  auto b = args[1];

  if (any_float(a, b))
    return make(std::min(to_double(a), to_double(b)));
  return make(std::min(a->as_int(), b->as_int()));
}

auto std_pow(ValuePtr, std::span<const ValuePtr> args) -> ValuePtr {
  double base = to_double(args[0]);
  double exp = to_double(args[1]);
  return make(std::pow(base, exp));
}

auto std_sqrt(ValuePtr, std::span<const ValuePtr> args) -> ValuePtr {
  return make(std::sqrt(to_double(args[0])));
}

auto std_floor(ValuePtr, std::span<const ValuePtr> args) -> ValuePtr {
  return make(std::floor(to_double(args[0])));
}

auto std_ceil(ValuePtr, std::span<const ValuePtr> args) -> ValuePtr {
  return make(std::ceil(to_double(args[0])));
}

auto std_round(ValuePtr, std::span<const ValuePtr> args) -> ValuePtr {
  return make(std::round(to_double(args[0])));
}

auto std_exit(ValuePtr, std::span<const ValuePtr>) -> ValuePtr { exit(0); }

auto array_insertar(ValuePtr self, std::span<const ValuePtr> args) -> ValuePtr {
  auto& arr = self->as_array();
  arr.push_back(args[0]);
  return make_null();
}

auto array_eliminar(ValuePtr self, std::span<const ValuePtr> args) -> ValuePtr {
  auto& arr = self->as_array();

  auto idx = args[0];

  if (!idx->is_int())
    throw RuntimeError("se esperaba entero");

  auto i = idx->as_int();

  if (i < 0 || i >= static_cast<int64_t>(arr.size()))
    throw RuntimeError("Out of Bounce");

  arr.erase(arr.begin() + i);

  return make_null();
}

auto array_contiene(ValuePtr self, std::span<const ValuePtr> args) -> ValuePtr {
  auto& arr = self->as_array();
  for (const auto& el : arr) {
    if (el == args[0])
      return make(true);
  }
  return make(false);
}


auto array_insertar_en(ValuePtr self, std::span<const ValuePtr> args) -> ValuePtr {
  auto& arr = self->as_array();

  auto idx = args[0];
  auto val = args[1];

  if (!idx->is_int())
    throw RuntimeError(
      "insertar_en(): el indice debe ser un entero"
    );

  auto i = idx->as_int();

  if (i < 0 ||
      i > static_cast<int64_t>(arr.size()))
    throw RuntimeError(
      std::format(
        "insertar_en(): indice {} fuera de rango",
        i
      )
    );

  arr.insert(arr.begin() + i, val);

  return make_null();
}

auto array_encuentra_index(ValuePtr self, std::span<const ValuePtr> args) -> ValuePtr {
  auto& arr = self->as_array();

  auto target = args[0];

  for (auto i{0uz}; i < arr.size(); ++i) {

    if (arr[i]->to_string() == target->to_string())
      return make(static_cast<int64_t>(i));
  }
  return make_null();
}
// STRING

auto string_separar(ValuePtr self, std::span<const ValuePtr> args) -> ValuePtr {
  auto& str = self->as_string();
  auto delim = args[0]->to_string();

  std::stringstream ss(str);
  std::string item;
  std::vector<ValuePtr> out;

  while (std::getline(ss, item, delim[0]))
    out.push_back(make(item));

  return make(out);
}

auto std_lower(ValuePtr self, std::span<const ValuePtr>) -> ValuePtr {
  auto str = self->as_string();
  std::transform(str.begin(), str.end(), str.begin(), ::tolower);
  return make(str);
}

auto std_upper(ValuePtr self, std::span<const ValuePtr>) -> ValuePtr {
  auto str = self->as_string();
  std::transform(str.begin(), str.end(), str.begin(), ::toupper);
  return make(str);
}

auto str_encuentra_index(ValuePtr self, std::span<const ValuePtr> args) -> ValuePtr {
  auto str = self->as_string();

  std::size_t r = str.find(args[0]->to_string());

  if (std::string::npos == r)
    return make_null();
  return make(static_cast<int64_t>(r));
}

