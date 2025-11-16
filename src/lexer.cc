#include "lexer.h"
#include <vector>
#include <iostream>

std::vector<Token> Lexer::Tokenize(){
  std::vector<Token> tokens;
  while(m_current_idx <= m_source_code.size()){
    char chr = advance();
    std::cout << chr << '\n';
    switch (chr) {
      case '(': tokens.emplace_back(TokenType::LPAREN, "("); break;
      case ')': tokens.emplace_back(TokenType::RPAREN, ")"); break;
      case '{': tokens.emplace_back(TokenType::LBRACE, "{"); break;
      case '}': tokens.emplace_back(TokenType::RBRACE, "}"); break;
      case ',': tokens.emplace_back(TokenType::COMMA, ","); break;
      case '+': tokens.emplace_back(TokenType::PLUS, "+"); break;
      case '-': tokens.emplace_back(TokenType::MINUS, "-"); break;
      case '*': tokens.emplace_back(TokenType::STAR, "*"); break;
      case '/': tokens.emplace_back(TokenType::SLASH, "/"); break;
      case '>': tokens.emplace_back(TokenType::GREATER_THAN, ">"); break;
      case '<': tokens.emplace_back(TokenType::LESSER_THAN, "<"); break;
      case '=': tokens.emplace_back(TokenType::ASSIGN, "="); break; 
    }
  }
  return tokens;
}
char Lexer::peek(size_t step = 0){}

char Lexer::advance(){
  return m_source_code[m_current_idx++];
}
