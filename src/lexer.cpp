#include "lexer.h"
#include <cctype>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace {
const std::unordered_map<std::string_view, TokenType> keywords {
  {"se",  TokenType::ASSIGN},
  {"mientras",  TokenType::WHILE},
  {"si",        TokenType::IF},
  {"sino",      TokenType::ELSE},
  {"var",       TokenType::VAR},
  {"constante", TokenType::CONST},
  {"func",        TokenType::FUNCTION},
  {"fin",        TokenType::RBRACE},
  {"haz",        TokenType::LBRACE},
  {"ret",       TokenType::RETURN},
  {"verdadero", TokenType::BOOL},
  {"falso",     TokenType::BOOL},
  {"clase",     TokenType::CLASS},
  {"este",     TokenType::SELF},
  {"o",     TokenType::OR},
  {"y",     TokenType::AND},
};

class Lexer final {
public:
  std::vector<Token> tokenize(const std::string& source);

private:
  std::size_t _idx{};
  std::size_t _row{0};
  std::size_t _col{1};
  std::string _source;
  std::vector<Token> tkns;


  void addToken(TokenType type, const std::string& str);

  bool atEnd();
  char peek(int steps = 0);
  char advance();
};
} // namespace

std::vector<Token> tokenize(const std::string& source){
  return Lexer{}.tokenize(source);
}

std::vector<Token> Lexer::tokenize(const std::string& source){
  _source = source;
  while(!atEnd()){
    char chr = advance();

    while(std::isspace(chr)){
      if(chr == '\n') {
        _col++;
        _row = 0;
      }
      chr = advance();
    }
    switch (chr) {
      case '#':
        while(peek() != '\n' && !atEnd())
        advance();
        break;
      case '(': addToken(TokenType::LPAREN, "("); break;
      case ')': addToken(TokenType::RPAREN, ")"); break;
      case '{': addToken(TokenType::LBRACE, "{"); break;
      case '}': addToken(TokenType::RBRACE, "}"); break;
      case ',': addToken(TokenType::COMMA, ",");  break;
      case '+': addToken(TokenType::PLUS, "+");   break;
      case '-': addToken(TokenType::MINUS, "-");  break;
      case '*': addToken(TokenType::STAR, "*");   break;
      case '.': addToken(TokenType::DOT, ".");   break;
      case '/': addToken(TokenType::SLASH, "/");  break;
      case '=': addToken(TokenType::EQUAL, "="); break;
      case ':':
        advance();
        while(std::isalnum(peek())) advance();
        break;
      case '>':
        if(peek() != '=')
          addToken(TokenType::GREATER_THAN, ">");
        else{
          addToken(TokenType::GREATER_OR_EQUAL, ">=");
          advance();
        }
        break;
      case '<':
        if(peek() != '=')
          addToken(TokenType::LESSER_THAN, "<");
        else{
          addToken(TokenType::LESSER_OR_EQUAL, "<=");
          advance();
        }
        break;
      case '!':
        if(peek() == '='){
          addToken(TokenType::NOT_EQUAL, "!=");
          advance();
        }else{
          addToken(TokenType::BANG, "!");
        }
        break;
      case '\'':
      case '\"':{
        advance(); // " or '
        std::size_t start = _idx - 1;
        while(peek() != chr) {
          if(peek(-1) == '\0') throw std::runtime_error("unterminated string literal");
          advance();
        }
        addToken(TokenType::STRING, _source.substr(start, _idx - start));
        advance(); // " or '
        break;
      }
      case '$':
        advance();
        while (peek() != '$'){
          if (atEnd()) throw std::runtime_error("Unterminated block comment");
          advance();
        }
        advance(); // Skip '$'
        break;
    }
    if(std::isdigit(chr)){
      TokenType number_type = TokenType::INTEGER;
      std::size_t start = _idx - 1;
      while(std::isdigit(peek())) advance();

      if( peek() == '.' && std::isdigit(peek(1))){
        advance();
        while(std::isdigit(peek())) advance();
        number_type = TokenType::FLOAT;
      }
      addToken(number_type, _source.substr(start, _idx - start));
    }
    if(std::isalpha(chr) || chr == '_'){
      std::size_t start = _idx - 1;
      while(std::isalnum(peek()) || peek() == '_') advance();
      std::string_view str{_source.data() + start, _idx - start };

      if (auto it = keywords.find(str); it != keywords.end()) 
        addToken(it->second, std::string(str));
      else
        addToken(TokenType::IDENTIFIER, std::string(str));
    }
  }
  addToken(TokenType::END_OF_FILE, "");
  return tkns;
}

bool Lexer::atEnd(){
  return _idx > _source.size();
}

char Lexer::advance(){
  if(atEnd())
    return '\0';

  auto chr = peek();
  _idx++;
  _row++;
  return chr;
}

char Lexer::peek(int steps){
  if(_idx + steps > _source.size())
    return '\0';
  return _source[_idx + steps];
}

void Lexer::addToken(TokenType type, const std::string& str){
  tkns.emplace_back(type, str, _row - str.size(), _col);
}
