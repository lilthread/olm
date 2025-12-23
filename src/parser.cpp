#include "parser.h"
#include "ast.h"
#include "lexer.h"
#include "memoryarena.h"
#include <stdexcept>
#include <vector>

ArenaVec Parser::parseProgram() {
  while (peek().type != TokenType::END_OF_FILE) {
    _ast.emplace_back(parseDeclaration());
  }
  return _ast;
}

ASTNode* Parser::parseExpression() {
  return parseAdditiveExpression();
}

ASTNode* Parser::parseAdditiveExpression() {
  ASTNode* left = parseMultDivExpr();

  while (peek().type == TokenType::PLUS or
         peek().type == TokenType::MINUS)
  {
    Token op = advance();
    ASTNode* right = parseMultDivExpr();
    left = _arena.allocate<BinaryOp>(left, right, op.literal);
  }

  return left;
}
ASTNode* Parser::parseMultDivExpr(){
  ASTNode* left = parseUnaryExpression();
  while (peek().type == TokenType::STAR or
         peek().type == TokenType::SLASH)
  {
    Token op = advance();
    ASTNode* right = parseUnaryExpression();
    left = _arena.allocate<BinaryOp>(left, right, op.literal);
  }

  return left;
}

ASTNode* Parser::parseUnaryExpression() {
  if (auto tkn = peek(); match(TokenType::BANG) or match(TokenType::MINUS)) {
    ASTNode* operand = parsePrimaryExpression();
    return  _arena.allocate<UnaryOp>(tkn.literal, operand);
  }
  return parsePrimaryExpression();
}

ASTNode* Parser::parsePrimaryExpression() {
  if (auto tkn = peek(); match(TokenType::INTEGER) or match(TokenType::FLOAT) or match(TokenType::STRING))
    return _arena.allocate<Literal>(tkn);
  else if (match(TokenType::LPAREN)) {
    ASTNode* expr = parseExpression();
    expect(TokenType::RPAREN, "Expected ')'");
    return expr;
  }
  throw std::runtime_error("Unexpected token: " + peek().literal);
}

ASTNode* Parser::parseDeclaration() {
  if(match(TokenType::VAR) or match(TokenType::CONST))
    return parseVar();
  else if(match(TokenType::FUNCTION))
    return parseFunction();
  else if (match(TokenType::CLASS))
    return parseClass();
  else if (peek().type == TokenType::IDENTIFIER and peek(1).type == TokenType::ASSIGN)
    return parseAssignment();
  throw std::runtime_error("Expected declaration: " + peek().literal);
}

VariableDecl* Parser::parseVar(){
  bool is_const = peek(-1).type == TokenType::CONST;
  const std::string& id = expect(TokenType::IDENTIFIER, "Expected identifier in variable declaration").literal;
  expect(TokenType::ASSIGN, "Expected assignment operator in variable declaration");
  auto expr = parseExpression();
  return _arena.allocate<VariableDecl>(is_const, id, expr);
}

ClassDecl* Parser::parseClass() {
  const std::string& name = expect(TokenType::IDENTIFIER, "Expected class name").literal;
  expect(TokenType::LBRACE, "Expected '{' after class name");

  ArenaVec members{ArenaAllocator<ASTNode*>{_arena}};
  while (peek().type != TokenType::RBRACE) {
    members.push_back(parseClassMember());
  }
  expect(TokenType::RBRACE, "Expected '}' after class");

  return _arena.allocate<ClassDecl>(name, members);
}

ASTNode* Parser::parseClassMember() {
  if (match(TokenType::FUNCTION)) {
    return parseFunction();
  }
  throw std::runtime_error("Expected class member");
}

FunctionDecl* Parser::parseFunction() {
  const std::string& name = expect(TokenType::IDENTIFIER, "Expected function name").literal;
  expect(TokenType::LPAREN, "Expected '('");

  Params params{ArenaAllocator<std::string>{_arena}};
  if(peek().type != TokenType::RPAREN){
    do {
      params.emplace_back(expect(TokenType::IDENTIFIER, "Expected parameter").literal); 
    } while (match(TokenType::COMMA));
  }
  expect(TokenType::RPAREN, "Expected ')'");

  ArenaVec body = parseBlock();
  return _arena.allocate<FunctionDecl>(name, params, body);
}

ArenaVec Parser::parseBlock() {
  expect(TokenType::LBRACE, "Expected '{'");
  ArenaVec body{ArenaAllocator<ASTNode*>{_arena}};

  while (!match(TokenType::RBRACE)) {
    body.push_back(parseStatement());
  }
  return body;
}

ASTNode* Parser::parseStatement() {
  if (peek().type == TokenType::IDENTIFIER and peek(1).type == TokenType::ASSIGN) {
    return parseAssignment();
  }else if(match(TokenType::IF))
    return parseIf();
  else if (match(TokenType::RETURN)) {
    const auto& value = expect(TokenType::IDENTIFIER, "Expected return value");
    // TODO: return  _arena.allocate<ReturnStatement>();
  }


  throw std::runtime_error("Unknown statement");
}

Assignment* Parser::parseAssignment(){
  const auto& id = advance().literal;
  expect(TokenType::ASSIGN, "Expected '='");
  ASTNode* expr = parseExpression();
  return  _arena.allocate<Assignment>(id, expr);
}

// TODO: IF statement
IfStatement* Parser::parseIf(){
  auto* condition = parsePrimaryExpression();
  ArenaVec body = parseBlock();
  exit(0);
}

const Token& Parser::peek(int steps) const {
  if (_idx + steps >= _tkns.size()) return _tkns.back();
  return _tkns[_idx + steps];
}
const Token& Parser::advance() {
  return _tkns[_idx++];
}
const Token& Parser::expect(TokenType type, const std::string message){
  if (peek().type != type) {
    throw std::runtime_error(message + " current: " + peek().literal);
  }
  return advance();
}

bool Parser::match(TokenType type) {
  if (peek().type ==  type) {
    advance();
    return true;
  }
  return false;
}


