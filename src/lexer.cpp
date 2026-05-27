#include "lexer.h"
#include <format>
#include <stdexcept>
#include <string>
#include <flat_map>
#include <string_view>

// TODO: FIX {ROW, COL} LOCATION MISMATCH

const std::flat_map<std::string_view, TokenType> keywords {
  {"se",        TokenType::ASSIGN},
  {"mientras",  TokenType::WHILE},
  {"si",        TokenType::IF},
  {"no",        TokenType::BANG},
  {"nulo",      TokenType::NIL},
  {"sino",      TokenType::ELSE},
  {"var",       TokenType::VAR},
  {"const",     TokenType::CONST},
  {"func",      TokenType::FUNCTION},
  {"fin",       TokenType::END},
  {"haz",       TokenType::DO},
  {"devolver",  TokenType::RETURN},
  {"verdadero", TokenType::BOOL},
  {"falso",     TokenType::BOOL},
  {"clase",     TokenType::CLASS},
  {"este",      TokenType::SELF},
  {"o",         TokenType::OR},
  {"y",         TokenType::AND},
  {"continuar", TokenType::CONTINUE},
};
Lexer::Lexer(std::string_view source) {
  _source = source;
  _loc.col = 1;
  _loc.row = 0;
}

auto Lexer::next() -> Token {
  while(!at_end()){
    char chr = advance();
    while(std::isspace(chr)){
      if(chr == '\n') {
        _loc.col++;
        _loc.row = 0;
      }
      chr = advance();
    }
    switch (chr) {
      using enum TokenType;
      case '(': return make_char(LPAREN, chr);
      case ')': return make_char(RPAREN, chr);
      case '{': return make_char(LBRACE, chr);
      case '}': return make_char(RBRACE, chr);
      case '[': return make_char(LBRACKET, chr);
      case ']': return make_char(RBRACKET, chr);
      case ',': return make_char(COMMA, chr);
      case '+': return make_char(PLUS, chr);
      case '*': return make_char(STAR, chr);
      case '.': return make_char(DOT, chr);
      case '/': return make_char(SLASH, chr);
      case '=': return make_char(EQUAL, chr);
      case ':': return make_char(COLON, chr);
      case '-':{
        if(peek() != '-')
          return make_char(MINUS, chr);
        while(peek() != '\n' && !at_end())
          advance();
        break;
      }
      case '>':{
        if(peek() != '=')
          return make_char(GREATER_THAN, chr);
        advance();
        return {GREATER_OR_EQUAL, ">=", {_loc.row - 2, _loc.col}};
      }case '<':{
        if(peek() != '=')
          return make_char(LESSER_THAN, chr);
        advance();
        return {LESSER_OR_EQUAL, "<=", {_loc.row - 2, _loc.col}};
      }case '!':
        if(peek() == '='){
          advance();
          return {NOT_EQUAL, "!=", {_loc.row - 2, _loc.col}};
        }else
          return make_char(BANG, chr);
      case '\'':
      case '\"':{
        std::size_t start = _idx;
        while(peek() != chr) { // AKA " or '
          if(peek(-1) == '\0')
            throw std::runtime_error(std::format("[{}] cadena sin terminar", _loc.to_string()));
          advance();
        }
        advance(); // Skip ' or ""
        auto str = _source.substr(start, _idx - start - 1);

        return {STRING, std::move(str), {_loc.row - str.size(), _loc.col}};
      }
      case '$':
        advance();
        while (peek() != '$'){
          if (at_end())
            throw std::runtime_error(std::format("[{}] bloque de comentario sin terminar, se esperaba '$'", _loc.to_string()));
          advance();
        }
        advance(); // Skip '$'
        break;
    }
    if(std::isdigit(chr))
      return get_number();
    else if(std::isalpha(chr) || chr == '_')
      return get_str();
  }
  return {TokenType::END_OF_FILE, ""};
}

auto Lexer::peek(int idx) -> char {
  if(_idx + idx > _source.size())
    return '\0';
  return _source[_idx + idx];
}

auto Lexer::advance() -> char {
  if(at_end())
    return '\0';
  auto chr = peek();
  _idx++;
  _loc.row++;
  return chr;
}

auto Lexer::at_end() const -> bool {
  return _idx > _source.size();
}

auto Lexer::make_char(TokenType ttype, char chr) -> Token {
  return {ttype, std::string(1, chr), {_loc.row - 1, _loc.col}};
}

auto Lexer::get_number() -> Token {
  auto number_type = TokenType::INTEGER;
  std::size_t start = _idx - 1;

  while(std::isdigit(peek()))
    advance();

  if(peek() == '.' ){
    if(!std::isdigit(peek(1)))
      throw std::runtime_error(std::format("[{}]", _loc.to_string()));
    advance();
    while(std::isdigit(peek())) advance();
    number_type = TokenType::FLOAT;
  }
  auto str = _source.substr(start, _idx - start);
  return {number_type, str, {_loc.row - str.size(), _loc.col}};
}

auto Lexer::get_str() -> Token {
  std::size_t start = _idx - 1;
  while(std::isalnum(peek()) || peek() == '_')
    advance();

  std::string_view str{_source.data() + start, _idx - start };
  auto row = _loc.row - str.size();

  if (auto it = keywords.find(str); it != keywords.end()) 
    return {it->second, str, {row, _loc.col}};
  return {TokenType::IDENTIFIER, str, {row, _loc.col}};
}
