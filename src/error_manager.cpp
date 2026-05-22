#include "error_manager.h"
#include "utilities.h"
#include <format>

auto SemanticError::message() const -> std::string {
  using enum SemanticErrorCode;
  switch (code) {
    case REDEFINITION:
      return std::format("'{}' ya fue declarado en este alcance", subject);
    case THIS_USED_OUTSIDE_CLASS:
      return "'este' usado fuera de una clase o metodo";
    case ASSIGNMENT_TO_CONST:
      return std::format("asignacion a constante '{}'", subject);
    case ASSIGNMENT_TO_CLASS:
      return std::format("asignacion a clase '{}'", subject);
    case ASSIGNMENT_TO_FUNC:
      return std::format("asignacion a funcion '{}'", subject);
    case ASSIGNMENT_TO_UNDECLARED:
      return std::format("asignacion a variable no declarada '{}'", subject);
    case RET_OUTSIDE_FUNC:
      return "'ret' usado fuera de una funcion";
    case UNDECLARED_ID:
      return std::format("identificador no declarado '{}'", subject);
    case UNDECLARED_FUNC:
      return std::format("funcion no declarada '{}'", subject);
    case FUNC_ARITY_MISMATCH:
      return std::format("'{}' espera {} argumento(s) pero recibio {}", subject, expected, got);
    case CTOR_ARITY_MISMATCH:
      return std::format("constructor '{}' espera {} argumento(s) pero recibio {}", subject, expected, got);
    case CLASS_NO_CTOR:
      return std::format("la clase '{}' no tiene constructor 'crear' pero recibio {} argumento(s)", subject, got);
    case NOT_A_FUNC:
      return std::format("'{}' no es una funcion", subject);
    case INVALID_ASSIGN_TARGET:
      return std::format("asignacion invalida: '{}' ", subject);
  }
  return "error semantico desconocido";
}

auto SemanticError::to_string() const -> std::string {
  if (location == SourceLocation{}){
    return std::format("{}", message());
  }
  return std::format("[{}] {}", location.to_string(), message());
}


auto ErrorManager::emit(SemanticError err) -> void {
  _errors.push_back(std::move(err));
}

auto ErrorManager::flush() -> void {
  if (!_errors.empty())
    throw SemanticException(std::move(_errors));
}

auto ErrorManager::has_errors() const -> bool { return !_errors.empty(); }

auto ErrorManager::errors() const -> const std::vector<SemanticError>& {
  return _errors;
}

auto ErrorManager::clear() -> void { _errors.clear(); }

