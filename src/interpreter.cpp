#include "interpreter.h"
#include <cassert>
#include <cstdint>
#include <format>
#include <iterator>
#include <memory>
#include <string>
#include <vector>
#include "builtins.h"
#include "nodes.h"
#include "runtime_values.h"
#include "error_manager.h"
#include "std.h"
using enum NodeType;

auto Environment::push() -> void { _scopes.emplace_back(); }
auto Environment::pop()  -> void { assert(!_scopes.empty()); _scopes.pop_back(); }

auto Environment::define(const std::string& name, ValuePtr val) -> void {
  _scopes.back()[name] = std::move(val);
}

auto Environment::get(const std::string& name) const -> ValuePtr {
  for (auto it = _scopes.rbegin(); it != _scopes.rend(); ++it) {
    if (auto found = it->find(name); found != it->end())
      return found->second;
  }
  throw RuntimeError(std::format("variable '{}' no definida", name));
}

auto Environment::set(const std::string& name, ValuePtr val) -> void {
  for (auto it = _scopes.rbegin(); it != _scopes.rend(); ++it) {
    if (auto found = it->find(name); found != it->end()) {
      found->second = std::move(val);
      return;
    }
  }
  throw RuntimeError(std::format("asignacion a variable no declarada '{}'", name));
}

auto Environment::has(const std::string& name) const -> bool {
  for (auto it = _scopes.rbegin(); it != _scopes.rend(); ++it)
    if (it->contains(name)) return true;
  return false;
}

auto Interpreter::run(const StmtsPtr& program) -> void {
  exec_stmts(program);
}

auto Interpreter::exec_stmts(const StmtsPtr& stmts) -> void {
  for (auto& s : stmts)
    exec_stmt(s.get());
}

auto Interpreter::exec_stmt(const IAST* node) -> void {
  if (!node)
    return;
  switch (node->node_type) {
    case VARIABLEDECL:    exec_var_decl  (static_cast<const VariableDecl*>  (node)); break;
    case FUNCTIONDECL:    exec_func_decl (static_cast<const FunctionDecl*>  (node)); break;
    case CLASSDECL:       exec_class_decl(static_cast<const ClassDecl*>     (node)); break;
    case ASSIGNMENT:      exec_assignment(static_cast<const Assignment*>    (node)); break;
    case IFSTATEMENT:     exec_if        (static_cast<const IfStatement*>   (node)); break;
    case WHILESTATEMENT:  exec_while     (static_cast<const WhileStatement*>(node)); break;
    case RETURNSTATEMENT: exec_return    (static_cast<const ReturnStatement*>(node));break;
    case CONTINUESTMT:    exec_continue  (static_cast<const ContinueStatement*>(node));break;
    case FUNCTIONCALL:    eval_call      (static_cast<const FunctionCall*>  (node)); break;
    case METHODCALL:      eval_method_call(static_cast<const MethodCall*>    (node)); break;
    default:
      throw RuntimeError("nodo de declaracion desconocido");
  }
}


auto Interpreter::exec_var_decl(const VariableDecl* node) -> void {
  auto val = eval(node->expr.get());
  _env.define(node->id, std::move(val));
}

auto Interpreter::exec_func_decl(const FunctionDecl* node) -> void {
  _functions[node->id] = node;
  _env.define(node->id, make_null());
}

auto Interpreter::exec_class_decl(const ClassDecl* node) -> void {
  auto def = std::make_shared<ClassDef>();
  def->name = node->id;

  for (auto& m : node->members) {
    if (m->node_type == VARIABLEDECL) {
      auto* vd = static_cast<const VariableDecl*>(m.get());
      def->fields.try_emplace(vd->id, vd->expr.get());
    } else if (m->node_type == FUNCTIONDECL) {
      auto* fn = static_cast<const FunctionDecl*>(m.get());
      def->methods[fn->id] = fn;
    }
  }

  _classes[node->id] = std::move(def);
  _env.define(node->id, make_null()); // sentinel
}

