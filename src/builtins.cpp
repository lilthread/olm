#include "builtins.h"
#include "error_manager.h"
#include "runtime_values.h"
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <memory>
#include <print>
#include <random>
#include <string>
#include <vector>
static auto to_double(const ValuePtr& v) -> double {
  if (v->is_float())
    return v->as_float();

  if (v->is_int())
    return static_cast<double>(v->as_int());

  throw RuntimeError("valor no numerico");
}

auto Builtins::is_builtin(const std::string& name) noexcept -> const BuiltinDesc* {
  for (auto& b : INFO)
    if (name == b.name) return &b;
  return nullptr;
}

auto Builtins::load_builtins(std::unordered_map<std::string, BuiltinFnImpl>& f) -> void {
  // Args.size() are check that's why I use [] instead of .at()
  f["escribe"] = [](std::vector<ValuePtr> args) -> ValuePtr {
    for (int64_t i{0uz}; i < args.size(); i++) {
      std::print("{}", args[i]->to_string());
      if (i + 1 < args.size())
        std::print(" ");
    }
    std::println("");
    return make_null();
  };
  f["leer"] = [](std::vector<ValuePtr> args) -> ValuePtr {
    std::string str;
    std::getline(std::cin, str);
    return std::make_shared<Value>(str);
  };
  f["aleatorio"] = [](std::vector<ValuePtr> args) -> ValuePtr {
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
  };
  f["entero"] = [](std::vector<ValuePtr> args) -> ValuePtr {
    auto str = args[0]->as_string();
    return make(static_cast<int64_t>(std::stoi(str)));
  };
  f["decimal"] = [](std::vector<ValuePtr> args) -> ValuePtr {
    auto str = args[0]->as_string();
    return make(std::stod(str));
  };

  f["cadena"] = [](std::vector<ValuePtr> args) -> ValuePtr {
    auto v = args[0];
    if (v->is_int()){
      return make(std::to_string(v->as_int()));
    } else if (v->is_float()){
      std::stringstream ss;
      ss << std::fixed << std::setprecision(2) << v->as_float();
      return make(ss.str());
    }
    return make_null();
  };
  f["bool"] = [](std::vector<ValuePtr> args) -> ValuePtr {
    return {};
  };
  f["longitud"] = [](std::vector<ValuePtr> args) -> ValuePtr {
    if (!args[0]->is_array())
      throw RuntimeError(std::format("'{}' no soporta longitud", args[0]->to_string()));
    return make(static_cast<int64_t>(args[0]->as_array().size()));
  };

  // NAMES IN ENGLISH LOL

  f["abs"] = [](std::vector<ValuePtr> args) -> ValuePtr {
    auto v = args[0];

    if (v->is_float())
      return make(std::abs(v->as_float()));
    else if(v->is_int())
      return make(static_cast<int64_t>(std::llabs(v->as_int())));
    return make_null();
  };
  f["max"] = [](std::vector<ValuePtr> args) -> ValuePtr {
    auto a = args.at(0);
    auto b = args.at(1);

    if (a->is_float() || b->is_float()) {
      double x = a->as_float();
      double y = b->as_float();
      return std::make_shared<Value>(std::max(x, y));
    }

    return std::make_shared<Value>(std::max(a->as_int(), b->as_int()));
  };
  f["min"] = [](std::vector<ValuePtr> args) -> ValuePtr {
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
  };
  f["pow"] = [](std::vector<ValuePtr> args) -> ValuePtr {
    double base = args.at(0)->as_float();
    double exp = args.at(1)->as_float();

    return make(std::pow(base, exp));
  };
  f["sqrt"] = [](std::vector<ValuePtr> args) -> ValuePtr {
    if (!args[0]->is_float())
      return make_null();
    double x = args[0]->as_float();
    return make(std::sqrt(x));
  };
  f["floor"] = [](std::vector<ValuePtr> args) -> ValuePtr {
    if (!args[0]->is_float())
      return make_null();
    double x = args.at(0)->as_float();
    return make(std::floor(x));
  };
  f["ceil"] = [](std::vector<ValuePtr> args) -> ValuePtr {
    if (!args[0]->is_float())
      return make_null();

    double x = args[0]->as_float();
    return make(std::ceil(x));
  };
  f["round"] = [](std::vector<ValuePtr> args) -> ValuePtr {
    if (!args[0]->is_float())
      return make_null();
    double x = args[0]->as_float();
    return make(std::round(x));
  };
  f["salir"] = [](std::vector<ValuePtr> args) -> ValuePtr {
    exit(0);
  };
}
