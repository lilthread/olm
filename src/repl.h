#pragma once
#include "interpreter.h"
#include "sema.h"
#include <string>
#include <vector>

class Repl final {
public:
  auto run() -> void;

private:
  Sema        _sema;
  Interpreter _interp;
  std::string _pending{};

  std::vector<StmtsPtr> _ast_store{};

  auto is_incomplete(std::string_view src) const -> bool;
  auto execute(const std::string& src) -> void; 
};
