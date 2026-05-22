#pragma once
#include "tokens.h"
#include "utilities.h"

class Lexer final {
public:
  [[nodiscard]] explicit Lexer(std::string_view const source);
  [[nodiscard]] auto next()         -> Token;
  [[nodiscard]] auto at_end() const -> bool;
private:
  std::string_view _source;
  SourceLocation _loc{};
  std::size_t _idx{0uz};

  auto advance()                            -> char;
  auto peek(int idx = 0)       -> char;
  auto make_char(TokenType ttype, char chr) -> Token;
  auto get_number() -> Token;
  auto get_str()    -> Token;
};
