#include "lexer.h"
#include <string>
#include <unordered_map>

namespace {
const std::unordered_map<std::string_view, TokenType> keywords {
  {"mientras",  TokenType::WHILE},
  {"si",        TokenType::IF},
  {"sino",      TokenType::ELSE},
  {"var",       TokenType::VAR},
  {"constante", TokenType::CONST},
  {"fn",        TokenType::FUNCTION},
  {"ret",       TokenType::RETURN},
  {"verdadero", TokenType::BOOL},
  {"falso",     TokenType::BOOL},
  {"clase",     TokenType::CLASS},
  {"self",     TokenType::SELF},
};
} // namespace

std::vector<Token> Lexer::tokenize(){
  std::vector<Token> tkns;
  while(!atEnd()){
    char chr = advance();
    while(std::isspace(chr)) chr = advance();
    switch (chr) {
      case 'y': tkns.emplace_back(TokenType::AND, "y");    break;
      case 'o': tkns.emplace_back(TokenType::AND, "o");    break;
      case '(': tkns.emplace_back(TokenType::LPAREN, "("); break;
      case ')': tkns.emplace_back(TokenType::RPAREN, ")"); break;
      case '{': tkns.emplace_back(TokenType::LBRACE, "{"); break;
      case '}': tkns.emplace_back(TokenType::RBRACE, "}"); break;
      case ',': tkns.emplace_back(TokenType::COMMA, ",");  break;
      case '+': tkns.emplace_back(TokenType::PLUS, "+");   break;
      case '-': tkns.emplace_back(TokenType::MINUS, "-");  break;
      case '*': tkns.emplace_back(TokenType::STAR, "*");   break;
      case '/': tkns.emplace_back(TokenType::SLASH, "/");  break;
      case ':':
        advance();
        while(std::isalnum(peek())) advance();
        break;
      case '=':
        if(peek() != '=')
          tkns.emplace_back(TokenType::ASSIGN, "=");
        else{
          tkns.emplace_back(TokenType::EQUAL, "==");
          advance();
        }
        break;
      case '>':
        if(peek() != '=')
          tkns.emplace_back(TokenType::GREATER_THAN, ">");
        else{
          tkns.emplace_back(TokenType::GREATER_OR_EQUAL, ">=");
          advance();
        }
        break;
      case '<':
        if(peek() != '=')
          tkns.emplace_back(TokenType::LESSER_THAN, "<");
        else{
          tkns.emplace_back(TokenType::LESSER_OR_EQUAL, "<=");
          advance();
        }
        break;
      case '#':
        while(peek() != '\n' && !atEnd()) advance();
        break;
      case '!':
        if(peek() == '='){
          tkns.emplace_back(TokenType::NOT_EQUAL, "!=");
          advance();
        }else{
          tkns.emplace_back(TokenType::BANG, "!");
        }
        break;
      /* TODO: CHECK FOR UNTERMINATED STRING LITERAL */
      case '\'':
      case '\"':
        advance();
        size_t start = _idx - 1;
        while(peek() != chr) advance();
        tkns.emplace_back(TokenType::STRING, _source.substr(start, _idx - start));
        advance(); // " or '
        break;
    }
    if(std::isdigit(chr)){
      TokenType number_type = TokenType::INTEGER;
      size_t start = _idx - 1;
      while(std::isdigit(peek())) advance();

      if( peek() == '.' && std::isdigit(peek(1))){
        advance();
        while(std::isdigit(peek())) advance();
        number_type = TokenType::FLOAT;
      }
      tkns.emplace_back(number_type, _source.substr(start, _idx - start));
    }
    if(std::isalpha(chr) || chr == '_'){
      size_t start = _idx - 1;
      while(std::isalnum(peek()) || peek() == '_') advance();
      std::string_view str{_source.data() + start, _idx - start };

      if (auto it = keywords.find(str); it != keywords.end()) 
        tkns.emplace_back(it->second, std::string(str));
      else
        tkns.emplace_back(TokenType::IDENTIFIER, std::string(str));
    }
    //tkns.emplace_back(TokenType::ILEGAL, std::to_string(chr));
  }
  tkns.emplace_back(TokenType::END_OF_FILE, "");
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
  return chr;
}

char Lexer::peek(size_t steps){
  if(_idx + steps > _source.size())
    return '\0';
  return _source[_idx + steps];
}

