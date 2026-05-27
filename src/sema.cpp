#include "sema.h"
#include <cstddef>
#include <cassert>
#include <string_view>
#include "builtins.h"
#include "nodes.h"
#include "std.h"
#include "utilities.h"


auto Scope::define(Symbol sym) -> void {
  _syms.insert_or_assign(sym.name, std::move(sym));
}

auto Scope::lookup(std::string_view name) const -> std::optional<Symbol> {
  if (auto it = _syms.find(name); it != _syms.end())
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
  if (_scopes.back().contains(sym.name)) {
    error(SemanticErrorCode::REDEFINITION, sym.location, sym.name.data());
  } else {
    _scopes.back().define(std::move(sym));
  }
}

auto Sema::resolve(std::string_view name) const -> std::optional<Symbol> {
  for (auto it = _scopes.rbegin(); it != _scopes.rend(); ++it)
    if (auto s = it->lookup(name)) return s;
  return std::nullopt;
}

auto Sema::error(SemanticErrorCode code, SourceLocation    loc, std::string       subject, std::size_t       expected, std::size_t       got) -> void {
  _errors.emit({code, loc, std::move(subject), expected, got});
}

auto Sema::analyze(const StmtsPtr& program) -> void {
  _errors.clear();
  _scopes.clear();
  _loop_depth = 0;
  _func_depth = 0;
  _in_class   = false;

  push_scope();

  for (const auto& b : FREE_FUNCTIONS) {
    _scopes.back().define({
      .name     = b.name.data(),
      .kind     = SymbolKind::FUNCTION,
      .arity    = b.arity,
      .defined  = true,
    });
  }
  check_stmts(program);
  pop_scope();

  _errors.flush();
}

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
    case NodeType::CONTINUESTMT:   check_continue   (static_cast<const ContinueStatement*>(node));break;
    case NodeType::FUNCTIONCALL:   check_func_call  (static_cast<const FunctionCall*>  (node)); break;
    case NodeType::METHODCALL:     check_method_call(static_cast<const MethodCall*>    (node)); break;
    default:                       check_expr(node); break;
  }
}