auto Interpreter::exec_assignment(const Assignment* node) -> void {
  auto val = eval(node->expr.get());

  if (auto lit = dynamic_cast<const Literal*>(node->target.get())) {
    const std::string& name = lit->token.literal;

    auto dot = name.find('.');
    if (dot == std::string::npos) {
      _env.set(name, std::move(val));
      return;
    }
    auto [inst, field] = resolve_member(name);
    inst->fields[field] = std::move(val);
    return;
  }
  if (auto idx = dynamic_cast<const IndexExpr*>(node->target.get())) {
    auto obj = eval(idx->object.get());
    auto index = eval(idx->index.get());

    if (!obj->is_array())
      throw RuntimeError("solo se puede indexar arreglos");

    else if (!index->is_int())
      throw RuntimeError("indice debe ser entero");

    auto& arr = obj->as_array();
    auto i = index->as_int();

    if (i < 0 || i >= (int64_t)arr.size())
      throw RuntimeError("indice fuera de rango");

    arr[i] = std::move(val);
    return;
  }
  throw RuntimeError("asignacion a objetivo invalido");
}


auto Interpreter::exec_if(const IfStatement* node) -> void {
  auto cond = eval(node->condition.get());
  if (cond->truthy()) { 
    exec_stmts(node->then_body);
    return;
  }

  std::visit([this](const auto& next) {
    using T = std::decay_t<decltype(next)>;
    if constexpr (std::is_same_v<T, StmtsPtr>) {
      exec_stmts(next);
    } else if constexpr (std::is_same_v<T, std::unique_ptr<IfStatement>>) {
      exec_if(next.get());
    }
  }, node->next);
}

auto Interpreter::exec_while(const WhileStatement* node) -> void {
  while (eval(node->condition.get())->truthy()) {
    _env.push();
    try {
      exec_stmts(node->body);
    } catch (ContinueSignal&){}
    _env.pop();
  }
}

auto Interpreter::exec_return(const ReturnStatement* node) -> void {
  auto val = eval(node->expr.get());
  throw ReturnSignal{std::move(val)};
}
 
auto Interpreter::exec_continue(const ContinueStatement*) -> void {
  throw ContinueSignal{};
}

auto Interpreter::eval(const IAST* node) -> ValuePtr {
  if (!node) return make_null();
  switch (node->node_type) {
    case LITERAL:      return eval_literal(static_cast<const Literal*>     (node));
    case BINARYOP:     return eval_binary (static_cast<const BinaryOp*>    (node));
    case UNARYOP:      return eval_unary  (static_cast<const UnaryOp*>     (node));
    case FUNCTIONCALL: return eval_call   (static_cast<const FunctionCall*>(node));
    case ARRAYDECL:    return eval_array  (static_cast<const ArrayDecl*>   (node));
    case METHODCALL:   return eval_method_call(static_cast<const MethodCall*> (node));
    case INDEXEXPR:    return eval_index_expr(static_cast<const IndexExpr*>(node));
    default:
      throw RuntimeError("nodo de expresion desconocido");
  }
}

auto Interpreter::eval_literal(const Literal* node) -> ValuePtr {
  switch (node->token.type) {
    case TokenType::INTEGER:
      return make(static_cast<int64_t>(std::stoll(node->token.literal)));
    case TokenType::FLOAT:
      return make(std::stod(node->token.literal));
    case TokenType::BOOL:
      return make(node->token.literal == "verdadero");
    case TokenType::STRING:
      return make(node->token.literal);
    case TokenType::SELF: {
      return _env.get("__este__");
    }
    case TokenType::IDENTIFIER: {
      auto& lit = node->token.literal;
      auto dot = lit.find('.');
      if (dot == std::string::npos)
        return _env.get(lit);

      const auto [inst, field] = resolve_member(lit);
      if (auto it = inst->fields.find(field); it != inst->fields.end())
        return it->second;
      throw RuntimeError(std::format("la instancia no tiene campo '{}'", field));
    }
    case TokenType::NIL: {
      return make_null();
    }
    default:
      throw RuntimeError(std::format("literal desconocido '{}'", node->token.literal));
  }
}

auto Interpreter::to_number(const ValuePtr& v, std::string_view op) -> std::pair<bool, double> {
  if (v->is_int())   return {false, static_cast<double>(v->as_int())};
  if (v->is_float()) return {true,  v->as_float()};
  throw RuntimeError(std::format("operador '{}' requiere numeros, obtuvo '{}'", op, v->to_string()));
}


