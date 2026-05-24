#pragma once
#include <memory>
#include <variant>
#include <vector>
#include <variant>
#include <flat_map>
#include "nodes.h"

struct Value;
struct ClassDef;
struct Instance;
using ValuePtr    = std::shared_ptr<Value>;
using InstancePtr = std::shared_ptr<Instance>;


static inline auto make(int64_t v)     -> ValuePtr { return std::make_shared<Value>(v); }
static inline auto make(double v)      -> ValuePtr { return std::make_shared<Value>(v); }
static inline auto make(bool v)        -> ValuePtr { return std::make_shared<Value>(v); }
static inline auto make(std::string v) -> ValuePtr { return std::make_shared<Value>(std::move(v)); }
static inline auto make(std::vector<ValuePtr> v) -> ValuePtr { return std::make_shared<Value>(std::move(v)); }
static inline auto make_null()         -> ValuePtr { return std::make_shared<Value>(); }

struct Value final {
  using Inner = std::variant<
    std::monostate,    // null
    int64_t, // std::numeric_limits<int64_t>::max() is the max
    double,
    bool,
    std::string,
    std::vector<ValuePtr>,
    InstancePtr
  >;

  Inner inner{std::monostate{}};

  Value() = default;
  explicit Value(int64_t v)               : inner(v) {}
  explicit Value(double v)                : inner(v) {}
  explicit Value(bool v)                  : inner(v) {}
  explicit Value(std::string v)           : inner(std::move(v)) {}
  explicit Value(std::vector<ValuePtr> v) : inner(std::move(v)) {}
  explicit Value(InstancePtr v)           : inner(std::move(v)) {}

  bool is_null()     const { return std::holds_alternative<std::monostate>(inner); }
  bool is_int()      const { return std::holds_alternative<int64_t>(inner); }
  bool is_float()    const { return std::holds_alternative<double>(inner); }
  bool is_bool()     const { return std::holds_alternative<bool>(inner); }
  bool is_string()   const { return std::holds_alternative<std::string>(inner); }
  bool is_array()    const { return std::holds_alternative<std::vector<ValuePtr>>(inner); }
  bool is_instance() const { return std::holds_alternative<InstancePtr>(inner); }

  // Accessors (unchecked)
  int64_t&              as_int()      { return std::get<int64_t>(inner); }
  double&               as_float()    { return std::get<double>(inner); }
  bool&                 as_bool()     { return std::get<bool>(inner); }
  std::string&          as_string()   { return std::get<std::string>(inner); }
  std::vector<ValuePtr>& as_array()   { return std::get<std::vector<ValuePtr>>(inner); }
  InstancePtr&          as_instance() { return std::get<InstancePtr>(inner); }

  const int64_t&               as_int()      const { return std::get<int64_t>(inner); }
  const double&                as_float()    const { return std::get<double>(inner); }
  const bool&                  as_bool()     const { return std::get<bool>(inner); }
  const std::string&           as_string()   const { return std::get<std::string>(inner); }
  const std::vector<ValuePtr>& as_array()   const { return std::get<std::vector<ValuePtr>>(inner); }
  const InstancePtr&           as_instance() const { return std::get<InstancePtr>(inner); }

  // Truthiness — everything is truthy except false and null
  bool truthy() const;
  auto to_string() const -> std::string;
};

struct ClassDef final {
  std::string name{};
  std::flat_map<std::string, const IAST*> fields;
  std::flat_map<std::string, const FunctionDecl*> methods;
};

struct Instance final {
  std::shared_ptr<ClassDef> klass;
  std::flat_map<std::string, ValuePtr> fields;
};


struct ReturnSignal final { ValuePtr value; };

struct ContinueSignal final {};
