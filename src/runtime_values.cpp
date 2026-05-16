#include "runtime_values.h"

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
    if constexpr (std::is_same_v<T, bool>)                   return v ? "verdadero" : "falso";
    if constexpr (std::is_same_v<T, int64_t>)                return std::to_string(v);
    if constexpr (std::is_same_v<T, double>)                 return std::to_string(v);
    if constexpr (std::is_same_v<T, std::string>)            return v;
    if constexpr (std::is_same_v<T, std::vector<ValuePtr>>) {
      std::string s = "[";
      for (std::size_t i = 0; i < v.size(); ++i) {
        s += v[i]->to_string();
        if (i + 1 < v.size()) s += ", ";
      }
      return s + "]";
    }
    if constexpr (std::is_same_v<T, InstancePtr>)
      return "<instancia de " + v->klass->name + ">";
    return "?";
  }, inner);
}