auto Interpreter::eval_binary(const BinaryOp* node) -> ValuePtr {
  using enum TokenType;
  if (node->op.type == AND) {
    auto l = eval(node->left.get());
    if (!l->truthy()) return make(false);
    return make(eval(node->right.get())->truthy());
  }
  if (node->op.type == OR) {
    auto l = eval(node->left.get());
    if (l->truthy()) return make(true);
    return make(eval(node->right.get())->truthy());
  }

  auto lv = eval(node->left.get());
  auto rv = eval(node->right.get());

  switch (node->op.type) {
    case PLUS: {
      if (lv->is_string() || rv->is_string())
        return make(lv->to_string() + rv->to_string());
      auto [lf, ln] = to_number(lv, "+");
      auto [rf, rn] = to_number(rv, "+");
      if (lf || rf) return make(ln + rn);
      return make(static_cast<int64_t>(ln) + static_cast<int64_t>(rn));
    }
    case MINUS: {
      auto [lf, ln] = to_number(lv, "-");
      auto [rf, rn] = to_number(rv, "-");
      if (lf || rf) return make(ln - rn);
      return make(static_cast<int64_t>(ln) - static_cast<int64_t>(rn));
    }
    case STAR: {
      auto [lf, ln] = to_number(lv, "*");
      auto [rf, rn] = to_number(rv, "*");
      if (lf || rf) return make(ln * rn);
      return make(static_cast<int64_t>(ln) * static_cast<int64_t>(rn));
    }
    case SLASH: {
      auto [lf, ln] = to_number(lv, "/");
      auto [rf, rn] = to_number(rv, "/");
      if (rn == 0.0)
        throw RuntimeError("division por cero");
      if (!lf && !rf && static_cast<int64_t>(rn) != 0)
        return make(static_cast<int64_t>(ln) / static_cast<int64_t>(rn));
      return make(ln / rn);
    }
    case EQUAL: {
      if (lv->is_int()    && rv->is_int())    return make(lv->as_int()    == rv->as_int());
      if (lv->is_float()  || rv->is_float()) {
        auto [_a, a] = to_number(lv, "==");
        auto [_b, b] = to_number(rv, "==");
        return make(a == b);
      }
      if (lv->is_bool()   && rv->is_bool())   return make(lv->as_bool()   == rv->as_bool());
      if (lv->is_string() && rv->is_string()) return make(lv->as_string() == rv->as_string());
      if (lv->is_null()   && rv->is_null())   return make(true);
      return make(false);
    }
    case NOT_EQUAL: {
      BinaryOp eq_node{nullptr, nullptr, Token{EQUAL, "="}};
      auto eq = [&]() -> bool {
        if (lv->is_int()    && rv->is_int())    return lv->as_int()    == rv->as_int();
        if (lv->is_float()  || rv->is_float()) {
          auto [_a, a] = to_number(lv, "!=");
          auto [_b, b] = to_number(rv, "!=");
          return a == b;
        }
        if (lv->is_bool()   && rv->is_bool())   return lv->as_bool()   == rv->as_bool();
        if (lv->is_string() && rv->is_string()) return lv->as_string() == rv->as_string();
        if (lv->is_null()   && rv->is_null())   return true;
        return false;
      }();
      return make(!eq);
    }
    case LESSER_THAN: {
      auto [lf, ln] = to_number(lv, "<");
      auto [rf, rn] = to_number(rv, "<");
      return make(ln < rn);
    }
    case GREATER_THAN: {
      auto [lf, ln] = to_number(lv, ">");
      auto [rf, rn] = to_number(rv, ">");
      return make(ln > rn);
    }
    case LESSER_OR_EQUAL: {
      auto [lf, ln] = to_number(lv, "<=");
      auto [rf, rn] = to_number(rv, "<=");
      return make(ln <= rn);
    }
    case GREATER_OR_EQUAL: {
      auto [lf, ln] = to_number(lv, ">=");
      auto [rf, rn] = to_number(rv, ">=");
      return make(ln >= rn);
    }
    default:
      throw RuntimeError(std::format("operador binario desconocido '{}'", node->op.literal));
  }
}

