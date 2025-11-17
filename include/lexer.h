#pragma once
#include <cstddef>
#include<vector>
#include <string>

enum class TokenType{
  LPAREN, RPAREN, LBRACE, RBRACE, COMMA,
  PLUS, MINUS, STAR, SLASH, EQUAL, NOT_EQUAL,
  BANG, ASSIGN, LESSER_THAN, GREATER_THAN, LESSER_OR_EQUAL, GREATER_OR_EQUAL,

  IDENTIFIER, STRING, NUMBER,
  RETURN, TRUE, FALSE, IF, ELSE, FUNCTION, AND, OR, LET, CONST, WHILE,
  END_OF_FILE, ILEGAL
};

struct Token final{
  TokenType type;
  std::string literal;
  Token(TokenType type, std::string literal): type(type), literal(std::move(literal)){}
};

class Lexer final{
public:
  explicit Lexer(const std::string& source_code): m_source_code(source_code), m_current_idx(0), m_line(1){}
  std::vector<Token> Tokenize();
private:
  const std::string m_source_code;
  size_t m_current_idx;
  size_t m_line;
  char advance();
  char peek(size_t step = 0);
};


