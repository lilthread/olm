#pragma once
#include <cstdint>
#include <string>
#include <vector>

enum class TokenType : uint8_t {
  VAR, CONST, ASSIGN, INTEGER, FLOAT, BOOL, STRING, IDENTIFIER, FUNCTION, CLASS, RETURN, IF, ELSE, WHILE, AND, OR, EQUAL, SELF,
  PLUS, MINUS, STAR, SLASH, LPAREN, RPAREN, LBRACE, RBRACE, COMMA, BANG, NOT_EQUAL,GREATER_THAN, LESSER_THAN, GREATER_OR_EQUAL, LESSER_OR_EQUAL,
  ILEGAL, END_OF_FILE
};

struct Token final{
  TokenType type;
  std::string literal;
  Token(TokenType type, std::string literal = ""): type(type), literal(std::move(literal)){}
};

class Lexer final {
public:
  Lexer(const std::string& source): _idx(0), _source(source){}
  std::vector<Token> tokenize();

private:
  size_t _idx;
  std::string _source;
  bool atEnd();
  char peek(size_t steps = 0);
  char advance();
};
