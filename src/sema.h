#pragma once
#include "nodes.h"
#include <cstddef>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <optional>
#include "utilities.h"
#include "error_manager.h"

enum SymbolKind { VARIABLE, CONSTANT, FUNCTION, CLASS, PARAMETER };

struct Symbol {
  std::string   name{};
  SymbolKind    kind{};
  std::size_t   arity{0};       // meaningful for function(...)
  std::size_t   ctor_arity{0};  // arity of 'crear' constructor (CLASS only)
  bool          has_ctor{false}; // whether the class defines 'crear'
  bool          defined{true};
  SourceLocation location{};
};

class Scope {
public:
  auto define(Symbol sym)                          -> void;
  auto lookup(std::string_view name) const         -> std::optional<Symbol>;
  auto contains(std::string_view name) const       -> bool;
private:
  std::unordered_map<std::string, Symbol> _syms;
};

class Sema final {
public:
  auto analyze(const StmtsPtr& program) -> void;

private:
  std::vector<Scope>         _scopes{};
  ErrorManager _errors;

  std::size_t _loop_depth{};
  std::size_t _func_depth{};
  bool _in_class{};

  auto push_scope()                               -> void;
  auto pop_scope()                                -> void;
  auto define(Symbol sym)                         -> void;
  auto resolve(std::string_view name) const       -> std::optional<Symbol>;

  auto error(SemanticErrorCode code, SourceLocation loc, std::string subject  = {}, std::size_t expected = 0, std::size_t got = 0) -> void;

  auto check_stmts(const StmtsPtr&)         -> void;
  auto check_stmt(const IAST*) -> void;
  auto check_expr(const IAST*) -> void;
  auto check_var_decl(const VariableDecl*) -> void;
  auto check_func_decl(const FunctionDecl*) -> void;
  auto check_class_decl(const ClassDecl*) -> void;
  auto check_assignment(const Assignment*) -> void;
  auto check_if(const IfStatement*) -> void;
  auto check_while(const WhileStatement*) -> void;
  auto check_return(const ReturnStatement*) -> void;
  auto check_func_call(const FunctionCall*) -> void;
  auto check_binary(const BinaryOp*) -> void;
  auto check_unary(const UnaryOp*) -> void;
  auto check_array(const ArrayDecl*) -> void;
  auto check_literal(const Literal*) -> void;
  auto check_method_call(const MethodCall*) -> void;
};
