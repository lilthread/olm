#pragma once
#include "builtins.h"
#include "nodes.h"
#include <memory>
#include <string>
#include <unordered_map>
#include "runtime_values.h"

class Environment {
public:
  Environment() { push(); }

  auto push() -> void;
  auto pop()  -> void;

  auto define(const std::string& name, ValuePtr val)  -> void;
  auto get(const std::string& name) const             -> ValuePtr;
  auto set(const std::string& name, ValuePtr val)     -> void;
  auto has(const std::string& name) const             -> bool;

private:
  std::vector<std::unordered_map<std::string, ValuePtr>> _scopes;
};

struct ReturnSignal {
  ValuePtr value;
};

class Interpreter final {
public:
  auto run(const StmtsPtr& program)     -> void;

private:
  Environment _env{};

  std::unordered_map<std::string, std::shared_ptr<ClassDef>> _classes;
  //std::unordered_map<std::string, BuiltinFnImpl>_builtins;
  std::unordered_map<std::string, const FunctionDecl*> _functions;
  auto call_builtin(const std::string& name, std::span<ValuePtr> args) -> ValuePtr;

  auto exec_stmts(const StmtsPtr& stmts) ->   void;
  auto exec_stmt(const IAST* node) ->         void;
  auto exec_var_decl(const VariableDecl*)  -> void;
  auto exec_func_decl(const FunctionDecl*) -> void;
  auto exec_class_decl(const ClassDecl*) ->   void;
  auto exec_assignment(const Assignment*) ->  void;
  auto exec_if(const IfStatement*) ->         void;
  auto exec_while(const WhileStatement*) ->   void;
  auto exec_return(const ReturnStatement*) -> void;
  auto eval(const IAST* node) ->          ValuePtr;
  auto eval_literal(const Literal*) ->    ValuePtr;
  auto eval_binary(const BinaryOp*) ->    ValuePtr;
  auto eval_unary(const UnaryOp*) ->      ValuePtr;
  auto eval_index_expr(const IndexExpr* node) -> ValuePtr;

  auto eval_method_call(const MethodCall* node) -> ValuePtr;
  auto eval_call(const FunctionCall*) ->  ValuePtr;
  auto eval_array(const ArrayDecl*) ->    ValuePtr;

  auto call_function(const FunctionDecl* fn, std::span<ValuePtr> args, InstancePtr self) -> ValuePtr;
  auto instantiate(const std::string& class_name, std::span<ValuePtr> args = {}) -> ValuePtr;

  auto resolve_member(const std::string& dotted) -> std::pair<InstancePtr, std::string>;


  auto dispatch_native_method(std::span<const NativeMethodDesc> methods, ValuePtr self, const MethodCall* node) -> ValuePtr;
  static auto to_number(const ValuePtr& v, std::string_view op = "") -> std::pair<bool, double>;
};
