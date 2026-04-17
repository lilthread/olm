#include "sema.h"
#include <format>
#include <cassert>

auto Scope::define(Symbol sym) -> void {
  _syms.insert_or_assign(sym.name, std::move(sym));
}

auto Scope::lookup(std::string_view name) const -> std::optional<Symbol> {
  if (auto it = _syms.find(std::string(name)); it != _syms.end())
    return it->second;
  return std::nullopt;
}

auto Scope::contains(std::string_view name) const -> bool {
  return _syms.contains(std::string(name));
}

auto Sema::push_scope() -> void { _scopes.emplace_back(); }
auto Sema::pop_scope()  -> void { assert(!_scopes.empty()); _scopes.pop_back(); }

auto Sema::define(Symbol sym) -> void {
  assert(!_scopes.empty());
  // Redeclaration in the SAME scope is always an error.
  if (_scopes.back().contains(sym.name))
    error(std::format("'{}' is already declared in this scope", sym.name));
  else
    _scopes.back().define(std::move(sym));
}

auto Sema::resolve(std::string_view name) const -> std::optional<Symbol> {
  for (auto it = _scopes.rbegin(); it != _scopes.rend(); ++it)
    if (auto s = it->lookup(name)) return s;
  return std::nullopt;
}

auto Sema::error(std::string msg) -> void {
  _errors.push_back({std::move(msg)});
}

auto Sema::flush_errors() -> void {
  if (!_errors.empty())
    throw SemanticException(std::move(_errors));
}

auto Sema::analyze(const StmtsPtr& program) -> void {
  _errors.clear();
  _scopes.clear();
  _loop_depth = 0;
  _func_depth = 0;
  _in_class   = false;

  push_scope(); // global scope
  check_stmts(program);
  pop_scope();

  flush_errors();
}

// ─────────────────────────────────────────────────────────────
//  Statement dispatch
// ─────────────────────────────────────────────────────────────

auto Sema::check_stmts(const StmtsPtr& stmts) -> void {
  for (auto& s : stmts) check_stmt(s.get());
}

auto Sema::check_stmt(const IAST* node) -> void {
  if (!node) return;
  switch (node->node_type) {
    case NodeType::VARIABLEDECL:   check_var_decl   (static_cast<const VariableDecl*>  (node)); break;
    case NodeType::FUNCTIONDECL:   check_func_decl  (static_cast<const FunctionDecl*>  (node)); break;
    case NodeType::CLASSDECL:      check_class_decl (static_cast<const ClassDecl*>     (node)); break;
    case NodeType::ASSIGNMENT:     check_assignment (static_cast<const Assignment*>    (node)); break;
    case NodeType::IFSTATEMENT:    check_if         (static_cast<const IfStatement*>   (node)); break;
    case NodeType::WHILESTATEMENT: check_while      (static_cast<const WhileStatement*>(node)); break;
    case NodeType::RETURNSTATEMENT:check_return     (static_cast<const ReturnStatement*>(node));break;
    case NodeType::FUNCTIONCALL:   check_func_call  (static_cast<const FunctionCall*>  (node)); break;
    default:                       check_expr(node); break;
  }
}

// ─────────────────────────────────────────────────────────────
//  Expression dispatch
// ─────────────────────────────────────────────────────────────

auto Sema::check_expr(const IAST* node) -> void {
  if (!node) return;
  switch (node->node_type) {
    case NodeType::LITERAL:      check_literal   (static_cast<const Literal*>     (node)); break;
    case NodeType::BINARYOP:     check_binary    (static_cast<const BinaryOp*>    (node)); break;
    case NodeType::UNARYOP:      check_unary     (static_cast<const UnaryOp*>     (node)); break;
    case NodeType::FUNCTIONCALL: check_func_call (static_cast<const FunctionCall*>(node)); break;
    case NodeType::ARRAYDECL:    check_array     (static_cast<const ArrayDecl*>   (node)); break;
    default: break;
  }
}

auto Sema::check_var_decl(const VariableDecl* node) -> void {
  check_expr(node->expr.get());
  define({
    .name    = node->id,
    .kind    = node->is_const ? SymbolKind::CONSTANT : SymbolKind::VARIABLE,
    .arity   = 0,
    .defined = true,
  });
}


auto Sema::check_func_decl(const FunctionDecl* node) -> void {
  bool already_preregistered = _in_class && _scopes.back().contains(node->id);
  if (!already_preregistered)
    define({
      .name  = node->id,
      .kind  = SymbolKind::FUNCTION,
      .arity = node->params.size(),
    });

  push_scope();
  ++_func_depth;

  for (auto& p : node->params)
    define({.name = p, .kind = SymbolKind::PARAMETER});

  check_stmts(node->body);

  --_func_depth;
  pop_scope();
}

// ─────────────────────────────────────────────────────────────
//  Class declaration:  clase id members fin
// ─────────────────────────────────────────────────────────────

