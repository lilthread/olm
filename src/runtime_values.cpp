#include "runtime_values.h"
#include <iomanip>
#include <ios>
#include <iostream>
#include <string>

auto Value::truthy() const -> bool {
  return std::visit([](const auto& v) -> bool {
    using T = std::decay_t<decltype(v)>;
    if constexpr (std::is_same_v<T, std::monostate>)           return false;
    if constexpr (std::is_same_v<T, bool>)                     return v;
    if constexpr (std::is_same_v<T, int64_t>)                  return v != 0z;
    if constexpr (std::is_same_v<T, double>)                   return v != 0.0;
    if constexpr (std::is_same_v<T, std::string>)              return !v.empty();
    if constexpr (std::is_same_v<T, std::vector<ValuePtr>>)    return !v.empty();
    if constexpr (std::is_same_v<T, InstancePtr>)              return v != nullptr;
    return false;
  }, inner);
}

auto Value::to_string() const -> std::string {
  return std::visit([](const auto& v) -> std::string {
    using T = std::decay_t<decltype(v)>;
    if constexpr (std::is_same_v<T, std::monostate>)         return "nulo";
    else if constexpr (std::is_same_v<T, bool>)                   return v ? "verdadero" : "falso";
    else if constexpr (std::is_same_v<T, int64_t>)                return std::to_string(v);
    else if constexpr (std::is_same_v<T, double>){
      std::string str = std::to_string (v);
      str.erase (str.find_last_not_of('0') + 1, std::string::npos);
      str.erase (str.find_last_not_of('.') + 1, std::string::npos);
      return str;
    }
    else if constexpr (std::is_same_v<T, std::string>)            return v;
    else if constexpr (std::is_same_v<T, std::vector<ValuePtr>>) {
      std::string s = "[";
      for (std::size_t i = 0; i < v.size(); ++i) {
        s += v[i]->to_string();
        if (i + 1 < v.size()) s += ", ";
      }
      return s + "]";
    }
    else if constexpr (std::is_same_v<T, InstancePtr>)
      return "<instancia de " + v->klass->name + ">";
    return "?";
  }, inner);
}


