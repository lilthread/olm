#pragma once
#include "utilities.h"
#include <string>

enum class TokenType : ushort {
  ILEGAL, VAR, CONST, ASSIGN, INTEGER, FLOAT, BOOL,
  CONTINUE, STRING, IDENTIFIER, FUNCTION, CLASS,
  RETURN, IF, ELSE, WHILE, AND, OR, EQUAL, SELF,
  DOT, COLON, PLUS, MINUS, STAR, SLASH, LPAREN,
  RPAREN, DO, END, COMMA, BANG, NOT_EQUAL,GREATER_THAN, NIL,
  LESSER_THAN, GREATER_OR_EQUAL, LESSER_OR_EQUAL, END_OF_FILE,
  LBRACE, RBRACE, LBRACKET, RBRACKET
};

struct Token final {
  TokenType type;
  std::string literal;
  SourceLocation loc;

  Token(TokenType type = TokenType::ILEGAL,
    std::string_view literal = "", SourceLocation loc = {}):
  type(type), literal(std::move(literal)), loc(loc){}

  friend constexpr auto operator==(Token t1, Token t2) -> bool {
    return t1.literal == t2.literal and t1.type == t2.type;
  }
};

