#include "std.h"
#include "error_manager.h"
#include <print>
#include <string>
#include <iostream>
#include <random>
#include <iomanip>

// Helper
static auto to_double(const ValuePtr& v) -> double {
  if (v->is_float())
    return v->as_float();

  if (v->is_int())
    return static_cast<double>(v->as_int());

  throw RuntimeError("valor no numerico");
}


auto escribe(ValuePtr self, std::span<const ValuePtr> args) -> ValuePtr {
  for (int64_t i{0uz}; i < args.size(); i++) {
    std::print("{}", args[i]->to_string());
    if (i + 1 < args.size())
      std::print(" ");
  }
  std::println("");
  return make_null();
}

auto leer(ValuePtr self, std::span<const ValuePtr> args) -> ValuePtr {
  std::string str;
  std::getline(std::cin, str);
  return std::make_shared<Value>(str);
}

auto aleatorio(ValuePtr self, std::span<const ValuePtr> args) -> ValuePtr {
    auto is_number = [](const ValuePtr& v) {
      return v->is_int() || v->is_float();
    };

    if (!is_number(args[0]) || !is_number(args[1]))
      throw RuntimeError("aleatorio solo acepta numeros");

    static std::mt19937 gen(std::random_device{}());

    if (args[0]->is_float() || args[1]->is_float()) {
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

auto entero(ValuePtr self, std::span<const ValuePtr> args) -> ValuePtr {
  auto str = args[0]->as_string();
  return make(static_cast<int64_t>(std::stoi(str)));
}

auto decimal(ValuePtr self, std::span<const ValuePtr> args) -> ValuePtr {
  auto str = args[0]->as_string();
  return make(std::stod(str));
}

auto cadena(ValuePtr self, std::span<const ValuePtr> args) -> ValuePtr {
    auto v = args[0];
    if (v->is_int()){
      return make(std::to_string(v->as_int()));
    } else if (v->is_float()){
      std::stringstream ss;
      ss << std::fixed << std::setprecision(2) << v->as_float();
      return make(ss.str());
    }
    return make_null();
}

auto longitud(ValuePtr self, std::span<const ValuePtr> args) -> ValuePtr {
  if (!args[0]->is_array())
    throw RuntimeError(std::format("'{}' no soporta longitud", args[0]->to_string()));
  return make(static_cast<int64_t>(args[0]->as_array().size()));
}

auto std_abs(ValuePtr self, std::span<const ValuePtr> args) -> ValuePtr {
  auto v = args[0];

  if (v->is_float())
    return make(std::abs(v->as_float()));
  else if(v->is_int())
    return make(static_cast<int64_t>(std::llabs(v->as_int())));
  return make_null();
}
auto std_max ( ValuePtr self, std::span<const ValuePtr> args) -> ValuePtr {
  auto a = args[0];
  auto b = args[1];

  if (a->is_float() || b->is_float()) {
    double x = a->as_float();
    double y = b->as_float();
    return std::make_shared<Value>(std::max(x, y));
  }
  return std::make_shared<Value>(std::max(a->as_int(), b->as_int()));
}

auto std_min(ValuePtr self, std::span<const ValuePtr> args) -> ValuePtr {
  if (!args[0] && !args[1])
    return make_null();
  auto a = args[0];
  auto b = args[1];

  if (a->is_float() || b->is_float()) {
    double x = a->as_float();
    double y = b->as_float();
    return make(std::min(x, y));
  }
  return make(std::min(a->as_int(), b->as_int()));
}

auto std_pow(ValuePtr self, std::span<const ValuePtr> args) -> ValuePtr {
  double base = args[0]->as_float();
  double exp = args[1]->as_float();

  return make(std::pow(base, exp));
}

auto std_sqrt(ValuePtr self, std::span<const ValuePtr> args) -> ValuePtr {
  if (!args[0]->is_float())
    return make_null();
  double x = args[0]->as_float();
  return make(std::sqrt(x));
}

auto std_floor(ValuePtr self, std::span<const ValuePtr> args) -> ValuePtr {
  if (!args[0]->is_float())
    return make_null();
  double x = args[0]->as_float();
  return make(std::floor(x));
}

auto std_ceil(ValuePtr self, std::span<const ValuePtr> args) -> ValuePtr {
  if (!args[0]->is_float())
    return make_null();

  double x = args[0]->as_float();
  return make(std::ceil(x));
}

auto std_round(ValuePtr self, std::span<const ValuePtr> args) -> ValuePtr {
  if (!args[0]->is_float())
    return make_null();
  double x = args[0]->as_float();
  return make(std::round(x));
}

auto std_exit(ValuePtr self, std::span<const ValuePtr> args) -> ValuePtr {
  exit(0);
}

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

auto find_builtin(std::span<const NativeMethodDesc> methods, std::string_view name) -> const NativeMethodDesc* {
  for (const auto& method : methods) {
    if (method.name == name)
      return &method;
  }
  return nullptr;
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
  return make(false);
}

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