auto Sema::check_expr(const IAST* node) -> void {
  if (!node) return;
  switch (node->node_type) {
    case NodeType::LITERAL:      check_literal   (static_cast<const Literal*>     (node)); break;
    case NodeType::BINARYOP:     check_binary    (static_cast<const BinaryOp*>    (node)); break;
    case NodeType::UNARYOP:      check_unary     (static_cast<const UnaryOp*>     (node)); break;
    case NodeType::FUNCTIONCALL: check_func_call (static_cast<const FunctionCall*>(node)); break;
    case NodeType::ARRAYDECL:    check_array     (static_cast<const ArrayDecl*>   (node)); break;
    case NodeType::METHODCALL:   check_method_call(static_cast<const MethodCall*>    (node)); break;
    case NodeType::INDEXEXPR: {
      auto* idx = static_cast<const IndexExpr*>(node);

      check_expr(idx->object.get());
      check_expr(idx->index.get());
      break;
    }
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
  bool already_preregistered = _in_class and _scopes.back().contains(node->id);
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

auto Sema::check_class_decl(const ClassDecl* node) -> void {
  bool has_ctor {};
  std::size_t ctor_arity = 0;
  for (auto& m : node->members) {
    if (m->node_type == NodeType::FUNCTIONDECL) {
      auto* fn = static_cast<const FunctionDecl*>(m.get());
      if (fn->id == "crear") {
        has_ctor   = true;
        ctor_arity = fn->params.size();
      }
    }
  }

  define({
    .name       = node->id,
    .kind       = SymbolKind::CLASS,
    .ctor_arity = ctor_arity,
    .has_ctor   = has_ctor,
  });

  push_scope();
  bool prev = _in_class;
  _in_class = true;

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
auto Sema::check_assignment(const Assignment* node) -> void {
  auto loc = node->loc;
  auto tkn = node->target->node_type;
  if (tkn == NodeType::LITERAL) {
    auto lit = static_cast<const Literal*>(node->target.get());
    if (lit->token.type != TokenType::IDENTIFIER) {
      error(SemanticErrorCode::INVALID_ASSIGN_TARGET, loc);
      return;
    }

    const std::string& name = lit->token.literal;

    if (name.rfind("este.", 0) == 0) {
      if (!_in_class)
        error(SemanticErrorCode::THIS_USED_OUTSIDE_CLASS, loc);
    } else if(name.find('.') == std::string::npos) {

      auto sym = resolve(name);

      if (!sym)
        error(SemanticErrorCode::ASSIGNMENT_TO_UNDECLARED, loc, name);
      else if (sym->kind == SymbolKind::CONSTANT)
        error(SemanticErrorCode::ASSIGNMENT_TO_CONST, loc, name);
      else if (sym->kind == SymbolKind::FUNCTION)
        error(SemanticErrorCode::ASSIGNMENT_TO_FUNC, loc, name);
      else if (sym->kind == SymbolKind::CLASS)
        error(SemanticErrorCode::ASSIGNMENT_TO_CLASS, loc, name);
    }
    check_expr(node->expr.get());
    return;
  }
  else if (NodeType::INDEXEXPR == tkn) {
    auto idx = static_cast<const IndexExpr*>(node->target.get());
    check_expr(idx->object.get());
    check_expr(idx->index.get());
    check_expr(node->expr.get());
    return;
  }
 
  error(SemanticErrorCode::INVALID_ASSIGN_TARGET, loc);
}

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

auto Sema::check_while(const WhileStatement* node) -> void {
  check_expr(node->condition.get());

  push_scope();
  ++_loop_depth;
  check_stmts(node->body);
  --_loop_depth;
  pop_scope();
}

auto Sema::check_return(const ReturnStatement* node) -> void {
  if (_func_depth == 0) {
    error(SemanticErrorCode::RET_OUTSIDE_FUNC, node->loc);
  }
  check_expr(node->expr.get());
}

auto Sema::check_continue(const ContinueStatement* node) -> void {
  if (_loop_depth == 0)
    error(SemanticErrorCode::CONTINUE_OUTSIDE_LOOP, node->loc);
}

auto Sema::check_method_call(const MethodCall* node) -> void {
  check_expr(node->object.get());
  for (auto& a : node->args) check_expr(a.get());
}

auto Sema::check_func_call(const FunctionCall* node) -> void {
  auto loc = node->loc;
  auto dot = node->id.find('.');
  if (dot != std::string::npos) {
    std::string root = node->id.substr(0, dot);
    if (root == "este") {
      if (!_in_class) {
        error(SemanticErrorCode::THIS_USED_OUTSIDE_CLASS, loc, root);
      }
    } else {
      if (!resolve(root)){
        error(SemanticErrorCode::UNDECLARED_ID, loc, root);
      }
    }
    for (auto& a : node->exprs)
      check_expr(a.get());
    return;
  }

  auto sym = resolve(node->id);
  if (!sym) {
    error(SemanticErrorCode::UNDECLARED_FUNC, loc, node->id);
  } else if (sym->kind == SymbolKind::CLASS) {
    if (!sym->has_ctor) {
      if (!node->exprs.empty())
        error(SemanticErrorCode::CLASS_NO_CTOR, loc, node->id, 0, node->exprs.size());
    } else {
      if (node->exprs.size() != sym->ctor_arity)
        error(SemanticErrorCode::CTOR_ARITY_MISMATCH, loc, node->id, sym->ctor_arity, node->exprs.size());
    }
  } else if (sym->kind != SymbolKind::FUNCTION)
    error(SemanticErrorCode::NOT_A_FUNC, loc, node->id);
  else {
    auto* desc = find_builtin(FREE_FUNCTIONS, node->id);
    bool variadic = desc and desc->variadic;
    if (!variadic and node->exprs.size() != sym->arity)
      error(SemanticErrorCode::FUNC_ARITY_MISMATCH, loc, node->id, sym->arity, node->exprs.size());
  }
  for (auto& a : node->exprs)
    check_expr(a.get());
}

auto Sema::check_binary(const BinaryOp* node) -> void {
  check_expr(node->left.get());
  check_expr(node->right.get());
}

auto Sema::check_unary(const UnaryOp* node) -> void {
  check_expr(node->operand.get());
}

auto Sema::check_array(const ArrayDecl* node) -> void {
  for (auto& el : node->data)
    check_expr(el.get());
}

auto Sema::check_literal(const Literal* node) -> void {
  auto loc = node->loc;
  using enum TokenType;
  switch (node->token.type) {
    case IDENTIFIER: {
      auto lit =  node->token.literal;
      auto dot  = node->token.literal.find('.');
      auto  root = (dot == std::string::npos) ? lit : lit.substr(0, dot);

      if (root == "este") {
        if (!_in_class)
          error(SemanticErrorCode::THIS_USED_OUTSIDE_CLASS, loc);
        break;
      }
      if (!resolve(root))
        error(SemanticErrorCode::UNDECLARED_ID, loc, root);
      break;
    }
    case SELF:
      if (!_in_class) {
        error(SemanticErrorCode::THIS_USED_OUTSIDE_CLASS, loc);
      }
      break;
    default: break;
  }
}

auto Sema::init_repl() -> void {
  _errors.clear();
  _scopes.clear();
  _loop_depth = 0;
  _func_depth = 0;
  _in_class   = false;
  push_scope();

  for (const auto& b : FREE_FUNCTIONS) {
    _scopes.back().define({
      .name     = b.name.data(),
      .kind     = SymbolKind::FUNCTION,
      .arity    = b.arity,
      .defined  = true,
    });
  }
}

auto Sema::analyze_incremental(const StmtsPtr& stmts) -> void {
  _errors.clear();
  check_stmts(stmts);
  _errors.flush();
}
