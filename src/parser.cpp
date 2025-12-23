#include "parser.h"
#include "ast.h"
#include "lexer.h"
#include "memoryarena.h"
#include <stdexcept>
#include <string>
#include <vector>

StmtSlice Parser::parseProgram() {
  _ast.reserve(_tkns.size() / 2);
  _ast.clear();
  while (peek().type != TokenType::END_OF_FILE) {
    _ast.push_back(parseDeclaration());
  }
  return makeSlice(_arena, _ast);
}

ASTNode* Parser::parseExpression() {
  return parseAdditiveExpression();
}

ASTNode* Parser::parseAdditiveExpression() {
  ASTNode* left = parseTerm();

  while (peek().type == TokenType::PLUS or
         peek().type == TokenType::MINUS)
  {
    Token op = advance();
    ASTNode* right = parseTerm();
    left = _arena.allocate<BinaryOp>(left, right, op.literal);
  }

  return left;
}
ASTNode* Parser::parseTerm(){
  ASTNode* left = parseUnaryExpression();
  while (peek().type == TokenType::STAR or
         peek().type == TokenType::SLASH or
         peek().type == TokenType::GREATER_THAN or
         peek().type == TokenType::GREATER_OR_EQUAL or
         peek().type == TokenType::LESSER_THAN or
         peek().type == TokenType::LESSER_OR_EQUAL or
         peek().type == TokenType::EQUAL or
         peek().type == TokenType::NOT_EQUAL
  )
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
  if (auto tkn = peek();
    match(TokenType::INTEGER) or
    match(TokenType::FLOAT) or
    match(TokenType::STRING) or
    match(TokenType::IDENTIFIER))
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
  else if(match(TokenType::IF))
    return parseIf();
  else if(match(TokenType::WHILE))
    return parseWhile();
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

  std::vector<ASTNode*> members;
  while (peek().type != TokenType::RBRACE) {
    members.push_back(parseClassMember());
  }
  expect(TokenType::RBRACE, "Expected '}' after class");

  StmtSlice memberSlice = makeSlice(_arena, members);
  return _arena.allocate<ClassDecl>(name, memberSlice);
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

  std::vector<std::string> params;
  if(peek().type != TokenType::RPAREN){
    do {
      params.emplace_back(expect(TokenType::IDENTIFIER, "Expected parameter").literal); 
    } while (match(TokenType::COMMA));
  }
  expect(TokenType::RPAREN, "Expected ')'");

  ParamSlice paramSlice = makeSlice(_arena, params);
  StmtSlice body = parseBlock();
  return _arena.allocate<FunctionDecl>(name, paramSlice, body);
}


StmtSlice Parser::parseBlock() {
  expect(TokenType::LBRACE, "Expected '{'");
  std::vector<ASTNode*> body;

  while (!match(TokenType::RBRACE)) {
    body.push_back(parseStatement());
  }
  return makeSlice(_arena, body);
}

ASTNode* Parser::parseStatement() {
  if (peek().type == TokenType::IDENTIFIER and peek(1).type == TokenType::ASSIGN)
    return parseAssignment();
  else if (match(TokenType::RETURN))
    return  parseReturn();

  throw std::runtime_error("Unknown statement");
}

Assignment* Parser::parseAssignment(){
  const auto& id = advance().literal;
  expect(TokenType::ASSIGN, "Expected '='");
  ASTNode* expr = parseExpression();
  return  _arena.allocate<Assignment>(id, expr);
}

IfStatement* Parser::parseIf(){
  ASTNode* condition = parseExpression();

  StmtSlice then_body = parseBlock();
  auto* stmt = _arena.allocate<IfStatement>(condition, then_body);

  if (match(TokenType::ELSE)) {
    if(match(TokenType::IF)){
      stmt->next = IfStatement::next::If;
      stmt->else_if = parseIf();
    } else {
      stmt->next = IfStatement::next::Else;
      stmt->else_block = parseBlock();
    }
  }
  return stmt;
}

ReturnStatement* Parser::parseReturn() {
  auto* expr = parseExpression(); return _arena.allocate<ReturnStatement>(expr);
}

WhileStatement* Parser::parseWhile(){
  auto* expr = parseExpression();
  auto body = parseBlock();
  return _arena.allocate<WhileStatement>(expr, body);
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


