#pragma once
#include <string>

enum class TokenType {
  VAR, CONST, ASSIGN, INTEGER, FLOAT, BOOL,
  STRING, IDENTIFIER, FUNCTION, CLASS, RETURN,
  IF, ELSE, WHILE, AND, OR, EQUAL, SELF, DOT, COLON,
  PLUS, MINUS, STAR, SLASH, LPAREN, RPAREN,
  DO, END, COMMA, BANG, NOT_EQUAL,GREATER_THAN,
  LESSER_THAN, GREATER_OR_EQUAL, LESSER_OR_EQUAL,
  ILEGAL, END_OF_FILE,  LBRACE, RBRACE, LBRACKET, RBRACKET
};

struct Token final {
  TokenType type;
  std::string literal;
  std::size_t row;
  std::size_t col;

  Token(TokenType type = TokenType::ILEGAL,
    std::string_view literal = "",
    std::size_t row = 0,
    std::size_t col = 0):
  type(type), literal(std::move(literal)), row(row), col(col) {}
};

inline constexpr auto operator==(Token t1, Token t2) -> bool {
  return t1.literal == t2.literal and t1.type == t2.type;
}