auto Interpreter::eval_unary(const UnaryOp* node) -> ValuePtr {
  auto val = eval(node->operand.get());
  switch (node->op) {
    case TokenType::MINUS:
      if (val->is_int())   return make(-val->as_int());
      if (val->is_float()) return make(-val->as_float());
      throw RuntimeError("'-' unario requiere un numero");
    case TokenType::BANG:
      return make(!val->truthy());
    default:
      throw RuntimeError("operador unario desconocido");
  }
}

auto Interpreter::call_builtin(const std::string& name, std::span<ValuePtr> args) -> ValuePtr {
  auto* desc = find_builtin(FREE_FUNCTIONS, name); 
  if (desc && !desc->variadic) {
    if (args.size() != desc->arity)
      throw RuntimeError(std::format(
        "'{}' espera {} argumento(s) pero recibio {}",
        name, desc->arity, args.size()));
  } 
  return desc->fn(nullptr, std::move(args));
}

auto Interpreter::eval_call(const FunctionCall* node) -> ValuePtr {
  if (node->id == "__index__") {
    auto arr  = eval(node->exprs[0].get());
    auto idx  = eval(node->exprs[1].get());
    if (!idx->is_int())
      throw RuntimeError("el indice debe ser un entero");
    if (arr->is_array()) {
      auto i = idx->as_int();
      auto& data = arr->as_array();
      if (i < 0 || static_cast<std::size_t>(i) >= data.size())
        throw RuntimeError(std::format("indice {} fuera de rango (tamaño {})", i, data.size()));
      return data[static_cast<std::size_t>(i)];
    } else if (arr->is_string()){
      auto i = idx->as_int();
      char data = arr->as_string()[i];
      return make(std::string{data});
    }
    throw RuntimeError("solo se puede indexar un arreglo");
  }

  if (is_builtin(node->id)) {
    std::vector<ValuePtr> args;
    args.reserve(node->exprs.size());
    for (auto& a : node->exprs)
      args.push_back(eval(a.get()));
    return call_builtin(node->id, args);
  }

  if (_classes.contains(node->id)) {
    std::vector<ValuePtr> args;
    args.reserve(node->exprs.size());
    for (auto& a : node->exprs)
      args.push_back(eval(a.get()));
    return instantiate(node->id, args);
  }

  auto dot = node->id.find('.');
  if (dot != std::string::npos) {
    std::string obj_name = node->id.substr(0, dot);
    std::string method   = node->id.substr(dot + 1);

    ValuePtr obj_val = (obj_name == "este")
                         ? _env.get("__este__")
                         : _env.get(obj_name);

    if (!obj_val->is_instance())
      throw RuntimeError(std::format("'{}' no es una instancia", obj_name));

    auto inst = obj_val->as_instance();
    auto it   = inst->klass->methods.find(method);
    if (it == inst->klass->methods.end())
      throw RuntimeError(std::format("'{}' no tiene metodo '{}'",
                                     inst->klass->name, method));

    std::vector<ValuePtr> args;
    args.reserve(node->exprs.size());
    for (auto& a : node->exprs)
      args.push_back(eval(a.get()));

    return call_function(it->second, args, inst);
  }

  auto fit = _functions.find(node->id);
  if (fit == _functions.end())
    throw RuntimeError(std::format("funcion '{}' no definida", node->id));

  std::vector<ValuePtr> args;
  args.reserve(node->exprs.size());
  for (auto& a : node->exprs)
    args.push_back(eval(a.get()));

  return call_function(fit->second, args, nullptr);
}

auto Interpreter::eval_array(const ArrayDecl* node) -> ValuePtr {
  std::vector<ValuePtr> items;
  items.reserve(node->data.size());
  for (auto& el : node->data)
    items.push_back(eval(el.get()));
  return std::make_shared<Value>(std::move(items));
}

