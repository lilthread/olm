#pragma once
#include "tokens.h"

class Lexer final {
public:
  [[nodiscard]] explicit Lexer(std::string_view const source): _source(source){}
  [[nodiscard]] auto next()         -> Token;
  [[nodiscard]] auto at_end() const -> bool;
private:
  std::size_t _idx{0uz};
  std::size_t _row{0uz};
  std::size_t _col{1uz};
  const std::string_view _source;

  auto advance()                            -> char;
  auto get_char_from_idx(int idx = 0)       -> char;
  auto make_char(TokenType ttype, char chr) -> Token;
};
