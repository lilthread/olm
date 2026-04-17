#pragma once
#include "nodes.h"
#include <cstddef>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <optional>
#include <stdexcept>

enum class SymbolKind { VARIABLE, CONSTANT, FUNCTION, CLASS, PARAMETER };

struct Symbol {
  std::string   name;
  SymbolKind    kind;
  std::size_t   arity{0};       // for FUNCTION
  bool          defined{true};
};

class Scope {
public:
  auto define(Symbol sym)                    -> void;
  auto lookup(std::string_view name) const   -> std::optional<Symbol>;
  auto contains(std::string_view name) const -> bool;
private:
  std::unordered_map<std::string, Symbol> _syms;
};

using SemanticError = std::string;

// Analyzer
class Sema final {
public:
  // Runs analysis; throws SemanticException (see below) if errors found.
  auto analyze(const StmtsPtr& program) -> void;

private:
  std::vector<Scope>         _scopes;
  std::vector<SemanticError> _errors;

  std::size_t _loop_depth{0};
  std::size_t _func_depth{0};
  bool _in_class{};

  auto push_scope() -> void;
  auto pop_scope() -> void;
  auto define(Symbol sym) -> void;
  auto resolve(std::string_view name) const -> std::optional<Symbol>;

  // Error helpers
  auto error(std::string msg) -> void;
  auto flush_errors() -> void;

  auto check_stmts(const StmtsPtr& stmts)   -> void;
  auto check_stmt(const IAST* node)         -> void;
  auto check_expr(const IAST* node)         -> void;
  auto check_var_decl(const VariableDecl*)  -> void;
  auto check_func_decl(const FunctionDecl*) -> void;
  auto check_class_decl(const ClassDecl*)   -> void;
  auto check_assignment(const Assignment*)  -> void;
  auto check_if(const IfStatement*)         -> void;
  auto check_while(const WhileStatement*)   -> void;
  auto check_return(const ReturnStatement*) -> void;
  auto check_func_call(const FunctionCall*) -> void;
  auto check_binary(const BinaryOp*)        -> void;
  auto check_unary(const UnaryOp*)          -> void;
  auto check_array(const ArrayDecl*)        -> void;
  auto check_literal(const Literal*)        -> void;
};


//  Exception that carries all accumulated errors
class SemanticException : public std::runtime_error {
public:
  std::vector<SemanticError> errors;
  explicit SemanticException(std::vector<SemanticError> errs)
      : std::runtime_error(build_msg(errs)), errors(std::move(errs)) {}
private:
  static auto build_msg(const std::vector<SemanticError>& errs) -> std::string {
    std::string out = "SemanticError(s):\n";
    for (auto& e : errs) out += "  • " + e + "\n";
    return out;
  }
};
