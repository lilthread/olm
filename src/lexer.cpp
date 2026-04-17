#include "lexer.h"
#include <stdexcept>
#include <string>
#include <flat_map>
#include <string_view>

const std::flat_map<std::string_view, TokenType> keywords {
  {"se",        TokenType::ASSIGN},
  {"mientras",  TokenType::WHILE},
  {"si",        TokenType::IF},
  {"sino",      TokenType::ELSE},
  {"var",       TokenType::VAR},
  {"const",     TokenType::CONST},
  {"func",      TokenType::FUNCTION},
  {"fin",       TokenType::END},
  {"haz",       TokenType::DO},
  {"ret",       TokenType::RETURN},
  {"verdadero", TokenType::BOOL},
  {"falso",     TokenType::BOOL},
  {"clase",     TokenType::CLASS},
  {"este",      TokenType::SELF},
  {"o",         TokenType::OR},
  {"y",         TokenType::AND},
};

auto Lexer::next() -> Token {
  while(!at_end()){
    char chr = advance();
    while(std::isspace(chr)){
      if(chr == '\n') {
        _col++;
        _row = 0;
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
        if(get_char_from_idx() != '-')
          return make_char(MINUS, chr);
        else {
          while(get_char_from_idx() != '\n' && !at_end())
          advance();
        }
        break;
      }
      case '>':
        if(get_char_from_idx() != '=')
          return make_char(GREATER_THAN, chr);
        else{
          advance();
          return {GREATER_OR_EQUAL, ">=", _row - 2, _col};
        }
      case '<':
        if(get_char_from_idx() != '=')
          return make_char(LESSER_THAN, chr);
        else{
          advance();
          return {LESSER_OR_EQUAL, "<=", _row - 2, _col};
        }
        break;
      case '!':
        if(get_char_from_idx() == '='){
          advance();
          return {NOT_EQUAL, "!=", _row - 2, _col};
        }else
          return make_char(BANG, chr);
      case '\'':
      case '\"':{
        std::size_t start = _idx;
        while(get_char_from_idx() != chr) { // AKA " or '
          if(get_char_from_idx(-1) == '\0')
            throw std::runtime_error("SyntaxError:unterminated string literal, line: " + std::to_string(_row) + " columna " + std::to_string(_col));
          advance();
        }
        advance(); // Skip ' or ""
        auto str = _source.substr(start, _idx - start - 1);

        return {STRING, std::move(str), _row - str.size(), _col};
      }
      case '$':
        advance();
        while (get_char_from_idx() != '$'){
          if (at_end())
            throw std::runtime_error("unterminated comment block, expected $");
          advance();
        }
        advance(); // Skip '$'
        break;
    }
    if(std::isdigit(chr)){
      auto number_type = TokenType::INTEGER;
      std::size_t start = _idx - 1;

      while(std::isdigit(get_char_from_idx()))
        advance();

      if(get_char_from_idx() == '.' && std::isdigit(get_char_from_idx(1))){
        advance();
        while(std::isdigit(get_char_from_idx())) advance();
        number_type = TokenType::FLOAT;
      }
      auto str = _source.substr(start, _idx - start);
      return {number_type, str, _row - str.size(), _col};
    }
    else if(std::isalpha(chr) || chr == '_'){
      std::size_t start = _idx - 1;
      while(std::isalnum(get_char_from_idx()) || get_char_from_idx() == '_')
        advance();

      std::string_view str{_source.data() + start, _idx - start };
      auto row = _row - str.size();

      if (auto it = keywords.find(str); it != keywords.end()) 
        return {it->second, str, row, _col};
      return {TokenType::IDENTIFIER, str, row, _col};
    }
  }
  return {TokenType::END_OF_FILE, ""};
}

auto Lexer::get_char_from_idx(int steps) -> char {
  if(_idx + steps > _source.size())
    return '\0';
  return _source[_idx + steps];
}

auto Lexer::advance() -> char {
  if(at_end())
    return '\0';
  auto chr = get_char_from_idx();
  _idx++;
  _row++;
  return chr;
}

auto Lexer::at_end() const -> bool {
  return _idx > _source.size();
}

auto Lexer::make_char(TokenType ttype, char chr) -> Token {
  return {ttype, std::string(1, chr), _row - 1, _col};
}