auto Interpreter::call_function(const FunctionDecl* fn, std::span<ValuePtr> args, InstancePtr self) -> ValuePtr {
  if (args.size() != fn->params.size())
    throw RuntimeError(std::format("'{}' espera {} argumento(s), obtuvo {}", fn->id.size(), fn->params.size(), args.size()));
  _env.push();

  if (self)
    _env.define("__este__", std::make_shared<Value>(self));

  for (auto i {0uz}; i < fn->params.size(); i++)
    _env.define(fn->params[i], std::move(args[i]));

  ValuePtr result = make_null();
  try {
    exec_stmts(fn->body);
  } catch (ReturnSignal& rs) {
    result = std::move(rs.value);
  }

  _env.pop();
  return result;
}


auto Interpreter::instantiate(const std::string& class_name, std::span<ValuePtr> args) -> ValuePtr {
  auto it = _classes.find(class_name);
  if (it == _classes.end())
    throw RuntimeError(std::format("clase '{}' no definida", class_name));

  auto& def  = it->second;
  auto  inst = std::make_shared<Instance>();
  inst->klass = def;


  _env.push();
  _env.define("__este__", std::make_shared<Value>(inst));
  for (const auto& [name, expr] : def->fields) {
    if (expr)
      inst->fields[name] = eval(expr);
    else
      inst->fields[name] = make_null();
  }
  _env.pop();


  auto ctor_it = def->methods.find("crear");
  if (ctor_it != def->methods.end())
    call_function(ctor_it->second, std::move(args), inst);
  else if (!args.empty())
    throw RuntimeError(std::format("clase '{}' no tiene constructor: ", class_name));
  return std::make_shared<Value>(inst);
}

auto Interpreter::resolve_member(const std::string& dotted) -> std::pair<InstancePtr, std::string> {
  auto dot     = dotted.find('.');
  auto root    = dotted.substr(0, dot);
  auto field   = dotted.substr(dot + 1);

  ValuePtr obj_val = (root == "este") ? _env.get("__este__") : _env.get(root);

  if (!obj_val->is_instance())
    throw RuntimeError(std::format("'{}' no es una instancia", root));

  return {obj_val->as_instance(), field};
}

auto Interpreter::eval_index_expr(const IndexExpr* node) -> ValuePtr {
  auto obj = eval(node->object.get());
  auto idx = eval(node->index.get());

  if (!idx->is_int())
    throw RuntimeError("el indice debe ser entero");

  auto i = idx->as_int();

  if (obj->is_array()) {
    auto& arr = obj->as_array();
    if (i < 0 || i >= arr.size())
      throw RuntimeError("indice fuera de rango");
    return arr[i];
  }else if (obj->is_string()) {
    const auto& s = obj->as_string();
    if (i < 0 || i >= (int)s.size())
      throw RuntimeError("indice fuera de rango");
    return make(std::string(1, s[i]));
  }

  throw RuntimeError("solo se puede indexar array o string");
}

auto Interpreter::dispatch_native_method(std::span<const NativeMethodDesc> methods, ValuePtr self, const MethodCall* node) -> ValuePtr {
  auto* method = find_builtin(methods, node->name);

  if (!method)
    throw RuntimeError("Invalido metodo");

  if (!method->variadic && node->args.size() != method->arity)
    throw RuntimeError("argumento(s) invalido(s)");

  std::vector<ValuePtr> args;

  args.reserve(node->args.size());

  for (const auto& arg : node->args)
    args.push_back(eval(arg.get()));

  return method->fn(self, args);
}

auto Interpreter::eval_method_call(const MethodCall* node) -> ValuePtr {
  auto obj = eval(node->object.get());

  if (obj->is_array()) {
    return dispatch_native_method(
      ARRAY_METHODS,
      obj,
      node
    );
  } else if (obj->is_string()) {
    return dispatch_native_method(
      STRING_METHODS,
      obj,
      node
    );
  }

  else if (obj->is_instance()) {
    auto  inst = obj->as_instance();
    auto  it   = inst->klass->methods.find(node->name);
    if (it == inst->klass->methods.end())
      throw RuntimeError(std::format("'{}' no tiene metodo '{}'",
                                     inst->klass->name, node->name));
    std::vector<ValuePtr> args;
    args.reserve(node->args.size());
    for (auto& a : node->args)
      args.push_back(eval(a.get()));
    return call_function(it->second, args, inst);
  } 

  throw RuntimeError("Metodo Invalido");
}
