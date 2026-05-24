#pragma once
#include <stdexcept>
#include <string>
#include <vector>
#include "utilities.h"

enum class SemanticErrorCode {
  REDEFINITION, THIS_USED_OUTSIDE_CLASS, ASSIGNMENT_TO_CONST, ASSIGNMENT_TO_CLASS,
  ASSIGNMENT_TO_FUNC, ASSIGNMENT_TO_UNDECLARED, RET_OUTSIDE_FUNC, UNDECLARED_ID, INVALID_ASSIGN_TARGET,
  UNDECLARED_FUNC, FUNC_ARITY_MISMATCH, CTOR_ARITY_MISMATCH, CLASS_NO_CTOR, NOT_A_FUNC, CONTINUE_OUTSIDE_LOOP
};

struct SemanticError final {
  SemanticErrorCode code{};
  SourceLocation    location{};
  std::string       subject{};
  std::size_t       expected{0};
  std::size_t       got{0};
 
  SemanticError(SemanticErrorCode c,
      SourceLocation l,
      std::string    subj     = {},
      std::size_t    expected = 0,
      std::size_t    got      = 0)
    : code(c), location(l), subject(std::move(subj)),
    expected(expected), got(got) {}
 
  auto message() const -> std::string;
  auto to_string() const -> std::string;
};

class ErrorManager final {
public:
  auto emit(SemanticError err)                       -> void;
  auto flush()                                       -> void;
  auto has_errors()  const                           -> bool;
  auto errors()      const -> const std::vector<SemanticError>&;
  auto clear()                                       -> void;
private:
  std::vector<SemanticError> _errors;
};

struct SemanticException final : std::runtime_error {
  std::vector<SemanticError> errors;
  explicit SemanticException(std::vector<SemanticError> errs)
      : std::runtime_error(build_msg(errs)), errors(std::move(errs)) {}
private:
  static auto build_msg(const std::vector<SemanticError>& errs) -> std::string {
    std::string out = "error semántico: \n";
    for (auto& e : errs)
      out += "  * " + e.to_string() + "\n";
    return out;
  }
};

struct RuntimeError final : std::runtime_error {
  explicit RuntimeError(const std::string& msg) :
    std::runtime_error("error de tiempo de ejecución: " + msg) {}
};

