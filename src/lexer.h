#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

enum class TokenType : uint8_t {
  VAR, CONST, ASSIGN, INTEGER, FLOAT, BOOL, STRING, IDENTIFIER, FUNCTION, CLASS, RETURN, IF, ELSE, WHILE, AND, OR, EQUAL, SELF, DOT,
  PLUS, MINUS, STAR, SLASH, LPAREN, RPAREN, LBRACE, RBRACE, COMMA, BANG, NOT_EQUAL,GREATER_THAN, LESSER_THAN, GREATER_OR_EQUAL, LESSER_OR_EQUAL,
  ILEGAL, END_OF_FILE
};

struct Token final{
  TokenType type;
  std::string literal;
  std::size_t row;
  std::size_t col;
  Token(TokenType type, std::string literal = "", std::size_t row = 0, std::size_t col = 0):
    type(type), literal(std::move(literal)), row(row), col(col){}
};

std::vector<Token> tokenize(const std::string& source);
