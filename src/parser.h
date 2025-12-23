#pragma once
#include <span>
#include <vector>
#include "memoryarena.h"
#include "lexer.h"
#include "ast.h"


// TODO: PARSE && || STMT
class Parser final {
public:
  StmtSlice parseProgram();

  Parser(const std::vector<Token>& tokens, MemoryArena& arena):
  _tkns(tokens), _idx(0), _arena(arena){}

private:
  const std::vector<Token>& _tkns;
  std::size_t _idx;
  std::vector<ASTNode*>_ast;
  MemoryArena& _arena;

  const Token& peek(int steps = 0) const;
  const Token& advance();
  bool match(TokenType type);
  const Token& expect(TokenType type, const std::string message);


  ASTNode* parseExpression();
  ASTNode* parseAdditiveExpression();
  ASTNode* parseTerm();
  ASTNode* parseUnaryExpression();
  ASTNode* parsePrimaryExpression();


  ASTNode* parseDeclaration();
  VariableDecl* parseVar();
  FunctionDecl* parseFunction();
  ClassDecl* parseClass();

  ReturnStatement* parseReturn();
  IfStatement* parseIf();
  WhileStatement* parseWhile();
  ASTNode* parseClassMember();

  StmtSlice parseBlock();
  ASTNode* parseStatement();
  Assignment* parseAssignment();
};