auto Sema::check_class_decl(const ClassDecl* node) -> void {
  define({.name = node->id, .kind = SymbolKind::CLASS});

  push_scope();
  bool prev = _in_class;
  _in_class = true;

  // Pre-register member functions so they can call each other.
  for (auto& m : node->members) {
    if (m->node_type == NodeType::FUNCTIONDECL) {
      auto* fn = static_cast<const FunctionDecl*>(m.get());
      if (!_scopes.back().contains(fn->id))
        _scopes.back().define({.name = fn->id, .kind = SymbolKind::FUNCTION, .arity = fn->params.size()});
    }
  }

  check_stmts(node->members);

  _in_class = prev;
  pop_scope();
}

// ─────────────────────────────────────────────────────────────
//  Assignment:  id se expr
// ─────────────────────────────────────────────────────────────

auto Sema::check_assignment(const Assignment* node) -> void {
  // Dotted names (e.g. "obj.field") — only check the root identifier.
  auto dot = node->id.find('.');
  std::string root = (dot == std::string::npos) ? node->id : node->id.substr(0, dot);

  if (root == "este") {
    if (!_in_class)
      error("'este' used outside of a class or method");
    check_expr(node->expr.get());
    return;
  }

  auto sym = resolve(root);
  if (!sym)
    error(std::format("assignment to undeclared variable '{}'", root));
  else if (sym->kind == SymbolKind::CONSTANT)
    error(std::format("assignment to constant '{}'", root));
  else if (sym->kind == SymbolKind::FUNCTION)
    error(std::format("assignment to function '{}'", root));
  else if (sym->kind == SymbolKind::CLASS)
    error(std::format("assignment to class '{}'", root));

  check_expr(node->expr.get());
}

// ─────────────────────────────────────────────────────────────
//  If statement
// ─────────────────────────────────────────────────────────────

auto Sema::check_if(const IfStatement* node) -> void {
  check_expr(node->condition.get());

  push_scope();
  check_stmts(node->then_body);
  pop_scope();

  if (std::holds_alternative<StmtsPtr>(node->next)) {
    push_scope();
    check_stmts(std::get<StmtsPtr>(node->next));
    pop_scope();
  } else if (std::holds_alternative<std::unique_ptr<IfStatement>>(node->next)) {
    check_if(std::get<std::unique_ptr<IfStatement>>(node->next).get());
  }
}

// ─────────────────────────────────────────────────────────────
//  While statement
// ─────────────────────────────────────────────────────────────

auto Sema::check_while(const WhileStatement* node) -> void {
  check_expr(node->condition.get());

  push_scope();
  ++_loop_depth;
  check_stmts(node->body);
  --_loop_depth;
  pop_scope();
}

// ─────────────────────────────────────────────────────────────
//  Return statement
// ─────────────────────────────────────────────────────────────

auto Sema::check_return(const ReturnStatement* node) -> void {
  if (_func_depth == 0)
    error("'ret' used outside of a function");
  check_expr(node->expr.get());
}

// ─────────────────────────────────────────────────────────────
//  Function call
// ─────────────────────────────────────────────────────────────

auto Sema::check_func_call(const FunctionCall* node) -> void {
  // Built-in subscript operator injected by the parser.
  if (node->id == "__index__") {
    for (auto& a : node->exprs) check_expr(a.get());
    return;
  }

  // For dotted calls (obj.method) just verify the root object is declared.
  auto dot = node->id.find('.');
  if (dot != std::string::npos) {
    std::string root = node->id.substr(0, dot);
    if (!resolve(root))
      error(std::format("undeclared identifier '{}'", root));
    for (auto& a : node->exprs) check_expr(a.get());
    return;
  }

  auto sym = resolve(node->id);
  if (!sym) {
    error(std::format("call to undeclared function '{}'", node->id));
  } else if (sym->kind != SymbolKind::FUNCTION) {
    error(std::format("'{}' is not a function", node->id));
  } else {
    // Arity check.
    if (node->exprs.size() != sym->arity)
      error(std::format("'{}' expects {} argument(s) but got {}",
                        node->id, sym->arity, node->exprs.size()));
  }

  for (auto& a : node->exprs) check_expr(a.get());
}

// ─────────────────────────────────────────────────────────────
//  Binary operation
// ─────────────────────────────────────────────────────────────

auto Sema::check_binary(const BinaryOp* node) -> void {
  check_expr(node->left.get());
  check_expr(node->right.get());
}

auto Sema::check_unary(const UnaryOp* node) -> void {
  check_expr(node->operand.get());
}

auto Sema::check_array(const ArrayDecl* node) -> void {
  for (auto& el : node->data) check_expr(el.get());
}

auto Sema::check_literal(const Literal* node) -> void {
  using enum TokenType;
  switch (node->token.type) {
    case IDENTIFIER: {
      // Dotted member accesses are stored as "obj.field" identifiers.
      auto dot  = node->token.literal.find('.');
      auto root = (dot == std::string::npos)
                    ? node->token.literal
                    : node->token.literal.substr(0, dot);
 
      if (root == "este") {
        if (!_in_class)
          error("'este' used outside of a class or method");
        break;
      }

      if (!resolve(root))
        error(std::format("use of undeclared identifier '{}'", root));
      break;
    }
    case SELF:
      if (!_in_class)
        error("'este' used outside of a class or method");
      break;
    default: break;
  }
}
