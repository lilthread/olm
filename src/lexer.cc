#include "lexer.h"
#include <unordered_set>
#include <vector>

const static std::unordered_set<std::string> keywords = {
  "let", "const", "function", "return", "while", "if", "else", "or", "and", "true", "false"
};

std::vector<Token> Lexer::Tokenize(){
  std::vector<Token> tokens;
  while(m_current_idx < m_source_code.size()){ 
    char chr = advance();
    switch (chr) {
      case '(': tokens.emplace_back(TokenType::LPAREN, "(");break;
      case ')': tokens.emplace_back(TokenType::RPAREN, ")");break;
      case '{': tokens.emplace_back(TokenType::LBRACE, "{");break;
      case '}': tokens.emplace_back(TokenType::RBRACE, "}");break;
      case ',': tokens.emplace_back(TokenType::COMMA, ",");break;
      case '+': tokens.emplace_back(TokenType::PLUS, "+");break;
      case '-': tokens.emplace_back(TokenType::MINUS, "-");break;
      case '*': tokens.emplace_back(TokenType::STAR, "*");break;
      case '/': tokens.emplace_back(TokenType::SLASH, "/");break;
      case '=':
        if(peek() == '='){
          tokens.emplace_back(TokenType::EQUAL, "==");
          advance();
        }else{
          tokens.emplace_back(TokenType::ASSIGN, "=");
        }
        break;
      case '!':
        if(peek() == '='){
          tokens.emplace_back(TokenType::NOT_EQUAL, "!=");
          advance();
        }else{
          tokens.emplace_back(TokenType::BANG, "!");
        }
        break;
      case '>':
        if(peek() == '='){
          tokens.emplace_back(TokenType::GREATER_OR_EQUAL, ">=");
          advance();
        }else{
          tokens.emplace_back(TokenType::GREATER_THAN, ">");
        }
        break;
      case '<':
        if(peek() == '='){
          tokens.emplace_back(TokenType::LESSER_OR_EQUAL, "<=");
          advance();
        }else{
          tokens.emplace_back(TokenType::LESSER_THAN, "<");
        }
        break;
      case '#':
        while(peek() != '\n' && peek() != '\0') advance();
        break;
      case '\n':
        m_line++;
        break;
      case '"':
        size_t start = m_current_idx - 1;
        while(peek() != '"'){
          if(peek() == '\n') m_line++;
          advance();
        }
        if (peek() == '"') advance();
        tokens.emplace_back(TokenType::STRING,
                            m_source_code.substr(start, m_current_idx - start));
        break;
    }
    if(std::isdigit(chr)){
      size_t start = m_current_idx - 1;
      while(std::isdigit(peek()))advance();
      if(peek() == '.' && std::isdigit(peek(1))){
        advance();
        while(std::isdigit(peek()))advance();
      }
      tokens.emplace_back(TokenType::NUMBER, m_source_code.substr(start, m_current_idx - start));
    }
    if(std::isalpha(chr) || chr == '_'){
      size_t start = m_current_idx - 1;
      while(std::isalnum(peek()) || peek() == '_')advance();
      std::string str = m_source_code.substr(start, m_current_idx - start);

      if(keywords.find(str) == keywords.end()) tokens.emplace_back(TokenType::IDENTIFIER, str);
      else if("while" == str) tokens.emplace_back(TokenType::WHILE, str);
      else if("if" == str) tokens.emplace_back(TokenType::IF, str);
      else if("else" == str) tokens.emplace_back(TokenType::ELSE, str);
      else if("let" == str) tokens.emplace_back(TokenType::LET, str);
      else if("const" == str) tokens.emplace_back(TokenType::CONST, str);
      else if("function" == str) tokens.emplace_back(TokenType::FUNCTION, str);
      else if("or" == str) tokens.emplace_back(TokenType::OR, str);
      else if("and" == str) tokens.emplace_back(TokenType::AND, str);
      else if("return" == str) tokens.emplace_back(TokenType::RETURN, str);
      else if("true" == str) tokens.emplace_back(TokenType::TRUE, str);
      else if("false" == str) tokens.emplace_back(TokenType::FALSE, str);
    }
  }
  return tokens;
}
char Lexer::peek(size_t step){
  if (m_current_idx + step < m_source_code.size()) {
    return m_source_code[m_current_idx + step];
  } else {
    return '\0';
  }
}
char Lexer::advance(){
  if (m_current_idx < m_source_code.size()) {
    char chr = m_source_code[m_current_idx];
    m_current_idx++;
    return chr;
  } else {
    return '\0';
  }
}
